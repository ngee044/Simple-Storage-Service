// internal/transport/mock.go
package transport

import (
	"sync"
)

var (
	mockEnabled    bool
	mockMutex      sync.RWMutex
	mockRedisStore map[string]string
	mockMessages   []interface{}
	mockRabbitErr  error
	mockRedisErr   error
)

func EnableMock() {
	mockMutex.Lock()
	defer mockMutex.Unlock()
	mockEnabled = true
	mockRedisStore = make(map[string]string)
	mockMessages = make([]interface{}, 0)
	mockRabbitErr = nil
	mockRedisErr = nil
}

func DisableMock() {
	mockMutex.Lock()
	defer mockMutex.Unlock()
	mockEnabled = false
	mockRedisStore = nil
	mockMessages = nil
}

func IsMockEnabled() bool {
	mockMutex.RLock()
	defer mockMutex.RUnlock()
	return mockEnabled
}

func SetMockRabbitError(err error) {
	mockMutex.Lock()
	defer mockMutex.Unlock()
	mockRabbitErr = err
}

func SetMockRedisError(err error) {
	mockMutex.Lock()
	defer mockMutex.Unlock()
	mockRedisErr = err
}

func GetMockMessages() []interface{} {
	mockMutex.RLock()
	defer mockMutex.RUnlock()
	return mockMessages
}

func GetMockRedisValue(key string) (string, bool) {
	mockMutex.RLock()
	defer mockMutex.RUnlock()
	val, ok := mockRedisStore[key]
	return val, ok
}

func mockRabbitMQPublish(message interface{}, guid string) error {
	mockMutex.Lock()
	defer mockMutex.Unlock()
	if mockRabbitErr != nil {
		return mockRabbitErr
	}
	mockMessages = append(mockMessages, message)
	return nil
}

func mockRedisSet(key, value string) error {
	mockMutex.Lock()
	defer mockMutex.Unlock()
	if mockRedisErr != nil {
		return mockRedisErr
	}
	mockRedisStore[key] = value
	return nil
}

func mockRedisGet(key string) (string, error) {
	mockMutex.RLock()
	defer mockMutex.RUnlock()
	if mockRedisErr != nil {
		return "", mockRedisErr
	}
	val, ok := mockRedisStore[key]
	if !ok {
		return "", nil
	}
	return val, nil
}

func mockRedisPing() error {
	mockMutex.RLock()
	defer mockMutex.RUnlock()
	return mockRedisErr
}

func mockRabbitPing() error {
	mockMutex.RLock()
	defer mockMutex.RUnlock()
	return mockRabbitErr
}
