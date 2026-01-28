#pragma once

#include <string>
#include <tuple>
#include <vector>
#include <optional>
#include <memory>
#include <cstdint>

namespace S3FileStorage
{
	class FileStorage
	{
	public:
		FileStorage(const std::string& s3_storage_path);
		~FileStorage();

		auto s3_storage_path() const -> const std::string& { return s3_storage_path_; }
		auto s3_storage_path(const std::string& s3_storage_path) -> void { s3_storage_path_ = s3_storage_path; }

		auto file_exists(const std::string& bucket_name, const std::string& object_key)
			-> std::tuple<bool, std::optional<std::string>>;

		auto create_folder(const std::string& folder_name)
			-> std::tuple<bool, std::optional<std::string>>;

		auto write_file(const std::string& bucket_name, const std::string& object_key,
						const std::vector<uint8_t>& data)
			-> std::tuple<bool, std::optional<std::string>>;

		auto write_file(const std::string& bucket_name, const std::string& object_key,
						const std::string& data)
			-> std::tuple<bool, std::optional<std::string>>;

		auto read_file(const std::string& bucket_name, const std::string& object_key)
			-> std::tuple<std::optional<std::vector<uint8_t>>, std::optional<std::string>>;

		auto delete_file(const std::string& bucket_name, const std::string& object_key)
			-> std::tuple<bool, std::optional<std::string>>;

		auto get_file_path(const std::string& bucket_name, const std::string& object_key)
			-> std::string;

		auto get_file_size(const std::string& bucket_name, const std::string& object_key)
			-> std::tuple<std::optional<size_t>, std::optional<std::string>>;

		// Legacy interface (deprecated)
		auto append_file(const std::string& file_name, const std::string& object_name)
			-> std::tuple<bool, std::optional<std::string>>;
		auto download_file(const std::string& file_name, const std::string& object_name)
			-> std::tuple<bool, std::optional<std::string>>;

	protected:
		auto load_storage() -> std::tuple<bool, std::optional<std::string>>;
		auto ensure_directory(const std::string& path) -> bool;

	private:
		std::string s3_storage_path_;
	};
}
