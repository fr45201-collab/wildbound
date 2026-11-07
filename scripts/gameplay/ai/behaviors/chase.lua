local Chase = {}

function Chase.evaluate(ctx)
    local dx = (ctx.target_x or ctx.x) - ctx.x
    local dy = (ctx.target_y or ctx.y) - ctx.y
    return {
        next_state = "CHASE",
        move_x = dx,
        move_y = dy,
        request_attack = false,
        attack_id = ctx.default_attack or "default",
        target_id = ctx.target_id or 0
    }
end

return Chase
