#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>

#define CMDLINE_MAX 512

/*
This file contains all the functions used to organize 
the different features of the shell program.
*/

void start();
void process(char *cmd);
void command(char *cmd);
int builtin(char *cmd);
int redirect(char *cmd);
int pipeline(char *cmd);
void start(){
    char cmd[CMDLINE_MAX];
    while (1) {

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
        if(!pipeline(cmd)){
             /* Builtin command */
            if(builtin(cmd)) break;

         /* Regular command */
            if(strcmp(cmd,"pwd") && strcmp(cmd,"cd"))
                command(cmd);
        }
    }
}

void command(char* cmd){
    pid_t pid;
    char temp[CMDLINE_MAX];
    strcpy(temp,cmd);
    int revert = redirect(cmd);
    char * tok1 = strtok_r(cmd," ",&cmd);
    char * tok2 = 0;
    int list=0;
    process(cmd);
    if(cmd[0]=='-'&&cmd[2]==' '){
        tok2 = strtok_r(cmd," ",&cmd);
        process(cmd);
        list = 1;
    }
    if(!strcmp(cmd,"")) cmd = NULL;
    char *args[] = {tok1,cmd,NULL};
    pid = fork();
    if(pid==0&&!list){
        execvp(tok1,args);
        perror("execvp error\n");
    }
    else if(pid==0&&list){
        execlp(tok1,tok2,cmd,NULL);
        perror("execlp error\n");
    }
    else if(pid>0){
        int status;
        wait(&status);
        dup2(revert,STDOUT_FILENO);
        fprintf(stderr, "+ completed '%s' [%d]\n",
            temp, status);
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

int redirect(char* cmd){
    char *meta = strchr(cmd,'>');
    if(meta){
        int std = dup(STDOUT_FILENO);
        meta = strtok_r(cmd,">",&cmd);
        process(cmd);
        int fd = open(cmd,O_WRONLY|O_CREAT,0644);
        dup2(fd,STDOUT_FILENO);
        close(fd);
        strcpy(cmd,meta);
        return std;
    }
    return 1;
}
int pipeline(char *cmd){
    char*track = strchr(cmd,'|');
    if(track){
        int count = 0;
        while(track[0]!='\0'){
            if(track[0]=='|'){
                count++;
            }
            track++;
        }
        char full[CMDLINE_MAX];
        char *line;
        strcpy(full,cmd);
        pid_t pid1;
        int fd[2],status[count+1];
        int stat;
        int std = dup(STDOUT_FILENO);
        pipe(fd);
        for(int i =0;i<count+1;i++){
            line = strtok_r(cmd,"|",&cmd);
            process(line);
            track = strtok_r(line," ",&line);
            process(line);
            if(!strcmp(line,"")) line = NULL;
            char *args[] = {track,line,NULL};
            if(!(pid1 = fork())){
                if(i==count){
                    close(fd[1]);
                    dup2(fd[0],STDIN_FILENO);
                    close(fd[0]);
                    execvp(args[0],args);
                }
                if(i==0){
                    close(fd[0]);
                    dup2(fd[1],STDOUT_FILENO);
                    close(fd[1]);
                    execvp(args[0],args);
                }
                if(i>0 && i<count){
                    dup2(fd[0],STDIN_FILENO);
                    dup2(fd[1],STDOUT_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                    execvp(args[0],args);
                }
            }
            close(fd[0]);
            close(fd[1]);
            wait(&stat);
            status[i] = stat;
        }
        dup2(std,STDOUT_FILENO);
        fprintf(stderr, "+ completed '%s'",full);
        for(int i=0;i<count+1;i++){
            fprintf(stderr,"[%d]",status[i]);
        }
        fprintf(stderr,"\n");
        return 1;
    }
    else return 0;
}

#endif