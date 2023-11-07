#include "../omniglass.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

void on_edge_slide(double value) {
    if (value < 0)
        printf("moved left\t\t%g\n", value);
    else if (value > 0)
        printf("moved right\t\t%g\n",value);
    fflush(stdout);
}

int main(int argc, char **argv) {
        
    struct omniglass *handle;
    if (omniglass_init(&handle) != OMNIGLASS_RESULT_SUCCESS)
        fprintf(stderr,"could not initialize omniglass.\n");
    omniglass_listen_gesture_edge(handle,&on_edge_slide);
    fflush(stdout);

    printf("starting event loop\n");
    while (true) {
        usleep(1000);
        omniglass_step(handle);
        fflush(stdout);
    }
    
    return 0;
}
