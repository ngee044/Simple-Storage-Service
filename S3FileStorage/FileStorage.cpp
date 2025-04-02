#include "FileStorage.h"

#include "Logger.h"
#include "fmt/format.h"

#include "boost/json.hpp"
#include "boost/json/parse.hpp"

#include "File.h"

#include <filesystem>

using namespace Utilities;

namespace S3FileStorage
{
	FileStorage::FileStorage(const std::string& s3_storage_path)
		: s3_storage_path_(s3_storage_path) 
	{
		
	}

	FileStorage::~FileStorage()
	{

	}

	auto FileStorage::file_exists(const std::string &bucket_name, const std::string& object_name, const std::string& file_name) -> std::tuple<bool, std::optional<std::string>>
	{
		if (s3_storage_path_.empty())
		{
			Logger::handle().write(LogTypes::Error, "S3 storage path is empty.");
			return { false, "S3 storage path is empty." };
		}

		if (bucket_name.empty() || object_name.empty() || file_name.empty())
		{
			Logger::handle().write(LogTypes::Error, "Invalid parameters.");
			return { false, "Invalid parameters." };
		}

		auto file_path = fmt::format("{}/{}/{}", s3_storage_path_, bucket_name, object_name);
		if (!std::filesystem::exists(file_path))
		{
			Logger::handle().write(LogTypes::Error, fmt::format("File does not exist: {}", file_path));
			return { false, fmt::format("File does not exist: {}", file_path) };
		}

		return { true, std::nullopt };
	}

	auto FileStorage::create_folder(const std::string &folder_name) -> std::tuple<bool, std::optional<std::string>>
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
			Logger::handle().write(LogTypes::Error, fmt::format("Folder already exists: {}", folder_path));
			return { false, fmt::format("Folder already exists: {}", folder_path) };
		}

		if (!std::filesystem::create_directory(folder_path))
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Failed to create folder: {}", folder_path));
			return { false, fmt::format("Failed to create folder: {}", folder_path) };
		}

		return { true, std::nullopt };
	}

	auto FileStorage::append_file(const std::string &file_name, const std::string &object_name) -> std::tuple<bool, std::optional<std::string>>
	{
		if (s3_storage_path_.empty() || file_name.empty() || object_name.empty())
		{
			Logger::handle().write(LogTypes::Error, fmt::format("Invalid parameters. main_path: {}, file name: {}, object name: {}", s3_storage_path_, file_name, object_name));
			return { false, "Invalid parameters." };
		}

		auto file_path = fmt::format("{}/{}", s3_storage_path_, object_name);
		if (std::filesystem::exists(file_path))
		{
			Logger::handle().write(LogTypes::Error, fmt::format("File already exists: {}", file_path));
			return { false, fmt::format("File already exists: {}", file_path) };
		}

		// TODO
		// Implement file upload logic here.

		return { true, std::nullopt };
	}

	auto FileStorage::download_file(const std::string &file_name, const std::string &object_name) -> std::tuple<bool, std::optional<std::string>>
	{
		return std::tuple<bool, std::optional<std::string>>();
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
			Logger::handle().write(LogTypes::Error, fmt::format("S3 storage path does not exist: {}", s3_storage_path_));
			return { false, fmt::format("S3 storage path does not exist: {}", s3_storage_path_) };
		}

		if (!std::filesystem::is_directory(s3_storage_path_))
		{
			Logger::handle().write(LogTypes::Error, fmt::format("S3 storage path is not a directory: {}", s3_storage_path_));
			return { false, fmt::format("S3 storage path is not a directory: {}", s3_storage_path_) };
		}

		return { true, std::nullopt };
	}
	
	auto FileStorage::get_file_path(const std::string &bucket_name, const std::string &object_name, const std::string &file_name) -> std::string
	{
		return fmt::format("{}/{}/{}/{}", s3_storage_path_, bucket_name, object_name, file_name);
	}
}