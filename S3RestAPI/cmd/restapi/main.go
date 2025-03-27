package main

import (
	"log"

	"github.com/gin-gonic/gin"

	"S3RestAPI/internal/controllers"
	"S3RestAPI/internal/transport"
)

func main() {

	if err := transport.SetupRabbitMQ("amqp://guest:guest@localhost:5672/", "request_message"); err != nil {
		log.Fatalf("Failed to connect to RabbitMQ: %v", err)
	}
	defer transport.CloseRabbitMQ()

	if err := transport.SetupRedis("localhost:6379", ""); err != nil {
		log.Fatalf("Failed to connect to Redis: %v", err)
	}
	defer transport.CloseRedis()

	router := gin.Default()

	/*
		// Use middleware for API group
		apiGroup := router.Group("/api")
		apiGroup.Use(middleware.AuthMiddleware())
		{
			apiGroup.POST("/buckets/:bucket/files", controllers.UploadFile)
			apiGroup.DELETE("/buckets/:bucket/files/:filename", controllers.RemoveFile)
		}
	*/

	router.POST("/buckets/:bucket/files", controllers.UploadFile)
	router.DELETE("/buckets/:bucket/files/:filename", controllers.RemoveFile)

	router.GET("/tasks/:id/status", controllers.TaskStatus)
	router.GET("/health", controllers.HealthCheck)

	router.POST("/buckets", controllers.CreateBucket)
	router.GET("/buckets/:bucket/files/:filename/download", controllers.DownloadFile)
	router.GET("/buckets/:bucket/files", controllers.ListFiles)

	log.Println("Starting REST API server on :8080...")
	if err := router.Run(":8080"); err != nil {
		log.Fatalf("Server run error: %v", err)
	}
}
