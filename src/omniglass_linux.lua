--hardcoded paths where the script will look for configuration files.
local paths = {
    "config.lua",
    --(lua does not recognize the home directory macro)
    --"~/.config/omniGlass/config.lua",
    "/usr/share/omniGlass/config.lua"
}

local config = {}

--initialize the evdev backend
do
    local configured = false
        for _, v in ipairs(paths)
        do
            local script, err = loadfile(v,"t")
            if script then
                config = script()
                config.sourcefile = v
                configured = true
            end
        end
    if not configured then
        local message = "omniglass configuration file not found.\n please provide a lua configuration file at one of the following locations:\n"
        for k, v in pairs(paths) do
            message = message .."\""..v.."\"\n"
        end
        print(message)
        error("no_config")
    end
end

if not config.touchpad_file_path then
    print("error: device file path for touchpad not set in \""..config.sourcefile.."\"")
    error("no_config")
end

local dameta = debug.getmetatable(platform)
print ("metatable for platform is:", dameta)

local linux_init_status = platform:evdev_init(config.touchpad_file_path)
if not (linux_init_status == "ok") then
    print("linux evdev init failed.")
    error("no_config")
end

-- (DISCARDABLE DEBUG ROUTINES)
-- scan for changes in points, dump point if different.

local function xor(a,b)
    return ((a and (not b)) or ((not a) and b))
end
local function point_delta(a,b)
    local transition = (a.pressed and not b)
    return {
        x = (a.x - b.x),
        y = (a.y - b.y),
        }
end

platform:parse_events()
local prev = platform:get_last_report()

-- while true do
--     platform:parse_events()
--     local current = platform:get_last_report()
--     print("touches:")
--     for k, v in ipairs(current.touches) do
--         print ("(", current.touches[k].x, ", ", current.touches[k].y, ")")
--     end
-- end

while true do
    platform:parse_events()
    local current = platform:get_last_report()
    local changed = false
    for k, _ in ipairs(prev.touches) do
        local delta = point_delta(current.touches[k],prev.touches[k])
        if (delta.x < 0) then
            print("slide left", delta.x)
            prev = current
        elseif (delta.x > 0) then
            print("slide right", delta.x)
            prev = current
        end
    end
end

return
