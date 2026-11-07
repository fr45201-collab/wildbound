package middleware

import (
	"net"
	"net/http"
	"strings"
	"sync"
	"time"

	"github.com/wildbound/backend/internal/response"
)

type clientBucket struct {
	tokens     float64
	lastRefill time.Time
}

type RateLimiter struct {
	mu         sync.Mutex
	clients    map[string]*clientBucket
	rate       float64 // tokens per second
	capacity   float64 // burst capacity
	enabled    bool
	stopChan   chan struct{}
}

func NewRateLimiter(ratePerSecond int, burst int, enabled bool) *RateLimiter {
	rl := &RateLimiter{
		clients:   make(map[string]*clientBucket),
		rate:      float64(ratePerSecond),
		capacity:  float64(burst),
		enabled:   enabled,
		stopChan:  make(chan struct{}),
	}

	if enabled {
		go rl.cleanupLoop()
	}

	return rl
}

func (rl *RateLimiter) Stop() {
	if rl.enabled {
		close(rl.stopChan)
	}
}

func (rl *RateLimiter) cleanupLoop() {
	ticker := time.NewTicker(5 * time.Minute)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			rl.mu.Lock()
			cutoff := time.Now().Add(-10 * time.Minute)
			for ip, bucket := range rl.clients {
				if bucket.lastRefill.Before(cutoff) {
					delete(rl.clients, ip)
				}
			}
			rl.mu.Unlock()
		case <-rl.stopChan:
			return
		}
	}
}

func extractIP(r *http.Request) string {
	if xff := r.Header.Get("X-Forwarded-For"); xff != "" {
		parts := strings.Split(xff, ",")
		return strings.TrimSpace(parts[0])
	}
	host, _, err := net.SplitHostPort(r.RemoteAddr)
	if err != nil {
		return r.RemoteAddr
	}
	return host
}

func (rl *RateLimiter) Allow(ip string) bool {
	if !rl.enabled {
		return true
	}

	rl.mu.Lock()
	defer rl.mu.Unlock()

	now := time.Now()
	bucket, exists := rl.clients[ip]
	if !exists {
		rl.clients[ip] = &clientBucket{
			tokens:     rl.capacity - 1.0,
			lastRefill: now,
		}
		return true
	}

	// Refill tokens
	elapsed := now.Sub(bucket.lastRefill).Seconds()
	bucket.tokens += elapsed * rl.rate
	if bucket.tokens > rl.capacity {
		bucket.tokens = rl.capacity
	}
	bucket.lastRefill = now

	if bucket.tokens >= 1.0 {
		bucket.tokens -= 1.0
		return true
	}

	return false
}

func (rl *RateLimiter) Middleware() func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			if !rl.enabled {
				next.ServeHTTP(w, r)
				return
			}

			ip := extractIP(r)
			if !rl.Allow(ip) {
				response.RenderError(w, http.StatusTooManyRequests, "RATE_LIMIT_EXCEEDED", "Too many requests. Please slow down.")
				return
			}

			next.ServeHTTP(w, r)
		})
	}
}
