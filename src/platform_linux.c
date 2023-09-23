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

void point_tracking_step(multitouch_report status){
    
};

//this function is where the linux touchpad implementation gets its touch points from the evdev interface
int parse_events(struct libevdev *dev, multitouch_report *report){
    // printf("touch reporting.\n");
    struct input_event ev;
    
    //pull next input event
    if((libevdev_next_event(dev,LIBEVDEV_READ_FLAG_NORMAL,&ev)) != LIBEVDEV_READ_STATUS_SUCCESS){
        //no success means no points available..
        return -1;
    }

    //HACK: hardwired 2-point multitouch. Must fix it to a "maxtouch" limit detected per-device.
    //get coordinates and tracking status of all multitouch slots.
    for (int i=0;i<2;i++){
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
    struct libevdev *dev = NULL;
    printf("opening event file: %s\n", argv[1]);
    int fd = open(argv[1],O_RDONLY | O_NONBLOCK);
    int rc = 1;
    rc = libevdev_new_from_fd(fd, &dev);
    if(rc<0) {
        fprintf(stderr, "failed to initialize libevdev (%s)\n", strerror(-rc));
    }
    printf("Input device name: \"%s\"\n", libevdev_get_name(dev));

    //diagnostics: check if selected device does have touchpad-like event reporting
    int touchpad_profile_types[] = {EV_SYN, EV_KEY, EV_ABS};
    int touchpad_profile_codes[] = {
        EV_ABS,ABS_X,
        EV_ABS,ABS_Y,
        EV_ABS,ABS_MT_SLOT,
        EV_ABS,ABS_MT_TRACKING_ID};
    for (int i = 0;i<3;i++){
        if(!libevdev_has_event_type(dev, touchpad_profile_types[i])) {
            printf("device at %s does not support event code %d. omniGlass will not consider it a touchpad.\n",argv[1], touchpad_profile_types[i]);
            return -1;
        }
    }
    for (int i = 0;i<4;i+=2){
        if(!libevdev_has_event_code(dev, touchpad_profile_codes[i], touchpad_profile_codes[i+1])) {
            printf("device at %s does not support event code %d. omniGlass will not consider it a touchpad\n.",argv[1], touchpad_profile_codes[i]);
            return -1;
        }
    }
    printf("the device selected is considered a touchpad.\n");
    
    //bad habit: presume touchpad has at least 2-point multitouch.
    touch_point *touch_pair = malloc(sizeof(touch_point) * 2);
    multitouch_report *report = malloc(sizeof(multitouch_report));
    report->touches = touch_pair;
    report->extended_touch_parameters = NULL;
    
    
    while(true){
        if(!parse_events(dev, report)){
            for(int i=0;i<2;i++){
                printf("touch%d:",i);
                if(report->touches[i].touched==0)
                    printf("(lifted) ");
                else{
                    printf("(%d,%d) ",report->touches[i].x,report->touches[i].y);
                }
            }
            printf("\n");
            fflush(stdout);
        }
        
    }
    
    
}
