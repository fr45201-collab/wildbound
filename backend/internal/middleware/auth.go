package middleware

import (
	"context"
	"net/http"
	"strings"

	"github.com/wildbound/backend/internal/auth"
	"github.com/wildbound/backend/internal/response"
)

func RequireAuth(tm *auth.TokenManager) func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			authHeader := r.Header.Get("Authorization")
			if authHeader == "" {
				response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Missing Authorization header")
				return
			}

			parts := strings.SplitN(authHeader, " ", 2)
			if len(parts) != 2 || !strings.EqualFold(parts[0], "Bearer") {
				response.RenderError(w, http.StatusUnauthorized, "INVALID_TOKEN_FORMAT", "Authorization header must be 'Bearer <token>'")
				return
			}

			claims, err := tm.ValidateAccessToken(parts[1])
			if err != nil {
				response.RenderError(w, http.StatusUnauthorized, "INVALID_OR_EXPIRED_TOKEN", err.Error())
				return
			}

			ctx := context.WithValue(r.Context(), auth.ContextKeyUserID, claims.UserID)
			ctx = context.WithValue(ctx, auth.ContextKeyUsername, claims.Username)
			next.ServeHTTP(w, r.WithContext(ctx))
		})
	}
}

func Authenticate(tm *auth.TokenManager) func(http.Handler) http.Handler {
	return RequireAuth(tm)
}
