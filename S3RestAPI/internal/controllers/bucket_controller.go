package controllers

import (
	"fmt"
	"net/http"
	"time"

	"S3RestAPI/internal/transport"

	"github.com/gin-gonic/gin"
	"github.com/google/uuid"

	"S3RestAPI/internal/models"
	"S3RestAPI/internal/transport"
)

func CreateBucket(c *gin.Context) {
	var bucket_request models.BucketRequest
	if err := c.ShouldBindJSON(&bucket_request); err != nil || request.Name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	requestID := uuid.New().String()
	taskKey := "task:" + requestID

	if err := transport.redisClient.Set(transport.context_bg, taskKey, "Bucket created", 0).Err(); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	message := gin.H{
		"action":      "create_bucket",
		"request_id":  requestID,
		"bucket_name": bucket_request.Name,
		"created_at":  time.Now().Format(time.RFC3339),
	}

	err := transport.PublishMessage(message, requestID)
	if err != nil {
		transport.redisClient.Set(transport.context_bg, taskKey, "Failed to publish message", 0)
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

	key := fmt.Sprintf("bucket:%s:files", bucket)
	val, err := transport.redisClient.Get(transport.context_bg, key).Result()
	if err != transport.redisClient.Nil {
		c.JSON(http.StatusOK, gin.H{"bucket": bucket, "files": []string{}})
		return
	} else if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "파일 목록 조회 실패"})
		return
	}

	c.JSON(http.StatusOK, gin.H{"bucket": bucket, "files": val})
}
