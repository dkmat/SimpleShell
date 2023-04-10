#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMDLINE_MAX 512

/*
This file contains all the functions used to organize 
the different features of the shell program.
*/

int start();
void command(char *cmd);

int start(){
    char cmd[CMDLINE_MAX];

        
    while (1) {
        char *nl;
        //int retval;

        /* Print prompt */
        printf("sshell$ ");
        fflush(stdout);

        /* Get command line */
        fgets(cmd, CMDLINE_MAX, stdin);

        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO)) {
            printf("%s", cmd);
            fflush(stdout);
        }

         /* Remove trailing newline from command line */
        nl = strchr(cmd, '\n');
        if (nl)
                *nl = '\0';

         /* Builtin command */
        if (!strcmp(cmd, "exit")) {
                fprintf(stderr, "Bye...\n");
                break;
        }

         /* Regular command */
        command(cmd);
    }
    return EXIT_SUCCESS;
}

void command(char* cmd){
    //strtok()
    pid_t pid;
    char *itr;
    char * args[]={};
    fprintf(stderr, "+ completed '%s': '['%d']'\n",
            cmd, 0/*exit status*/);
}
#endif