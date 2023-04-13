#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

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
        process(cmd);
         /* Builtin command */
        if(builtin(cmd)) break;

         /* Regular command */
        if(strcmp(cmd,"pwd") && strcmp(cmd,"cd"))
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
        fprintf(stderr, "+ completed '%s' [%d]\n",
            temp, status/*exit status*/);
    }else{
        perror("fork error\n");
    }
}

int builtin(char* cmd){
    if (!strcmp(cmd, "exit")) {
        fprintf(stderr,"Bye...\n+ completed 'exit' [%d]\n",EXIT_SUCCESS);
        return 1;
    }
    if(!strcmp(cmd,"pwd")){
        char buf[CMDLINE_MAX];
        if(getcwd(buf,sizeof(buf))!=NULL){
            fprintf(stderr, "%s\n+ completed '%s' [%d]\n", buf,cmd, EXIT_SUCCESS);
        }
        else{
            perror("getcwd error\n");
        }
    }
    char substr[3];
    strncpy(substr,cmd,2);
    substr[2] = '\0';
    if(!strcmp(substr,"cd")){
        char original[CMDLINE_MAX];
        strcpy(original,cmd);
        char *tok2 = strtok_r(cmd," ",&cmd);
        process(cmd);
        int retval = chdir(cmd);
        if(retval==-1){
            perror("chdir error\n");
        }
        fprintf(stderr,"+ completed '%s' [%d]\n", original, retval);
        *cmd = *tok2;
    }
    return 0;
}

void process(char* cmd){
    //fprintf(stderr,"in process!\n");
    char *begin = cmd;
    char *end = cmd + strlen(cmd) -1;
    while(isspace(*begin)){
        ++begin;
    }
    while(isspace(*end)){
        --end;
    }
    *(end + 1) = '\0';
    memmove(cmd, begin, strlen(begin)+1);
}

struct filter{
    char *full;
    char *com;
    char *argument;
};
#endif