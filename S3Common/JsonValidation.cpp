#include "JsonValidation.h"

namespace S3Common
{
	namespace JsonValidation
	{
		auto validate_create_bucket_value(const boost::json::value json_value) -> std::tuple<bool, std::optional<std::string>>
		{
			if (!json_value.is_object())
			{
				return { false, "Invalid JSON object" };
			}

			auto message = json_value.as_object();

			if (!message.contains("bucket_name") || !message.at("bucket_name").is_string())
			{
				return { false, "Invalid bucket_name" };
			}

			if (!message.contains("region") || !message.at("region").is_string())
			{
				return { false, "Invalid region" };
			}

			return { true, std::nullopt };
		}

		auto validate_upload_file_value(const boost::json::value json_value) -> std::tuple<bool, std::optional<std::string>>
		{
			if (!json_value.is_object())
			{
				return { false, "Invalid JSON object" };
			}

			auto message = json_value.as_object();

			if (!message.contains("bucket_name") || !message.at("bucket_name").is_string())
			{
				return { false, "Invalid bucket_name" };
			}

			if (!message.contains("file_name") || !message.at("file_name").is_string())
			{
				return { false, "Invalid file_name" };
			}

			if (!message.contains("object_name") || !message.at("object_name").is_string())
			{
				return { false, "Invalid object_name" };
			}

			return { true, std::nullopt };
		}

		auto validate_download_file_value(const boost::json::value json_value) -> std::tuple<bool, std::optional<std::string>>
		{
			if (!json_value.is_object())
			{
				return { false, "Invalid JSON object" };
			}

			auto message = json_value.as_object();

			if (!message.contains("bucket_name") || !message.at("bucket_name").is_string())
			{
				return { false, "Invalid bucket_name" };
			}

			if (!message.contains("file_name") || !message.at("file_name").is_string())
			{
				return { false, "Invalid file_name" };
			}

			if (!message.contains("object_name") || !message.at("object_name").is_string())
			{
				return { false, "Invalid object_name" };
			}

			return { true, std::nullopt };
		}
	}
}