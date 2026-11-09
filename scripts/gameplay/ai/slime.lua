-- ============================================================================
-- WILDBOUND SLIME AI SCRIPT (LUA 5.4)
-- Archetype: Slow, viscous swamp dweller. Splits/retreats at low HP.
-- ============================================================================

local SlimeAI = {}

function SlimeAI.update_enemy(ctx)
    local result = {
        next_state = ctx.state or "IDLE",
        target_id = ctx.target_id or 0,
        move_x = 0.0,
        move_y = 0.0,
        request_attack = false,
        attack_id = ctx.default_attack or "slime_slam"
    }

    -- 1. Dead check
    if ctx.state == "DEAD" or (ctx.hp_percent and ctx.hp_percent <= 0.0) then
        result.next_state = "DEAD"
        return result
    end

    -- 2. Retreat check: low HP condition (splits / retreats if HP <= 25%)
    local retreat_threshold = ctx.retreat_hp or 0.25
    if ctx.can_retreat ~= false and ctx.hp_percent and ctx.hp_percent <= retreat_threshold and ctx.has_target then
        result.next_state = "RETREAT"
        local dx = (ctx.x or 0.0) - (ctx.target_x or 0.0)
        local dy = (ctx.y or 0.0) - (ctx.target_y or 0.0)
        result.move_x = dx
        result.move_y = dy
        return result
    end

    -- 3. Combat Detection & Attack Range
    local detection_radius = ctx.detection_radius or 110.0
    local attack_radius = ctx.attack_radius or 26.0

    if ctx.has_target and ctx.target_dist and ctx.target_dist <= detection_radius then
        if ctx.target_dist <= attack_radius then
            -- Close-range slam attack
            result.next_state = "ATTACK"
            result.move_x = 0.0
            result.move_y = 0.0
            result.request_attack = true
            result.attack_id = ctx.default_attack or "slime_slam"
            return result
        else
            -- Slime oozes towards target
            result.next_state = "CHASE"
            result.move_x = (ctx.target_x or 0.0) - (ctx.x or 0.0)
            result.move_y = (ctx.target_y or 0.0) - (ctx.y or 0.0)
            return result
        end
    end

    -- 4. Investigate last known position if target was recently lost
    if ctx.has_memory and ctx.state == "CHASE" then
        result.next_state = "INVESTIGATE"
        result.move_x = (ctx.last_known_x or ctx.x or 0.0) - (ctx.x or 0.0)
        result.move_y = (ctx.last_known_y or ctx.y or 0.0) - (ctx.y or 0.0)
        return result
    end

    -- 5. Roaming / Leash return to swamp mire spawn point
    local spawn_dx = (ctx.spawn_x or ctx.x or 0.0) - (ctx.x or 0.0)
    local spawn_dy = (ctx.spawn_y or ctx.y or 0.0) - (ctx.y or 0.0)
    local spawn_dist = math.sqrt(spawn_dx * spawn_dx + spawn_dy * spawn_dy)

    if spawn_dist > (ctx.patrol_radius or 90.0) then
        result.next_state = "RETURN"
        result.move_x = spawn_dx
        result.move_y = spawn_dy
    elseif spawn_dist > 25.0 then
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
SlimeAI.update = SlimeAI.update_enemy

-- Register into wildbound engine table
if wildbound == nil then wildbound = {} end
if wildbound.ai == nil then wildbound.ai = {} end
wildbound.ai.slime_update = SlimeAI.update_enemy

return SlimeAI

