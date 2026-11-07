local Retreat = {}

function Retreat.evaluate(ctx)
    local dx = ctx.x - (ctx.target_x or ctx.x)
    local dy = ctx.y - (ctx.target_y or ctx.y)
    return {
        next_state = "RETREAT",
        move_x = dx,
        move_y = dy,
        request_attack = false,
        attack_id = ctx.default_attack or "default",
        target_id = ctx.target_id or 0
    }
end

return Retreat
