#include "S3Service.h"
#include "JsonValidation.h"

#include "Logger.h"
#include "Converter.h"
#include "ThreadWorker.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

#include "boost/json.hpp"
#include "boost/json/parse.hpp"

using namespace S3Common::JsonValidation;

namespace S3Service
{
	S3ServiceMain::S3ServiceMain(std::shared_ptr<Configurations> configurations)
		: configurations_(configurations)
		, is_running_(false)
		, thread_pool_(nullptr)
		, work_queue_consume_(nullptr)
		, file_storage_(nullptr)
	{
		if (configurations_ == nullptr)
		{
			Logger::handle().write(LogTypes::Error, "Configurations is not initialized.");
			return;
		}

		file_storage_ = std::make_shared<FileStorage>(configurations_->storage_path());
		if (file_storage_ == nullptr)
		{
			Logger::handle().write(LogTypes::Error, "Failed to create FileStorage.");
			return;
		}

		auto conn_str = fmt::format("user={} dbname={} password={} host={} port={}", 
			configurations_->database_user_name(), 
			configurations_->database_name(), 
			configurations_->database_password(), 
			configurations_->database_server_ip(), 
			configurations_->database_server_port()
		);

		auto db = std::make_shared<Database::PostgresDB>(conn_str);
		if (db == nullptr)
		{
			Logger::handle().write(LogTypes::Error, "Failed to create PostgresDB.");
			return;
		}

		s3_meta_db_ = std::make_shared<S3MetaDB::S3MetaDB>(db);
		if (s3_meta_db_ == nullptr)
		{
			Logger::handle().write(LogTypes::Error, "Failed to create S3MetaDB.");
			return;
		}

		commands_.insert({ "create_bucket", std::bind(&S3ServiceMain::create_bucket, this, std::placeholders::_1) });
		commands_.insert({ "upload_file", std::bind(&S3ServiceMain::upload_file, this, std::placeholders::_1) });
		commands_.insert({ "download_file", std::bind(&S3ServiceMain::download_file, this, std::placeholders::_1) });
		
		commands_.insert({"test_command", [](const std::string& message) -> std::tuple<bool, std::optional<std::string>> {
			Logger::handle().write(LogTypes::Information, fmt::format("Test command received: {}", message));
			return { true, std::nullopt };
		}});
	}

	S3ServiceMain::~S3ServiceMain()
	{
		stop();
	}

	auto S3ServiceMain::start() -> std::tuple<bool, std::optional<std::string>>
	{
		auto [result, error_message] = create_thread_pool();
		if (!result)
		{
			Logger::handle().write(LogTypes::Error, error_message.value());
			return { false, error_message };
		}

		SSLOptions ssl_options;
		ssl_options.use_ssl(configurations_->use_ssl());
		ssl_options.ca_cert(configurations_->ca_cert());
		ssl_options.engine(configurations_->engine());
		ssl_options.client_cert(configurations_->client_cert());
		ssl_options.client_key(configurations_->client_key());

		work_queue_consume_ = std::make_shared<WorkQueueConsume>(configurations_->rabbit_mq_host(), 
															 configurations_->rabbit_mq_port(), 
															 configurations_->rabbit_mq_user_name(),
															 configurations_->rabbit_mq_password(), ssl_options);
														 
		std::tie(result, error_message) = work_queue_consume_->start();
		if (!result)
		{
			Logger::handle().write(LogTypes::Error, error_message.value());
			return { false, error_message };
		}

		Logger::handle().write(LogTypes::Information, "work queue consume started");

		std::tie(result, error_message) = work_queue_consume_->connect(60);
		if (!result)
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to connect work queue consume: {}", error_message.value()));
			return { false, fmt::format("Failed to connect work queue consume: {}", error_message.value()) };
		}
		Logger::handle().write(LogTypes::Information, "work queue consume connected");

		std::tie(result, error_message) = consume_queue();
		if (!result)
		{
			destroy_thread_pool();
			work_queue_consume_->stop();
			work_queue_consume_.reset();
	
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to consume queue: {}", error_message.value()));
			return { false, fmt::format("Failed to consume queue: {}", error_message.value()) };
		}
		return { true, std::nullopt };
	}

	auto S3ServiceMain::wait_stop() -> std::tuple<bool, std::optional<std::string>>
	{
		if (work_queue_consume_ == nullptr)
		{
			Logger::handle().write(LogTypes::Error, "QueueConsume is not initialized.");
			return { false, "QueueConsume is not initialized." };
		}

		return work_queue_consume_->wait_stop();
	}

	auto S3ServiceMain::stop() -> void
	{
		if (work_queue_consume_ != nullptr)
		{
			work_queue_consume_->stop();
			work_queue_consume_.reset();
		}
	
		destroy_thread_pool();
	}

	auto S3ServiceMain::create_thread_pool() -> std::tuple<bool, std::optional<std::string>>
	{
		destroy_thread_pool();

		try
		{	
			thread_pool_ = std::make_shared<ThreadPool>();
		}
		catch(const std::bad_alloc& e)
		{
			return { false, fmt::format("Memory allocation failed to ThreadPool: {}", e.what()) };
		}
		
		for (auto i = 0; i < configurations_->high_priority_count(); i++)
		{
			std::shared_ptr<ThreadWorker> worker;
			try
			{
				worker = std::make_shared<ThreadWorker>(std::vector<JobPriorities>{ JobPriorities::High });
			}
			catch(const std::bad_alloc& e)
			{
				return { false, fmt::format("Memory allocation failed to ThreadWorker: {}", e.what()) };
			}
	
			thread_pool_->push(worker);
		}
	
		for (auto i = 0; i < configurations_->normal_priority_count(); i++)
		{
			std::shared_ptr<ThreadWorker> worker;
			try
			{
				worker = std::make_shared<ThreadWorker>(std::vector<JobPriorities>{ JobPriorities::Normal, JobPriorities::High });
			}
			catch(const std::bad_alloc& e)
			{
				return { false, fmt::format("Memory allocation failed to ThreadWorker: {}", e.what()) };
			}
	
			thread_pool_->push(worker);
		}
	
		for (auto i = 0; i < configurations_->low_priority_count(); i++)
		{
			std::shared_ptr<ThreadWorker> worker;
			try
			{
				worker = std::make_shared<ThreadWorker>(std::vector<JobPriorities>{ JobPriorities::Low });
			}
			catch(const std::bad_alloc& e)
			{
				return { false, fmt::format("Memory allocation failed to ThreadWorker: {}", e.what()) };
			}
	
			thread_pool_->push(worker);
		}
	
		auto [result, message] = thread_pool_->start();
		if (!result)
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to start thread pool: {}", message.value()));
			return { false, message.value() };
		}
	
		return { true, std::nullopt };
	}

	auto S3ServiceMain::destroy_thread_pool() -> void
	{
		if (thread_pool_ == nullptr)
		{
			return;
		}
	
		thread_pool_->stop();
		thread_pool_.reset();
	}

	auto S3ServiceMain::consume_queue() -> std::tuple<bool, std::optional<std::string>>
	{
		if (work_queue_consume_ == nullptr)
		{
			Logger::handle().write(LogTypes::Error, "QueueConsume is not initialized.");
			return { false, "QueueConsume is not initialized." };
		}

		auto [declred_name, declred_error] = work_queue_consume_->channel_open(work_queue_consume_channel_id_, configurations_->consume_queue_name());
		if (declred_name == std::nullopt)
		{
			Logger::handle().write(LogTypes::Error, declred_error.value());
			return { false, declred_error };
		}

		auto [prepare_success, prepare_error] = work_queue_consume_->prepare_consume();
		if (!prepare_success)
		{
			Logger::handle().write(LogTypes::Error, prepare_error.value());
			return { false, prepare_error };
		}

		auto [consume_success, consume_error] = work_queue_consume_->register_consume(work_queue_consume_channel_id_, configurations_->consume_queue_name(),
			[&](const std::string& queue_name, const std::string message, const std::string& message_type) -> std::tuple<bool, std::optional<std::string>>
			{
				auto json_value = boost::json::parse(message);
				if (!json_value.is_object())
				{
					Logger::handle().write(LogTypes::Error, "Invalid message format.");
					return { false, "Invalid message format." };
				}

				// TODO
				// Add a message in addition to the command
				auto received_message = json_value.as_object();
				if (!received_message.contains("command") || !received_message.at("command").is_string())
				{
					Logger::handle().write(LogTypes::Error, "Invalid message format.");
					return { false, "Invalid message format." };
				}

				auto command = received_message.at("command").as_string().data();
				if (commands_.find(command) == commands_.end())
				{
					Logger::handle().write(LogTypes::Error, fmt::format("Invalid command: {}", command));
					return { false, fmt::format("Invalid command: {}", command) };
				}

				auto [result, error_message] = commands_[command](message);
				if (!result)
				{
					Logger::handle().write(LogTypes::Error, error_message.value());
					return { false, error_message };
				}

				return { true, std::nullopt };
			});

		auto [result, message] = work_queue_consume_->start_consume();
		if (!result)
		{
			Logger::handle().write(LogTypes::Error, message.value());
			return { false, message };
		}

		return { true, std::nullopt };
	}

	auto S3ServiceMain::create_bucket(const std::string &message) -> std::tuple<bool, std::optional<std::string>>
	{
		auto json_value = boost::json::parse(message);

		auto [valid, error_message] = validate_create_bucket_value(json_value);
		if (!valid)
		{
			return { false, error_message };
		}

		auto received_message = json_value.as_object();

		auto bucket_name = received_message.at("bucket_name").as_string().data();
		auto region = received_message.at("region").as_string().data();

		auto [result, exists_error] = s3_meta_db_->bucket_exists(bucket_name);
		if (result)
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Bucket '{}' already exists. = {}", bucket_name, exists_error.value()));
			return { false, fmt::format("Bucket '{}' already exists. = {}", bucket_name, exists_error.value()) };
		}

		auto [create_result, create_error] = s3_meta_db_->create_bucket(bucket_name);
		if (!create_result)
		{
			return { false, fmt::format("Failed to create bucket: {}", create_error.value()) };
		}

		auto [create_folder_result, create_folder_error] = file_storage_->create_folder(bucket_name);
		if (!create_folder_result)
		{
			return { false, fmt::format("Failed to create folder: {}", create_folder_error.value()) };
		}

		return { true, std::nullopt };
	}

	auto S3ServiceMain::upload_file(const std::string &message) -> std::tuple<bool, std::optional<std::string>>
	{
		auto json_value = boost::json::parse(message);

		auto [valid, error_message] = validate_upload_file_value(json_value);
		if (!valid)
		{
			return { false, error_message };
		}

		auto received_message = json_value.as_object();

		auto bucket_name = received_message.at("bucket_name").as_string().data();
		auto object_name = received_message.at("object_name").as_string().data();
		auto file_name = received_message.at("file_name").as_string().data();

		auto [result, error] = s3_meta_db_->bucket_exists(bucket_name);
		if (!result)
		{
			return { false, fmt::format("[upload_file] Bucket '{}' does not exist. = {}", bucket_name, error.value()) };
		}

		std::tie(result, error) = file_storage_->append_file(file_name, object_name);
		if (!result)
		{
			return { false, fmt::format("[upload_file] Failed to upload file: {}", error.value()) };
		}

		auto [update_result, update_error] = s3_meta_db_->update_object(bucket_name, object_name, file_name);
		if (!update_result)
		{
			return { false, fmt::format("[upload_file] Failed to update object: {}", update_error.value()) };
		}

		return { true, std::nullopt };
	}

	auto S3ServiceMain::download_file(const std::string &message) -> std::tuple<bool, std::optional<std::string>>
	{
		auto json_value = boost::json::parse(message);

		auto [valid, error_message] = validate_download_file_value(json_value);
		if (!valid)
		{
			return { false, error_message };
		}

		auto received_message = json_value.as_object();

		auto bucket_name = received_message.at("bucket_name").as_string().data();
		auto object_name = received_message.at("object_name").as_string().data();
		auto file_name = received_message.at("file_name").as_string().data();

		auto [result, exists_error] = s3_meta_db_->bucket_exists(bucket_name);
		if (!result)
		{
			return { false, fmt::format("[download_file] Bucket '{}' does not exist. = {}", bucket_name, exists_error.value()) };
		}

		auto [file_exists, file_error] = file_storage_->file_exists(bucket_name, object_name, file_name);
		if (!file_exists)
		{
			return { false, fmt::format("[download_file] File '{}' does not exist. = {}", file_name, file_error.value()) };
		}

		auto file_path = file_storage_->get_file_path(bucket_name, object_name, file_name);

		return { true, std::nullopt };
	}
}