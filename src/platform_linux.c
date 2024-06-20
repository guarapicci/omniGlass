
/** \file platform_linux.c
 *  \brief linux-evdev implementation for the omniglass touchpad platform
 */

#include <asm-generic/errno-base.h>
#include <linux/input-event-codes.h>
#include <omniGlass/constants.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "platform.h"

#include <linux/input.h>
//this is not the path shown by the official example,
//but it is the actual path for both debian, arch and SUSE.
#include "libevdev/libevdev.h"

#include <dirent.h>

#define PLATFORM_CLASS_NAME_META "platform_meta"
#define PLATFORM_CLASS_NAME_GLOBAL "platform"

#define PLATFORM_DEFAULT_DEVICE_DIRECTORY "/dev/input"

typedef enum {
    OMNIGLASS_PLATFORM_NO_CONFIG = -2,
    OMNIGLASS_PLATFORM_EVDEV_INIT_FAILED = -1,
    OMNIGLASS_PLATFORM_EVDEV_INIT_SUCCESS = 0
} omniglass_platform_status;

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
    bool scan_for_devices = lua_toboolean(vm, 3);

    DIR *directory = opendir(PLATFORM_DEFAULT_DEVICE_DIRECTORY);
    if(directory == NULL){
        fprintf(stderr, "Omniglass expects device file(s) at  %s, but this folder could not be opened. "
            "Aborting touchpad initialization.\n", PLATFORM_DEFAULT_DEVICE_DIRECTORY);
        return 0;
    }

    //try the exact device file chosen at the configuration
    printf("trying selected event file: %s\n", touchpad_file_path);
    int fd = open(touchpad_file_path,O_RDONLY | O_NONBLOCK);
    int rc = libevdev_new_from_fd(fd, &(platform->touchpad_handle));
    if(rc<0) {
        fprintf(stderr, "failed to initialize libevdev with a default touchpad (%s).", strerror(-rc));
        if(!scan_for_devices){
            fprintf(stderr,"\n");
            return 0;
        }
        else{
            fprintf(stderr, " Omniglass will fall back to any touchpad available.\n");

        }
    }
    while (true){

        rc = libevdev_new_from_fd(fd, &(platform->touchpad_handle));
        //keep trying for file descriptors until either a touchpad is detected or we run out of devices to check.
        if(rc<0) {
            close(fd);
            struct dirent *current_directory_entry = readdir(directory);
            if(current_directory_entry == NULL){
                fprintf(stderr, "No touchpads were found at %s. Aborting touchpad initialization.\n", PLATFORM_DEFAULT_DEVICE_DIRECTORY);
                lua_pushinteger(vm, OMNIGLASS_PLATFORM_EVDEV_INIT_FAILED);
                return 0;
            }
            char devpath[512];
            sprintf(devpath, "%s/%s", PLATFORM_DEFAULT_DEVICE_DIRECTORY, current_directory_entry->d_name);
            fd = open(devpath, O_RDONLY | O_NONBLOCK);
            continue;
        }
                /**start evdev*/
        const char *device_name = libevdev_get_name(platform->touchpad_handle);
        printf("Input device: \"%s\"", device_name);

        /**diagnostics: check if selected device does have touchpad-like event reporting*/
        int touchpad_profile_types[] = {EV_SYN, EV_KEY, EV_ABS};
        int touchpad_profile_codes[] = {
            EV_ABS,ABS_X,
            EV_ABS,ABS_Y,
            EV_ABS,ABS_MT_SLOT,
            EV_ABS,ABS_MT_TRACKING_ID};
        bool device_approved = true;
        for (int i = 0;i<3;i++){
            if(!libevdev_has_event_type(platform->touchpad_handle, touchpad_profile_types[i])) {
                printf(" does not support event type %d.",
                    touchpad_profile_types[i]);
                device_approved &= false;
                break;
            }
        }
        if(device_approved){
            for (int i = 0;i<4;i+=2){
                int type = touchpad_profile_codes[i];
                int code = touchpad_profile_codes[i+1];
                if(!libevdev_has_event_code(platform->touchpad_handle, type, code)) {
                    printf(" does not support event  (T%d, C%d).",
                        type, code);
                    device_approved &= false;
                    break;
                }
            }
        }

        //autodetect number of slots in a multitouch device.
        if(device_approved){
            platform->max_touchpoints = libevdev_get_num_slots(platform->touchpad_handle);
            if(platform->max_touchpoints < 1){
                printf(" Does not have touch slots.");
                device_approved &= false;
            }
        }
        if(!device_approved){
            printf(" Not a touchpad\n.");
            fd = -1;
            continue;
        }
        else
            printf(" is a touchpad.\n");
        touch_point *touch_slots = malloc(sizeof(touch_point) * (platform->max_touchpoints));
        multitouch_report *report = malloc(sizeof(multitouch_report));
        report->touches = touch_slots;
        report->extended_touch_parameters = NULL;
        platform->report = report;

        printf("successfully allocated evdev for a %d-point touchpad\n.", platform->max_touchpoints);
        fflush(stdout);

        //if we got this far, then the hardware is set.
        lua_pushinteger(vm,OMNIGLASS_PLATFORM_EVDEV_INIT_SUCCESS);
        return 1;
    }
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
    for(int evdev_catched_up = 0; evdev_catched_up == 0;){
        int status = libevdev_next_event(dev,LIBEVDEV_READ_FLAG_NORMAL,&ev);
        if( status == -EAGAIN){
            evdev_catched_up = 1;
            break;
        }
        else{
            if(status < 0){
                fprintf(stderr, "Cannot read events from touchpad. Check the device file\n");
                lua_pushstring(vm, "OMNIGLASS_TOUCHPAD_LOST");
                return 1;
            }
        }
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
    return 1;
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


/** (LUA-FACING)
 ** push coordinates of top-right corner into the virtual machine as a point
 */
int platform_get_touchpad_boundaries(lua_State *vm){
    struct platform *platform = luaL_checkudata(vm,1,PLATFORM_CLASS_NAME_META);
    struct libevdev *evdev = platform->touchpad_handle;

    int max_x = 0;
    int max_y = 0;
    max_x = libevdev_get_abs_maximum(evdev, ABS_MT_POSITION_X);
    max_y = libevdev_get_abs_maximum(evdev, ABS_MT_POSITION_Y);
//     printf("c-side dump: %d %d", max_x, max_y);
    lua_newtable(vm);
        lua_pushstring(vm, "max_x");
            lua_pushnumber(vm, (double) max_x);
                lua_settable(vm,-3);
        lua_pushstring(vm, "max_y");
            lua_pushnumber(vm, (double) max_y);
                lua_settable(vm,-3);
    return 1;
}

//platform functions implemented by the linux backend.
static luaL_Reg platform_funcs [] = {
    {"evdev_init", platform_evdev_init},
    {"get_last_report", platform_get_last_report},
    {"parse_events", platform_parse_events},
    {"get_touchpad_boundaries", platform_get_touchpad_boundaries},
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
    luaL_dofile(vm, OMNIGLASS_ENV_LIB_FOLDER "/omniglass_linux.lua");
    int platform_init_status = luaL_checkint(vm, -1);
    if(platform_init_status != OMNIGLASS_PLATFORM_EVDEV_INIT_SUCCESS)
        return OMNIGLASS_PLATFORM_INIT_NO_TOUCHPAD;
    else
        return OMNIGLASS_PLATFORM_INIT_SUCCESS;
}
