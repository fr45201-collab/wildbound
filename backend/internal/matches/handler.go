package matches

import (
	"encoding/json"
	"errors"
	"net/http"
	"strconv"

	"github.com/go-chi/chi/v5"
	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/middleware"
	"github.com/wildbound/backend/internal/response"
)

type Handler struct {
	service *Service
}

func NewHandler(service *Service) *Handler {
	return &Handler{service: service}
}

func (h *Handler) SubmitResult(w http.ResponseWriter, r *http.Request) {
	var req SubmitMatchResultRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_REQUEST", "Invalid JSON request body")
		return
	}

	record, wasCreated, err := h.service.SubmitMatchResult(r.Context(), &req)
	if err != nil {
		if errors.Is(err, ErrInvalidMatchData) {
			response.RenderError(w, http.StatusBadRequest, "INVALID_MATCH_DATA", err.Error())
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to process match result")
		return
	}

	statusCode := http.StatusCreated
	if !wasCreated {
		statusCode = http.StatusOK // Idempotent response
	}

	response.RenderSuccess(w, statusCode, map[string]interface{}{
		"created": wasCreated,
		"match":   record,
	})
}

func (h *Handler) GetMatch(w http.ResponseWriter, r *http.Request) {
	idStr := chi.URLParam(r, "match_id")
	if idStr == "" {
		response.RenderError(w, http.StatusBadRequest, "INVALID_ID", "Match ID required")
		return
	}

	record, err := h.service.GetMatch(r.Context(), idStr)
	if err != nil {
		if errors.Is(err, ErrMatchNotFound) {
			response.RenderError(w, http.StatusNotFound, "MATCH_NOT_FOUND", "Match record not found")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve match")
		return
	}

	response.RenderSuccess(w, http.StatusOK, record)
}

func (h *Handler) GetMyMatches(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	page, _ := strconv.Atoi(r.URL.Query().Get("page"))
	limit, _ := strconv.Atoi(r.URL.Query().Get("limit"))

	res, err := h.service.ListPlayerMatches(r.Context(), userID, page, limit)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve match history")
		return
	}

	response.RenderSuccess(w, http.StatusOK, res)
}

func (h *Handler) GetPlayerMatches(w http.ResponseWriter, r *http.Request) {
	idStr := chi.URLParam(r, "player_id")
	playerID, err := uuid.Parse(idStr)
	if err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_ID", "Invalid player ID format")
		return
	}

	page, _ := strconv.Atoi(r.URL.Query().Get("page"))
	limit, _ := strconv.Atoi(r.URL.Query().Get("limit"))

	res, err := h.service.ListPlayerMatches(r.Context(), playerID, page, limit)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve match history")
		return
	}

	response.RenderSuccess(w, http.StatusOK, res)
}
