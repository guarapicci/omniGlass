
-- a sequence of touchpoints over time (follows the touch_point structure)
timeseries = {
    {
        x = 0,
        y = 0;
        pressed = false;
    }
}

--basic operations dealing with touch points
function xor(a,b)
    return ((a and (not b)) or ((not a) and b))
end
function point_delta(a,b)
    local changed = xor(a.touched,b.touched)
    local transition = 0
    if (b.touched) then
        if (not a.touched) then
            transition = -1
        end
    elseif (a.touched) then
        transition = 1
    end
    return {
        x = (a.x - b.x),
        y = (a.y - b.y),
        transition = transition
        }
end

-- state machine task list.
-- every member in this table should keep track of a sequence of contact points to check for a gesture.
statemachines = {
    slide = nil
}

-- slide gesture detector task.
-- current implementation: create a 2d direction vector from a two-point (previous -> current) path.
-- trigger 
function create_task_slide ()
    local newtask = coroutine.create(function()
        local previous = platform:get_last_report()
        local current = previous
        while true do
            current = platform:get_last_report()
            local changed = false
            for k, _ in ipairs(prev.touches) do
                local delta = point_delta(current.touches[k],prev.touches[k])
                if (delta.x ~= 0  and k == 1) then
                    trigger_gesture_slide(delta.x)
                end
            end
            prev = current
            coroutine.yield()
        end
    end
    )
    return newtask
end

function listen_gesture_slide()
    statemachines.slide = create_task_slide()
end

function disable_gesture_slide()
    statemachines.slide = nil
end

function step()
    platform:parse_events()
    
    --step through all registered gesture state machines
    for name,task in statemachines do
        coroutine.resume(task)
    end
end
