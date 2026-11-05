local Idle = {}

function Idle.evaluate(ctx)
    return {
        next_state = "IDLE",
        move_x = 0.0,
        move_y = 0.0,
        request_attack = false
    }
end

return Idle

