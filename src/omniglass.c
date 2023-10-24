#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>

#include "omniglass.h"
#include "platform.h"

/** handle for the library, in evdev implementation */
struct omniglass{
    struct platform *platform; /**<touchpad device backends (evdev/HID/etc.) and all of their associated state go here*/
    lua_State *vm; /**<lua Virtual Machine (abbreviated as "vm")*/
};

static const struct luaL_Reg omniglass_platform [] = {
    {"parse_events",platform_parse_events},
    {NULL,NULL}
};

/**initialization function.
 * this function MUST be called before everything else. It will give you a handle that is required for all the other functions of the user-facing API.*/
int ommniglass_init(struct omniglass **handle){
    *handle = malloc(sizeof(struct omniglass));
    
    //prepare a lua instance.
    lua_State *vm = luaL_newstate();
    if(vm == NULL)
        return ENOMEM;
    luaL_openlibs(vm);
    (*handle)->vm = vm;
    
    //initialize subsystems.
    platform_init(&((*handle)->platform), vm);
}
