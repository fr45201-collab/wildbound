package auth

import "time"

type RegisterRequest struct {
	Username         string `json:"username"`
	Email            string `json:"email"`
	Password         string `json:"password"`
	DisplayName      string `json:"display_name,omitempty"`
	DeviceIdentifier string `json:"device_identifier,omitempty"`
	DeviceName       string `json:"device_name,omitempty"`
}

type LoginRequest struct {
	EmailOrUsername  string `json:"email_or_username"`
	Password         string `json:"password"`
	DeviceIdentifier string `json:"device_identifier,omitempty"`
	DeviceName       string `json:"device_name,omitempty"`
}

type RefreshRequest struct {
	RefreshToken string `json:"refresh_token"`
}

type TokenPairResponse struct {
	AccessToken      string    `json:"access_token"`
	TokenType        string    `json:"token_type"`
	ExpiresAt        time.Time `json:"expires_at"`
	RefreshToken     string    `json:"refresh_token"`
	RefreshExpiresAt time.Time `json:"refresh_expires_at"`
}

type AuthUserResponse struct {
	ID          string    `json:"id"`
	Username    string    `json:"username"`
	Email       string    `json:"email"`
	DisplayName string    `json:"display_name"`
	CreatedAt   time.Time `json:"created_at"`
}

type AuthResponse struct {
	User   AuthUserResponse  `json:"user"`
	Tokens TokenPairResponse `json:"tokens"`
}
