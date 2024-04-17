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
    
    omniglass_raw_specifications touchpad_specifications;
    omniglass_raw_report last_raw_report;

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
omniglass_gesture_operation_result omniglass_listen_gesture_slide(struct omniglass *handle, omniglass_callback_slide callback, void *passthrough){
    lua_State *vm = handle->vm;
    lua_getglobal(vm,"listen_gesture_slide");
        lua_pushlightuserdata(vm, callback);
            lua_pushlightuserdata(vm, passthrough);
                lua_call(vm, 2, 0);
    return OMNIGLASS_API_GESTURE_OPERATION_SUCCESS;
}

/** remove the listener for touch slide gestures. 
 *  @param handle a handle to omniglass.
 * */
void omniglass_disable_gesture_slide(struct omniglass *handle){
    lua_getglobal(handle->vm,"disable_gesture_slide");
    lua_call(handle->vm, 0, 0);
    return;
}

/** (LUA-FACING)
 *  trigger the application's registered callback for the slide action
 */
int trigger_gesture_slide(lua_State *vm){
    struct omniglass *handle = luaL_checkudata(vm,1,OMNIGLASS_CLASS_NAME_META);
    omniglass_callback_slide callback = lua_touserdata(vm, 2);
    double slide_x = luaL_checknumber(vm,3);
    double slide_y = luaL_checknumber(vm,4);
    void *passthrough = lua_touserdata(vm,5);
    callback(slide_x, slide_y, passthrough);
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
/**(PUBLIC)
 * register callback for touch started
 */
omniglass_operation_results omniglass_listen_gesture_touches_changed
    (struct omniglass *handle, omniglass_callback_touches_changed callback, void *passthrough)
{
    lua_State *vm = handle->vm;

    omniglass_raw_report *filtered_report = malloc(sizeof(omniglass_raw_report));
    filtered_report->points_max = handle->touchpad_specifications.max_points;
    filtered_report->points = calloc(sizeof(omniglass_raw_touchpoint),filtered_report->points_max);
    lua_getglobal(vm, "listen_gesture_touched");
        lua_pushlightuserdata(vm,callback);
            lua_pushlightuserdata(vm, filtered_report);
                lua_pushlightuserdata(vm,passthrough);
    printf("setting up touch start gesture\n");
    fflush(stdout);
    lua_call(vm, 3, 0);
    return OMNIGLASS_RESULT_SUCCESS;
}

/**(LUA-FACING)
 * trigger registered callback for "points have started touching", passing
 */
int trigger_gesture_touches_changed(lua_State *vm){
    /*signature of parameters expected from stack:
     * (omniglass address, callback address,
     *   report table, raw report address,
     *   passthrough)
     */
    struct omniglass *handle = luaL_checkudata(vm,1,OMNIGLASS_CLASS_NAME_META);
    omniglass_callback_touches_changed callback = (omniglass_callback_touches_changed)lua_touserdata(vm,2);
    omniglass_raw_report *report = lua_touserdata(vm, 4);
    void *passthrough = lua_touserdata(vm,5);
    lua_pushvalue(vm, 3);
    for (int i= 0; i < report->points_max; i++){
        lua_pushnumber(vm, i+1);
            lua_gettable(vm, -2);
            lua_pushstring(vm,"touched");
                lua_gettable(vm, -2);
                report->points[i].is_touching = lua_toboolean(vm, -1);
                lua_pop(vm,1);
            lua_pushstring(vm,"x");
                lua_gettable(vm,-2);
                report->points[i].x = luaL_checknumber(vm,-1);
                lua_pop(vm,1);
            lua_pushstring(vm,"y");
                lua_gettable(vm,-2);
                report->points[i].y = luaL_checknumber(vm,-1);
                lua_pop(vm,2);
    };
    callback(report, passthrough);
    return 0;
}

/**(LUA-FACING)
 * copy transformed touch points back to the public multitouch report
 */
int push_public_report(lua_State *vm){
    struct omniglass *handle = luaL_checkudata(vm,1,OMNIGLASS_CLASS_NAME_META);
    int touch_count = luaL_checkinteger(vm, 2);
    // printf("touch count is %d\n", touch_count);
    for (int i = 0; i < touch_count; i++){
        lua_pushnumber(vm,i+1);
            lua_gettable(vm, 3);
            lua_pushstring(vm,"touched");
                lua_gettable(vm,4);
                // printf("indexing \"touched\" field\n");
                handle->last_raw_report.points[i].is_touching = lua_toboolean(vm,5);
                lua_pop(vm,1);
            lua_pushstring(vm,"x");
                lua_gettable(vm,4);
                // printf("indexing \"x\" field\n");
                handle->last_raw_report.points[i].x = luaL_checknumber(vm,5);
                lua_pop(vm,1);
            lua_pushstring(vm,"y");
                lua_gettable(vm,4);
                // printf("indexing \"y\" field\n");
                handle->last_raw_report.points[i].y = luaL_checknumber(vm,5);
                lua_pop(vm,2);
    };
    return 0;
}

/**(LUA-FACING)
 * (requires fully initialized platform!)
 * hand transformed touchpad specifications to the C-side state.
 */
int push_public_touchpad_specifications(lua_State *vm){
    struct omniglass *handle = luaL_checkudata(vm,1,OMNIGLASS_CLASS_NAME_META);
    omniglass_raw_specifications *spec = &(handle->touchpad_specifications);
    spec->width = luaL_checknumber(vm, 2);
    spec->height = luaL_checknumber(vm, 3);
    spec->max_points = luaL_checkinteger(vm, 4);
    return 0;
}

omniglass_operation_results omniglass_get_touchpad_parameters(struct omniglass *handle, omniglass_raw_specifications **specs){
    (*specs) = &(handle->touchpad_specifications);
    return OMNIGLASS_RESULT_SUCCESS;
}

luaL_Reg core_api_cfuncs [] = {
    {"trigger_gesture_slide", trigger_gesture_slide},
    {"trigger_gesture_edge", trigger_gesture_edge},
    {"trigger_gesture_touches_changed", trigger_gesture_touches_changed},
    {"push_public_report", push_public_report},
    {"push_public_touchpad_specifications", push_public_touchpad_specifications},
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
    
    //create omniglass state structure with "safe" initial values.
    *handle = lua_newuserdata(vm, sizeof(struct omniglass));
    (*handle)->cslide = NULL;
    (*handle)->vm = vm;

    omniglass_raw_specifications *spec = &((*handle)->touchpad_specifications);
    spec->width = 0.0;
    spec->height = 0.0;
    spec->max_points = 0;

    (*handle)->last_raw_report.points=NULL;

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
    (*handle)->last_raw_report.points=malloc(sizeof(struct omniglass_raw_touchpoint) * spec->max_points); //this might actually belong in the platform layer
    printf("touchpad spec: width %2fmm, height %2fmm, %d-touch max\n", spec->width, spec->height, spec->max_points);

    printf("initialized omniglass core\n");

    fflush(stdout);
    return OMNIGLASS_RESULT_SUCCESS;
}

