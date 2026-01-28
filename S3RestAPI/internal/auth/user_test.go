package auth_test

import (
	"testing"

	"S3RestAPI/internal/auth"
)

func TestHashPassword(t *testing.T) {
	password := "testpassword123"
	hash, err := auth.HashPassword(password)

	if err != nil {
		t.Fatalf("Expected no error, got %v", err)
	}

	if hash == password {
		t.Error("Hash should not equal plaintext password")
	}

	if len(hash) < 20 {
		t.Error("Hash seems too short")
	}
}

func TestCheckPassword(t *testing.T) {
	password := "testpassword123"
	hash, _ := auth.HashPassword(password)

	if !auth.CheckPassword(password, hash) {
		t.Error("Expected password to match hash")
	}

	if auth.CheckPassword("wrongpassword", hash) {
		t.Error("Expected wrong password to not match hash")
	}
}

func TestUserStore_CreateUser(t *testing.T) {
	store := auth.NewUserStore()

	user, err := store.CreateUser("testuser", "password123")
	if err != nil {
		t.Fatalf("Expected no error, got %v", err)
	}

	if user.Username != "testuser" {
		t.Errorf("Expected username 'testuser', got '%s'", user.Username)
	}

	if user.ID == "" {
		t.Error("Expected user ID to be set")
	}
}

func TestUserStore_CreateUser_Duplicate(t *testing.T) {
	store := auth.NewUserStore()

	_, _ = store.CreateUser("testuser", "password123")
	_, err := store.CreateUser("testuser", "password456")

	if err != auth.ErrUserAlreadyExists {
		t.Errorf("Expected ErrUserAlreadyExists, got %v", err)
	}
}

func TestUserStore_GetUser(t *testing.T) {
	store := auth.NewUserStore()

	created, _ := store.CreateUser("testuser", "password123")
	found, err := store.GetUser("testuser")

	if err != nil {
		t.Fatalf("Expected no error, got %v", err)
	}

	if found.ID != created.ID {
		t.Error("Expected to find the same user")
	}
}

func TestUserStore_GetUser_NotFound(t *testing.T) {
	store := auth.NewUserStore()

	_, err := store.GetUser("nonexistent")

	if err != auth.ErrUserNotFound {
		t.Errorf("Expected ErrUserNotFound, got %v", err)
	}
}

func TestUserStore_Authenticate_Success(t *testing.T) {
	store := auth.NewUserStore()

	_, _ = store.CreateUser("testuser", "password123")
	user, err := store.Authenticate("testuser", "password123")

	if err != nil {
		t.Fatalf("Expected no error, got %v", err)
	}

	if user.Username != "testuser" {
		t.Errorf("Expected username 'testuser', got '%s'", user.Username)
	}
}

func TestUserStore_Authenticate_WrongPassword(t *testing.T) {
	store := auth.NewUserStore()

	_, _ = store.CreateUser("testuser", "password123")
	_, err := store.Authenticate("testuser", "wrongpassword")

	if err != auth.ErrInvalidPassword {
		t.Errorf("Expected ErrInvalidPassword, got %v", err)
	}
}

func TestUserStore_Authenticate_UserNotFound(t *testing.T) {
	store := auth.NewUserStore()

	_, err := store.Authenticate("nonexistent", "password")

	if err != auth.ErrUserNotFound {
		t.Errorf("Expected ErrUserNotFound, got %v", err)
	}
}

func TestUserStore_UpdatePassword(t *testing.T) {
	store := auth.NewUserStore()

	_, _ = store.CreateUser("testuser", "oldpassword")
	err := store.UpdatePassword("testuser", "newpassword")

	if err != nil {
		t.Fatalf("Expected no error, got %v", err)
	}

	_, err = store.Authenticate("testuser", "newpassword")
	if err != nil {
		t.Error("Expected authentication with new password to succeed")
	}

	_, err = store.Authenticate("testuser", "oldpassword")
	if err == nil {
		t.Error("Expected authentication with old password to fail")
	}
}

func TestUserStore_DeleteUser(t *testing.T) {
	store := auth.NewUserStore()

	_, _ = store.CreateUser("testuser", "password123")
	err := store.DeleteUser("testuser")

	if err != nil {
		t.Fatalf("Expected no error, got %v", err)
	}

	_, err = store.GetUser("testuser")
	if err != auth.ErrUserNotFound {
		t.Error("Expected user to be deleted")
	}
}

func TestUserStore_ListUsers(t *testing.T) {
	store := auth.NewUserStore()

	store.CreateUser("user1", "pass1")
	store.CreateUser("user2", "pass2")
	store.CreateUser("user3", "pass3")

	users := store.ListUsers()

	if len(users) != 3 {
		t.Errorf("Expected 3 users, got %d", len(users))
	}
}

func TestGetUserStore_DefaultAdmin(t *testing.T) {
	store := auth.GetUserStore()

	user, err := store.Authenticate("admin", "admin")
	if err != nil {
		t.Fatalf("Expected default admin user to exist, got %v", err)
	}

	if user.Username != "admin" {
		t.Errorf("Expected username 'admin', got '%s'", user.Username)
	}
}
