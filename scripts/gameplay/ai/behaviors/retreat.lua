local Retreat = {}

function Retreat.evaluate(ctx)
    local dx = ctx.x - ctx.target_x
    local dy = ctx.y - ctx.target_y

    return {
        next_state = "RETREAT",
        move_x = dx,
        move_y = dy,
        request_attack = false
    }
end

return Retreat

