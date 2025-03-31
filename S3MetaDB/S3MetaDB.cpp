#include "S3MetaDB.h"

#include "Logger.h"

#include "fmt/format.h"

using namespace Utilities;

namespace S3MetaDB
{
	S3MetaDB::S3MetaDB(std::shared_ptr<Database::PostgresDB> db) 
		: db_(std::move(db)) 
	{

	}

	auto S3MetaDB::bucket_exists(const std::string& bucket_name) -> std::tuple<bool, std::optional<std::string>>
	{
		auto check_bucket_query = fmt::format("SELECT bucket_name FROM s3_buckets WHERE bucket_name = '{}';", bucket_name);
		auto [result, error] = db_->execute_query_and_get_result(check_bucket_query);
		if (!result.has_value())
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to execute query: {}", error.value()));
			return { false, error.value() };
		}

		if (result.value().empty())
		{
			Logger::handle().write(LogTypes::Information, fmt::format("Bucket '{}' does not exist.", bucket_name));
			return { false, std::nullopt };
		}

		Logger::handle().write(LogTypes::Information, fmt::format("Bucket '{}' exists.", bucket_name));

		return { true, std::nullopt };
	}

	auto S3MetaDB::create_bucket(const std::string& bucket_name) -> std::tuple<bool, std::optional<std::string>>
	{
		std::string safe_bucket_name = db_->escape_string(bucket_name);
		auto create_bucket_query = fmt::format("INSERT INTO s3_buckets (bucket_name) VALUES ('{}');", safe_bucket_name);
		auto [insert_result, insert_error] = db_->execute_query(create_bucket_query);

		if (!insert_result)
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to create bucket: {}", insert_error.value()));
			return { false, insert_error.value() };
		}

		Logger::handle().write(LogTypes::Information, fmt::format("Bucket '{}' created successfully.", bucket_name));
		
		return { true, std::nullopt };
	}

	auto S3MetaDB::update_object(const std::string& bucket_name, const std::string& object_name, std::int64_t file_size) -> std::tuple<bool, std::optional<std::string>>
	{
		auto check_bucket_query = fmt::format("SELECT bucket_name FROM s3_buckets WHERE bucket_name = '{}';", bucket_name);

		auto [result, error] = db_->execute_query_and_get_result(check_bucket_query);
		if (!result.has_value())
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to execute query: {}", error.value()));
			return { false, error.value() };
		}

		if (result.value().empty())
		{
			auto insert_query = fmt::format("INSERT INTO s3_buckets (bucket_name) VALUES ({}, '{}', {});", bucket_name, object_name, file_size);
			auto [insert_result, insert_error] = db_->execute_query(insert_query);
			if (!insert_result)
			{
				Logger::handle().write(LogTypes::Error, fmt::format("Failed to insert object: {}", insert_error.value()));
				return { false, insert_error.value() };
			}

			Logger::handle().write(LogTypes::Information, fmt::format("Object '{}' created successfully in bucket '{}'.", object_name, bucket_name));
			return { true, std::nullopt };
		}

		int object_id = std::get<int>(result.value()[0][0]);
		auto update_query = fmt::format("UPDATE s3_buckets SET file_size = {}, updated_at = NOW() WHERE bucket_name = {};", file_size, bucket_name);

		auto [update_result, update_error] = db_->execute_query_and_get_result(update_query);
		if (!update_result.has_value())
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to update object: {}", update_error.value()));
			return { false, update_error.value() };
		}

		if (update_result->empty())
   		{
			// 아직 오브젝트가 없다면 INSERT
			auto update_query = fmt::format(
				"INSERT INTO s3_objects (bucket_id, object_key, file_size) "
				"VALUES ({}, '{}', {});",
				object_name, object_name, file_size);

			auto [success, err2Opt] = db_->execute_query(update_query);
			if (!success)
			{
				return { false, err2Opt.value() };
			}
		}
		else
		{
			// 이미 있으면 UPDATE (file_size, updated_at 등)
			int object_name = std::get<int>((*update_result)[0][0]);
			auto update_query = fmt::format(
				"UPDATE s3_objects "
				"SET file_size = {}, updated_at = NOW() "
				"WHERE id = {};",
				file_size, object_name);

			auto [success, err2Opt] = db_->execute_query(update_query);
			if (!success)
			{
				return { false, err2Opt.value() };;
			}
		}

		return { true, std::nullopt };
	}
}