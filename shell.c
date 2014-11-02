/*
	Based on code from http://dumbified.wordpress.com/2010/04/25/how-to-write-a-shell-in-c-using-fork-and-execv/
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>

#define BUFFSIZE 512
#define DEBUG false
#define DELIMITERS " \t"
#define SUCCESS 0
#define FAILURE 1

//struct for built-in shell functions
struct builtin {
    const char* label;
    void (*op)(char*);  //function pointer to built-in's operation function
};

int countArgs(char* buffer) {

    int word_count = 0;
    bool inword = false;
    const char* args = buffer;

    do switch(*args) {
        case '\0':
        case ' ': case '\t':
           if(inword) { inword = false; word_count++; }
           break;
        default: inword = true;
    } while(*args++);

#if DEBUG
    printf("countArgs: Buffer contents\n");
    puts(buffer);
    printf("\ncountArgs: %d arguments\n", word_count);
#endif
    return word_count;
}

void parse(char* buffer, char** arguments) {

    char* parsed = strtok(buffer, DELIMITERS);  //Break buffer into words
    char* ch;
    int i = 0;  //Index for char** arguments

    while (parsed != NULL) {
        ch = strrchr(parsed,'\n');  //Find last occurance of newline.
         if(ch) {
            *ch = 0;  //Remove newline character
        }
        arguments[i] = parsed;

#if DEBUG
    printf("parse: parsed pointer: *%s*\n", parsed);
    printf("parse: Contents of arguments at index %d\n",i);
    puts(arguments[i]);
#endif
        i++;
        parsed = strtok(NULL, DELIMITERS);  //Increment to next word
    }
}

bool check_builtins(struct builtin* bfunc, char* buffer, int bfunc_size) {
    //returns true if buffer contains a built-in function, false otherwise.
    char arg[BUFFSIZE];
    strcpy(arg, buffer);
    int i;

    for(i = 0; i < bfunc_size; i++) {
        //note: arg has a newline character on the end.
        if(strlen(arg)== strlen((bfunc[i]).label)+1) {
            arg[strlen((bfunc[i]).label)] = 0;   //truncate arg
            if (strcmp((bfunc[i]).label, arg) == 0){
                (bfunc[i]).op(buffer);
                return true;
            }
        }
    }
    return false;
}

void close_shell() {
    exit(SUCCESS);
}

void cd(char* buffer) {
    //incomplete
    printf("cd not yet implemented.\n");
}

int main(int argc, char **argv) {

    //bfunc[] is an array of built-in functions that MUST be
    //ordered from longest to shortest in terms of it's label
    struct builtin bfunc[] = {
        {.label = "exit", .op = &close_shell},
        {.label = "cd", .op = &cd}
    };
    int bfunc_size = (int) (sizeof(bfunc)/sizeof(bfunc[0]));

    //buffer is to hold user commands
    char buffer[BUFFSIZE] = { 0 };  //zero every elerment of the buffer
    char* path = "/bin/";

    while(1)
    {
        //print the prompt
        printf("myShell&gt: ");

        fgets(buffer, BUFFSIZE, stdin);
        if(!check_builtins(bfunc, buffer, bfunc_size)) {
            int pid = fork();

            if(pid < 0) {
                fprintf(stderr, "Unable to fork new process.\n");
            }
            if(pid > 0) {
                //Parent code
                wait(NULL);
            }
            if(pid == 0) {
                //Child code
                int num_of_args = countArgs(buffer);
                //arguments to be passed to execv
                char *arguments[num_of_args+1];
                parse(buffer, arguments);

                //Requirement of execv
                arguments[num_of_args] = NULL;

                char prog[BUFFSIZE];
                strcpy(prog, path);

                //Concancate the program name to path
                strcat(prog, arguments[0]);
                execv(prog, arguments);

                //Following will only run if execv fails
                fprintf(stderr, "%s: Command not found.\n",arguments[0]);
                return FAILURE;
            }
        }
    }
    return SUCCESS;
}