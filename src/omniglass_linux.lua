
local paths = {
    "~/.config/omniGlass/config.lua",
    "/usr/share/omniGlass/config.lua"
}

local config = {}

local function init()
    do 
        local configured = false
            for _, v in paths
            do
                local script, err = loadfile(v,"t")
                if script then
                    script.configure(config)
                    config.sourcefile = v
                    configured = true
                end
            end
        if not configured then
            local message = "omniglass configuration file not found.\n please provide a lua configuration file at one of the following locations:\n"
            for k, v in pairs(paths) do
                errmsg = errmsg.."\""..v.."\"\n"
            end
            print(message)
            return "no_config"
        end
    end
    if not config.touchpad_file_path then
        print("error: device file path for touchpad not set in \""..config.sourcefile.."\"")
        return "no_config"
    end
    local linux_init_status = platform_linux:init(config.touchpad_file_path)
    if not (linux_init_status == "ok") then
        print "linux evdev init failed."
        return "no_config"
    end
    return "ok"
end

local function step()
    local platform_linux:step

end
