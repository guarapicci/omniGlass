#include <stdlib.h>
#include <stdio.h>
#include <omniGlass/constants.h>
#include "../omniglass.h"

int main(int argc, char **argv){
    printf("the init test\n");
    struct omniglass *dummy;
    if (omniglass_init(&dummy) == OMNIGLASS_RESULT_SUCCESS)
        return 0;
    else
        return -1;
}
