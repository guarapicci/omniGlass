#include <stdlib.h>
#include <stdio.h>
#include "../omniglass.h"

int main(int argc, char **argv){
    printf("the init test\n");
    struct omniglass *dummy;
    omniglass_init(&dummy);
    return 0;
}
