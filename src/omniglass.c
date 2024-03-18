#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "constants.h"
#include "omniglass.h"
#include "platform.h"

#define OMNIGLASS_CLASS_NAME_META "omniglass_meta"
#define OMNIGLASS_CLASS_NAME_GLOBAL "omniglass"

/** handle for the library*/
struct omniglass{
    struct platform *platform; /**<touchpad device backends (evdev/HID/etc.) and all of their associated state go here*/
    lua_State *vm; /**<lua Virtual Machine (abbreviated as "vm")*/
    
    // gesture trigger callbacks
    // function pointers registered here will be called when their respective gesture happens.
    omniglass_callback_slide cslide; /**< called on slide left/right.*/
    omniglass_callback_edge cedge;
    omniglass_callback_edge cedge_left;
    omniglass_callback_edge cedge_right;
    omniglass_callback_edge cedge_top;
    omniglass_callback_edge cedge_bottom;
};

/**step function. user code must schedule to call this at ~100hz or more for responsiveness*/
int omniglass_step(struct omniglass *handle){
    //call the step function at the lua VM
    lua_getglobal(handle->vm,"step");
    lua_call(handle->vm,0,0);
    
    return 0;
}


/** register a listener for touch slide gestures. 
 *  @param handle a handle to omniglass.
 * @param callback this function will be called whenever a slide gesture is detected.
 * @param passthrough an optional pointer containing data that can be used by the callback.
 */
omniglass_gesture_operation_result omniglass_listen_gesture_slide(struct omniglass *handle, omniglass_callback_slide callback){
    lua_State *vm = handle->vm;
    handle->cslide = callback;
    lua_getglobal(vm,"listen_gesture_slide");
        lua_call(vm, 0, 0);
    return OMNIGLASS_API_GESTURE_OPERATION_SUCCESS;
}

/** remove the listener for touch slide gestures. 
 *  @param handle a handle to omniglass.
 * */
void omniglass_disable_gesture_slide(struct omniglass *handle){
    lua_getglobal(handle->vm,"disable_gesture_slide");
    lua_call(handle->vm, 0, 0);
    handle->cslide = NULL;
    return;
}

/** (LUA-FACING)
 *  trigger the application's registered callback for the slide action
 */
int trigger_gesture_slide(lua_State *vm){
    struct omniglass *handle = luaL_checkudata(vm,1,OMNIGLASS_CLASS_NAME_META);
    double slide_amount = luaL_checknumber(vm,2);
    handle->cslide(luaL_checknumber(vm,2));    
    return 0;
}

/** register a listener for edge slide gestures.
 *  @param handle a handle to omniglass.
 * @param callback this function will be called whenever a slide is detected at the bottom edge.
 */
omniglass_gesture_operation_result omniglass_listen_gesture_edge(struct omniglass *handle, omniglass_callback_edge callback, omniglass_touchpad_edge edge, void *passthrough){
    lua_State *vm = handle->vm;
    lua_getglobal(vm,"listen_gesture_edge");
        lua_pushnumber(vm, edge);
            lua_pushlightuserdata(vm, callback);
                lua_pushlightuserdata(vm, passthrough);
                    lua_call(vm, 3, 0);
    return OMNIGLASS_API_GESTURE_OPERATION_SUCCESS;
}

omniglass_gesture_operation_result omniglass_listen_gesture_edge_left(struct omniglass *handle, omniglass_callback_edge callback, void *passthrough){
    return omniglass_listen_gesture_edge(handle, callback, OMNIGLASS_EDGE_LEFT, passthrough);
}

omniglass_gesture_operation_result omniglass_listen_gesture_edge_right(struct omniglass *handle, omniglass_callback_edge callback, void *passthrough){
    return omniglass_listen_gesture_edge(handle, callback, OMNIGLASS_EDGE_RIGHT, passthrough);
}

omniglass_gesture_operation_result omniglass_listen_gesture_edge_top(struct omniglass *handle, omniglass_callback_edge callback, void *passthrough){
    return omniglass_listen_gesture_edge(handle, callback, OMNIGLASS_EDGE_TOP, passthrough);
}

omniglass_gesture_operation_result omniglass_listen_gesture_edge_bottom(struct omniglass *handle, omniglass_callback_edge callback, void *passthrough){
    return omniglass_listen_gesture_edge(handle, callback, OMNIGLASS_EDGE_BOTTOM, passthrough);
}

/** remove the listeners for any edge slide gestures.
 *  @param handle a handle to omniglass.
 * */
void omniglass_disable_gesture_edge(struct omniglass *handle){
    lua_getglobal(handle->vm,"disable_gesture_edge");
    lua_call(handle->vm, 0, 0);
    handle->cedge = NULL;
    return;
}

/** (LUA-FACING)
 *  trigger the application's registered callback for an edge slide action
 */
int trigger_gesture_edge(lua_State *vm){
    struct omniglass *handle = luaL_checkudata(vm,1,OMNIGLASS_CLASS_NAME_META);
    omniglass_callback_edge callback = (omniglass_callback_edge)lua_touserdata(vm, 2);
    double slide_amount = luaL_checknumber(vm,3);
    void *passthrough = lua_touserdata(vm,4);
    callback(slide_amount, passthrough);
    return 0;
}


luaL_Reg core_api_cfuncs [] = {
    {"trigger_gesture_slide", trigger_gesture_slide},
    {"trigger_gesture_edge", trigger_gesture_edge},
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
    printf("creating omniglass handle\n");
    fflush(stdout);
    
    //create omniglass state structure with valid initial values.
    *handle = lua_newuserdata(vm, sizeof(struct omniglass));
    (*handle)->cslide = NULL;
    (*handle)->vm = vm;
    
    printf("pushing core C API.\n");
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
    if(luaL_dofile((*handle)->vm, OMNIGLASS_ENV_LIB_FOLDER "/omniglass_core.lua")){
        printf("could not initialize omniglass lua core. Error: %s", luaL_checkstring((*handle)->vm,-1));
        return OMNIGLASS_RESULT_BOOTSTRAP_FAILED;
    }
    printf("initialized omniglass core\n");
    fflush(stdout);
    return OMNIGLASS_RESULT_SUCCESS;
}

