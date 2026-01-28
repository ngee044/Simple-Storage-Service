#pragma once

#include <string>
#include <optional>
#include <tuple>
#include <memory>
#include <chrono>

namespace S3Common
{
	class RedisClient
	{
	public:
		RedisClient(const std::string& host, int port, const std::string& password = "");
		~RedisClient();

		auto connect() -> std::tuple<bool, std::optional<std::string>>;
		auto disconnect() -> void;
		auto is_connected() const -> bool;

		auto set(const std::string& key, const std::string& value,
				 std::chrono::seconds ttl = std::chrono::seconds(0))
			-> std::tuple<bool, std::optional<std::string>>;

		auto get(const std::string& key)
			-> std::tuple<std::optional<std::string>, std::optional<std::string>>;

		auto del(const std::string& key)
			-> std::tuple<bool, std::optional<std::string>>;

	private:
		std::string host_;
		int port_;
		std::string password_;
		bool connected_;

		class Impl;
		std::unique_ptr<Impl> impl_;
	};
}
