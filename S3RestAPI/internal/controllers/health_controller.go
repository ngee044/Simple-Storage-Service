package controllers

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

func HealthCheck(c *gin.Context) {
	if err := transport.redisClient.Ping(transport.context_bg).Err(); err != nil {
		c.JSON(http.StatusServiceUnavailable, gin.H{
			"status": "unhealthy",
			"error":  "Redis ping fail: " + err.Error(),
		})
		return
	}

	if transport.rabbitChannel == nil || transport.rabbitConn == nil {
		c.JSON(http.StatusServiceUnavailable, gin.H{
			"status": "unhealthy",
			"error":  "RabbitMQ channel/conn nil",
		})
		return
	}

	c.JSON(http.StatusOK, gin.H{"status": "OK"})
}
