#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAX_WORD_LENGTH 30

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

void executeCommand(char* cmd, char **argv){

    // put a '/' before the command
    char *buff;
    buff = (char *)malloc(strlen(cmd) + 1);
    buff[0] = '/';
    strcpy(buff + 1, cmd);
    argv[0] = buff;
    
    // char *argv[MAX_WORD_SIZE];

    exec(argv[0], argv);
    exit(0);

}

void pipeThis(int numWords, char **argv){

    int p[2];

    //find where the rightmost pipe is
    int pipeIndex = 0;
    for(int i = numWords; i > 0; i--){
        if(strcmp(argv[i], "|")==0){
            pipeIndex = i;
            break;
        }  
    }


    //if there is a pipe
    if(pipeIndex != 0){
        //allocate memory for the words on each side
        char *leftArgs[MAX_WORD_LENGTH];
        char *rightArgs[MAX_WORD_LENGTH];

        //copy the words into the correct arrays depending on which side of the pipe the are on
        for(int i = 0; i < numWords; i++){

            if(i < pipeIndex){
                leftArgs[i] = argv[i];
            }
            else if(i > pipeIndex){
                rightArgs[i - pipeIndex - 1] = argv[i];
            }

        }

        //cat README | grep the | grep bug
        //cat README | grep m | grep p | grep i
        pipe(p);

        if(fork() == 0) {
            close(1);
            dup(p[1]);
            close(p[0]);
            close(p[1]);
            pipeThis(pipeIndex, leftArgs);

        } 
        if(fork() == 0){
            close(0);
            dup(p[0]);
            close(p[0]);
            close(p[1]);
            pipeThis(pipeIndex, rightArgs);

        }


        close(p[0]);
        close(p[1]);
        wait(0);
        wait(0);

        exit(0);

    }
    else{//there is no pipe
        executeCommand(argv[0], argv);
    }

}

int editInput(int argc, char **argv){

    int newArgc = argc;

    for (int i = 0; i < argc; i++) {
        if(strcmp(argv[i], "<")==0){
            argv[i] = argv[i+1];
            argv[i+1] = "|";
            argv[i+2] = argv[i-1];
            argv[i-1] = "cat";
            newArgc = argc + 1;
        }
    }

    return newArgc;
}

void redirect(int numWords, char **argv){

    //find location and direction of redirect
    //int redirectLocation = 0;
    int redirectType = -1;
    for(int i = 0; i < numWords; i++){
        if (strcmp(argv[i], ">")){
            //redirectLocation = i;
            redirectType = 1;
        }

    }


    if(redirectType == 0){

        int fd = open(argv[numWords-1],O_CREATE | O_TRUNC | O_WRONLY);
        if(fork()==0){

            close(1);
            dup(fd);
            close(fd);
            for(int i = 1; i < numWords; i++){
                argv[i] = 0;
            }
            executeCommand(argv[0], argv);
            exit(1);
        }


    }
    
}

int main(void) {
    //make a whole shell :)

    int shell_running = 1;

    //setup our input buffer
    char in_buff[1028], *b;
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

        //get the individual words from user input
        char **argv_temp = malloc2dCharArray(MAXARG, MAX_WORD_LENGTH);
        int argc;
        argc = parseUserInput(b, argv_temp);

        char **argv = malloc(argc * sizeof(char*));
        for(int i = 0; i < argc; i++){
            argv[i] = malloc(sizeof(argv_temp[i]) * sizeof(char));
            strcpy(argv[i], argv_temp[i]);
        }

        //swaps < for a pipe version
        argc = editInput(argc, argv);
        
        int containsPipe = 0;
        int containsRedirect = 0;
 
        // Put the separated words into argv
        for (int i = 0; i < argc; i++) {
            //check if argv contains a pipe
            if(strcmp(argv[i], "|")==0){
                containsPipe = 1;
            }
            else if(strcmp(argv[i], ">")==0 || strcmp(argv[i], "<")==0){
                containsRedirect= 1;
            }
        }

        //handle the special case for cd
        if(strcmp(argv[0], "cd")==0){
            chdir(argv[1]);
        }
        else if(strcmp(argv[0], "exit")==0){
            shell_running = 0;
        }
        else if(containsRedirect== 1){
            redirect(argc, argv);
        }
        //handles pipes
        else if(containsPipe == 1){
            if(fork()==0){
                pipeThis(argc, argv);
            }wait(0);
        }
        //for when theres no pipes
        else{
            if(fork() == 0)
                executeCommand(argv[0], argv);
            wait(0);
        }

        //emptys argv
        /*for(int i = 0; i < argc; i++){
            for(int j = 0; argv[i][j] != '\0'; j++){
                argv[i][j] = '\0';
            }
        }*/

    }

    exit(0);
}
