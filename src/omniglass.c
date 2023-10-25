#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>

#include "omniglass.h"
#include "platform.h"

/** handle for the library*/
struct omniglass{
    struct platform *platform; /**<touchpad device backends (evdev/HID/etc.) and all of their associated state go here*/
    lua_State *vm; /**<lua Virtual Machine (abbreviated as "vm")*/
};

/**step function. user code must schedule to call this at ~100hz or more for responsiveness*/
int omniglass_step(struct omniglass *handle){
    ;
    
    return 0;
}

static const struct luaL_Reg omniglass_platform [] = {
    {"parse_events",platform_parse_events},
    {NULL,NULL}
};

/**initialization function.
 * this function MUST be called before everything else. It will give you a handle that is required for all the other functions of the user-facing API.*/
omniglass_operation_results omniglass_init(struct omniglass **handle){
    *handle = malloc(sizeof(struct omniglass));
    
    //prepare a lua instance.
    lua_State *vm = luaL_newstate();
    if(vm == NULL)
        return OMNIGLASS_RESULT_NOMEM;
    luaL_openlibs(vm);
    (*handle)->vm = vm;
    
    //initialize subsystems.
    if(platform_init(&((*handle)->platform), vm)
        != OMNIGLASS_PLATFORM_INIT_SUCCESS) {
        fprintf(stderr,"failed to initialize omniglass platform.\n");
        return OMNIGLASS_RESULT_BOOTSTRAP_FAILED;
    }
    return OMNIGLASS_RESULT_SUCCESS;
}
