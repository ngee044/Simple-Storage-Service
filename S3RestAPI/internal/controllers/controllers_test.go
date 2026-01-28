package controllers_test

import (
	"bytes"
	"encoding/json"
	"fmt"
	"net/http"
	"net/http/httptest"
	"testing"

	"S3RestAPI/internal/controllers"
	"S3RestAPI/internal/transport"

	"github.com/gin-gonic/gin"
)

func setupRouter() *gin.Engine {
	gin.SetMode(gin.TestMode)
	transport.EnableMock()

	r := gin.New()
	r.GET("/health", controllers.HealthCheck)
	r.POST("/login", controllers.LoginHandler)
	r.POST("/buckets", controllers.CreateBucket)
	r.GET("/buckets/:bucket/files", controllers.ListFiles)
	r.POST("/buckets/:bucket/files", controllers.UploadFile)
	r.GET("/buckets/:bucket/files/:filename/download", controllers.DownloadFile)
	r.DELETE("/buckets/:bucket/files/:filename", controllers.RemoveFile)
	r.GET("/tasks/:id/status", controllers.TaskStatus)

	return r
}

func teardown() {
	transport.DisableMock()
}

func TestHealthCheck_Success(t *testing.T) {
	router := setupRouter()
	defer teardown()

	req, _ := http.NewRequest("GET", "/health", nil)
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusOK {
		t.Errorf("Expected status %d, got %d", http.StatusOK, w.Code)
	}

	var response map[string]string
	json.Unmarshal(w.Body.Bytes(), &response)

	if response["status"] != "OK" {
		t.Errorf("Expected status 'OK', got '%s'", response["status"])
	}
}

func TestHealthCheck_RedisError(t *testing.T) {
	router := setupRouter()
	defer teardown()

	transport.SetMockRedisError(fmt.Errorf("redis connection failed"))

	req, _ := http.NewRequest("GET", "/health", nil)
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusServiceUnavailable {
		t.Errorf("Expected status %d, got %d", http.StatusServiceUnavailable, w.Code)
	}
}

func TestHealthCheck_RabbitMQError(t *testing.T) {
	router := setupRouter()
	defer teardown()

	transport.SetMockRabbitError(fmt.Errorf("rabbitmq connection failed"))

	req, _ := http.NewRequest("GET", "/health", nil)
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusServiceUnavailable {
		t.Errorf("Expected status %d, got %d", http.StatusServiceUnavailable, w.Code)
	}
}

func TestLogin_Success(t *testing.T) {
	router := setupRouter()
	defer teardown()

	body := map[string]string{
		"username": "admin",
		"password": "admin",
	}
	jsonBody, _ := json.Marshal(body)

	req, _ := http.NewRequest("POST", "/login", bytes.NewBuffer(jsonBody))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusOK {
		t.Errorf("Expected status %d, got %d", http.StatusOK, w.Code)
	}

	var response map[string]string
	json.Unmarshal(w.Body.Bytes(), &response)

	if response["token"] == "" {
		t.Error("Expected token in response")
	}
}

func TestLogin_InvalidCredentials(t *testing.T) {
	router := setupRouter()
	defer teardown()

	body := map[string]string{
		"username": "wrong",
		"password": "wrong",
	}
	jsonBody, _ := json.Marshal(body)

	req, _ := http.NewRequest("POST", "/login", bytes.NewBuffer(jsonBody))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusUnauthorized {
		t.Errorf("Expected status %d, got %d", http.StatusUnauthorized, w.Code)
	}
}

func TestLogin_BadRequest(t *testing.T) {
	router := setupRouter()
	defer teardown()

	req, _ := http.NewRequest("POST", "/login", bytes.NewBuffer([]byte("invalid json")))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusBadRequest {
		t.Errorf("Expected status %d, got %d", http.StatusBadRequest, w.Code)
	}
}

func TestCreateBucket_Success(t *testing.T) {
	router := setupRouter()
	defer teardown()

	body := map[string]string{
		"bucket_name": "test-bucket",
	}
	jsonBody, _ := json.Marshal(body)

	req, _ := http.NewRequest("POST", "/buckets", bytes.NewBuffer(jsonBody))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusAccepted {
		t.Errorf("Expected status %d, got %d", http.StatusAccepted, w.Code)
	}

	var response map[string]string
	json.Unmarshal(w.Body.Bytes(), &response)

	if response["request_id"] == "" {
		t.Error("Expected request_id in response")
	}

	messages := transport.GetMockMessages()
	if len(messages) != 1 {
		t.Errorf("Expected 1 message published, got %d", len(messages))
	}
}

func TestCreateBucket_EmptyName(t *testing.T) {
	router := setupRouter()
	defer teardown()

	body := map[string]string{
		"bucket_name": "",
	}
	jsonBody, _ := json.Marshal(body)

	req, _ := http.NewRequest("POST", "/buckets", bytes.NewBuffer(jsonBody))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusBadRequest {
		t.Errorf("Expected status %d, got %d", http.StatusBadRequest, w.Code)
	}
}

func TestCreateBucket_BadRequest(t *testing.T) {
	router := setupRouter()
	defer teardown()

	req, _ := http.NewRequest("POST", "/buckets", bytes.NewBuffer([]byte("invalid json")))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusBadRequest {
		t.Errorf("Expected status %d, got %d", http.StatusBadRequest, w.Code)
	}
}

func TestListFiles_Success(t *testing.T) {
	router := setupRouter()
	defer teardown()

	req, _ := http.NewRequest("GET", "/buckets/test-bucket/files", nil)
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusAccepted {
		t.Errorf("Expected status %d, got %d", http.StatusAccepted, w.Code)
	}

	var response map[string]interface{}
	json.Unmarshal(w.Body.Bytes(), &response)

	if response["request_id"] == "" {
		t.Error("Expected request_id in response")
	}

	if response["bucket"] != "test-bucket" {
		t.Errorf("Expected bucket 'test-bucket', got '%s'", response["bucket"])
	}
}

func TestUploadFile_Success(t *testing.T) {
	router := setupRouter()
	defer teardown()

	body := map[string]string{
		"filename": "test.txt",
	}
	jsonBody, _ := json.Marshal(body)

	req, _ := http.NewRequest("POST", "/buckets/test-bucket/files", bytes.NewBuffer(jsonBody))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusAccepted {
		t.Errorf("Expected status %d, got %d", http.StatusAccepted, w.Code)
	}

	var response map[string]string
	json.Unmarshal(w.Body.Bytes(), &response)

	if response["request_id"] == "" {
		t.Error("Expected request_id in response")
	}
}

func TestUploadFile_EmptyFilename(t *testing.T) {
	router := setupRouter()
	defer teardown()

	body := map[string]string{
		"filename": "",
	}
	jsonBody, _ := json.Marshal(body)

	req, _ := http.NewRequest("POST", "/buckets/test-bucket/files", bytes.NewBuffer(jsonBody))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusBadRequest {
		t.Errorf("Expected status %d, got %d", http.StatusBadRequest, w.Code)
	}
}

func TestUploadFile_BadRequest(t *testing.T) {
	router := setupRouter()
	defer teardown()

	req, _ := http.NewRequest("POST", "/buckets/test-bucket/files", bytes.NewBuffer([]byte("invalid")))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusBadRequest {
		t.Errorf("Expected status %d, got %d", http.StatusBadRequest, w.Code)
	}
}

func TestDownloadFile_Success(t *testing.T) {
	router := setupRouter()
	defer teardown()

	req, _ := http.NewRequest("GET", "/buckets/test-bucket/files/test.txt/download", nil)
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusAccepted {
		t.Errorf("Expected status %d, got %d", http.StatusAccepted, w.Code)
	}

	var response map[string]string
	json.Unmarshal(w.Body.Bytes(), &response)

	if response["request_id"] == "" {
		t.Error("Expected request_id in response")
	}
}

func TestRemoveFile_Success(t *testing.T) {
	router := setupRouter()
	defer teardown()

	req, _ := http.NewRequest("DELETE", "/buckets/test-bucket/files/test.txt", nil)
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusAccepted {
		t.Errorf("Expected status %d, got %d", http.StatusAccepted, w.Code)
	}

	var response map[string]string
	json.Unmarshal(w.Body.Bytes(), &response)

	if response["request_id"] == "" {
		t.Error("Expected request_id in response")
	}
}

func TestTaskStatus_Success(t *testing.T) {
	router := setupRouter()
	defer teardown()

	transport.EnableMock()
	_ = transport.RedisDataSet("test-request-id", "completed")

	req, _ := http.NewRequest("GET", "/tasks/test-request-id/status", nil)
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusOK {
		t.Errorf("Expected status %d, got %d", http.StatusOK, w.Code)
	}

	var response map[string]string
	json.Unmarshal(w.Body.Bytes(), &response)

	if response["request_id"] != "test-request-id" {
		t.Errorf("Expected request_id 'test-request-id', got '%s'", response["request_id"])
	}

	if response["status"] != "completed" {
		t.Errorf("Expected status 'completed', got '%s'", response["status"])
	}
}

func TestTaskStatus_NotFound(t *testing.T) {
	router := setupRouter()
	defer teardown()

	req, _ := http.NewRequest("GET", "/tasks/nonexistent/status", nil)
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusOK {
		t.Errorf("Expected status %d, got %d", http.StatusOK, w.Code)
	}
}

func TestCreateBucket_RabbitMQError(t *testing.T) {
	router := setupRouter()
	defer teardown()

	transport.SetMockRabbitError(fmt.Errorf("rabbitmq publish failed"))

	body := map[string]string{
		"bucket_name": "test-bucket",
	}
	jsonBody, _ := json.Marshal(body)

	req, _ := http.NewRequest("POST", "/buckets", bytes.NewBuffer(jsonBody))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusInternalServerError {
		t.Errorf("Expected status %d, got %d", http.StatusInternalServerError, w.Code)
	}
}

func TestUploadFile_RabbitMQError(t *testing.T) {
	router := setupRouter()
	defer teardown()

	transport.SetMockRabbitError(fmt.Errorf("rabbitmq publish failed"))

	body := map[string]string{
		"filename": "test.txt",
	}
	jsonBody, _ := json.Marshal(body)

	req, _ := http.NewRequest("POST", "/buckets/test-bucket/files", bytes.NewBuffer(jsonBody))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusInternalServerError {
		t.Errorf("Expected status %d, got %d", http.StatusInternalServerError, w.Code)
	}
}

func TestDownloadFile_RabbitMQError(t *testing.T) {
	router := setupRouter()
	defer teardown()

	transport.SetMockRabbitError(fmt.Errorf("rabbitmq publish failed"))

	req, _ := http.NewRequest("GET", "/buckets/test-bucket/files/test.txt/download", nil)
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusInternalServerError {
		t.Errorf("Expected status %d, got %d", http.StatusInternalServerError, w.Code)
	}
}

func TestRemoveFile_RabbitMQError(t *testing.T) {
	router := setupRouter()
	defer teardown()

	transport.SetMockRabbitError(fmt.Errorf("rabbitmq publish failed"))

	req, _ := http.NewRequest("DELETE", "/buckets/test-bucket/files/test.txt", nil)
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusInternalServerError {
		t.Errorf("Expected status %d, got %d", http.StatusInternalServerError, w.Code)
	}
}

func TestListFiles_RabbitMQError(t *testing.T) {
	router := setupRouter()
	defer teardown()

	transport.SetMockRabbitError(fmt.Errorf("rabbitmq publish failed"))

	req, _ := http.NewRequest("GET", "/buckets/test-bucket/files", nil)
	w := httptest.NewRecorder()
	router.ServeHTTP(w, req)

	if w.Code != http.StatusInternalServerError {
		t.Errorf("Expected status %d, got %d", http.StatusInternalServerError, w.Code)
	}
}
