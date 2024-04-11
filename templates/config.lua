-- (required) add your touchpad device file path here 
return {
    -- path to the touchpad device file (usually at /dev/input or /dev/input/by-path)
    touchpad_file_path = "/dev/input/by-path/platform-AMDI0010:01-event-mouse",
    
    -- how big the edges of the screen are.
    --  0.0 is no edge; 0.33 means up to a third of the distance to the opposite edge;
    -- 1.0 means all the touchpad is edges (recommended not to go past 0.45)
    edge_width = 0.1,

    -- ratio of device units to real-world millimeters.
    --  This may vary from device to device.
    scale = 0.036,

    -- axis inversion.
    --  with this you can flip up/down and left/right directions.
    invert_y = true,
    invert_x = false
}
