package validation

import (
	"errors"
	"net/mail"
	"regexp"
	"strings"
	"unicode"
)

var (
	ErrInvalidEmail       = errors.New("invalid email address format")
	ErrInvalidUsername    = errors.New("username must be 3-32 characters (alphanumeric, underscores, hyphens only)")
	ErrPasswordTooShort   = errors.New("password must be at least 8 characters long")
	ErrPasswordComplexity = errors.New("password must contain at least one letter and one number")
	ErrInvalidDisplayName = errors.New("display name must be between 2 and 64 characters")
)

var usernameRegex = regexp.MustCompile(`^[a-zA-Z0-9_-]{3,32}$`)

func ValidateEmail(email string) (string, error) {
	trimmed := strings.TrimSpace(email)
	if trimmed == "" {
		return "", ErrInvalidEmail
	}
	addr, err := mail.ParseAddress(trimmed)
	if err != nil {
		return "", ErrInvalidEmail
	}
	// Normalize to lowercase
	return strings.ToLower(addr.Address), nil
}

func ValidateUsername(username string) (string, error) {
	trimmed := strings.TrimSpace(username)
	if !usernameRegex.MatchString(trimmed) {
		return "", ErrInvalidUsername
	}
	return trimmed, nil
}

func ValidatePassword(password string) error {
	if len(password) < 8 {
		return ErrPasswordTooShort
	}
	var hasLetter, hasDigit bool
	for _, c := range password {
		if unicode.IsLetter(c) {
			hasLetter = true
		} else if unicode.IsDigit(c) {
			hasDigit = true
		}
	}
	if !hasLetter || !hasDigit {
		return ErrPasswordComplexity
	}
	return nil
}

func ValidateDisplayName(displayName string) (string, error) {
	trimmed := strings.TrimSpace(displayName)
	if len(trimmed) < 2 || len(trimmed) > 64 {
		return "", ErrInvalidDisplayName
	}
	return trimmed, nil
}

func ClampPagination(page, limit int) (int, int) {
	if page < 1 {
		page = 1
	}
	if limit < 1 {
		limit = 20
	} else if limit > 100 {
		limit = 100
	}
	return page, limit
}

