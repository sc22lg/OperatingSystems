#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAX_WORD_LENGTH 50

char **malloc2dCharArray(int dim1, int dim2){

    char **cpp = malloc(dim1 * sizeof(char *));      // Allocate the row pointers
    cpp[0] = malloc(dim1 * dim2 * sizeof(char)); // Allocate all the elements
    for(int i = 1; i < dim1; i++)
    cpp[i] = cpp[0] + i * dim2;

    return cpp;

}

int parseUserInput(char *b, char **argv) {

    //write each word into argv
    int wordCount = 0;
    char buffer[MAX_WORD_LENGTH];
    int bufferIndex = 0;
    int inWord = 0;  // Flag to track if we are inside a word

    while(*b != '\0'){
        //printf("%c", *b);

        char currentChar = *b;

        if (currentChar != ' ' && currentChar != '\n' && currentChar != '\0') {
            buffer[bufferIndex] = currentChar;
            bufferIndex++;
            inWord = 1;
        } else if (inWord) {
            buffer[bufferIndex] = '\0';  // Null-terminate the word
            strcpy(argv[wordCount], buffer);

            wordCount++;
            bufferIndex = 0;
            inWord = 0;

        }
        b++;

    }

    if (inWord) {
        // Handle the last word if it's not followed by a space
        buffer[bufferIndex] = '\0';  // Null-terminate the last word
        strcpy(argv[wordCount], buffer);
        wordCount++;
    }

    return wordCount;

}

int executeCommand(char* cmd, char *arguments[]){

    //put a '/' before the command
    char *buff;
    buff = (char *)malloc(strlen(cmd) + 1);
    buff[0] = '/';
    strcpy(buff + 1, cmd);

    int temp = 0;
    int f = fork();
    if (f < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    } 
    else if (f == 0){
        printf("arguments before exec: %s, %s\n", arguments[0], arguments[1]);
        temp = exec(buff, arguments);
    }
    else{
        wait(0);
    }

    return temp;
}

int pipeThis(int numWords, char **argv){

    printf("recurring\n");

    //find where the rightmost pipe is
    int pipeIndex = 0;
    for(int i = numWords; i > 0; i--){
        if(strcmp(argv[i], "|")==0){
            pipeIndex = i;
            break;
        }  
    }


    //allocate memory for the words on each side
    char *leftArgs[MAX_WORD_LENGTH];
    char *rightArgs[MAX_WORD_LENGTH];

    int lastPipe = 1;

    //copy the words into the correct arrays depending on which side of the pipe the are on
    for(int i = 0; i < numWords; i++){

        if(i < pipeIndex){
            leftArgs[i] = argv[i];
            printf("left got: %s\n", leftArgs[i]);
            //check if the left args have a pipe in them
            if(strcmp(argv[i], "|") == 0){
                lastPipe = 0;
            }
            //printf("left: %s\n", leftArgs[i]);
        }
        else if(i > pipeIndex){
            rightArgs[i - pipeIndex - 1] = argv[i];
            printf("right got: %s\n", rightArgs[i - pipeIndex - 1]);
        }

    }
    if(lastPipe == 0){
        printf("sending: %s,%s,%s,%s,%s\n", leftArgs[0], leftArgs[1], leftArgs[2], leftArgs[3], leftArgs[4]);
        pipeThis(pipeIndex, leftArgs);
    }

    int p[2];

    printf("arguments before fork: %s, %s, %s, %s\n", leftArgs[0], leftArgs[1], rightArgs[0], rightArgs[1]);
    //cat README | grep the | grep bug
    pipe(p);
    if(fork() == 0) {
        close(1);
        dup(p[1]);
        close(p[1]);
        close(p[0]);
        executeCommand(leftArgs[0], leftArgs);
        exit(0);
    } 
    if(fork() == 0){
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      executeCommand(rightArgs[0], rightArgs);
      exit(0);
    }

    close(p[0]);
    close(p[1]);
    wait(0);
    wait(0);

    return 0;

}

/*int pipeChain(int numWords, char **argv){

    //find the position of the last pipe and how many pipes there are
    int lastPipeIndex = 0;
    int pipeCount = 0;
    for(int i = 0; i < numWords; i++){
        if(strcmp(argv[i], "|")==0){
            pipeCount++;
            lastPipeIndex = i;
        }  
    }
    printf("pipe count: %d\n", pipeCount);

    if(pipeCount == 1){
        printf("arguments before pipe function: %s, %s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3], argv[4]);
        pipeThis(numWords, argv);
    }
    else if(pipeCount >= 1){

        //create a new set of words, leaving out the command after the last pipe
        char **newArgs = malloc2dCharArray(lastPipeIndex, MAX_WORD_LENGTH);
        for(int i = 0; i < lastPipeIndex; i++){
            strcpy(newArgs[i], argv[i]);
            printf("string: %s\n", newArgs[i]);
        }
        //send the new arguments and its length to this function recursively
        pipeChain(lastPipeIndex, newArgs);
    }

    return 0;
}*/

int main(void) {
    //make a whole shell :)
    
    int shell_running = 1;

    //setup our input buffer
    char in_buff[512], *b;
    b = in_buff;

    //a loop which asks for a command then executes the user input
    while(shell_running == 1){

        //clear the input buffer
        for(int i = 0; i < sizeof(in_buff); i++){
            in_buff[i] = '\0';
        }

        //take an input from the user
        printf(">>>");
        read(0, b, sizeof(in_buff));
        printf("Thank you for your user input\n");

        //get the individual words from user input
        char **argv = malloc2dCharArray(MAXARG, MAX_WORD_LENGTH);
        int argc;
        argc = parseUserInput(b, argv);
        
        int containsPipe = 0;
        char *arguments[50]; 
        // Put the separated words into arguments
        for (int i = 0; i < argc; i++) {
            arguments[i] = argv[i];
            //printf("argument: %s\n", arguments[i]);
            //check if arguments contains a pipe
            if(strcmp(arguments[i], "|")==0){
                containsPipe = 1;
            }
        }

        int err = 0;
        //handle the special case for cd
        if(strcmp(argv[0], "cd")==0){
            chdir(arguments[1]);
        }
        else if(strcmp(argv[0], "exit")==0){
            shell_running = 0;
        }
        //handles pipes
        else if(containsPipe == 1){
            //err = pipeChain(argc, arguments);
            err = pipeThis(argc, arguments);
        }
        //for when theres no pipes
        else{
            err = executeCommand(argv[0], arguments);
        }
        printf("exec: %d\n", err);

        //emptys arguments
        for(int i = 0; i < argc; i++){
            for(int j = 0; arguments[i][j] != '\0'; j++){
                arguments[i][j] = '\0';
            }
        }

    }
    return 0;
}
