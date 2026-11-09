-- ============================================================================
-- WILDBOUND MARSH VIPER / SNAKE AI SCRIPT (LUA 5.4)
-- Archetype: Agile ambush predator. Fast strikes, evasive retreat on injury.
-- ============================================================================

local SnakeAI = {}

function SnakeAI.update_enemy(ctx)
    local result = {
        next_state = ctx.state or "IDLE",
        target_id = ctx.target_id or 0,
        move_x = 0.0,
        move_y = 0.0,
        request_attack = false,
        attack_id = ctx.default_attack or "viper_strike"
    }

    -- 1. Dead check
    if ctx.state == "DEAD" or (ctx.hp_percent and ctx.hp_percent <= 0.0) then
        result.next_state = "DEAD"
        return result
    end

    -- 2. Evasive Retreat: high-sensitivity retreat threshold (35% HP)
    local retreat_threshold = ctx.retreat_hp or 0.35
    if ctx.can_retreat ~= false and ctx.hp_percent and ctx.hp_percent <= retreat_threshold and ctx.has_target then
        result.next_state = "RETREAT"
        -- Slither evasively away from target
        local dx = (ctx.x or 0.0) - (ctx.target_x or 0.0)
        local dy = (ctx.y or 0.0) - (ctx.target_y or 0.0)
        result.move_x = dx
        result.move_y = dy
        return result
    end

    -- 3. Combat Detection & Attack Range
    local detection_radius = ctx.detection_radius or 150.0
    local attack_radius = ctx.attack_radius or 34.0

    if ctx.has_target and ctx.target_dist and ctx.target_dist <= detection_radius then
        if ctx.target_dist <= attack_radius then
            -- Rapid venomous strike
            result.next_state = "ATTACK"
            result.move_x = 0.0
            result.move_y = 0.0
            result.request_attack = true
            result.attack_id = ctx.default_attack or "viper_strike"
            return result
        else
            -- Slither aggressively towards prey
            result.next_state = "CHASE"
            result.move_x = (ctx.target_x or 0.0) - (ctx.x or 0.0)
            result.move_y = (ctx.target_y or 0.0) - (ctx.y or 0.0)
            return result
        end
    end

    -- 4. Investigate last known scent / vibrations if target was lost
    if ctx.has_memory and ctx.state == "CHASE" then
        result.next_state = "INVESTIGATE"
        result.move_x = (ctx.last_known_x or ctx.x or 0.0) - (ctx.x or 0.0)
        result.move_y = (ctx.last_known_y or ctx.y or 0.0) - (ctx.y or 0.0)
        return result
    end

    -- 5. Ambush Posture & Guard Post near nesting grounds
    local spawn_dx = (ctx.spawn_x or ctx.x or 0.0) - (ctx.x or 0.0)
    local spawn_dy = (ctx.spawn_y or ctx.y or 0.0) - (ctx.y or 0.0)
    local spawn_dist = math.sqrt(spawn_dx * spawn_dx + spawn_dy * spawn_dy)

    if spawn_dist > (ctx.patrol_radius or 110.0) then
        result.next_state = "RETURN"
        result.move_x = spawn_dx
        result.move_y = spawn_dy
    elseif spawn_dist > 35.0 then
        result.next_state = "PATROL"
        result.move_x = spawn_dx
        result.move_y = spawn_dy
    else
        -- Coiled ambush state
        result.next_state = "IDLE"
        result.move_x = 0.0
        result.move_y = 0.0
    end

    return result
end

-- Compatibility aliases
SnakeAI.update = SnakeAI.update_enemy

-- Register into wildbound engine table (both snake_update and viper_update alias)
if wildbound == nil then wildbound = {} end
if wildbound.ai == nil then wildbound.ai = {} end
wildbound.ai.snake_update = SnakeAI.update_enemy
wildbound.ai.viper_update = SnakeAI.update_enemy

return SnakeAI

