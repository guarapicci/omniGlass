
/** \file platform_linux.c
 *  \brief linux-evdev implementation for the omniglass touchpad platform
 */
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "platform.h"

#include <linux/input.h>
//this is not the path shown by the official example,
//but it is the actual path for both debian, arch and SUSE.
#include <libevdev-1.0/libevdev/libevdev.h>

#define PLATFORM_CLASS_NAME_META "platform_meta"
#define PLATFORM_CLASS_NAME_GLOBAL "platform"

/** all state for the linux platform*/
struct platform{
    struct libevdev *touchpad_handle; /**<the evdev-specific handle to the touchpad device*/
    struct multitouch_report *report; /**<the latest set of touch contacts checked for*/
    int max_touchpoints; /**<how many simultaneous touches the device can track.*/
};

//pushes a touch point table in the lua stack.
//(must match the platform's touch point format)
void _push_new_touch_point(lua_State *vm, touch_point *point){
    lua_newtable(vm);
        lua_pushstring(vm,"x");
            lua_pushnumber(vm, point->x);
    lua_settable(vm,-3);
        lua_pushstring(vm,"y");
            lua_pushnumber(vm,point->y);
    lua_settable(vm,-3);
        lua_pushstring(vm,"touched");
            lua_pushboolean(vm,point->touched);
    lua_settable(vm,-3);
}

/**(LUA-FACING)
initialization function that detects a touchpad and sets up datastructures and functions needed by the lua VM.
*/
int platform_evdev_init(lua_State *vm){
    
    //get parameters: platform state pointer and touchpad device file path
    struct platform *platform = luaL_checkudata(vm,1,PLATFORM_CLASS_NAME_META);
    const char *touchpad_file_path = luaL_checkstring(vm,2);
    
    /**start evdev*/
    printf("opening event file: %s\n", touchpad_file_path);
    int fd = open(touchpad_file_path,O_RDONLY | O_NONBLOCK);
    int rc = 1;
    rc = libevdev_new_from_fd(fd, &(platform->touchpad_handle));
    if(rc<0) {
        fprintf(stderr, "failed to initialize libevdev (%s)\n", strerror(-rc));
    }
    printf("Input device name: \"%s\"\n", libevdev_get_name(platform->touchpad_handle));

    /**diagnostics: check if selected device does have touchpad-like event reporting*/
    int touchpad_profile_types[] = {EV_SYN, EV_KEY, EV_ABS};
    int touchpad_profile_codes[] = {
        EV_ABS,ABS_X,
        EV_ABS,ABS_Y,
        EV_ABS,ABS_MT_SLOT,
        EV_ABS,ABS_MT_TRACKING_ID};
    for (int i = 0;i<3;i++){
        if(!libevdev_has_event_type(platform->touchpad_handle, touchpad_profile_types[i])) {
            printf("device at %s does not support event code %d. omniGlass will not consider it a touchpad.\n",touchpad_file_path, touchpad_profile_types[i]);
            return 0;
        }
    }
    for (int i = 0;i<4;i+=2){
        if(!libevdev_has_event_code(platform->touchpad_handle, touchpad_profile_codes[i], touchpad_profile_codes[i+1])) {
            printf("device at %s does not support event code %d. omniGlass will not consider it a touchpad\n.",touchpad_file_path, touchpad_profile_codes[i]);
            return 0;
        }
    }
    printf("the device selected is considered a touchpad.\n");
    
    //autodetect number of slots in a multitouch device.
    platform->max_touchpoints = libevdev_get_num_slots(platform->touchpad_handle);
    touch_point *touch_slots = malloc(sizeof(touch_point) * (platform->max_touchpoints));
    multitouch_report *report = malloc(sizeof(multitouch_report));
    report->touches = touch_slots;
    report->extended_touch_parameters = NULL;
    platform->report = report;
    
    printf("successfully allocated evdev: %d \n.", platform->max_touchpoints);
    fflush(stdout);
    
    lua_pushstring(vm,"ok");
    return 1;
}

/** (LUA-FACING)
 * this function is where the linux touchpad implementation gets its touch points from the evdev interface
 */
int platform_parse_events(lua_State *vm){
    
    struct platform *platform = luaL_checkudata(vm,1,PLATFORM_CLASS_NAME_META); //parameter 1 is platform userdata
    
    // printf("touch reporting.\n");
    struct input_event ev;
    
    struct libevdev *dev = platform->touchpad_handle;
    int slot_count= platform->max_touchpoints;
    struct multitouch_report *report = platform->report;
    //pull next input event
    if((libevdev_next_event(dev,LIBEVDEV_READ_FLAG_NORMAL,&ev)) != LIBEVDEV_READ_STATUS_SUCCESS){
        //no success means no points available..
        lua_pushstring(vm,"no_ev");
        return 1;
    }

    //fetch current touch position for all multitouch slots.
    for (int i=0; i < slot_count; i++){
        int tracking_id = 0;
        libevdev_fetch_slot_value(dev, i, ABS_MT_TRACKING_ID, &tracking_id);
        if(tracking_id < 0){
            report->touches[i].touched = false;
        }
        else{
            report->touches[i].touched = true;
        }
        int xval = 0;
        int yval = 0;
        libevdev_fetch_slot_value(
            dev,
            i,
            ABS_MT_POSITION_X, 
            &(report->touches[i].x)
        );
        libevdev_fetch_slot_value(
            dev,
            i,
            ABS_MT_POSITION_Y,
            &(report->touches[i].y)
        );
    }
    fflush(stdout);
    
    lua_pushstring(vm,"ok");
    return 0;
};

/** (LUA-FACING)
 *  push latest report from the platform into lua 
 */
int platform_get_last_report(lua_State *vm){
    struct platform *platform = luaL_checkudata(vm,1,PLATFORM_CLASS_NAME_META);
    
    multitouch_report *current = platform->report;
    lua_newtable(vm);
        lua_pushstring(vm,"touches");
            lua_newtable(vm);
            for(int i=0; i<platform->max_touchpoints; i++){
                // printf("pushing a touchpoint");
                lua_pushinteger(vm, i+1);
                    _push_new_touch_point(vm, &(platform->report->touches[i]));
                lua_settable(vm, -3);
            }
    lua_settable(vm,-3);
    // printf("pushed touch report to stack");
    return 1;   
}

//functions used during linux backend initialization.
static luaL_Reg platform_funcs [] = {
    {"evdev_init", platform_evdev_init},
    {"get_last_report", platform_get_last_report},
    {"parse_events", platform_parse_events},
    {NULL,NULL}
};

/** this function pushes the entire platform internal API and state into a lua table
    @param handle a pointer to a pointer of a handle to create.
    @param vm an already-initialized lua state to be injected with the internal platform API
 */
omniglass_init_result platform_init(struct platform **handle, lua_State *vm){
    // struct platform *new = malloc(sizeof(struct platform));
    
    // create the platform handle as pointer to userdata 
    struct platform *new = lua_newuserdata(vm, sizeof(struct platform));   // the "platform" userdata is a pointer to the platform state.
    if(new==NULL)
        return ENOMEM;
    *handle = new;
        luaL_newmetatable(vm, PLATFORM_CLASS_NAME_META);    // the "platform" metatable holds native operations
            lua_pushstring(vm,"__index");
                lua_newtable(vm);
                    luaL_register(vm, NULL, platform_funcs);
                    /* 
                    *define here any other stuff you expect inside the global platform table...
                    */
            lua_settable(vm, -3);
        lua_setmetatable(vm, -2);   // userdata member access proxies to the metatable's index table.
    lua_setglobal(vm, PLATFORM_CLASS_NAME_GLOBAL);
    
    //run the real configuration process as a lua script.
    if(luaL_dofile(vm, "omniglass_linux.lua")){
        printf("omniglass could not connect to an available touchpad device.\nerror: %s", luaL_checkstring(vm,-1));
        return OMNIGLASS_PLATFORM_INIT_NO_TOUCHPAD;
    }
    
    return OMNIGLASS_PLATFORM_INIT_SUCCESS;
}
