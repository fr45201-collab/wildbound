package matches

import (
	"time"

	"github.com/google/uuid"
)

type MatchParticipantPayload struct {
	UserID                 *uuid.UUID `json:"user_id,omitempty"`
	ParticipantDisplayName string     `json:"participant_display_name"`
	TeamID                 int        `json:"team_id"`
	Placement              int        `json:"placement"`
	Eliminations           int        `json:"eliminations"`
	Deaths                 int        `json:"deaths"`
	Assists                int        `json:"assists"`
	DamageDealt            float64    `json:"damage_dealt"`
	DamageReceived         float64    `json:"damage_received"`
	SurvivalSeconds        float64    `json:"survival_seconds"`
	ResultStatus           string     `json:"result_status"`
}

type SubmitMatchResultRequest struct {
	MatchID           string                    `json:"match_id"`
	GameMode          string                    `json:"game_mode"`
	MapID             string                    `json:"map_id"`
	StartedAt         time.Time                 `json:"started_at"`
	EndedAt           time.Time                 `json:"ended_at"`
	DurationSeconds   int                       `json:"duration_seconds"`
	WinnerUserID      *uuid.UUID                `json:"winner_user_id,omitempty"`
	WinnerDisplayName string                    `json:"winner_display_name,omitempty"`
	ResultStatus      string                    `json:"result_status,omitempty"`
	Participants      []MatchParticipantPayload `json:"participants"`
}

type MatchRecord struct {
	ID                uuid.UUID                 `json:"id"`
	MatchID           string                    `json:"match_id"`
	GameMode          string                    `json:"game_mode"`
	MapID             string                    `json:"map_id"`
	StartedAt         time.Time                 `json:"started_at"`
	EndedAt           time.Time                 `json:"ended_at"`
	DurationSeconds   int                       `json:"duration_seconds"`
	ParticipantCount  int                       `json:"participant_count"`
	WinnerUserID      *uuid.UUID                `json:"winner_user_id,omitempty"`
	WinnerDisplayName string                    `json:"winner_display_name"`
	ResultStatus      string                    `json:"result_status"`
	CreatedAt         time.Time                 `json:"created_at"`
	Participants      []MatchParticipantPayload `json:"participants,omitempty"`
}

type PlayerMatchSummary struct {
	MatchRecordID          uuid.UUID `json:"match_record_id"`
	MatchID                string    `json:"match_id"`
	GameMode               string    `json:"game_mode"`
	DurationSeconds        int       `json:"duration_seconds"`
	EndedAt                time.Time `json:"ended_at"`
	Placement              int       `json:"placement"`
	ParticipantCount       int       `json:"participant_count"`
	Eliminations           int       `json:"eliminations"`
	Deaths                 int       `json:"deaths"`
	Assists                int       `json:"assists"`
	DamageDealt            float64   `json:"damage_dealt"`
	SurvivalSeconds        float64   `json:"survival_seconds"`
	ParticipantResultStatus string   `json:"participant_result_status"`
}

type PaginatedMatchesResponse struct {
	Matches    []PlayerMatchSummary `json:"matches"`
	TotalCount int                  `json:"total_count"`
	Page       int                  `json:"page"`
	Limit      int                  `json:"limit"`
	TotalPages int                  `json:"total_pages"`
}
