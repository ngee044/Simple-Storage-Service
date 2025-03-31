#include "FileStorage.h"

#include "Logger.h"
#include "fmt/format.h"

#include "boost/json.hpp"
#include "boost/json/parse.hpp"

#include "File.h"


namespace S3FileStorage
{
	FileStorage::FileStorage(const std::string& s3_storage_path)
		: s3_storage_path_(s3_storage_path) 
	{
		
	}

	FileStorage::~FileStorage()
	{
		
	}
}