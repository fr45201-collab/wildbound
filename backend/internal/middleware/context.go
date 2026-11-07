package middleware

import (
	"context"

	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/auth"
)

func GetUserID(ctx context.Context) uuid.UUID {
	val := ctx.Value(auth.ContextKeyUserID)
	if val == nil {
		return uuid.Nil
	}
	if u, ok := val.(uuid.UUID); ok {
		return u
	}
	if s, ok := val.(string); ok {
		if parsed, err := uuid.Parse(s); err == nil {
			return parsed
		}
	}
	return uuid.Nil
}

func GetUsername(ctx context.Context) string {
	val := ctx.Value(auth.ContextKeyUsername)
	if val == nil {
		return ""
	}
	if s, ok := val.(string); ok {
		return s
	}
	return ""
}
