local Idle = {}

function Idle.evaluate(ctx)
    return {
        next_state = "IDLE",
        move_x = 0.0,
        move_y = 0.0,
        request_attack = false,
        attack_id = ctx.default_attack or "default",
        target_id = ctx.target_id or 0
    }
end

return Idle
