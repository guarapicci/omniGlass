# TODO
## V2 - C-side evdev with touchpad probing
- (00/06) LIBEVDEV DEVICE DETECTION
    - (00/03) define platform interface for touchpad parameters
        - touchpad bounding box (min/max XY)
        - touchpad unit scale (in millimeters per unit)
        - find touchpad (file, device ID, bus enumeration, etc.)
    - (00/03) implement libevdev touchpad bounding box and unit scale detection

## changelog
### V1 - C-side evdev backend:
- (00/06) LIBEVDEV INTERFACE TO LUA VM
    - (OVERWORK)(04/02) integrate libevdev into a C object file.
        - This involved reading up on CMake to manage dependencies.
    - (SHELVED)(00/02) get min/max coordinate values, check for multitouch max-fingers
        - shelved for now.
    - (SHELVED)(00/02) inject touchpad refresh hook into lua-side code (mainly `lua_pushcfuntion()` calls)
        - could not go far enough past the platform code to start any lua logic.
- (OVERWORK)(05/02) printf statement on point coordinate change
    - Simply put, i had many wrong assumptions about how libevdev actually operates.
        - **highlight: a lack of events does not mean EAGAIN.**
        - some internal state on multitouch points is saved on each `next_event` call, however calling it without new events will discard such state.
