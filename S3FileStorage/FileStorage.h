#pragma once

#include <string>
#include <tuple>

#include <optional>
#include <memory>

namespace S3FileStorage
{
	class FileStorage
	{
	public:
		FileStorage(const std::string& s3_storage_path);
		~FileStorage();

		auto s3_storage_path() const -> const std::string& { return s3_storage_path_; }
		auto s3_storage_path(const std::string& s3_storage_path) -> void { s3_storage_path_ = s3_storage_path; }

		auto file_exists(const std::string& bucket_name, const std::string& object_name, const std::string &file_name) -> std::tuple<bool, std::optional<std::string>>;
		auto create_folder(const std::string& folder_name) -> std::tuple<bool, std::optional<std::string>>;
		auto append_file(const std::string &file_name, const std::string &object_name) -> std::tuple<bool, std::optional<std::string>>;
		auto download_file(const std::string &file_name, const std::string &object_name) -> std::tuple<bool, std::optional<std::string>>;

		auto get_file_path(const std::string &bucket_name, const std::string &object_name, const std::string &file_name) -> std::string;
	protected:
		auto load_storage() -> std::tuple<bool, std::optional<std::string>>;

	private:
		std::string s3_storage_path_;

		
	};
}
