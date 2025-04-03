package main

import (
	"fmt"
	"log"
	"os"

	"github.com/gin-gonic/gin"
	"github.com/joho/godotenv"

	"S3RestAPI/internal/controllers"
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

	rabbitmqDns := getEnv("RABBITMQ_DNS", "amqp://guest:guest@localhost:5672/")
	rabbitmqQueue := getEnv("RABBITMQ_QUEUE", "")

	redisDns := getEnv("REDIS_ADDR", "localhost:6379")
	redisPassword := getEnv("REDIS_PASS", "")

	serverPort := getEnv("SERVER_PORT", "8080")

	if err := transport.SetupRabbitMQ(rabbitmqDns, rabbitmqQueue); err != nil {
		log.Fatalf("Failed to connect to RabbitMQ: %v", err)
	}
	defer transport.CloseRabbitMQ()

	if err := transport.SetupRedis(redisDns, redisPassword); err != nil {
		log.Fatalf("Failed to connect to Redis: %v", err)
	}
	defer transport.CloseRedis()

	router := gin.Default()
	router.POST("/testapi", controllers.SendTestCommand)
	router.GET("/health", controllers.HealthCheck)
	router.POST("/login", controllers.LoginHandler)
	/*
		// Use middleware for API group
		apiGroup := router.Group("/api")
		apiGroup.Use(middleware.AuthMiddleware())
		{
			router.POST("/buckets/:bucket/files", controllers.UploadFile)
			router.DELETE("/buckets/:bucket/files/:filename", controllers.RemoveFile)

			router.GET("/tasks/:id/status", controllers.TaskStatus)

			router.POST("/buckets", controllers.CreateBucket)
			router.GET("/buckets/:bucket/files/:filename/download", controllers.DownloadFile)
			router.GET("/buckets/:bucket/files", controllers.ListFiles)
		}
	*/
	router.POST("/buckets/:bucket/files", controllers.UploadFile)
	router.DELETE("/buckets/:bucket/files/:filename", controllers.RemoveFile)

	router.GET("/tasks/:id/status", controllers.TaskStatus)

	router.POST("/buckets", controllers.CreateBucket)
	router.GET("/buckets/:bucket/files/:filename/download", controllers.DownloadFile)
	router.GET("/buckets/:bucket/files", controllers.ListFiles)

	addr := ":" + serverPort
	log.Printf("Server is running on %s\n", addr)
	if err := router.Run(addr); err != nil {
		log.Fatalf("Server run error: %v", err)
	}
}
