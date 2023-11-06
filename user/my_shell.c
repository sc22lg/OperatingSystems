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
        temp = exec(buff, arguments);
    }
    else{
        wait(0);
    }

    return temp;
}

int pipeThis(int numWords, char **argv){

    //find where the pipe is
    int pipeIndex = 0;
    for(int i = 0; i < numWords; i++){
        if(strcmp(argv[i], "|")==0){
            pipeIndex = i;
        }  
    }


    //allocate memory for the words on each side
    char *leftArgs[MAX_WORD_LENGTH];
    char *rightArgs[MAX_WORD_LENGTH];

    //copy the words into the correct arrays depending on which side of the pipe the are on
    for(int i = 0; i < numWords; i++){

        if(strcmp(argv[i], "|") != 0){
            if(i < pipeIndex){
                leftArgs[i] = argv[i];
                //printf("left: %s\n", leftArgs[i]);
            }
            else if(i > pipeIndex){
                rightArgs[i - pipeIndex - 1] = argv[i];
                //printf("right: %s\n", rightArgs[i - pipeIndex - 1]);
            }

        }
    }


    int p[2];

    pipe(p);
    //child to run leftArgs
    if(fork() == 0){
        //execute command will write to the pipe
        close(1);
        dup(p[1]);
        close(p[1]);
        close(p[0]);
        executeCommand(leftArgs[0], leftArgs);
        exit(0);
    }
    //parent to run rightArgs
    else{
        //execute command will read from the pipe
        close(0);
        dup(p[0]);
        close(p[1]);
        close(p[0]);
        wait(0);
        executeCommand(rightArgs[0], rightArgs);
        
    }


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

    if(pipeCount == 1){
        pipeThis(numWords, argv);
    }
    else if(pipeCount >= 1){

        //create a new set of words, leaving out the command after the last pipe
        char **newArgs = malloc2dCharArray(lastPipeIndex, MAX_WORD_LENGTH);
        for(int i = 0; i < lastPipeIndex; i++){
            strcpy(newArgs[i], argv[i]);
        }
        //send the new arguments and its length to this function recursively
        pipeChain(lastPipeIndex-1, newArgs);
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
        printf("thanks for your user input :)");

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
