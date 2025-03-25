#include <string>
#include <tuple>
#include <optional>

#include "boost/json.hpp"
#include "boost/json/parse.hpp"

namespace S3Common
{
	namespace JsonValidation
	{
		auto validate_create_bucket_value(const boost::json::value json_value) -> std::tuple<bool, std::optional<std::string>>;
		auto validate_upload_file_value(const boost::json::value json_value) -> std::tuple<bool, std::optional<std::string>>;
		auto validate_download_file_value(const boost::json::value json_value) -> std::tuple<bool, std::optional<std::string>>;
	}
}