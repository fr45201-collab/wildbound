package middleware

import (
	"crypto/subtle"
	"net/http"

	"github.com/wildbound/backend/internal/response"
)

func RequireServerAuth(serverSecret string) func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			token := r.Header.Get("X-Server-Token")
			if token == "" {
				response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED_SERVER", "Missing X-Server-Token header")
				return
			}

			if subtle.ConstantTimeCompare([]byte(token), []byte(serverSecret)) != 1 {
				response.RenderError(w, http.StatusForbidden, "FORBIDDEN_SERVER", "Invalid server authentication credentials")
				return
			}

			next.ServeHTTP(w, r)
		})
	}
}

func RequireCoordinatorAuth(coordSecret string) func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			token := r.Header.Get("X-Coordinator-Token")
			if token == "" {
				response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED_COORDINATOR", "Missing X-Coordinator-Token header")
				return
			}

			if subtle.ConstantTimeCompare([]byte(token), []byte(coordSecret)) != 1 {
				response.RenderError(w, http.StatusForbidden, "FORBIDDEN_COORDINATOR", "Invalid coordinator authentication credentials")
				return
			}

			next.ServeHTTP(w, r)
		})
	}
}
