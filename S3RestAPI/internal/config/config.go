package config

import (
	"crypto/rand"
	"encoding/base64"
	"log"
	"os"
	"sync"
)

var (
	jwtSecret     []byte
	jwtSecretOnce sync.Once
)

func GetJWTSecret() []byte {
	jwtSecretOnce.Do(func() {
		secret := os.Getenv("JWT_SECRET")
		if secret == "" {
			log.Println("WARNING: JWT_SECRET not set, using generated random secret")
			log.Println("WARNING: This is not suitable for production. Set JWT_SECRET environment variable.")
			jwtSecret = generateRandomSecret()
		} else {
			jwtSecret = []byte(secret)
		}
	})
	return jwtSecret
}

func generateRandomSecret() []byte {
	b := make([]byte, 32)
	_, err := rand.Read(b)
	if err != nil {
		log.Fatal("Failed to generate random secret:", err)
	}
	return []byte(base64.StdEncoding.EncodeToString(b))
}

func GetEnv(key, defaultValue string) string {
	val := os.Getenv(key)
	if val == "" {
		return defaultValue
	}
	return val
}
