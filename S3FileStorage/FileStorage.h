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


	protected:

	private:
		std::string s3_storage_path_;

		
	};
}
