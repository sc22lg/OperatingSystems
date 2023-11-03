#include "kernel/types.h"
#include "user/user.h"


int main(void){

    int p[2];
    //open a pipe on p
    pipe(p);
    int forkid = fork();

    //determine if this is the parent or child
    if(forkid != 0){ //parent

        char data = 'P';
        int pid = getpid();

        //write one byte
        write(p[1], &data, 1);
        //wait for child process to finish
        wait(0);
        //read data from the pipe and print
        read(p[0], &data, 1);
        printf("%d: revived pong, %c\n", pid, data);

        exit(0);

    }
    else{//child

        char data;
        int pid = getpid();

        //read 1 byte from it to data
        read(p[0], &data, 1);

        //print confirmation and data recieved
        printf("%d: recived ping, %c\n", pid, data);

        //change data and write back to the pipe
        data = 'R';
        write(p[1], &data, 1);

        exit(0);

    }

    exit(0);
}