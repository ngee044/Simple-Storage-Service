#pragma once

#include "PostgresDB.h"

#include <string>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>

namespace S3MetaDB
{
	struct ObjectInfo
	{
		int id;
		std::string object_key;
		int64_t file_size;
		std::string created_at;
		std::string updated_at;
	};

	class S3MetaDB
	{
	public:
		explicit S3MetaDB(std::shared_ptr<Database::PostgresDB> db);
		~S3MetaDB() = default;

		auto bucket_exists(const std::string& bucket_name)
			-> std::tuple<bool, std::optional<std::string>>;

		auto create_bucket(const std::string& bucket_name)
			-> std::tuple<bool, std::optional<std::string>>;

		auto delete_bucket(const std::string& bucket_name)
			-> std::tuple<bool, std::optional<std::string>>;

		auto update_object(const std::string& bucket_name, const std::string& object_key, const std::string& file_name)
			-> std::tuple<bool, std::optional<std::string>>;

		auto list_objects(const std::string& bucket_name)
			-> std::tuple<std::optional<std::vector<ObjectInfo>>, std::optional<std::string>>;

		auto delete_object(const std::string& bucket_name, const std::string& object_key)
			-> std::tuple<bool, std::optional<std::string>>;

		auto get_bucket_id(const std::string& bucket_name)
			-> std::tuple<std::optional<int>, std::optional<std::string>>;

	private:
		std::shared_ptr<Database::PostgresDB> db_;
	};
}
