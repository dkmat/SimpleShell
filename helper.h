#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

/*
This file contains all the functions used to organize 
the different features of the shell program.
*/

void start();
void process(char *cmd);
void command(char *cmd);
int builtin(char *cmd);
//void copy(struct filter *correct,char *cmd);
void start(){
    char cmd[CMDLINE_MAX];
    while (1) {
        //int retval;

        /* Print prompt */
        printf("sshell@ucd$ ");
        fflush(stdout);

        /* Get command line */
        fgets(cmd, CMDLINE_MAX, stdin);

        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO)) {
            printf("%s", cmd);
            fflush(stdout);
        }
         /* Remove trailing newline from command line */
        char *nl;
        nl = strchr(cmd, '\n');
        if (nl)
            *nl = '\0';

         /* Builtin command */
        if(builtin(cmd)) break;

         /* Regular command */
        command(cmd);
    }
}

void command(char* cmd){
    pid_t pid;
    char temp[CMDLINE_MAX];
    strcpy(temp,cmd);
    process(cmd);
    char * tok1 = strtok_r(cmd," ",&cmd);
    if(!strcmp(cmd,"")) cmd = NULL;
    else if(*(cmd+0)==' ') process(cmd);
    char *args[] = {tok1,cmd,NULL};
    pid = fork();
    if(pid==0){
        execvp(tok1,args);
        perror("execvp error\n");
    }
    else if(pid>0){
        int status;
        wait(&status);
        fprintf(stderr, "+ completed '%c' [%d]\n",
            *(cmd+0), status/*exit status*/);
    }else{
        perror("fork error");
    }
}

int builtin(char* cmd){
    if (!strcmp(cmd, "exit")) {
        fprintf(stderr,"Bye...\n+ completed 'exit' [%d]\n",EXIT_SUCCESS);
        return 1;
    }
    return 0;
}

void process(char* cmd){
    while(*(cmd+0) == ' '){
        ++cmd;
    }
}

struct filter{
    char *full;
    char *com;
    char *argument;
};
#endif