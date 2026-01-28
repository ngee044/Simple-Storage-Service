package main

import (
	"fmt"
	"log"
	"os"

	"github.com/gin-gonic/gin"
	"github.com/joho/godotenv"

	"S3RestAPI/internal/controllers"
	"S3RestAPI/internal/middleware"
	"S3RestAPI/internal/transport"
)

func getEnv(key, defaultValue string) string {
	val := os.Getenv(key)
	if val == "" {
		return defaultValue
	}
	return val
}

func main() {
	env := os.Getenv("APP_ENV")
	if env == "" {
		env = "development"
	}

	envFile := fmt.Sprintf(".env.%s", env)
	if err := godotenv.Load(envFile); err != nil {
		log.Println("No .env file found")
	}

	rabbitmqDsn := getEnv("RABBITMQ_DSN", "amqp://guest:guest@localhost:5672/")
	rabbitmqQueue := getEnv("RABBITMQ_QUEUE", "")

	redisDns := getEnv("REDIS_ADDR", "localhost:6379")
	redisPassword := getEnv("REDIS_PASS", "")

	serverPort := getEnv("SERVER_PORT", "8080")

	if err := transport.SetupRabbitMQ(rabbitmqDsn, rabbitmqQueue); err != nil {
		log.Fatalf("Failed to connect to RabbitMQ: %v", err)
	}
	defer transport.CloseRabbitMQ()

	if err := transport.SetupRedis(redisDns, redisPassword); err != nil {
		log.Fatalf("Failed to connect to Redis: %v", err)
	}
	defer transport.CloseRedis()

	router := gin.Default()

	// Public endpoints (no authentication required)
	router.GET("/health", controllers.HealthCheck)
	router.POST("/login", controllers.LoginHandler)
	router.POST("/register", controllers.RegisterHandler)

	// Protected endpoints (authentication required)
	api := router.Group("/")
	api.Use(middleware.AuthMiddleware())
	{
		api.POST("/buckets", controllers.CreateBucket)
		api.GET("/buckets/:bucket/files", controllers.ListFiles)
		api.POST("/buckets/:bucket/files", controllers.UploadFile)
		api.GET("/buckets/:bucket/files/:filename/download", controllers.DownloadFile)
		api.DELETE("/buckets/:bucket/files/:filename", controllers.RemoveFile)
		api.GET("/tasks/:id/status", controllers.TaskStatus)
	}

	addr := ":" + serverPort
	log.Printf("Server is running on %s\n", addr)
	if err := router.Run(addr); err != nil {
		log.Fatalf("Server run error: %v", err)
	}
}
