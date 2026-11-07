-- ============================================================================
-- WILDBOUND SKELETON AI SCRIPT (LUA 5.4)
-- Archetype: Relentless undead guardian. Never retreats; fights to the death.
-- ============================================================================

local SkeletonAI = {}

function SkeletonAI.update_enemy(ctx)
    local result = {
        next_state = ctx.state or "IDLE",
        target_id = ctx.target_id or 0,
        move_x = 0.0,
        move_y = 0.0,
        request_attack = false,
        attack_id = "skeleton_blade"
    }

    -- 1. Dead check
    if ctx.state == "DEAD" or (ctx.hp_percent and ctx.hp_percent <= 0.0) then
        result.next_state = "DEAD"
        return result
    end

    -- 2. Skeletons NEVER retreat (fearless undead trait)
    -- Even if HP is low, ignore retreat requests and press the attack!

    -- 3. Combat Detection & Attack Range
    local detection_radius = ctx.detection_radius or 160.0
    local attack_radius = ctx.attack_radius or 36.0

    if ctx.has_target and ctx.target_dist and ctx.target_dist <= detection_radius then
        if ctx.target_dist <= attack_radius then
            -- Slash with blade
            result.next_state = "ATTACK"
            result.move_x = 0.0
            result.move_y = 0.0
            result.request_attack = true
            result.attack_id = "skeleton_blade"
            return result
        else
            -- Relentlessly pursue target
            result.next_state = "CHASE"
            result.move_x = ctx.target_x - ctx.x
            result.move_y = ctx.target_y - ctx.y
            return result
        end
    end

    -- 4. Investigate last seen target position
    if ctx.has_memory and ctx.state == "CHASE" then
        result.next_state = "INVESTIGATE"
        result.move_x = (ctx.last_known_x or ctx.x) - ctx.x
        result.move_y = (ctx.last_known_y or ctx.y) - ctx.y
        return result
    end

    -- 5. Guard post / Patrol around spawn origin
    local spawn_dx = (ctx.spawn_x or ctx.x) - ctx.x
    local spawn_dy = (ctx.spawn_y or ctx.y) - ctx.y
    local spawn_dist = math.sqrt(spawn_dx * spawn_dx + spawn_dy * spawn_dy)

    if spawn_dist > (ctx.patrol_radius or 100.0) then
        result.next_state = "RETURN"
        result.move_x = spawn_dx
        result.move_y = spawn_dy
    elseif spawn_dist > 35.0 then
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
SkeletonAI.update = SkeletonAI.update_enemy

-- Register into wildbound engine table
if wildbound == nil then wildbound = {} end
if wildbound.ai == nil then wildbound.ai = {} end
wildbound.ai.skeleton_update = SkeletonAI.update_enemy

return SkeletonAI

