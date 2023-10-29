#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "omniglass.h"
#include "platform.h"

#define OMNIGLASS_CLASS_NAME_META "omniglass_meta"
#define OMNIGLASS_CLASS_NAME_GLOBAL "omniglass"

/** handle for the library*/
struct omniglass{
    struct platform *platform; /**<touchpad device backends (evdev/HID/etc.) and all of their associated state go here*/
    lua_State *vm; /**<lua Virtual Machine (abbreviated as "vm")*/
    omniglass_callback_slide cslide;
};

/**step function. user code must schedule to call this at ~100hz or more for responsiveness*/
int omniglass_step(struct omniglass *handle){
    //call the step function at the lua VM
    lua_pushstring(handle->vm,"step");
    lua_call(handle->vm,0,0);
    
    return 0;
}


/** register a listener for touch slide gestures. 
 *  @param handle a handle to omniglass.
 * @param callback this function will be called whenever a slide gesture is detected.
 */
omniglass_api_result omniglass_listen_gesture_slide(struct omniglass *handle, omniglass_callback_slide callback){
    lua_State *vm = handle->vm;
    handle->cslide = callback;
    lua_getglobal(vm,"listen_gesture_slide");
        lua_call(vm,0,0);
    return OMNIGLASS_API_GESTURE_OK;
}

int trigger_gesture_slide(lua_State *vm){
    struct omniglass *handle = luaL_checkudata(vm,1,OMNIGLASS_CLASS_NAME_META);
    double slide_amount = luaL_checknumber(vm,2);
    handle->cslide(luaL_checknumber(vm,2));    
    return 0;
}

luaL_Reg core_api_cfuncs [] = {
    {"trigger_gesture_slide", trigger_gesture_slide},
    {NULL, NULL}
};

/**initialization function.
 * this function MUST be called before everything else. It will give you a handle that is required for all the other functions of the user-facing API.
 @param handle pointer to a pointer of omniglass.
 */
omniglass_operation_results omniglass_init(struct omniglass **handle){
    
    //prepare a lua instance.
    lua_State *vm = luaL_newstate();
    if(vm == NULL)
        return OMNIGLASS_RESULT_NOMEM;
    luaL_openlibs(vm);
    (*handle)->vm = vm;
    
    //create omniglass state structure with valid initial values.
    *handle = lua_newuserdata(vm, sizeof(struct omniglass));
    (*handle)->cslide = NULL;
    
        //push core C functions into lua omniglass table
        luaL_newmetatable(vm, OMNIGLASS_CLASS_NAME_META);    // the "omniglass" metatable holds native operations
                lua_pushstring(vm,"__index");
                    lua_newtable(vm);
                        luaL_register(vm, NULL, core_api_cfuncs);
                        /* 
                        *define here any other stuff you expect inside the global omniglass table...
                        */
                lua_settable(vm, -3);
            lua_setmetatable(vm, -2);   // userdata member access proxies to the metatable's index table.
        lua_setglobal(vm, OMNIGLASS_CLASS_NAME_GLOBAL);
    
    //Load platform (configures underlying input drivers)
    if(platform_init(&((*handle)->platform), vm)
        != OMNIGLASS_PLATFORM_INIT_SUCCESS) {
        fprintf(stderr,"failed to initialize omniglass platform.\n");
        return OMNIGLASS_RESULT_BOOTSTRAP_FAILED;
    }
    
    //Load core logic (configures gesture detection and event emission)
    luaL_dofile((*handle)->vm, "./omniglass_core.lua");
    
    return OMNIGLASS_RESULT_SUCCESS;
}

