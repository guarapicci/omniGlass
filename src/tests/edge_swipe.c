#include "../omniglass.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

void on_edge_slide(double value, void *data) {
    if (value < 0)
        printf("moved on edge, negative\t\t%g\n", value);
    else if (value > 0)
        printf("moved on edge, positive\t\t%g\n",value);
    fflush(stdout);
}

int main(int argc, char **argv) {
        
    struct omniglass *handle;
    if (omniglass_init(&handle) != OMNIGLASS_RESULT_SUCCESS)
        fprintf(stderr,"could not initialize omniglass.\n");
    omniglass_listen_gesture_edge(handle,&on_edge_slide, OMNIGLASS_EDGE_LEFT, NULL);
    fflush(stdout);

    printf("starting event loop\n");
    while (true) {
        usleep(1000);
        omniglass_step(handle);
        fflush(stdout);
    }
    
    return 0;
}
