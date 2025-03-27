package middleware

import (
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
	// "github.com/golang-jwt/jwt/v4" // 실제 JWT 파싱/검증용 라이브러리
)

func AuthMiddleware() gin.HandlerFunc {
	return func(c *gin.Context) {
		authHeader := c.GetHeader("Authorization")
		if !strings.HasPrefix(authHeader, "Bearer ") {
			c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{"error": "Unauthorized"})
			return
		}
		tokenString := strings.TrimPrefix(authHeader, "Bearer ")

		// 실제 JWT 검증 로직 (예: jwt.Parse)
		// if tokenInvalid { ... }
		// 검증 성공 시 다음으로
		c.Next()
	}
}
