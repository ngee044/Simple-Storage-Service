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
		std::string safe_bucket_name = db_->escape_string(bucket_name);
		auto check_bucket_query = fmt::format("SELECT bucket_name FROM s3_buckets WHERE bucket_name = '{}';", safe_bucket_name);
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

	auto S3MetaDB::update_object(const std::string& bucket_name, const std::string& object_key, const std::string& file_name) -> std::tuple<bool, std::optional<std::string>>
	{
		std::string safe_bucket_name = db_->escape_string(bucket_name);
		std::string safe_object_key = db_->escape_string(object_key);

		auto get_bucket_query = fmt::format(
			"SELECT id FROM s3_buckets WHERE bucket_name = '{}';",
			safe_bucket_name
		);

		auto [bucket_result, bucket_error] = db_->execute_query_and_get_result(get_bucket_query);
		if (!bucket_result.has_value())
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to execute query: {}", bucket_error.value()));
			return { false, bucket_error.value() };
		}

		if (bucket_result.value().empty())
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Bucket '{}' does not exist.", bucket_name));
			return { false, fmt::format("Bucket '{}' does not exist.", bucket_name) };
		}

		int bucket_id = std::get<int>(bucket_result.value()[0][0]);

		auto check_object_query = fmt::format(
			"SELECT id FROM s3_objects WHERE bucket_id = {} AND object_key = '{}';",
			bucket_id, safe_object_key
		);

		auto [object_result, object_error] = db_->execute_query_and_get_result(check_object_query);
		if (!object_result.has_value())
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to check object: {}", object_error.value()));
			return { false, object_error.value() };
		}

		if (object_result.value().empty())
		{
			auto insert_query = fmt::format(
				"INSERT INTO s3_objects (bucket_id, object_key, file_size) "
				"VALUES ({}, '{}', 0);",
				bucket_id, safe_object_key
			);

			auto [insert_result, insert_error] = db_->execute_query(insert_query);
			if (!insert_result)
			{
				Logger::handle().write(LogTypes::Error, fmt::format("Failed to insert object: {}", insert_error.value()));
				return { false, insert_error.value() };
			}

			Logger::handle().write(LogTypes::Information, fmt::format("Object '{}' created in bucket '{}'.", object_key, bucket_name));
		}
		else
		{
			int object_id = std::get<int>(object_result.value()[0][0]);
			auto update_query = fmt::format(
				"UPDATE s3_objects SET updated_at = NOW() WHERE id = {};",
				object_id
			);

			auto [update_success, update_error] = db_->execute_query(update_query);
			if (!update_success)
			{
				Logger::handle().write(LogTypes::Error, fmt::format("Failed to update object: {}", update_error.value()));
				return { false, update_error.value() };
			}

			Logger::handle().write(LogTypes::Information, fmt::format("Object '{}' updated in bucket '{}'.", object_key, bucket_name));
		}

		return { true, std::nullopt };
	}

	auto S3MetaDB::get_bucket_id(const std::string& bucket_name)
		-> std::tuple<std::optional<int>, std::optional<std::string>>
	{
		std::string safe_bucket_name = db_->escape_string(bucket_name);
		auto query = fmt::format("SELECT id FROM s3_buckets WHERE bucket_name = '{}';", safe_bucket_name);

		auto [result, error] = db_->execute_query_and_get_result(query);
		if (!result.has_value())
		{
			return { std::nullopt, error.value() };
		}

		if (result.value().empty())
		{
			return { std::nullopt, fmt::format("Bucket '{}' does not exist.", bucket_name) };
		}

		return { std::get<int>(result.value()[0][0]), std::nullopt };
	}

	auto S3MetaDB::delete_bucket(const std::string& bucket_name)
		-> std::tuple<bool, std::optional<std::string>>
	{
		std::string safe_bucket_name = db_->escape_string(bucket_name);
		auto query = fmt::format("DELETE FROM s3_buckets WHERE bucket_name = '{}';", safe_bucket_name);

		auto [result, error] = db_->execute_query(query);
		if (!result)
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to delete bucket: {}", error.value()));
			return { false, error.value() };
		}

		Logger::handle().write(LogTypes::Information, fmt::format("Bucket '{}' deleted.", bucket_name));
		return { true, std::nullopt };
	}

	auto S3MetaDB::list_objects(const std::string& bucket_name)
		-> std::tuple<std::optional<std::vector<ObjectInfo>>, std::optional<std::string>>
	{
		auto [bucket_id, bucket_error] = get_bucket_id(bucket_name);
		if (!bucket_id.has_value())
		{
			return { std::nullopt, bucket_error };
		}

		auto query = fmt::format(
			"SELECT id, object_key, file_size, "
			"to_char(created_at, 'YYYY-MM-DD HH24:MI:SS') as created_at, "
			"to_char(updated_at, 'YYYY-MM-DD HH24:MI:SS') as updated_at "
			"FROM s3_objects WHERE bucket_id = {} ORDER BY object_key;",
			bucket_id.value()
		);

		auto [result, error] = db_->execute_query_and_get_result(query);
		if (!result.has_value())
		{
			return { std::nullopt, error.value() };
		}

		std::vector<ObjectInfo> objects;
		for (const auto& row : result.value())
		{
			ObjectInfo info;
			info.id = std::get<int>(row[0]);
			info.object_key = std::get<std::string>(row[1]);
			info.file_size = std::get<int64_t>(row[2]);
			info.created_at = std::get<std::string>(row[3]);
			info.updated_at = std::get<std::string>(row[4]);
			objects.push_back(info);
		}

		Logger::handle().write(LogTypes::Information,
			fmt::format("Listed {} objects in bucket '{}'.", objects.size(), bucket_name));

		return { objects, std::nullopt };
	}

	auto S3MetaDB::delete_object(const std::string& bucket_name, const std::string& object_key)
		-> std::tuple<bool, std::optional<std::string>>
	{
		auto [bucket_id, bucket_error] = get_bucket_id(bucket_name);
		if (!bucket_id.has_value())
		{
			return { false, bucket_error };
		}

		std::string safe_object_key = db_->escape_string(object_key);
		auto query = fmt::format(
			"DELETE FROM s3_objects WHERE bucket_id = {} AND object_key = '{}';",
			bucket_id.value(), safe_object_key
		);

		auto [result, error] = db_->execute_query(query);
		if (!result)
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to delete object: {}", error.value()));
			return { false, error.value() };
		}

		Logger::handle().write(LogTypes::Information,
			fmt::format("Object '{}' deleted from bucket '{}'.", object_key, bucket_name));

		return { true, std::nullopt };
	}
}