# TODO
## V1 - C-side evdev backend:
- (00/06) LIBEVDEV INTERFACE TO LUA VM
    - (02/02) integrate libevdev into a C object file.
    - (00/02) get min/max coordinate values, check for multitouch max-fingers
    - (00/02) inject touchpad refresh hook into lua-side code (mainly `lua_pushcfuntion()` calls)
- (02/02) printf statement on point coordinate change

Sweet heavens, why did i think this would be easy?
