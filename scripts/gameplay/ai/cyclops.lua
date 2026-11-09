-- ============================================================================
-- WILDBOUND MOUNTAIN CYCLOPS AI SCRIPT (LUA 5.4)
-- Archetype: Relentless highland brute. Devastating sweeps, NEVER RETREATS.
-- ============================================================================

local CyclopsAI = {}

function CyclopsAI.update_enemy(ctx)
    local result = {
        next_state = ctx.state or "IDLE",
        target_id = ctx.target_id or 0,
        move_x = 0.0,
        move_y = 0.0,
        request_attack = false,
        attack_id = ctx.default_attack or "cyclops_club"
    }

    -- 1. Dead check
    if ctx.state == "DEAD" or (ctx.hp_percent and ctx.hp_percent <= 0.0) then
        result.next_state = "DEAD"
        return result
    end

    -- 2. Fearless Mountain Brute: Cyclops NEVER RETREATS under any circumstance
    -- Even if HP is low (e.g. 1%), the Cyclops ignores retreat requests and presses forward.

    -- 3. Combat Detection & Devastating Attack Reach
    -- Commanding mountain vantage points grant superior detection range (220) and reach (48)
    local detection_radius = ctx.detection_radius or 220.0
    local attack_radius = ctx.attack_radius or 48.0

    if ctx.has_target and ctx.target_dist and ctx.target_dist <= detection_radius then
        if ctx.target_dist <= attack_radius then
            -- Massive sweeping club smash
            result.next_state = "ATTACK"
            result.move_x = 0.0
            result.move_y = 0.0
            result.request_attack = true
            result.attack_id = ctx.default_attack or "cyclops_club"
            return result
        else
            -- Heavy stomping pursuit
            result.next_state = "CHASE"
            result.move_x = (ctx.target_x or 0.0) - (ctx.x or 0.0)
            result.move_y = (ctx.target_y or 0.0) - (ctx.y or 0.0)
            return result
        end
    end

    -- 4. Investigate last known trespasser position
    if ctx.has_memory and ctx.state == "CHASE" then
        result.next_state = "INVESTIGATE"
        result.move_x = (ctx.last_known_x or ctx.x or 0.0) - (ctx.x or 0.0)
        result.move_y = (ctx.last_known_y or ctx.y or 0.0) - (ctx.y or 0.0)
        return result
    end

    -- 5. Mountain territory patrol & Leash return
    local spawn_dx = (ctx.spawn_x or ctx.x or 0.0) - (ctx.x or 0.0)
    local spawn_dy = (ctx.spawn_y or ctx.y or 0.0) - (ctx.y or 0.0)
    local spawn_dist = math.sqrt(spawn_dx * spawn_dx + spawn_dy * spawn_dy)

    if spawn_dist > (ctx.patrol_radius or 140.0) then
        result.next_state = "RETURN"
        result.move_x = spawn_dx
        result.move_y = spawn_dy
    elseif spawn_dist > 40.0 then
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

-- Compatibility aliases
CyclopsAI.update = CyclopsAI.update_enemy

-- Register into wildbound engine table
if wildbound == nil then wildbound = {} end
if wildbound.ai == nil then wildbound.ai = {} end
wildbound.ai.cyclops_update = CyclopsAI.update_enemy

return CyclopsAI

