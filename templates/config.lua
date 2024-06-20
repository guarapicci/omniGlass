-- (required) add your touchpad device file path here 
return {
    --Path to the primary touchpad device file (usually at /dev/input or /dev/input/by-path).
    -- This device will be prioritized over all others.
    touchpad_file_path = "",

    --If the selected touchpad is not available,
    -- try scanning the available devices for anything that looks like a touchpad.
    scan_for_touchpads = false,
    
    --How big the edges of the screen are.
    -- 0.0 is no edge;
    -- 0.33 means up to a third of the distance to the opposite edge;
    -- 1.0 means all the touchpad is all the edges simultaneously (recommended not to go past 0.45)
    edge_width = 0.1,

    --Ratio of real-world millimeters per device unit.
    -- (scale = width/(right_edge_X - left_edge_X))
    -- This is NOT a sensitivity value. A scale too low or too high might break gestures.
    -- In future revisions this value may be auto-detected from HID descriptors.
    -- This may vary from device to device.
    scale = 0.028,
    
    --axis inversion.
    --with this you can flip up/down and left/right directions.
    invert_y = true,
    invert_x = false
}
