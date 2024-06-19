#include "../omniglass.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>


struct state {
    int elapsed;
    int stars;
};


void on_touches_changed(omniglass_raw_report *report, void *passthrough){
    struct state *ext_state = passthrough;
    ext_state->elapsed++;
    printf("(%d) touch change detected. touches are at :", (ext_state->elapsed));
    for (int i = 0; i < report->points_max; i++){
        omniglass_raw_touchpoint *current_point = &(report->points[i]);
        if (current_point->is_touching)
            printf("(%f,%f)",current_point->x, current_point->y);
        else
            printf("(no touch)");
        if (i < (report->points_max - 1))
            printf(",");
    }
    printf("\n");
}

int main(int argc, char **argv) {
    struct omniglass *handle;
    struct state top_state = {
        0,
        0
    };
    if (omniglass_init(&handle) != OMNIGLASS_RESULT_SUCCESS){
        fprintf(stderr,"could not initialize omniglass.\n");
        return -1;
    }
    omniglass_listen_gesture_touches_changed(handle, on_touches_changed, &top_state);
    fflush(stdout);

    printf("starting event loop\n");
    while (true) {
        usleep(1000);
        omniglass_step(handle);
        fflush(stdout);
    }
    
    return 0;
}
