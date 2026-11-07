package validation

import (
	"testing"
)

func TestValidateEmail(t *testing.T) {
	validEmails := []string{
		"player@example.com",
		"hunter.wild@bound.io",
		"TEST@Domain.COM",
		"user+tag@domain.co.uk",
	}

	for _, email := range validEmails {
		norm, err := ValidateEmail(email)
		if err != nil {
			t.Errorf("expected email %q to be valid, got error: %v", email, err)
		}
		if norm == "" {
			t.Errorf("expected normalized email for %q to not be empty", email)
		}
	}

	invalidEmails := []string{
		"",
		"plainaddress",
		"@missingusername.com",
		"username@.com",
		"spaces in@email.com",
		"toolong@" + string(make([]byte, 260)) + ".com",
	}

	for _, email := range invalidEmails {
		_, err := ValidateEmail(email)
		if err == nil {
			t.Errorf("expected email %q to be invalid, but got nil error", email)
		}
	}
}

func TestValidateUsername(t *testing.T) {
	validUsernames := []string{
		"wild_hunter",
		"Player123",
		"shadow_bow",
		"player-name",
		"abc_123_xyz",
	}

	for _, u := range validUsernames {
		norm, err := ValidateUsername(u)
		if err != nil {
			t.Errorf("expected username %q to be valid, got: %v", u, err)
		}
		if norm == "" {
			t.Errorf("expected non-empty normalized username for %q", u)
		}
	}

	invalidUsernames := []string{
		"ab",                      // too short (<3)
		"very_long_username_that_exceeds_32_characters_limit", // too long (>32)
		"hunter@wild",             // invalid char '@'
		"player#name",             // invalid char '#'
		"name with spaces",
		"",
	}

	for _, u := range invalidUsernames {
		_, err := ValidateUsername(u)
		if err == nil {
			t.Errorf("expected username %q to be invalid, but got nil error", u)
		}
	}
}

func TestValidatePassword(t *testing.T) {
	validPasswords := []string{
		"Hunter1234!",
		"SecureP4ssword",
		"wildbound2026",
	}

	for _, p := range validPasswords {
		if err := ValidatePassword(p); err != nil {
			t.Errorf("expected password to be valid, got: %v", err)
		}
	}

	invalidPasswords := []string{
		"short1",         // < 8 chars
		"alllettersonly", // no digits
		"1234567890",     // no letters
		"",
	}

	for _, p := range invalidPasswords {
		if err := ValidatePassword(p); err == nil {
			t.Errorf("expected password %q to be invalid, but got nil error", p)
		}
	}
}

func TestValidateDisplayName(t *testing.T) {
	validNames := []string{
		"WildHunter",
		"Ghost Arrow",
		"Player 007",
	}

	for _, name := range validNames {
		norm, err := ValidateDisplayName(name)
		if err != nil {
			t.Errorf("expected display name %q to be valid, got: %v", name, err)
		}
		if norm == "" {
			t.Errorf("expected non-empty display name for %q", name)
		}
	}

	invalidNames := []string{
		"a", // too short (<2)
		"this display name is way too long to be allowed because it is more than 32 characters",
		"   ", // empty after trim
	}

	for _, name := range invalidNames {
		_, err := ValidateDisplayName(name)
		if err == nil {
			t.Errorf("expected display name %q to be invalid, but got nil error", name)
		}
	}
}
