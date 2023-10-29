
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
function create_task_slide ()
    local newtask = coroutine.create(function()
        local previous = platform:get_last_report()
        local current = previous
        while true do
--             print("checking for slide")
            current = platform:get_last_report()
            local changed = false
            for k, _ in ipairs(previous.touches) do
--                 print("onetouch")
                local delta = point_delta(current.touches[k],previous.touches[k])
                if (delta.x ~= 0  and k == 1) then
                    print("slide detected ", delta.x)
                    omniglass:trigger_gesture_slide(delta.x)
                end
            end
--             print("slide check over")
            previous = current
            coroutine.yield()
        end
    end
    )
    return newtask
end
-- print("ms1 state machines")

function listen_gesture_slide()
    print("registered event")
    statemachines.slide = create_task_slide()
end

function disable_gesture_slide()
    statemachines.slide = nil
end

function step()
    platform:parse_events()
--     print("events acquired")
    --step through all registered gesture state machines
    for name,task in pairs(statemachines) do
        coroutine.resume(task)
    end
end
print("ms2: \n\t create_task_slide", create_task_slide)
