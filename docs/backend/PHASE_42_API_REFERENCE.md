# Wildbound — Phase 42: API Reference

## 1. Base URL & Protocol
- **Base URL:** `http://127.0.0.1:8080/api/v1`
- **Format:** JSON (`application/json; charset=utf-8`)
- **Success Envelope:**
  ```json
  {
    "success": true,
    "data": { ... },
    "error": null
  }
  ```
- **Error Envelope:**
  ```json
  {
    "success": false,
    "data": null,
    "error": {
      "code": "ERROR_CODE_STRING",
      "message": "Human-readable explanation of error"
    }
  }
  ```

---

## 2. Authentication Endpoints

### 2.1 Register
- **Endpoint:** `POST /api/v1/auth/register`
- **Auth:** None (Public)
- **Request Body:**
  ```json
  {
    "username": "hunter_alpha",
    "email": "hunter@wildbound.io",
    "password": "SecurePassword2026!",
    "display_name": "Hunter Alpha"
  }
  ```
- **Response `201 Created`:**
  ```json
  {
    "success": true,
    "data": {
      "user": {
        "id": "e2f074a3-76ef-4613-8ad4-b677a288c5ef",
        "username": "hunter_alpha",
        "email": "hunter@wildbound.io",
        "display_name": "Hunter Alpha",
        "created_at": "2026-11-07T18:00:00Z"
      },
      "tokens": {
        "access_token": "eyJhbGciOi...",
        "token_type": "Bearer",
        "expires_at": "2026-11-07T18:15:00Z",
        "refresh_token": "aW44-...",
        "refresh_expires_at": "2026-11-14T18:00:00Z"
      }
    },
    "error": null
  }
  ```

### 2.2 Login
- **Endpoint:** `POST /api/v1/auth/login`
- **Auth:** None (Public)
- **Request Body:**
  ```json
  {
    "email_or_username": "hunter_alpha",
    "password": "SecurePassword2026!"
  }
  ```
- **Response `200 OK`:** Same payload as Register.

### 2.3 Refresh Tokens
- **Endpoint:** `POST /api/v1/auth/refresh`
- **Auth:** None (Public)
- **Request Body:**
  ```json
  {
    "refresh_token": "aW44-..."
  }
  ```
- **Response `200 OK`:** Returns new rotated `TokenPairResponse`.

### 2.4 Logout
- **Endpoint:** `POST /api/v1/auth/logout`
- **Auth:** Bearer Token
- **Request Body:**
  ```json
  {
    "refresh_token": "aW44-..."
  }
  ```
- **Response `200 OK`:** Revokes active refresh token session in database.

### 2.5 Auth Me
- **Endpoint:** `GET /api/v1/auth/me`
- **Auth:** Bearer Token
- **Response `200 OK`:** Returns authenticated user record.

---

## 3. Player Profile Endpoints

### 3.1 Get Profile (Self)
- **Endpoint:** `GET /api/v1/players/me`
- **Auth:** Bearer Token
- **Response `200 OK`:**
  ```json
  {
    "success": true,
    "data": {
      "id": "893c...",
      "user_id": "e2f0...",
      "display_name": "Hunter Alpha",
      "biography": "Master marksman",
      "avatar_cosmetic_id": "cosmetic_avatar_default",
      "preferred_game_mode": "solo",
      "created_at": "2026-11-07T18:00:00Z",
      "updated_at": "2026-11-07T18:00:00Z"
    },
    "error": null
  }
  ```

### 3.2 Update Profile (Self)
- **Endpoint:** `PATCH /api/v1/players/me`
- **Auth:** Bearer Token
- **Request Body:**
  ```json
  {
    "display_name": "Shadow Ranger",
    "biography": "Master of the Whispering Pines",
    "avatar_cosmetic_id": "cosmetic_avatar_default",
    "preferred_game_mode": "solo_br"
  }
  ```

### 3.3 Public Player Profile
- **Endpoint:** `GET /api/v1/players/{player_id}`
- **Auth:** None (Public)
- **Response `200 OK`:** Sanitized public profile.

---

## 4. Cosmetics & Inventory Endpoints

### 4.1 List Catalog Definitions
- **Endpoint:** `GET /api/v1/cosmetics?category={category}`
- **Auth:** None (Public)
- **Response `200 OK`:** Array of available cosmetic items.

### 4.2 List Owned Inventory
- **Endpoint:** `GET /api/v1/players/me/cosmetics`
- **Auth:** Bearer Token
- **Response `200 OK`:** Array of player-owned items with `is_equipped` status.

### 4.3 Equip Cosmetic
- **Endpoint:** `POST /api/v1/players/me/cosmetics/equip`
- **Auth:** Bearer Token
- **Request Body:**
  ```json
  {
    "cosmetic_id": "cosmetic_hat_adventurer"
  }
  ```
- **Response `200 OK`:** Confirms slot equip and updates profile/inventory.

---

## 5. Match Records & Statistics Endpoints

### 5.1 Player Progression Statistics
- **Endpoint:** `GET /api/v1/players/me/statistics`
- **Auth:** Bearer Token
- **Response `200 OK`:**
  ```json
  {
    "success": true,
    "data": {
      "user_id": "e2f0...",
      "matches_played": 12,
      "matches_won": 4,
      "eliminations": 48,
      "deaths": 8,
      "assists": 15,
      "total_damage": 12850.5,
      "total_damage_received": 3400.0,
      "total_survival_seconds": 10420.0,
      "highest_placement": 1,
      "win_rate": 33.33,
      "kill_death_ratio": 6.0,
      "average_damage": 1070.88,
      "average_survival_seconds": 868.33,
      "updated_at": "2026-11-07T18:30:00Z"
    },
    "error": null
  }
  ```

### 5.2 Player Match History
- **Endpoint:** `GET /api/v1/players/me/matches?page=1&limit=20`
- **Auth:** Bearer Token
- **Response `200 OK`:** Paginated history of matches with placement and score breakdown.

---

## 6. Internal Server Endpoints

### 6.1 Submit Match Results
- **Endpoint:** `POST /api/v1/internal/matches/results`
- **Auth:** Header `X-Server-Token: <token>`
- **Request Body:** `SubmitMatchResultRequest`
- **Response `201 Created` / `200 OK`:** Transactionally records match and updates player statistics with idempotency protection.

### 6.2 Grant Cosmetic
- **Endpoint:** `POST /api/v1/internal/cosmetics/grant`
- **Auth:** Header `X-Server-Token: <token>`
- **Request Body:**
  ```json
  {
    "user_id": "e2f0...",
    "cosmetic_id": "cosmetic_hat_adventurer"
  }
  ```

