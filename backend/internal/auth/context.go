package auth

type ContextKey string

const (
	ContextKeyUserID   ContextKey = "user_id"
	ContextKeyUsername ContextKey = "username"
)
