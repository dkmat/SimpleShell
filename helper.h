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

void start();
void command(char *cmd);

void start(){
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
}

void command(char* cmd){
    
    pid_t pid;
    int retval;
    char * args[]={cmd};
    char * tok1 = strtok(cmd," ");
    pid = fork();
    if(pid==0){
        retval = execlp(cmd,args);
        fprintf(stderr, "+ completed '%s': [%d]\n",
            cmd, retval/*exit status*/);
    }
    else if(pid>0){
        int status;
        waitpid(pid,&status,0);
        retval = EXIT_SUCCESS;
        fprintf(stderr, "+ completed '%s': [%d]\n",
            cmd, retval/*exit status*/);
    }else{
        retval = pid;
        fprintf(stderr, "+ completed '%s': [%d]\n",
            cmd, retval/*exit status*/);
    }
}
#endif