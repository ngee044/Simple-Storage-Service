#pragma once

#include "PostgresDB.h"

#include <string>
#include <memory>
#include <optional>
#include <tuple>


namespace S3MetaDB
{
	class S3MetaDB
	{
		public:
			explicit S3MetaDB(std::shared_ptr<Database::PostgresDB> db);
			~S3MetaDB() = default;

			auto bucket_exists(const std::string& bucket_name) -> std::tuple<bool, std::optional<std::string>>;
			auto create_bucket(const std::string& bucket_name) -> std::tuple<bool, std::optional<std::string>>;
			auto update_object(const std::string& bucket_name, const std::string& object_name, const std::string& file_name) -> std::tuple<bool, std::optional<std::string>>;

		private:
			std::shared_ptr<Database::PostgresDB> db_;
	};

}