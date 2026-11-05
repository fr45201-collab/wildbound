local Investigate = {}

function Investigate.evaluate(ctx)
    local dx = (ctx.last_known_x or ctx.x) - ctx.x
    local dy = (ctx.last_known_y or ctx.y) - ctx.y

    return {
        next_state = "INVESTIGATE",
        move_x = dx,
        move_y = dy,
        request_attack = false
    }
end

return Investigate

