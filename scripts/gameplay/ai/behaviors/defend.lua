local Defend = {}

function Defend.evaluate(ctx)
    local dx = ctx.x - (ctx.target_x or ctx.x)
    local dy = ctx.y - (ctx.target_y or ctx.y)

    return {
        next_state = "DEFEND",
        move_x = dx,
        move_y = dy,
        request_attack = false
    }
end

return Defend

