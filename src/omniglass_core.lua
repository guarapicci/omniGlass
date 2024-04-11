--require("mobdebug").start()

--everything touchpad related that the whole program uses
touchpad =
{
    boundaries =
    {
        max_x = 0,
        max_y = 0
    },
    last_touch_report_public=
    {
        {
            touched = false,
            x = 0,
            y = 0,
        },
        {
            touched = false,
            x = 0,
            y = 0
        }
    },
    capabilities =
    {
        width = 0,
        height = 0,
        touch_count = 0
    }
}

--track time
time_elapsed=0

print("lua-side parameter dump")
touchpad.boundaries = platform:get_touchpad_boundaries()
for k, v in pairs(touchpad.boundaries) do
    print(k, v)
end
for k, v in pairs(config) do
    print(k, v)
end

touchpad.capabilities = {
    width = touchpad.boundaries.max_x * config.scale,
    height = touchpad.boundaries.max_y * config.scale,
    touch_count = #(platform:get_last_report().touches) --careful if you replace the platform, this value must be constant per-touchpad
}

omniglass:push_public_touchpad_specifications(touchpad.capabilities.width, touchpad.capabilities.height, touchpad.capabilities.touch_count)

-- every touch point from the platform goes through here.
-- this centralizes all correction, flipping and transformations applied globally to touchpad contact points
function getpoints()
    local function transform_touchpoint(point)
        local result = {}

        --coordinate system inversion
        if (config.invert_y) then do result.y = (touchpad.boundaries.max_y - point.y) end
        else do result.y = point.y end end

        if (config.invert_x) then do result.x = (touchpad.boundaries.max_x - point.x) end
        else do result.x = point.x end end
        
        result.touched = point.touched

        --real-world size of touchpad movement units
        result.x = result.x * config.scale
        result.y = result.y * config.scale

        return result
    end
    local transformed_points = {}
    local report = platform:get_last_report()
    for k, v in ipairs(report.touches) do
        transformed_points[k] = transform_touchpoint(report.touches[k])
    end
    touchpad.last_touch_report_public = transformed_points
    return transformed_points
end

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
    slide = nil,
    edge = nil,
}

--(HACK) 3-sample threshold for "touch-started".
--  a clean implementation requires a timer for touch debouncing
--  (as in, "only touching for at least 50ms is real touching")
function create_task_touched(callback, report, passthrough)
    local newtask = coroutine.create(function()
        local touching_debounced = {}
        local time_since_touched = {}
        local triggered = {}
        print("setting up for touch start")

        --3 samples of holding until a touch is actually recognized.
        local debounce_time_threshold = 3

        for i = 1, touchpad.capabilities.touch_count , 1 do
            touching_debounced[i] = false
            time_since_touched[i] = 0
        end

        while true do
            coroutine.yield()
            local released = false
            local current = touchpad.last_touch_report_public --just an alias for shorthand.
            --  do debouncing (only accept touches after a certain
            --amount of time still touching)
            for i = 1, touchpad.capabilities.touch_count , 1 do
--                 print("i ", i)
--                 print(current, current[1], "x", current[1].x, "\ty", current[1].y, "\t touched:", current[1].touched)
                if (current[i].touched) then
--                     print("stepping touch check.")
                    time_since_touched[i] = time_since_touched[i] + 1
                    if(time_since_touched[i]) > debounce_time_threshold then
                        touching_debounced[i] = true
                    end
                else
                    time_since_touched[i] = 0
                    if (touching_debounced[i]) then
                       released=true
                    end
                    touching_debounced[i] = false
                    triggered[i]=false
                end
            end

            --  trigger events if points are still touched after debouncing or if any point was released
            for i = 1, touchpad.capabilities.touch_count , 1 do
                if(touching_debounced[i] and (not triggered[i])
                    or released) then
                    triggered[i] = true
                    local touch_dump_debounced = {}

                    --  do not modify the public touch report values;
                    --copy only their coordinates into debounced points instead.
                    for j = 1, touchpad.capabilities.touch_count, 1 do
                        touch_dump_debounced[j] = {}
                        touch_dump_debounced[j].touched = touching_debounced[j]
                        touch_dump_debounced[j].x = touchpad.last_touch_report_public[j].x
                        touch_dump_debounced[j].y = touchpad.last_touch_report_public[j].y
                    end
                        omniglass:trigger_gesture_touches_changed(callback, touch_dump_debounced, report, passthrough)
                    break
                end
            end
        end
    end
    )
    return newtask
end

function listen_gesture_touched(callback, report, passthrough)
    print("attaching \"touched\" callback")
    statemachines.touched = create_task_touched(callback, report, passthrough)
end

-- one-finger-slide gesture detector task.
-- current implementation: create a 2d direction vector from a two-point (previous -> current) path.
function create_task_slide ()
    local newtask = coroutine.create(function()
        local previous = getpoints()
        local current = previous
        while true do
--             print("checking for slide")
            current = getpoints()
            local changed = false
            local delta = point_delta(current[1], previous[1])
            if ( (delta.x ~= 0 or delta.y ~= 0)
                and (current[1].touched and previous[1].touched)) then
                omniglass:trigger_gesture_slide(delta.x,delta.y)
            end
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


function create_task_edge (selected_edge, callback, passthrough)
    local newtask = coroutine.create(function()
        print("creating the edge swipe state machine")
        local previous = getpoints()
        local current = previous

        local started_in_edge = false;

        --TODO: GET GENERATED INDEXES FROM C ENUMS
        local C_ENUM_OMNIGLASS_TOUCHPAD_EDGE_PLACEHOLDER_PLEASE_FIX = {
            [0] = "left",
            [1] = "right",
            [2] = "top",
            [3] = "bottom"
        }

        local edge_checkers = {
            left = function(current, delta)
                local boundary = touchpad.capabilities.width * config.edge_width
                return (current.touched and current.x < boundary), (delta.y)
            end,
            right = function(current, delta)
                local boundary = touchpad.capabilities.width - (touchpad.capabilities.width * config.edge_width)
                return (current.touched and current.x > boundary), (delta.y)
            end,
            bottom = function (current, delta)
                local boundary = (touchpad.capabilities.height * config.edge_width)
                return (current.touched and current.y < boundary), (delta.x)
            end,
            top = function(current, delta)
                local boundary = touchpad.capabilities.height - (touchpad.capabilities.height * config.edge_width)
                return (current.touched and current.y > boundary), (delta.x)
            end
        }
        print(string.format("\t}"))
        print("reached edge init for "..C_ENUM_OMNIGLASS_TOUCHPAD_EDGE_PLACEHOLDER_PLEASE_FIX[selected_edge])
        while true do
            current = getpoints()

            local delta = point_delta(current[1], previous[1])
            local edge = C_ENUM_OMNIGLASS_TOUCHPAD_EDGE_PLACEHOLDER_PLEASE_FIX[selected_edge]
            local in_border, offset = edge_checkers[edge](current[1], delta)

            --"started in edge" means contact point 1 began its contact inside the selected edge
            if (current[1].touched) then
                if ((not previous[1].touched) and in_border) then
                    started_in_edge = true
                end
            else
                started_in_edge = false
            end



--             print(string.format("%d; %d; %s", current[1].x, current[1].y, tostring(current[1].touched)))
--             print("current point touch is", current.touched)
            
--             make sure there has been a contact point for at least the last 2 readings.
--             this is meant to avoid cases where lifting a finger in one spot
--             and then touching on another counts as a sliding motion.
            if((previous[1].touched == true)
                and (current[1].touched == true)
                and started_in_edge
                and in_border
                and offset ~= 0) then
                    omniglass:trigger_gesture_edge(callback, offset, passthrough)
            else
--                 print("not touched.")
            end
            previous=current
            coroutine.yield()
        end
    end)
    return newtask
end

function listen_gesture_edge(selected_edge, callback, passthrough)
    print("registered event")
    local edges = {"left","right","top","bottom"}
    local task_title = "edge_"..edges[selected_edge + 1]
    statemachines[task_title] = create_task_edge(selected_edge, callback, passthrough)
end

function disable_gesture_edge()
    local edges = {"left","right","top","bottom"}
    for _, variation in pairs(edges) do
        statemachines["edge_"..variation] = nil
    end
end


function step()
    platform:parse_events()
    getpoints()
--     print("events acquired")
--     step through all registered gesture state machines
    --(FIXME fix broken public report calls to expose the touchpad's points)
    omniglass:push_public_report(#touchpad.last_touch_report_public, touchpad.last_touch_report_public)
    for name,task in pairs(statemachines) do
        local running, condition = coroutine.resume(task)
        if not running then
            print(string.format("task %s died. reason: %s", name, condition))
            statemachines[name] = nil
        end
    end
--     print("public touch report pushed.")
end
print("ms2: \n\t create_task_slide", create_task_slide)
