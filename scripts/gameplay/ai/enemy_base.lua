-- ============================================================================
-- WILDBOUND ENEMY AI STATE MACHINE (LUA 5.4)
-- Expanded for Phase 29: AI & Navigation Expansion
-- ============================================================================

local AI = {}

function AI.update_enemy(ctx)
    local result = {
        next_state = ctx.state,
        target_id = ctx.target_id or 0,
        move_x = 0.0,
        move_y = 0.0,
        request_attack = false,
        attack_id = "default"
    }

    -- 1. If Dead, do nothing
    if ctx.state == "DEAD" or ctx.hp_percent <= 0.0 then
        result.next_state = "DEAD"
        return result
    end

    -- 2. Retreat check: low HP condition (if retreat is permitted)
    if ctx.can_retreat ~= false and ctx.hp_percent <= (ctx.retreat_hp or 0.2) and ctx.has_target then
        result.next_state = "RETREAT"
        -- Move directly opposite to target position
        local dx = ctx.x - ctx.target_x
        local dy = ctx.y - ctx.target_y
        result.move_x = dx
        result.move_y = dy
        return result
    end

    -- 3. Combat & Detection Logic
    if ctx.has_target and ctx.target_dist <= (ctx.detection_radius or 140.0) then
        if ctx.target_dist <= (ctx.attack_radius or 32.0) then
            -- Target within melee attack range
            result.next_state = "ATTACK"
            result.move_x = 0.0
            result.move_y = 0.0
            result.request_attack = true
            return result
        else
            -- Target detected: Chase
            result.next_state = "CHASE"
            local dx = ctx.target_x - ctx.x
            local dy = ctx.target_y - ctx.y
            result.move_x = dx
            result.move_y = dy
            return result
        end
    end

    -- 4. Target lost but recently seen: Investigate
    if ctx.has_memory and ctx.state == "CHASE" then
        result.next_state = "INVESTIGATE"
        local dx = (ctx.last_known_x or ctx.x) - ctx.x
        local dy = (ctx.last_known_y or ctx.y) - ctx.y
        result.move_x = dx
        result.move_y = dy
        return result
    end

    -- 5. No Target in range: Return or Patrol or Idle
    local spawn_dx = (ctx.spawn_x or ctx.x) - ctx.x
    local spawn_dy = (ctx.spawn_y or ctx.y) - ctx.y
    local spawn_dist = math.sqrt(spawn_dx * spawn_dx + spawn_dy * spawn_dy)

    if spawn_dist > 120.0 then
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

-- Register into wildbound engine table
if wildbound == nil then wildbound = {} end
if wildbound.ai == nil then wildbound.ai = {} end
wildbound.ai.update_enemy = AI.update_enemy

return AI
