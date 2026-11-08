# Wildbound — Server-Side Gameplay Validation

**Phase:** 47  
**Status:** Completed  
**Subsystems:** ServerSecurityManager, MatchManager, CombatSystem, World  

---

## 1. Authoritative Gameplay Validation Principles

Gameplay integrity requires that the server independently checks every player action against:
1. Physical boundaries and spatial limits.
2. Character capability and state (speed, cooldowns, equipment, inventory).
3. Chronological validity and rate limits.
4. Concurrency and transactional uniqueness (single-claim items).

---

## 2. Movement Validation Algorithm

Located in `ServerSecurityManager::ValidateMovement()`:

```cpp
bool ServerSecurityManager::ValidateMovement(uint16_t clientId, uint32_t participantId,
                                            Vector2 currentPos, Vector2 requestedPos, float dt,
                                            float maxAllowedSpeed, Vector2& outValidatedPos)
```

### Steps:
1. **Numerical Sanity**: Verify `std::isfinite(requestedPos.x)` and `std::isfinite(requestedPos.y)`. If non-finite, snap to `currentPos`, log `SecurityEventType::NonFiniteCoordinates`, return `false`.
2. **Effective Delta Time Clamping**: Clamp `dt` to $[0.001\text{ s}, 0.25\text{ s}]$ to prevent simulation pause exploits.
3. **Displacement Distance**: $\Delta x = x_{\text{req}} - x_{\text{cur}}$, $\Delta y = y_{\text{req}} - y_{\text{cur}}$, $d = \sqrt{\Delta x^2 + \Delta y^2}$.
4. **Allowed Distance Boundary**:
   $$d_{\text{max}} = (\text{maxAllowedSpeed} \times \text{effectiveDt} \times 1.35) + 8.0\text{ px}$$
   *(35% margin for network jitter + 8px position quantization)*.
5. **Teleport Anomaly Detection**:
   - If $d > 3.5 \times d_{\text{max}}$, classify as `TeleportAttempt`.
   - Snap `outValidatedPos` back to `currentPos`.
   - Log `Critical` security event.
6. **Speed Clamping**:
   - If $d_{\text{max}} < d \le 3.5 \times d_{\text{max}}$, classify as `InvalidMovementSpeed`.
   - Scale movement vector: $\vec{v}_{\text{clamped}} = \vec{v}_{\text{cur}} + \frac{d_{\text{max}}}{d} \Delta\vec{p}$.
   - Store clamped position in `outValidatedPos`.
   - Log `Warning` security event.
7. **Nominal Case**: If $d \le d_{\text{max}}$, accept position `outValidatedPos = requestedPos`.

---

## 3. Combat Action Validation Algorithm

Located in `ServerSecurityManager::ValidateAttack()`:

```cpp
bool ServerSecurityManager::ValidateAttack(uint16_t clientId, uint32_t participantId,
                                          const std::string& weaponId, Vector2 attackerPos, Vector2 targetPos,
                                          float maxWeaponRange, bool isWeaponReady)
```

### Checks:
1. **Weapon Readiness**: If `!isWeaponReady`, reject with `AttackCooldownViolation`.
2. **Consecutive Attack Rate**: If server-recorded timestamp shows elapsed time $< 150\text{ ms}$ since last attack from this participant, reject with `AttackCooldownViolation`.
3. **Finite Coordinates**: Verify attacker and target positions are finite.
4. **Max Weapon Range Boundary**:
   $$\text{Range}_{\text{max}} = (\text{maxWeaponRange} \times 1.25) + 12.0\text{ px}$$
   - If distance exceeds $\text{Range}_{\text{max}}$, reject with `AttackRangeViolation` (`SecuritySeverity::Violation`).
5. **Success**: Update participant's last attack timestamp and return `true`.

---

## 4. Atomic Loot Pickup Transaction Resolution

Located in `ServerSecurityManager::ValidateLootPickup()`:

```cpp
bool ServerSecurityManager::ValidateLootPickup(uint16_t clientId, uint32_t participantId,
                                              uint64_t lootId, Vector2 playerPos, Vector2 lootPos,
                                              float maxPickupRadius)
```

### Concurrency & Distance Guarantees:
1. **Atomic Resolution (`m_claimedLootIds`)**:
   - Checks `m_claimedLootIds.find(lootId)`.
   - If found, loot was already collected by another player or prior tick!
   - Immediately reject second claimant with `SecurityEventType::DuplicateLootClaim`.
2. **Physical Reach Validation**:
   - Distance between player position and loot entity position must satisfy:
     $$d \le (\text{maxPickupRadius} \times 1.30) + 16.0\text{ px}$$
   - Prevents "vacuum loot" exploits where clients collect items across the island map.
3. **Commitment**:
   - If within reach and unclaimed, `m_claimedLootIds.insert(lootId)` atomically locks ownership, and `WorldItem::MarkCollected()` removes it from world simulation.

