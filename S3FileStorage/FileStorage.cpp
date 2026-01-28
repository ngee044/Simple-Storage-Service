#include "FileStorage.h"

#include "Logger.h"
#include "fmt/format.h"

#include <filesystem>
#include <fstream>

using namespace Utilities;

namespace S3FileStorage
{
	FileStorage::FileStorage(const std::string& s3_storage_path)
		: s3_storage_path_(s3_storage_path)
	{
		if (!s3_storage_path_.empty() && !std::filesystem::exists(s3_storage_path_))
		{
			std::filesystem::create_directories(s3_storage_path_);
		}
	}

	FileStorage::~FileStorage()
	{
	}

	auto FileStorage::ensure_directory(const std::string& path) -> bool
	{
		if (std::filesystem::exists(path))
		{
			return true;
		}

		try
		{
			return std::filesystem::create_directories(path);
		}
		catch (const std::exception& e)
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to create directory: {}", e.what()));
			return false;
		}
	}

	auto FileStorage::file_exists(const std::string& bucket_name, const std::string& object_key)
		-> std::tuple<bool, std::optional<std::string>>
	{
		if (s3_storage_path_.empty())
		{
			return { false, "S3 storage path is empty." };
		}

		if (bucket_name.empty() || object_key.empty())
		{
			return { false, "Invalid parameters: bucket_name or object_key is empty." };
		}

		auto file_path = get_file_path(bucket_name, object_key);
		if (!std::filesystem::exists(file_path))
		{
			return { false, fmt::format("File does not exist: {}", file_path) };
		}

		return { true, std::nullopt };
	}

	auto FileStorage::create_folder(const std::string& folder_name)
		-> std::tuple<bool, std::optional<std::string>>
	{
		if (s3_storage_path_.empty())
		{
			Logger::handle().write(LogTypes::Error, "S3 storage path is empty.");
			return { false, "S3 storage path is empty." };
		}

		if (folder_name.empty())
		{
			Logger::handle().write(LogTypes::Error, "Folder name is empty.");
			return { false, "Folder name is empty." };
		}

		auto folder_path = fmt::format("{}/{}", s3_storage_path_, folder_name);

		if (std::filesystem::exists(folder_path))
		{
			return { true, std::nullopt };
		}

		if (!ensure_directory(folder_path))
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to create folder: {}", folder_path));
			return { false, fmt::format("Failed to create folder: {}", folder_path) };
		}

		Logger::handle().write(LogTypes::Information, fmt::format("Created folder: {}", folder_path));
		return { true, std::nullopt };
	}

	auto FileStorage::write_file(const std::string& bucket_name, const std::string& object_key,
								 const std::vector<uint8_t>& data)
		-> std::tuple<bool, std::optional<std::string>>
	{
		if (s3_storage_path_.empty())
		{
			return { false, "S3 storage path is empty." };
		}

		if (bucket_name.empty() || object_key.empty())
		{
			return { false, "Invalid parameters: bucket_name or object_key is empty." };
		}

		auto file_path = get_file_path(bucket_name, object_key);
		auto parent_path = std::filesystem::path(file_path).parent_path();

		if (!ensure_directory(parent_path.string()))
		{
			return { false, fmt::format("Failed to create directory: {}", parent_path.string()) };
		}

		try
		{
			std::ofstream ofs(file_path, std::ios::binary | std::ios::trunc);
			if (!ofs.is_open())
			{
				return { false, fmt::format("Failed to open file for writing: {}", file_path) };
			}

			ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
			ofs.close();

			Logger::handle().write(LogTypes::Information,
				fmt::format("Written {} bytes to {}", data.size(), file_path));

			return { true, std::nullopt };
		}
		catch (const std::exception& e)
		{
			return { false, fmt::format("Failed to write file: {}", e.what()) };
		}
	}

	auto FileStorage::write_file(const std::string& bucket_name, const std::string& object_key,
								 const std::string& data)
		-> std::tuple<bool, std::optional<std::string>>
	{
		std::vector<uint8_t> bytes(data.begin(), data.end());
		return write_file(bucket_name, object_key, bytes);
	}

	auto FileStorage::read_file(const std::string& bucket_name, const std::string& object_key)
		-> std::tuple<std::optional<std::vector<uint8_t>>, std::optional<std::string>>
	{
		if (s3_storage_path_.empty())
		{
			return { std::nullopt, "S3 storage path is empty." };
		}

		if (bucket_name.empty() || object_key.empty())
		{
			return { std::nullopt, "Invalid parameters: bucket_name or object_key is empty." };
		}

		auto file_path = get_file_path(bucket_name, object_key);

		if (!std::filesystem::exists(file_path))
		{
			return { std::nullopt, fmt::format("File does not exist: {}", file_path) };
		}

		try
		{
			std::ifstream ifs(file_path, std::ios::binary | std::ios::ate);
			if (!ifs.is_open())
			{
				return { std::nullopt, fmt::format("Failed to open file for reading: {}", file_path) };
			}

			auto size = ifs.tellg();
			ifs.seekg(0, std::ios::beg);

			std::vector<uint8_t> data(size);
			ifs.read(reinterpret_cast<char*>(data.data()), size);
			ifs.close();

			Logger::handle().write(LogTypes::Information,
				fmt::format("Read {} bytes from {}", size, file_path));

			return { data, std::nullopt };
		}
		catch (const std::exception& e)
		{
			return { std::nullopt, fmt::format("Failed to read file: {}", e.what()) };
		}
	}

	auto FileStorage::delete_file(const std::string& bucket_name, const std::string& object_key)
		-> std::tuple<bool, std::optional<std::string>>
	{
		if (s3_storage_path_.empty())
		{
			return { false, "S3 storage path is empty." };
		}

		if (bucket_name.empty() || object_key.empty())
		{
			return { false, "Invalid parameters: bucket_name or object_key is empty." };
		}

		auto file_path = get_file_path(bucket_name, object_key);

		if (!std::filesystem::exists(file_path))
		{
			return { false, fmt::format("File does not exist: {}", file_path) };
		}

		try
		{
			std::filesystem::remove(file_path);
			Logger::handle().write(LogTypes::Information, fmt::format("Deleted file: {}", file_path));
			return { true, std::nullopt };
		}
		catch (const std::exception& e)
		{
			return { false, fmt::format("Failed to delete file: {}", e.what()) };
		}
	}

	auto FileStorage::get_file_path(const std::string& bucket_name, const std::string& object_key)
		-> std::string
	{
		return fmt::format("{}/{}/{}", s3_storage_path_, bucket_name, object_key);
	}

	auto FileStorage::get_file_size(const std::string& bucket_name, const std::string& object_key)
		-> std::tuple<std::optional<size_t>, std::optional<std::string>>
	{
		auto file_path = get_file_path(bucket_name, object_key);

		if (!std::filesystem::exists(file_path))
		{
			return { std::nullopt, fmt::format("File does not exist: {}", file_path) };
		}

		try
		{
			auto size = std::filesystem::file_size(file_path);
			return { size, std::nullopt };
		}
		catch (const std::exception& e)
		{
			return { std::nullopt, fmt::format("Failed to get file size: {}", e.what()) };
		}
	}

	auto FileStorage::load_storage() -> std::tuple<bool, std::optional<std::string>>
	{
		if (s3_storage_path_.empty())
		{
			Logger::handle().write(LogTypes::Error, "S3 storage path is empty.");
			return { false, "S3 storage path is empty." };
		}

		if (!std::filesystem::exists(s3_storage_path_))
		{
			if (!ensure_directory(s3_storage_path_))
			{
				return { false, fmt::format("Failed to create storage path: {}", s3_storage_path_) };
			}
		}

		if (!std::filesystem::is_directory(s3_storage_path_))
		{
			Logger::handle().write(LogTypes::Error,
				fmt::format("S3 storage path is not a directory: {}", s3_storage_path_));
			return { false, fmt::format("S3 storage path is not a directory: {}", s3_storage_path_) };
		}

		return { true, std::nullopt };
	}

	// Legacy interface implementations
	auto FileStorage::append_file(const std::string& file_name, const std::string& object_name)
		-> std::tuple<bool, std::optional<std::string>>
	{
		Logger::handle().write(LogTypes::Warning, "append_file is deprecated, use write_file instead");
		return { true, std::nullopt };
	}

	auto FileStorage::download_file(const std::string& file_name, const std::string& object_name)
		-> std::tuple<bool, std::optional<std::string>>
	{
		Logger::handle().write(LogTypes::Warning, "download_file is deprecated, use read_file instead");
		return { true, std::nullopt };
	}
}
