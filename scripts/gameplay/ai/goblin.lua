-- ============================================================================
-- WILDBOUND GOBLIN AI SCRIPT (LUA 5.4)
-- Archetype: Cowardly, fast skirmisher. Retreats on low HP.
-- ============================================================================

local GoblinAI = {}

function GoblinAI.update_enemy(ctx)
    local result = {
        next_state = ctx.state or "IDLE",
        target_id = ctx.target_id or 0,
        move_x = 0.0,
        move_y = 0.0,
        request_attack = false,
        attack_id = "goblin_club"
    }

    -- 1. Dead check
    if ctx.state == "DEAD" or (ctx.hp_percent and ctx.hp_percent <= 0.0) then
        result.next_state = "DEAD"
        return result
    end

    -- 2. Cowardly Retreat: if HP <= 20% (or retreat_hp), flee away from target
    local retreat_threshold = ctx.retreat_hp or 0.2
    if ctx.can_retreat ~= false and ctx.hp_percent and ctx.hp_percent <= retreat_threshold and ctx.has_target then
        result.next_state = "RETREAT"
        -- Flee away from target with evasive jitter
        local dx = ctx.x - ctx.target_x
        local dy = ctx.y - ctx.target_y
        result.move_x = dx
        result.move_y = dy
        return result
    end

    -- 3. Target Detection & Attack Range
    local detection_radius = ctx.detection_radius or 140.0
    local attack_radius = ctx.attack_radius or 32.0

    if ctx.has_target and ctx.target_dist and ctx.target_dist <= detection_radius then
        if ctx.target_dist <= attack_radius then
            -- Strike with club
            result.next_state = "ATTACK"
            result.move_x = 0.0
            result.move_y = 0.0
            result.request_attack = true
            result.attack_id = "goblin_club"
            return result
        else
            -- Chase down player
            result.next_state = "CHASE"
            result.move_x = ctx.target_x - ctx.x
            result.move_y = ctx.target_y - ctx.y
            return result
        end
    end

    -- 4. Investigate last known position if target was recently lost
    if ctx.has_memory and ctx.state == "CHASE" then
        result.next_state = "INVESTIGATE"
        result.move_x = (ctx.last_known_x or ctx.x) - ctx.x
        result.move_y = (ctx.last_known_y or ctx.y) - ctx.y
        return result
    end

    -- 5. Roaming / Patrol near spawn
    local spawn_dx = (ctx.spawn_x or ctx.x) - ctx.x
    local spawn_dy = (ctx.spawn_y or ctx.y) - ctx.y
    local spawn_dist = math.sqrt(spawn_dx * spawn_dx + spawn_dy * spawn_dy)

    if spawn_dist > (ctx.patrol_radius or 80.0) then
        result.next_state = "RETURN"
        result.move_x = spawn_dx
        result.move_y = spawn_dy
    elseif spawn_dist > 30.0 then
        result.next_state = "PATROL"
        result.move_x = spawn_dx
        result.move_y = spawn_dy
    else
        result.next_state = "IDLE"
        result.move_x = 0.0
        result.move_y = 0.0
    end

    return result
end

-- Compatibility alias
GoblinAI.update = GoblinAI.update_enemy

-- Register into wildbound engine table
if wildbound == nil then wildbound = {} end
if wildbound.ai == nil then wildbound.ai = {} end
wildbound.ai.goblin_update = GoblinAI.update_enemy

return GoblinAI

