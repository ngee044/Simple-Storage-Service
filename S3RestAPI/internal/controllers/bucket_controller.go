package controllers

import (
	"net/http"
	"time"

	"S3RestAPI/internal/transport"

	"github.com/gin-gonic/gin"
	"github.com/google/uuid"

	"S3RestAPI/internal/models"
)

func CreateBucket(c *gin.Context) {
	var bucket_request models.BucketRequest
	if err := c.ShouldBindJSON(&bucket_request); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	if bucket_request.BucketName == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Bucket name is required"})
		return
	}

	requestID := uuid.New().String()

	if err := transport.RedisDataSet(requestID, "create_bucket"); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	message := gin.H{
		"action":      "create_bucket",
		"request_id":  requestID,
		"bucket_name": bucket_request.BucketName,
		"created_at":  time.Now().Format(time.RFC3339),
	}

	err := transport.RabbitMQPublishMessage(message, requestID)
	if err != nil {
		transport.RedisDataSet(requestID, "Failed to publish message")
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	// result 202 Accepted
	c.JSON(http.StatusAccepted, gin.H{"request_id": requestID})
}

func ListFiles(c *gin.Context) {
	bucket := c.Param("bucket")
	if bucket == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Bucket name required"})
		return
	}

	requestID := uuid.New().String()
	message := gin.H{
		"action":      "bucket_file_list",
		"request_id":  requestID,
		"bucket_name": bucket,
	}

	err := transport.RabbitMQPublishMessage(message, requestID)
	if err != nil {
		transport.RedisDataSet(requestID, "Failed to publish message")
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	response_message := gin.H{
		"action":     "bucket_file_list",
		"bucket":     bucket,
		"request_id": requestID,
	}

	c.JSON(http.StatusAccepted, response_message)
}
