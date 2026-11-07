package cosmetics

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

func (h *Handler) getUserID(r *http.Request) (uuid.UUID, error) {
	uid := middleware.GetUserID(r.Context())
	if uid == uuid.Nil {
		return uuid.Nil, errors.New("missing or invalid user context")
	}
	return uid, nil
}

// ListCatalogue handles GET /api/v1/cosmetics
func (h *Handler) ListCatalogue(w http.ResponseWriter, r *http.Request) {
	q := r.URL.Query()
	category := q.Get("category")
	slot := q.Get("slot")
	rarity := q.Get("rarity")
	search := q.Get("search")

	limit := 50
	if l := q.Get("limit"); l != "" {
		if parsed, err := strconv.Atoi(l); err == nil && parsed > 0 {
			limit = parsed
		}
	}

	offset := 0
	if o := q.Get("offset"); o != "" {
		if parsed, err := strconv.Atoi(o); err == nil && parsed >= 0 {
			offset = parsed
		}
	}

	res, err := h.service.ListCatalogue(r.Context(), category, slot, rarity, search, limit, offset)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve cosmetic catalogue")
		return
	}

	response.RenderSuccess(w, http.StatusOK, res)
}

// ListPlayerCosmetics handles GET /api/v1/players/me/cosmetics
func (h *Handler) ListPlayerCosmetics(w http.ResponseWriter, r *http.Request) {
	userID, err := h.getUserID(r)
	if err != nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Valid authentication token required")
		return
	}

	items, err := h.service.ListPlayerCosmetics(r.Context(), userID)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve player cosmetics")
		return
	}

	response.RenderSuccess(w, http.StatusOK, items)
}

// GetMyLoadout handles GET /api/v1/players/me/loadout
func (h *Handler) GetMyLoadout(w http.ResponseWriter, r *http.Request) {
	userID, err := h.getUserID(r)
	if err != nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Valid authentication token required")
		return
	}

	loadout, err := h.service.GetPlayerLoadout(r.Context(), userID)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve player loadout")
		return
	}

	response.RenderSuccess(w, http.StatusOK, loadout)
}

// EquipItem handles POST /api/v1/players/me/loadout/equip
func (h *Handler) EquipItem(w http.ResponseWriter, r *http.Request) {
	userID, err := h.getUserID(r)
	if err != nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Valid authentication token required")
		return
	}

	var req EquipSlotRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_REQUEST", "Invalid JSON request body")
		return
	}

	loadout, err := h.service.EquipItem(r.Context(), userID, req.CosmeticID, req.ExpectedVersion)
	if err != nil {
		if errors.Is(err, ErrCosmeticNotFound) {
			response.RenderError(w, http.StatusNotFound, "COSMETIC_NOT_FOUND", "Cosmetic does not exist")
			return
		}
		if errors.Is(err, ErrCosmeticNotOwned) {
			response.RenderError(w, http.StatusForbidden, "COSMETIC_NOT_OWNED", "Player does not own this cosmetic item")
			return
		}
		if errors.Is(err, ErrCosmeticUnavailable) {
			response.RenderError(w, http.StatusBadRequest, "COSMETIC_UNAVAILABLE", "Cosmetic is not currently available for equip")
			return
		}
		if errors.Is(err, ErrLoadoutVersionConflict) {
			response.RenderError(w, http.StatusConflict, "LOADOUT_VERSION_CONFLICT", "Stale loadout version detected")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to equip cosmetic: "+err.Error())
		return
	}

	response.RenderSuccess(w, http.StatusOK, loadout)
}

// UnequipSlot handles POST /api/v1/players/me/loadout/unequip
func (h *Handler) UnequipSlot(w http.ResponseWriter, r *http.Request) {
	userID, err := h.getUserID(r)
	if err != nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Valid authentication token required")
		return
	}

	var req UnequipSlotRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_REQUEST", "Invalid JSON request body")
		return
	}

	loadout, err := h.service.UnequipSlot(r.Context(), userID, req.Slot, req.ExpectedVersion)
	if err != nil {
		if errors.Is(err, ErrLoadoutVersionConflict) {
			response.RenderError(w, http.StatusConflict, "LOADOUT_VERSION_CONFLICT", "Stale loadout version detected")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to unequip slot: "+err.Error())
		return
	}

	response.RenderSuccess(w, http.StatusOK, loadout)
}

// UpdateMyLoadout handles PUT /api/v1/players/me/loadout
func (h *Handler) UpdateMyLoadout(w http.ResponseWriter, r *http.Request) {
	userID, err := h.getUserID(r)
	if err != nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Valid authentication token required")
		return
	}

	var req UpdateLoadoutRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_REQUEST", "Invalid JSON request body")
		return
	}

	loadout, err := h.service.UpdateFullLoadout(r.Context(), userID, req.Cosmetics, req.ExpectedVersion)
	if err != nil {
		if errors.Is(err, ErrCosmeticNotFound) {
			response.RenderError(w, http.StatusNotFound, "COSMETIC_NOT_FOUND", "One or more cosmetics do not exist")
			return
		}
		if errors.Is(err, ErrCosmeticNotOwned) {
			response.RenderError(w, http.StatusForbidden, "COSMETIC_NOT_OWNED", "Player does not own all specified cosmetics")
			return
		}
		if errors.Is(err, ErrLoadoutVersionConflict) {
			response.RenderError(w, http.StatusConflict, "LOADOUT_VERSION_CONFLICT", "Stale loadout version detected")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to update loadout: "+err.Error())
		return
	}

	response.RenderSuccess(w, http.StatusOK, loadout)
}

// GetServerVerifiedLoadout handles GET /api/v1/internal/players/{user_id}/loadout (server-authenticated)
func (h *Handler) GetServerVerifiedLoadout(w http.ResponseWriter, r *http.Request) {
	targetIDStr := chi.URLParam(r, "user_id")
	targetID, err := uuid.Parse(targetIDStr)
	if err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_USER_ID", "Target user ID must be a valid UUID")
		return
	}

	res, err := h.service.GetServerVerifiedLoadout(r.Context(), targetID)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to verify server loadout: "+err.Error())
		return
	}

	response.RenderSuccess(w, http.StatusOK, res)
}

// GrantCosmetic handles POST /api/v1/internal/cosmetics/grant (server-authenticated)
func (h *Handler) GrantCosmetic(w http.ResponseWriter, r *http.Request) {
	var req GrantCosmeticRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_REQUEST", "Invalid JSON request body")
		return
	}

	if err := h.service.GrantCosmetic(r.Context(), req.UserID, req.CosmeticID, req.Source); err != nil {
		if errors.Is(err, ErrCosmeticNotFound) {
			response.RenderError(w, http.StatusNotFound, "COSMETIC_NOT_FOUND", "Cosmetic does not exist")
			return
		}
		if errors.Is(err, ErrCosmeticUnavailable) {
			response.RenderError(w, http.StatusBadRequest, "COSMETIC_UNAVAILABLE", "Cosmetic is not available for grant")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to grant cosmetic: "+err.Error())
		return
	}

	response.RenderSuccess(w, http.StatusOK, map[string]interface{}{
		"granted":     true,
		"user_id":     req.UserID,
		"cosmetic_id": req.CosmeticID,
	})
}
