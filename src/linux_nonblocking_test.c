#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
int main(int argc, char **argv){

    int fd = open("dapipe",O_NONBLOCK);
    char snartum[1024];
    printf("pipe has been opened for reading.\n");
    fflush(stdout);
    while(true){
        int result = read(fd,snartum,1024);
        fflush(stdin);
        if(result == EAGAIN ){
            printf("no input.");
        }
        else{
            for (int i=0;i<result;i++){
                printf("*");
            }
        }
        usleep(600000);
        fflush(stdout);
    }
}
