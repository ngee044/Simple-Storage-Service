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
	{

	}

	S3ServiceMain::~S3ServiceMain()
	{
	}

	auto S3ServiceMain::start() -> std::tuple<bool, std::optional<std::string>>
	{
		return std::tuple<bool, std::optional<std::string>>();
	}

	auto S3ServiceMain::wait_stop() -> std::tuple<bool, std::optional<std::string>>
	{
		if (work_queue_consume_ == nullptr)
		{
			Logger::handle().write(LogTypes::Error, "KafkaQueueConsume is not initialized.");
			return { false, "KafkaQueueConsume is not initialized." };
		}

		return work_queue_consume_->wait_stop();
	}

	auto S3ServiceMain::stop() -> void
	{

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
		return std::tuple<bool, std::optional<std::string>>();
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

		// TODO


		return { true, std::nullopt };
	}

	auto S3ServiceMain::upload_file(const std::string &message) -> std::tuple<bool, std::optional<std::string>>
	{
		return { true, std::nullopt };
	}

	auto S3ServiceMain::download_file(const std::string &message) -> std::tuple<bool, std::optional<std::string>>
	{
		return { true, std::nullopt };
	}
}