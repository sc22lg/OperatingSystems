#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int main(void) {
    //make a whole shell :)
    
    int shell_running = 1;

    //setup our input buffer
    char in_buff[512], *b;
    for(int i = 0; i < sizeof(in_buff); i++){
        in_buff[i] = '\0';
    }

    b = in_buff;

    //a loop which asks for a command then executes the user input
    while(shell_running == 1){

        //take an input from the user
        printf(">>>");
        read(0, b, sizeof(in_buff));

        while(*b != '\0'){
            printf("%c", *b);
            b++;
        }
        

    }
    return 0;
}
