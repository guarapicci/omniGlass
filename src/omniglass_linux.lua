--hardcoded paths where the script will look for configuration files.
local paths = {
    "config.lua",
    --(lua does not recognize the home directory macro)
    --"~/.config/omniGlass/config.lua",
    "/etc/omniGlass/config.lua"
}

-- global configuration table (with everything the user defined for their setup)
-- TODO: move the acquisition of this config script to the core lua init
config = {}

--mirror of enumerations from C-side platform code.
-- (make sure the codes match on both ends!)
status= {
    NO_CONFIG = -2,
    EVDEV_INIT_FAILED = -1,
    EVDEV_INIT_SUCCESS = 0
}

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
        return status.NO_CONFIG
    end
end

if not config.touchpad_file_path then
    print("error: device file path for touchpad not set in \""..config.sourcefile.."\"")
    return status.NO_CONFIG
end

local dameta = debug.getmetatable(platform)
print ("metatable for platform is:", dameta)

local linux_init_status = platform:evdev_init(config.touchpad_file_path)
if not (linux_init_status == status.EVDEV_INIT_SUCCESS) then
    print("linux evdev init failed.")
    return status.EVDEV_INIT_FAILED
end

return status.EVDEV_INIT_SUCCESS
