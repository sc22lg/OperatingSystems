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

        if (currentChar != ' ') {
            buffer[bufferIndex] = currentChar;
            bufferIndex++;
            inWord = 1;
        } else if (inWord) {
            buffer[bufferIndex] = '\0';  // Null-terminate the word
            for (int j = 0; buffer[j] != '\0'; j++) {
                argv[wordCount][j] = buffer[j];
                
            }
            argv[wordCount][bufferIndex] = '\0';  // Null-terminate the word in the result array
            wordCount++;
            bufferIndex = 0;
            inWord = 0;
        }
        b++;

    }

    if (inWord) {
        // Handle the last word if it's not followed by a space
        buffer[bufferIndex] = '\0';  // Null-terminate the last word
        for (int j = 0; buffer[j] != '\0'; j++) {
            argv[wordCount][j] = buffer[j];
        }
        argv[wordCount][bufferIndex] = '\0';  // Null-terminate the last word in the result array
        wordCount++;
    }
    
    return wordCount;

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

        char **argv = malloc2dCharArray(MAXARG, MAX_WORD_LENGTH);
        int argc;
        argc = parseUserInput(b, argv);
        
        // Print the separated words
        for (int i = 0; i < argc; i++) {
            printf("Word %d: %s\n", i + 1, argv[i]);
        }

        printf("number of words: %d\n", argc);

        //execute the command

        int f = fork();
        if (f < 0) {

            fprintf(2, "fork failed\n");
            exit(1);

        } 
        else if (f == 0){
            
            fprintf(2, "argv[0]=%s", argv[0]);
            fprintf(2, "argv[1]=%s", argv[1]);
            fprintf(2, "argv[2]=%s", argv[2]);
            fprintf(2, "argv[3]=%s", argv[3]);
            int temp = exec(argv[0], argv);
            printf("temp: %d\n", temp);

        }
        else{
            wait(0);
        }
        


    }
    return 0;
}
