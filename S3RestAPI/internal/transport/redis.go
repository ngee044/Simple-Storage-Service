// internal/transport/redis.go
package transport

import (
	"context"
	"fmt"
	"time"

	"github.com/go-redis/redis"
)

var redisClient *redis.Client
var context_bg = context.Background()

func SetupRedis(addr, password string) error {
	redisClient = redis.NewClient(&redis.Options{
		Addr:     addr,
		Password: password,
		DB:       0,
	})

	if err := redisClient.Ping().Err(); err != nil {
		return fmt.Errorf("Failed to connect to Redis: %w", err)
	}

	return nil
}

func CloseRedis() {
	if redisClient != nil {
		redisClient.Close()
	}
}

func RedisDataSet(key, value string) error {
	return redisClient.Set(key, value, 600*time.Second).Err()
}

func RedisDataGet(key string) (string, error) {
	return redisClient.Get(key).Result()
}

func RedisPing() error {
	if redisClient == nil {
		return fmt.Errorf("Redis client is nil")
	}

	_, err := redisClient.Ping().Result()
	if err != nil {
		return fmt.Errorf("Redis ping fail: %w", err)
	}
	return nil
}
