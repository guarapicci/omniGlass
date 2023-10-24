#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>

#include "platform.h"

#include <linux/input.h>
//this is not the path shown by the official example,
//but it is the actual path for both debian, arch and SUSE.
#include "libevdev-1.0/libevdev/libevdev.h"

#define PLATFORM_CLASS_NAME "platform"

/** all state for the linux platform*/
struct platform{
    struct libevdev *touchpad_handle; /**<the evdev-specific handle to the touchpad device*/
    struct multitouch_report *report; /**<the latest set of touch contacts checked for*/
    int max_touchpoints; /**<how many simultaneous touches the device can track.*/
};

struct init_config {
    char *touchpad_file_path;
};

/**
initialization function that detects a touchpad and sets up datastructures and functions needed by the lua VM.
*/
int platform_evdev_init(lua_State *vm){
    
    //get parameters: platform state pointer and touchpad device file path
    struct platform *platform = luaL_checkudata(vm,1,PLATFORM_CLASS_NAME);
    char *touchpad_file_path = luaL_checkstring(vm,2);
    
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
    return 0;
}

/**
 * this function is where the linux touchpad implementation gets its touch points from the evdev interface
 */
int platform_parse_events(struct platform *platform){
    // printf("touch reporting.\n");
    struct input_event ev;
    
    struct libevdev *dev = platform->touchpad_handle;
    int slot_count= platform->max_touchpoints;
    struct multitouch_report *report = platform->report;
    //pull next input event
    if((libevdev_next_event(dev,LIBEVDEV_READ_FLAG_NORMAL,&ev)) != LIBEVDEV_READ_STATUS_SUCCESS){
        //no success means no points available..
        return -1;
    }

    /**fetch current touch position for all multitouch slots.*/
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
    
    return 0;
};

static luaL_reg plaftorm_funcs [] = {
    {"evdev_init",platform_evdev_init},
    {NULL,NULL}
};

int platform_init(struct platform **handle, lua_State *vm){
    struct platform *new = malloc(sizeof(struct platform));
    if(new==NULL)
        return ENOMEM;
    *handle = new;
    
    //add evdev-specific functionality to lua
    luaL_newmetatable(vm,PLATFORM_CLASS_NAME);
    
    lua_pushlightuserdata(vm, (*handle));
    lua_pushstring(vm,"__index");
    luaL_register(vm,NULL,)
    
    //run the real configuration process as a lua script.
    luaL_dofile(vm, "omniglass_linux.lua");
    
    struct init_config cfg;
    cfg.touchpad_file_path = luaL_checkstring(vm,1);
    platform_evdev_init(*handle, cfg.touchpad_file_path);
    
    return 0;
}

int main (int argc, char ** argv){
    struct platform *platform;
    platform_evdev_init(platform, argv[1]);
    
    while(true){
        if(!platform_parse_events(platform)){
            for(int i=0;i<platform->max_touchpoints;i++){
                printf("touch%d:",i);
                if(platform->report->touches[i].touched==0)
                    printf("(lifted) ");
                else{
                    printf("(%d,%d) ",platform->report->touches[i].x,platform->report->touches[i].y);
                }
            }
            printf("\n");
            fflush(stdout);
        }
        
    }
    
    
}
