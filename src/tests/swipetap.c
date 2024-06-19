#include "../omniglass.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct app_data {
        int counter;
        char *message;
    } app_data;

void on_slide(double x, double y, void *passthrough) {
    app_data *passed_data = passthrough;
    if (x < 0)
        printf("moved left\t%g", x);
    else if (x > 0)
        printf("moved right\t%g",x);
    if (y > 0)
        printf(",\tmoved down\t%g",y);
    else if (y < 0)
        printf(",\tmoved up\t%g",y);
    printf("\n");
    passed_data->counter++;
    if ( (passed_data->counter) % 10 == 0)
        printf("%s\n", passed_data->message);
    fflush(stdout);
}

int main(int argc, char **argv) {

    app_data somedata = {
        0,
        "10 reports have passed."
    };
        
    struct omniglass *handle;
    if (omniglass_init(&handle) != OMNIGLASS_RESULT_SUCCESS){
        fprintf(stderr,"could not initialize omniglass.\n");
        return -1;
    }
    omniglass_listen_gesture_slide(handle,&on_slide, &somedata);
    fflush(stdout);
    //8 milliseconds polling rate for touchpad events
    struct timespec poll_rate = {0, 8000000};
    printf("starting event loop\n");
    while (true) {
        omniglass_step(handle);
    }
    
    return 0;
}
