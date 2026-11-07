local Return = {}

function Return.evaluate(ctx)
    local spawn_dx = (ctx.spawn_x or ctx.x) - ctx.x
    local spawn_dy = (ctx.spawn_y or ctx.y) - ctx.y
    return {
        next_state = "RETURN",
        move_x = spawn_dx,
        move_y = spawn_dy,
        request_attack = false,
        attack_id = ctx.default_attack or "default",
        target_id = ctx.target_id or 0
    }
end

return Return
