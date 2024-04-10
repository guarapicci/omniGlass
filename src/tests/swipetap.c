#include "../omniglass.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

void on_slide(double x, double y) {
    if (x < 0)
        printf("moved left\t%g", x);
    else if (x > 0)
        printf("moved right\t%g",x);
    if (y > 0)
        printf(",\tmoved down\t%g",y);
    else if (y < 0)
        printf(",\tmoved up\t%g",y);
    printf("\n");
    fflush(stdout);
}

int main(int argc, char **argv) {
        
    struct omniglass *handle;
    if (omniglass_init(&handle) != OMNIGLASS_RESULT_SUCCESS)
        fprintf(stderr,"could not initialize omniglass.\n");
    omniglass_listen_gesture_slide(handle,&on_slide);
    fflush(stdout);
    //8 milliseconds polling rate for touchpad events
    struct timespec poll_rate = {0, 8000000};
    printf("starting event loop\n");
    while (true) {
        omniglass_step(handle);
    }
    
    return 0;
}
