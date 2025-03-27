// internal/transport/redis.go
package transport

import (
	"context"
	"fmt"

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

	_, err := redisClient.Ping().Result()
	if err != nil {
		return fmt.Errorf("Redis ping fail: %w", err)
	}
	return nil
}

func CloseRedis() {
	if redisClient != nil {
		redisClient.Close()
	}
}
