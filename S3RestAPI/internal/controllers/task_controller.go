package controllers

import (
	"S3RestAPI/internal/transport"
	"errors"
	"net/http"

	"github.com/gin-gonic/gin"
	"github.com/go-redis/redis"
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

	// val: "Running", "Success", or "Fail"
	c.JSON(http.StatusOK, gin.H{
		"request_id": id,
		"status":     val,
	})
}
