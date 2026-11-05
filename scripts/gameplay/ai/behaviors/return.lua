local Return = {}

function Return.evaluate(ctx)
    local dx = (ctx.spawn_x or ctx.x) - ctx.x
    local dy = (ctx.spawn_y or ctx.y) - ctx.y

    return {
        next_state = "RETURN",
        move_x = dx,
        move_y = dy,
        request_attack = false
    }
end

return Return

