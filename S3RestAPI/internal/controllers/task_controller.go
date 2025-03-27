package controllers

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

func TaskStatus(c *gin.Context) {
	id := c.Param("id")
	if id == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "작업 ID가 필요합니다"})
		return
	}
	taskKey := "task:" + id
	val, err := transport.redisClient.Get(transport.context_bg, taskKey).Result()
	if val == nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "해당 작업 ID를 찾을 수 없습니다"})
		return
	} else if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "작업 상태 조회 실패"})
		return
	}

	// val: "Running", "Success", or "Fail"
	c.JSON(http.StatusOK, gin.H{
		"request_id": id,
		"status":     val,
	})
}
