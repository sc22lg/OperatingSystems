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
        printf("buffer:%s<-\n", buffer);
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

        //get the individual words from user input
        char **argv = malloc2dCharArray(MAXARG, MAX_WORD_LENGTH);
        int argc;
        argc = parseUserInput(b, argv);
        
        char *arguments[50]; 
        // Put the separated words into arguments
        for (int i = 0; i < argc; i++) {
            //printf("Word %d: %s\n", i + 1, argv[i]);
            arguments[i] = argv[i];
            printf("argument: %s\n", arguments[i]);
            
        }

        //printf("number of words: %d\n", argc);
        int err = 0;
        //handle the special case for cd
        if(strcmp(argv[0], "cd")==0){
            chdir(arguments[1]);
        }
        else{
            err = executeCommand(argv[0], arguments);
        }
        //err = executeCommand(argv[0], arguments);
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
