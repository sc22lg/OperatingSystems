#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAX_WORD_LENGTH 50

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

        //write each word into argv
        char argv[MAXARG][50];
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
        
        // Print the separated words
        for (int i = 0; i < wordCount; i++) {
            printf("Word %d: %s\n", i + 1, argv[i]);
        }

        int argc = wordCount;
        printf("%d\n", argc);

    }
    return 0;
}
