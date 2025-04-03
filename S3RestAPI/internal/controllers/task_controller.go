package controllers

import (
	"S3RestAPI/internal/transport"
	"errors"
	"net/http"

	"github.com/gin-gonic/gin"
	"github.com/go-redis/redis"
	"github.com/google/uuid"
)

func TaskStatus(c *gin.Context) {
	id := c.Param("id")
	if id == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "작업 ID가 필요합니다"})
		return
	}

	val, err := transport.RedisDataGet(id)
	if err != nil {
		if errors.Is(err, redis.Nil) {
			val = "Not yet consumed"
		} else {
			c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
			return
		}
	}

	c.JSON(http.StatusOK, gin.H{
		"request_id": id,
		"status":     val,
	})
}

func SendTestCommand(c *gin.Context) {
	cmd := c.Param("command")
	send_message := c.Param("message")
	if cmd == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "command가 필요합니다"})
		return
	}

	requestID := uuid.New().String()

	message := gin.H{
		"request_id": requestID,
		"command":    "test_command",
		"message":    send_message,
	}

	err := transport.RabbitMQPublishMessage(message, requestID)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	c.JSON(http.StatusAccepted, gin.H{"request_id": requestID})
}
