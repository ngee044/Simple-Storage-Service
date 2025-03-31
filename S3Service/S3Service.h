#pragma once

#include "Configurations.h"
#include "ThreadPool.h"
#include "Logger.h"
#include "WorkQueueConsume.h"
#include "FileStorage.h"
#include "S3MetaDB.h"


#include <string>
#include <memory>
#include <map>
#include <tuple>
#include <functional>
#include <optional>

using namespace Thread;
using namespace RabbitMQ;
using namespace S3FileStorage;

namespace S3Service
{
	class S3ServiceMain
	{
	public:
		S3ServiceMain(std::shared_ptr<Configurations> configurations);
		~S3ServiceMain();

		auto start() -> std::tuple<bool, std::optional<std::string>>;
		auto wait_stop() -> std::tuple<bool, std::optional<std::string>>;
		auto stop() -> void;
	
	protected:
		auto create_thread_pool() -> std::tuple<bool, std::optional<std::string>>;
		auto destroy_thread_pool() -> void;

		auto consume_queue() -> std::tuple<bool, std::optional<std::string>>;

		auto create_bucket(const std::string& message) -> std::tuple<bool, std::optional<std::string>>;
		auto upload_file(const std::string& message) -> std::tuple<bool, std::optional<std::string>>;
		auto download_file(const std::string& message) -> std::tuple<bool, std::optional<std::string>>;

	private:
		std::map<std::string, std::function<std::tuple<bool, std::optional<std::string>>(const std::string&)>> commands_;
		bool is_running_;
		
		std::shared_ptr<WorkQueueConsume> work_queue_consume_;
		std::shared_ptr<Configurations> configurations_;
		std::shared_ptr<ThreadPool> thread_pool_;

		const int work_queue_consume_channel_id_ = 1;

		std::shared_ptr<FileStorage> file_storage_;
		std::shared_ptr<S3MetaDB::S3MetaDB> s3_meta_db_;
	
	};
}