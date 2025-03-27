package controllers

import (
	"net/http"

	"S3RestAPI/internal/transport"

	"github.com/gin-gonic/gin"
)

func HealthCheck(c *gin.Context) {
	if redis_err := transport.RedisPing(); redis_err != nil {
		c.JSON(http.StatusServiceUnavailable, gin.H{"status": "unhealthy", "error": redis_err.Error()})
		return
	}

	if rabbit_err := transport.RabbitMQPing(); rabbit_err != nil {
		c.JSON(http.StatusServiceUnavailable, gin.H{"status": "unhealthy", "error": rabbit_err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{"status": "OK"})
}
