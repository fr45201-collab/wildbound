local Chase = {}

function Chase.evaluate(ctx)
    if not ctx.has_target then
        return { next_state = "IDLE", move_x = 0.0, move_y = 0.0, request_attack = false }
    end

    local dx = ctx.target_x - ctx.x
    local dy = ctx.target_y - ctx.y

    return {
        next_state = "CHASE",
        move_x = dx,
        move_y = dy,
        request_attack = false
    }
end

return Chase

