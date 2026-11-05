local Attack = {}

function Attack.evaluate(ctx)
    return {
        next_state = "ATTACK",
        move_x = 0.0,
        move_y = 0.0,
        request_attack = true
    }
end

return Attack

