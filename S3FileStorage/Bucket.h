#pragma once

#include <string>
#include <tuple>
#include <vector>
#include <map>
#include <optional>

namespace S3FileStorage
{
	struct S3File
	{
		std::string file_name;
		std::int64_t file_size;
		std::string file_path;
	};

	class Bucket
	{
	public:
		Bucket(const std::string& bucket_name);

		auto bucket_name() const -> const std::string& { return bucket_name_; }
		auto bucket_name(const std::string& bucket_name) -> void { bucket_name_ = bucket_name; }

	protected:

	private:
		std::string bucket_name_;
		std::map<std::string, std::vector<S3File>> files_;
	};
}