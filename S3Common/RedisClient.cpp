#include "RedisClient.h"

#include <sw/redis++/redis++.h>

namespace S3Common
{
	class RedisClient::Impl
	{
	public:
		std::unique_ptr<sw::redis::Redis> redis;
	};

	RedisClient::RedisClient(const std::string& host, int port, const std::string& password)
		: host_(host)
		, port_(port)
		, password_(password)
		, connected_(false)
		, impl_(std::make_unique<Impl>())
	{
	}

	RedisClient::~RedisClient()
	{
		disconnect();
	}

	auto RedisClient::connect() -> std::tuple<bool, std::optional<std::string>>
	{
		try
		{
			sw::redis::ConnectionOptions opts;
			opts.host = host_;
			opts.port = port_;
			if (!password_.empty())
			{
				opts.password = password_;
			}
			opts.socket_timeout = std::chrono::milliseconds(5000);

			impl_->redis = std::make_unique<sw::redis::Redis>(opts);
			impl_->redis->ping();
			connected_ = true;
			return { true, std::nullopt };
		}
		catch (const sw::redis::Error& e)
		{
			connected_ = false;
			return { false, std::string(e.what()) };
		}
	}

	auto RedisClient::disconnect() -> void
	{
		if (impl_ && impl_->redis)
		{
			impl_->redis.reset();
		}
		connected_ = false;
	}

	auto RedisClient::is_connected() const -> bool
	{
		return connected_;
	}

	auto RedisClient::set(const std::string& key, const std::string& value,
						  std::chrono::seconds ttl)
		-> std::tuple<bool, std::optional<std::string>>
	{
		if (!connected_ || !impl_->redis)
		{
			return { false, "Not connected to Redis" };
		}

		try
		{
			if (ttl.count() > 0)
			{
				impl_->redis->setex(key, ttl, value);
			}
			else
			{
				impl_->redis->set(key, value);
			}
			return { true, std::nullopt };
		}
		catch (const sw::redis::Error& e)
		{
			return { false, std::string(e.what()) };
		}
	}

	auto RedisClient::get(const std::string& key)
		-> std::tuple<std::optional<std::string>, std::optional<std::string>>
	{
		if (!connected_ || !impl_->redis)
		{
			return { std::nullopt, "Not connected to Redis" };
		}

		try
		{
			auto result = impl_->redis->get(key);
			if (result)
			{
				return { *result, std::nullopt };
			}
			return { std::nullopt, std::nullopt };
		}
		catch (const sw::redis::Error& e)
		{
			return { std::nullopt, std::string(e.what()) };
		}
	}

	auto RedisClient::del(const std::string& key)
		-> std::tuple<bool, std::optional<std::string>>
	{
		if (!connected_ || !impl_->redis)
		{
			return { false, "Not connected to Redis" };
		}

		try
		{
			impl_->redis->del(key);
			return { true, std::nullopt };
		}
		catch (const sw::redis::Error& e)
		{
			return { false, std::string(e.what()) };
		}
	}
}
