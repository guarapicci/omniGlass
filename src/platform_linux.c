#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "lua5.1/lua.h"
#include "platform.h"

#include <linux/input.h>
//this is not the path shown by the official example,
//but it is the actual path for both debian, arch and SUSE.
#include "libevdev-1.0/libevdev/libevdev.h"


/** handle for the library, in evdev implementation */
struct omniglass{
    struct libevdev *touchpad_handle; /**<the evdev-specific handle to the touchpad device*/
    struct multitouch_report *report;
    int max_touchpoints;
};

void point_tracking_step(multitouch_report *status){
    
};

/**
initialization function that detects a touchpad and sets up datastructures and functions needed by the lua VM.
*/
void platform_evdev_init(struct omniglass *state, char *touchpad_file_path){
    /**start evdev*/
    printf("opening event file: %s\n", touchpad_file_path);
    int fd = open(touchpad_file_path,O_RDONLY | O_NONBLOCK);
    int rc = 1;
    rc = libevdev_new_from_fd(fd, &(state->touchpad_handle));
    if(rc<0) {
        fprintf(stderr, "failed to initialize libevdev (%s)\n", strerror(-rc));
    }
    printf("Input device name: \"%s\"\n", libevdev_get_name(state->touchpad_handle));

    /**diagnostics: check if selected device does have touchpad-like event reporting*/
    int touchpad_profile_types[] = {EV_SYN, EV_KEY, EV_ABS};
    int touchpad_profile_codes[] = {
        EV_ABS,ABS_X,
        EV_ABS,ABS_Y,
        EV_ABS,ABS_MT_SLOT,
        EV_ABS,ABS_MT_TRACKING_ID};
    for (int i = 0;i<3;i++){
        if(!libevdev_has_event_type(state->touchpad_handle, touchpad_profile_types[i])) {
            printf("device at %s does not support event code %d. omniGlass will not consider it a touchpad.\n",touchpad_file_path, touchpad_profile_types[i]);
            return;
        }
    }
    for (int i = 0;i<4;i+=2){
        if(!libevdev_has_event_code(state->touchpad_handle, touchpad_profile_codes[i], touchpad_profile_codes[i+1])) {
            printf("device at %s does not support event code %d. omniGlass will not consider it a touchpad\n.",touchpad_file_path, touchpad_profile_codes[i]);
            return;
        }
    }
    printf("the device selected is considered a touchpad.\n");
    
    //autodetect number of slots in a multitouch device.
    state->max_touchpoints = libevdev_get_num_slots(state->touchpad_handle);
    touch_point *touch_slots = malloc(sizeof(touch_point) * (state->max_touchpoints));
    multitouch_report *report = malloc(sizeof(multitouch_report));
    report->touches = touch_slots;
    report->extended_touch_parameters = NULL;
    state->report = report;
    
    printf("successfully allocated evdev: %d \n.", state->max_touchpoints);
    fflush(stdout);
}

/**
 * this function is where the linux touchpad implementation gets its touch points from the evdev interface
 */
int platform_evdev_parse_events(struct omniglass *state){
    // printf("touch reporting.\n");
    struct input_event ev;
    
    struct libevdev *dev = state->touchpad_handle;
    int slot_count= state->max_touchpoints;
    struct multitouch_report *report = state->report;
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

int main (int argc, char ** argv){
    struct omniglass *state = malloc(sizeof(struct omniglass));
    platform_evdev_init(state, argv[1]);
    
    while(true){
        if(!platform_evdev_parse_events(state)){
            for(int i=0;i<state->max_touchpoints;i++){
                printf("touch%d:",i);
                if(state->report->touches[i].touched==0)
                    printf("(lifted) ");
                else{
                    printf("(%d,%d) ",state->report->touches[i].x,state->report->touches[i].y);
                }
            }
            printf("\n");
            fflush(stdout);
        }
        
    }
    
    
}
