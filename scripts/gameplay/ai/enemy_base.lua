-- ============================================================================
-- WILDBOUND ENEMY AI STATE MACHINE (LUA 5.4)
-- Core enemy base behavior, modularized for Phase 46
-- ============================================================================

local AI = {}

-- Safe require for modular behavior modules
local function safe_require(module_name)
    local ok, mod = pcall(require, module_name)
    if ok and type(mod) == "table" and type(mod.evaluate) == "function" then
        return mod
    end
    return nil
end

local BehaviorIdle = safe_require("idle") or safe_require("behaviors.idle")
local BehaviorPatrol = safe_require("patrol") or safe_require("behaviors.patrol")
local BehaviorChase = safe_require("chase") or safe_require("behaviors.chase")
local BehaviorAttack = safe_require("attack") or safe_require("behaviors.attack")
local BehaviorRetreat = safe_require("retreat") or safe_require("behaviors.retreat")
local BehaviorInvestigate = safe_require("investigate") or safe_require("behaviors.investigate")
local BehaviorReturn = safe_require("return") or safe_require("behaviors.return")
local BehaviorDefend = safe_require("defend") or safe_require("behaviors.defend")

function AI.update_enemy(ctx)
    local result = {
        next_state = ctx.state or "IDLE",
        target_id = ctx.target_id or 0,
        move_x = 0.0,
        move_y = 0.0,
        request_attack = false,
        attack_id = ctx.default_attack or "default"
    }

    -- 1. If Dead or HP <= 0, do nothing
    if ctx.state == "DEAD" or (ctx.hp_percent and ctx.hp_percent <= 0.0) then
        result.next_state = "DEAD"
        return result
    end

    -- 2. Retreat check: low HP condition (if retreat is permitted)
    if ctx.can_retreat ~= false and ctx.hp_percent and ctx.hp_percent <= (ctx.retreat_hp or 0.2) and ctx.has_target then
        if BehaviorRetreat then
            return BehaviorRetreat.evaluate(ctx)
        end
        result.next_state = "RETREAT"
        result.move_x = ctx.x - ctx.target_x
        result.move_y = ctx.y - ctx.target_y
        return result
    end

    -- 3. Combat & Detection Logic
    local detection_radius = ctx.detection_radius or 140.0
    local attack_radius = ctx.attack_radius or 32.0

    if ctx.has_target and ctx.target_dist and ctx.target_dist <= detection_radius then
        if ctx.target_dist <= attack_radius then
            -- Target within melee attack range
            if BehaviorAttack then
                return BehaviorAttack.evaluate(ctx)
            end
            result.next_state = "ATTACK"
            result.move_x = 0.0
            result.move_y = 0.0
            result.request_attack = true
            return result
        else
            -- Target detected: Chase
            if BehaviorChase then
                return BehaviorChase.evaluate(ctx)
            end
            result.next_state = "CHASE"
            result.move_x = ctx.target_x - ctx.x
            result.move_y = ctx.target_y - ctx.y
            return result
        end
    end

    -- 4. Target lost but recently seen: Investigate
    if ctx.has_memory and ctx.state == "CHASE" then
        if BehaviorInvestigate then
            return BehaviorInvestigate.evaluate(ctx)
        end
        result.next_state = "INVESTIGATE"
        result.move_x = (ctx.last_known_x or ctx.x) - ctx.x
        result.move_y = (ctx.last_known_y or ctx.y) - ctx.y
        return result
    end

    -- 5. No Target in range: Return or Patrol or Idle
    local spawn_dx = (ctx.spawn_x or ctx.x) - ctx.x
    local spawn_dy = (ctx.spawn_y or ctx.y) - ctx.y
    local spawn_dist = math.sqrt(spawn_dx * spawn_dx + spawn_dy * spawn_dy)

    if spawn_dist > 120.0 then
        if BehaviorReturn then
            return BehaviorReturn.evaluate(ctx)
        end
        result.next_state = "RETURN"
        result.move_x = spawn_dx
        result.move_y = spawn_dy
    elseif spawn_dist > 40.0 then
        if BehaviorPatrol then
            return BehaviorPatrol.evaluate(ctx)
        end
        result.next_state = "PATROL"
        result.move_x = spawn_dx
        result.move_y = spawn_dy
    else
        if BehaviorIdle then
            return BehaviorIdle.evaluate(ctx)
        end
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
