#include "../omniglass.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>


void edge_slide_report(double value, char *flavour_text) {
    if (value < 0)
        printf("moved on %s edge, negative\t\t%g\n", flavour_text, value);
    else if (value > 0)
        printf("moved on %s edge, positive\t\t%g\n", flavour_text, value);
    fflush(stdout);
}

void on_edge_slide_left(double value, void *data){
    edge_slide_report(value, "left");
}
void on_edge_slide_bottom(double value, void *data) {
    edge_slide_report(value, "bottom");
}
void on_edge_slide_top(double value, void *data) {
    edge_slide_report(value, "top");
}
void on_edge_slide_right(double value, void *data) {
    edge_slide_report(value, "right");
}
int main(int argc, char **argv) {
        
    struct omniglass *handle;
    if (omniglass_init(&handle) != OMNIGLASS_RESULT_SUCCESS){
        fprintf(stderr,"could not initialize omniglass.\n");
        return -1;
    }
    omniglass_listen_gesture_edge_left(handle, on_edge_slide_left, NULL);
    omniglass_listen_gesture_edge_bottom(handle, on_edge_slide_bottom, NULL);
    omniglass_listen_gesture_edge_top(handle, on_edge_slide_top, NULL);
    omniglass_listen_gesture_edge_right(handle, on_edge_slide_right, NULL);
    fflush(stdout);

    printf("starting event loop\n");
    while (true) {
        usleep(1000);
        omniglass_step(handle);
        fflush(stdout);
    }
    
    return 0;
}
