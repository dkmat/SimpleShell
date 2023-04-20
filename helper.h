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
#define MAX_ARGS 16
#define MAX_PIPE 3
#define PIPE_FD 2
#define STR_MAX 26
#define ASCII_A 97
/*
This file contains all the functions used to organize 
the different features of the shell program.
*/

void start(); // Start the shell
void process(char *cmd); // Remove trailing and leading whitespace from cmd parameter
void command(char *cmd); // Execute command in cmd using fork()+exec()+wait() method
void pipCommand(char *cmd, int last);//Execute commands that are being piped
int builtin(char *cmd, char* env_var[]);//Execute the builtin commands based on cmd
int redirect(char *cmd);//Redirect output of a command if there is a '>' in cmd
int pipeline(char *cmd);//Group commands together so they interact through the pipe
int parseError(char *cmd);//Check for command lines errors and if any display them
int environVar(char *cmd, char* env_var[] );//Make the extra feature environment variables
void start(){ //All function in this file are called through this function(acts like main)
    char cmd[CMDLINE_MAX];
    char* env_var[STR_MAX];
    for(int i=0;i<STR_MAX;i++){
        env_var[i]="";
    }
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
        /*Remove leading and trailing whitespace from command line*/
        process(cmd);
        /*Check command line for $ and make environment variable*/
        if(environVar(cmd,env_var)){

        }
        else if(!parseError(cmd)){//If no parsing errors then continue otherwise show error

            if(!pipeline(cmd)){

                /* Builtin command */
                if(builtin(cmd,env_var)) break;

                /* Regular command */
                if(strcmp(cmd,"pwd") && strcmp(cmd,"cd") && strcmp(cmd,"set"))
                    command(cmd);
            }
        }
        
    }
}

int parseError(char *cmd){ //We check for all possible parsing errors one by one
    char original[CMDLINE_MAX];
    strcpy(original,cmd);
    char *tok = strtok(original," ");
    process(original);
    int count = 0;
    while(tok!=NULL){
        count++;
        tok = strtok(NULL," ");
    }
    if(count>=MAX_ARGS) {
        fprintf(stderr,"Error: too many process arguments\n");
        return 1;
    }
    strcpy(original,cmd);
    if(cmd[0]=='>'||cmd[0]=='|'){
        fprintf(stderr,"Error: missing command\n");
        return 1;
    }
    char * missPipe = strchr(cmd,'|');
    if(missPipe!=NULL && cmd[strlen(cmd)-1]=='|'){
        fprintf(stderr,"Error: missing command\n");
        return 1;
    }
    char *miss_redir = strchr(cmd,'>');
    if(miss_redir!=NULL && cmd[strlen(cmd)-1]=='>'){
        fprintf(stderr,"Error: no output file\n");
        return 1;
    }
    if(miss_redir!= NULL && missPipe!=NULL && miss_redir < missPipe){
        fprintf(stderr,"Error: mislocated output redirection\n");
        return 1;
    }
    return 0;
}

void command(char* cmd){ //The function used to make syscalls and also call for redirection
    pid_t pid;
    char full[CMDLINE_MAX];
    strcpy(full,cmd);
    int stdi = dup(STDERR_FILENO);
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
        dup2(stdi,STDERR_FILENO);
        fprintf(stderr,"Error: command not found\n");
        exit(EXIT_FAILURE);
    }
    else if(pid==0&&list){
        execlp(tok1,tok2,cmd,NULL);
        dup2(stdi,STDERR_FILENO);
        fprintf(stderr,"Error: command not found\n");
        exit(EXIT_FAILURE);
    }
    if(pid>0){
        int status;
        wait(&status);
        dup2(revert,STDOUT_FILENO);
        dup2(stdi,STDERR_FILENO);
        fprintf(stderr, "+ completed '%s' [%d]\n",
            full, WEXITSTATUS(status));
    }
}

int builtin(char* cmd, char* env_var[]){ //This function is used to parse through the built-in commands
    if (!strcmp(cmd, "exit")) {
        fprintf(stderr,"Bye...\n+ completed 'exit' [%d]\n",EXIT_SUCCESS);
        return 1;
    }
    if(!strcmp(cmd,"pwd")){
        char buf[CMDLINE_MAX];
        if(getcwd(buf,sizeof(buf))!=NULL){
            fprintf(stdout, "%s\n",buf);
            fprintf(stderr,"+ completed '%s' [%d]\n",cmd, EXIT_SUCCESS);
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
            fprintf(stderr,"Error: cannot cd into directory\n");
            retval = EXIT_FAILURE;
        }
        fprintf(stderr,"+ completed '%s' [%d]\n", original, WEXITSTATUS(retval));
        *cmd = *tok2;
    }
    char setEnv[4]; /*Setting environmental variables*/
    strncpy(setEnv,cmd,3);
    setEnv[3] = '\0';
    if(!strcmp(setEnv,"set")){
        char original[CMDLINE_MAX];
        strcpy(original,cmd);
        strtok_r(cmd," ",&cmd);
        process(cmd);
        char letter = cmd[0];
        char* check = strtok_r(cmd," ",&cmd);
        process(check);
        process(cmd);
        char keep[CMDLINE_MAX];
        strcpy(keep,cmd);
        int len = strlen(check);
        if(len==1){
            int index = letter-ASCII_A;
            env_var[index]= keep;
            fprintf(stderr,"+ completed '%s' [%d]\n",original,WEXITSTATUS(EXIT_SUCCESS));
        }
        else{
            fprintf(stderr,"Error: invalid variable name\n");
        }
        strcpy(cmd,"set");
    }
    return 0;
}

void process(char* cmd){ //This function removes the trailing newline
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
    char *meta = strchr(cmd,'>'); //meta is for meta characters
    if(meta){
        int std = dup(STDOUT_FILENO); //copy std to STDOUT_FILENO before redirection
        meta = strstr(cmd,">&");
        char *redir;
        if(meta){
            redir = strtok_r(cmd,"&",&cmd);
            process(cmd);
            redir = strtok(redir,">");
            process(redir);
        }else{
            redir = strtok_r(cmd,">",&cmd);
            process(cmd);
        }
        int fd = open(cmd,O_WRONLY|O_CREAT,0644);
        dup2(fd,STDOUT_FILENO);
        if(meta){
            dup2(fd,STDERR_FILENO);
        }
        close(fd);
        strcpy(cmd,redir);
        return std;
    }
    return 1;
}
void pipCommand(char *cmd,int last){ //This function executes the commands on the command line through the pipe. 
    int revert; //It ensures all the commands are able to interact with each other through the pipe. 
    if(last){
        revert = redirect(cmd);
    }
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
    if(!list){
        execvp(tok1,args);
        fprintf(stderr,"Error: command not found\n");
        exit(EXIT_FAILURE);
    }
    else if(list){
        execlp(tok1,tok2,cmd,NULL);
        fprintf(stderr,"Error: command not found\n");
        exit(EXIT_FAILURE);
    }
    if(last){
        dup2(revert,STDOUT_FILENO);
    }
    
}

int pipeline(char *cmd){ //This function does the actual piping of the command line
    char*track = strchr(cmd,'|');
    if(track){
        int i;
        int count = 0;
        while(track[0]!='\0'){
            if(track[0]=='|'){
                count++;
            }
            track++;
        }
        char full[CMDLINE_MAX];
        char *splitComs[count+1];
        strcpy(full,cmd);
        for(i=0;i<=count;i++){
            if(i==count){
                splitComs[i]=cmd;
                process(splitComs[i]);
            }
            else{
                splitComs[i]=strtok_r(cmd,"|",&cmd);
                process(splitComs[i]);
            }
            
        }
        int status[count+1];
        pid_t pid1,pid2;
        int fd1[2];
        if(count==1){
            pipe(fd1);
            pid1 = fork();
            if(pid1 ==0){
                close(fd1[0]);
                dup2(fd1[1],STDOUT_FILENO);
                close(fd1[1]);
                pipCommand(splitComs[0],0);
            }
            pid2 = fork();
            if(pid2 == 0){
                close(fd1[1]);
                dup2(fd1[0],STDIN_FILENO);
                close(fd1[0]);
                pipCommand(splitComs[1],1);
            }
            close(fd1[0]);
            close(fd1[1]);
            waitpid(pid1,&status[0],0);
            waitpid(pid2,&status[1],0);
        }
        else if(count ==2){
            pid_t pid3;
            int fd2[2];
            pipe(fd1);
            pipe(fd2);
            pid1 = fork();
            if(pid1==0){
                close(fd1[0]);
                close(fd2[0]);
                close (fd2[1]);
                dup2(fd1[1],STDOUT_FILENO);
                close(fd1[1]);
                pipCommand(splitComs[0],0);
            }
            pid2 = fork();
            if(pid2 ==0){
                close(fd1[1]);
                close(fd2[0]);
                dup2(fd1[0],STDIN_FILENO);
                close(fd1[0]);
                dup2(fd2[1],STDOUT_FILENO);
                close(fd2[1]);
                pipCommand(splitComs[1],0);
            }
            pid3 = fork();
            if(pid3 == 0){
                close(fd1[0]);
                close(fd1[1]);
                close(fd2[1]);
                dup2(fd2[0],STDIN_FILENO);
                close(fd2[0]);
                pipCommand(splitComs[2],1);
            }
            close(fd1[0]);
            close(fd1[1]);
            close(fd2[0]);
            close(fd2[1]);
            waitpid(pid1,&status[0],0);
            waitpid(pid2,&status[1],0);
            waitpid(pid3,&status[2],0);
        }
        else if(count ==3){
            pid_t pid3,pid4;
            int fd2[2],fd3[2];
            pipe(fd1);
            pipe(fd2);
            pipe(fd3);
            pid1 = fork();
            if(pid1==0){
                close(fd1[0]);
                close(fd2[0]);
                close (fd2[1]);
                close(fd3[0]);
                close(fd3[1]);
                dup2(fd1[1],STDOUT_FILENO);
                close(fd1[1]);
                pipCommand(splitComs[0],0);
            }
            pid2 = fork();
            if(pid2 ==0){
                close(fd1[1]);
                close(fd2[0]);
                close(fd3[0]);
                close(fd3[1]);
                dup2(fd1[0],STDIN_FILENO);
                close(fd1[0]);
                dup2(fd2[1],STDOUT_FILENO);
                close(fd2[1]);
                pipCommand(splitComs[1],0);
            }
            pid3 = fork();
            if(pid3 == 0){
                close(fd1[0]);
                close(fd1[1]);
                close(fd2[1]);
                close(fd3[0]);
                dup2(fd2[0],STDIN_FILENO);
                close(fd2[0]);
                dup2(fd3[1],STDOUT_FILENO);
                close(fd3[1]);
                pipCommand(splitComs[2],0);
            }
            pid4 = fork();
            if(pid4==0){
                close(fd1[0]);
                close(fd1[1]);
                close(fd2[0]);
                close(fd2[1]);
                close(fd3[1]);
                dup2(fd3[0],STDIN_FILENO);
                close(fd3[0]);
                pipCommand(splitComs[3],1);
            }
            close(fd1[0]);
            close(fd1[1]);
            close(fd2[0]);
            close(fd2[1]);
            close(fd3[0]);
            close(fd3[1]);
            waitpid(pid1,&status[0],0);
            waitpid(pid2,&status[1],0);
            waitpid(pid3,&status[2],0);
            waitpid(pid4,&status[3],0);
        }
        fprintf(stderr, "+ completed '%s'",full);
        for(int i=0;i<=count;i++){
            fprintf(stderr,"[%d]",WEXITSTATUS(status[i]));
        }
        fprintf(stderr,"\n");
        return 1;
    }
    else return 0;
}
int environVar(char* cmd, char* env_var[]){ //This function implements simple environment variables. It ensures that string variables, specifically, 26 characters from a-z can be used as a part of a command.
    char* letter = strchr(cmd,'$');
    char original[CMDLINE_MAX]; 
    strcpy(original,cmd);
    int valid=0;
    if(letter){
        while(letter[0]!=' ' && letter[0] != '\0'){
            valid++;
            ++letter;
        }
        if(valid>2){
            fprintf(stderr,"Error: invalid variable name\n");
            return 1;
        }
        else{
            letter = strchr(cmd,'$');
            ++letter;
            valid = letter[0]-ASCII_A;
            if(valid>STR_MAX-1 || valid<0){
                fprintf(stderr,"Error: invalid variable name\n");
                return 1;
            }
        }
        char *orig = original;
        char* multi = strtok_r(orig,"$",&orig);
        int index = orig[0]-ASCII_A;
        multi = strcat(multi,env_var[index]);
        char * tok1 = strtok_r(multi," ",&multi);
        process(multi);
        if(!strcmp(multi,"")) multi = NULL;
        char *args[] = {tok1,multi,NULL};
        pid_t pid;
        pid = fork();
        if(pid==0){
            execvp(tok1,args);
            fprintf(stderr,"Error: command not found\n");
            exit(EXIT_FAILURE);
        }
        if(pid>0){
            int status;
            wait(&status);
            fprintf(stderr, "+ completed '%s' [%d]\n",
            cmd, WEXITSTATUS(status));
        }
        return 1;
    }
    return 0;
}

#endif