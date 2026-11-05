local Patrol = {}

function Patrol.evaluate(ctx)
    local dx = ctx.spawn_x - ctx.x
    local dy = ctx.spawn_y - ctx.y

    return {
        next_state = "PATROL",
        move_x = dx,
        move_y = dy,
        request_attack = false
    }
end

return Patrol

