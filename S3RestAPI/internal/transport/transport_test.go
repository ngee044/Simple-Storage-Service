package transport_test

import (
	"testing"

	"S3RestAPI/internal/transport"
)

func TestMockEnable(t *testing.T) {
	transport.EnableMock()
	defer transport.DisableMock()

	if !transport.IsMockEnabled() {
		t.Error("Expected mock to be enabled")
	}
}

func TestMockDisable(t *testing.T) {
	transport.EnableMock()
	transport.DisableMock()

	if transport.IsMockEnabled() {
		t.Error("Expected mock to be disabled")
	}
}

func TestMockRedisSetGet(t *testing.T) {
	transport.EnableMock()
	defer transport.DisableMock()

	err := transport.RedisDataSet("test-key", "test-value")
	if err != nil {
		t.Errorf("Expected no error, got %v", err)
	}

	val, err := transport.RedisDataGet("test-key")
	if err != nil {
		t.Errorf("Expected no error, got %v", err)
	}

	if val != "test-value" {
		t.Errorf("Expected 'test-value', got '%s'", val)
	}
}

func TestMockRedisPing(t *testing.T) {
	transport.EnableMock()
	defer transport.DisableMock()

	err := transport.RedisPing()
	if err != nil {
		t.Errorf("Expected no error, got %v", err)
	}
}

func TestMockRabbitMQPing(t *testing.T) {
	transport.EnableMock()
	defer transport.DisableMock()

	err := transport.RabbitMQPing()
	if err != nil {
		t.Errorf("Expected no error, got %v", err)
	}
}

func TestMockRabbitMQPublish(t *testing.T) {
	transport.EnableMock()
	defer transport.DisableMock()

	msg := map[string]string{"action": "test", "data": "test-data"}
	err := transport.RabbitMQPublishMessage(msg, "test-guid")
	if err != nil {
		t.Errorf("Expected no error, got %v", err)
	}

	messages := transport.GetMockMessages()
	if len(messages) != 1 {
		t.Errorf("Expected 1 message, got %d", len(messages))
	}
}

func TestMockRedisError(t *testing.T) {
	transport.EnableMock()
	defer transport.DisableMock()

	transport.SetMockRedisError(nil)

	err := transport.RedisPing()
	if err != nil {
		t.Errorf("Expected no error, got %v", err)
	}
}

func TestGetMockRedisValue(t *testing.T) {
	transport.EnableMock()
	defer transport.DisableMock()

	_ = transport.RedisDataSet("key1", "value1")

	val, ok := transport.GetMockRedisValue("key1")
	if !ok {
		t.Error("Expected key to exist")
	}
	if val != "value1" {
		t.Errorf("Expected 'value1', got '%s'", val)
	}

	_, ok = transport.GetMockRedisValue("nonexistent")
	if ok {
		t.Error("Expected key to not exist")
	}
}
