// internal/controllers/file_controller.go
package controllers

import (
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
	if err := c.ShouldBindJSON(&request); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	if request.Filename == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Filename required"})
		return
	}

	requestId := uuid.New().String()

	message := models.TaskMessage{
		Action:     "upload_file",
		BucketName: bucketName,
		FileName:   request.Filename,
		RequestID:  requestId,
	}

	err := transport.RabbitMQPublishMessage(message, requestId)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to publish message"})
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

	requestId := uuid.New().String()
	message := models.TaskMessage{
		Action:     "download_file",
		BucketName: bucket,
		FileName:   filename,
		RequestID:  requestId,
	}

	err := transport.RabbitMQPublishMessage(message, requestId)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to publish message"})
		return
	}

	c.JSON(http.StatusAccepted, gin.H{"request_id": requestId})
}

func RemoveFile(c *gin.Context) {
	bucket := c.Param("bucket")
	filename := c.Param("filename")
	if bucket == "" || filename == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "bucket filename required"})
		return
	}

	requestId := uuid.New().String()
	message := models.TaskMessage{
		Action:     "remove_file",
		BucketName: bucket,
		FileName:   filename,
		RequestID:  requestId,
	}

	err := transport.RabbitMQPublishMessage(message, requestId)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to publish message"})
		return
	}

	c.JSON(http.StatusAccepted, gin.H{"request_id": requestId})
}
