local Defend = {}

function Defend.evaluate(ctx)
    return {
        next_state = "DEFEND",
        move_x = 0.0,
        move_y = 0.0,
        request_attack = false,
        attack_id = ctx.default_attack or "default",
        target_id = ctx.target_id or 0
    }
end

return Defend
