// internal/controllers/file_controller.go
package controllers

import (
	"fmt"
	"net/http"

	"S3RestAPI/internal/models"
	"S3RestAPI/internal/transport"

	"github.com/gin-gonic/gin"
	"github.com/google/uuid"
)

func UploadFile(c *gin.Context) {
	bucketName := c.Param("bucket")
	if bucketName == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Bucket name required"})
		return
	}

	var request models.FileRequest
	if err := c.ShouldBindJSON(&request); err != nil || request.Name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	requestId := uuid.New().String()
	taskKey := "task:" + requestId
	if err := transport.redisClient.Set(transport.ContextBg, taskKey, "File uploaded", 0).Err(); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	message := models.TaskMessage{
		Action:     "upload_file",
		BucketName: bucketName,
		Filename:   request.Name,
		RequestID:  requestId,
	}

	err := transport.PublishMessage(message, requestId)
	if err != nil {
		transport.redisClient.Set(transport.ContextBg, taskKey, "Failed to publish message", 0)
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	c.JSON(http.StatusAccepted, gin.H{"request_id": requestId})
}

func DownloadFile(c *gin.Context) {
	bucket := c.Param("bucket")
	filename := c.Param("filename")
	if bucket == "" || filename == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "bucket filename required"})
		return
	}

	key := fmt.Sprintf("file:%s:%s:download", bucket, filename)
	val, err := transport.redisClient.Get(transport.context_bg, key).Result()
	if err == transport.redisClient.Nil() {
		c.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	} else if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	c.Data(http.StatusOK, "application/json; charset=utf-8", []byte(val))
}

func RemoveFile(c *gin.Context) {
	bucket := c.Param("bucket")
	filename := c.Param("filename")
	if bucket == "" || filename == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "bucket filename required"})
		return
	}
	requestId := uuid.New().String()
	taskKey := "task:" + requestId
	if err := transport.redisClient.Set(transport.context_bg, taskKey, "Running", 0).Err(); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to set task status"})
		return
	}

	message := models.TaskMessage{
		Action:     "remove_file",
		BucketName: bucket,
		Filename:   filename,
		RequestID:  requestId,
	}

	err := transport.PublishMessage(message, requestId)
	if err != nil {
		transport.redisClient.Set(transport.context_bg, taskKey, "Fail", 0)
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to publish message"})
		return
	}

	c.JSON(http.StatusAccepted, gin.H{"correlation_id": requestId})
}
