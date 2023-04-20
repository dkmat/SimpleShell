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
#define MAX_PIPE 3
#define PIPE_FD 2

/*
This file contains all the functions used to organize 
the different features of the shell program.
*/

void start();
void process(char *cmd);
void command(char *cmd);
void pipCommand(char *cmd, int last);
int builtin(char *cmd);
int redirect(char *cmd);
int pipeline(char *cmd);
int parseError(char *cmd);
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

        if(!parseError(cmd)){

            if(!pipeline(cmd)){

                /* Builtin command */
                if(builtin(cmd)) break;

                /* Regular command */
                if(strcmp(cmd,"pwd") && strcmp(cmd,"cd"))
                    command(cmd);
            }
        }
        
    }
}
int parseError(char *cmd){
    char original[CMDLINE_MAX];
    strcpy(original,cmd);
    char *tok = strtok(original," ");
    process(original);
    int count = 0;
    while(tok!=NULL){
        count++;
        tok = strtok(NULL," ");
    }
    if(count>=16) {
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
    char * miss = strchr(cmd,'>');
    if(miss!=NULL && cmd[strlen(cmd)-1]=='>'){
        fprintf(stderr,"Error: no output file\n");
        return 1;
    }
    if(miss!= NULL && missPipe!=NULL && miss < missPipe){
        fprintf(stderr,"Error: mislocated output redirection\n");
        return 1;
    }
    return 0;
}
void command(char* cmd){
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

int builtin(char* cmd){
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
        fprintf(stderr,"+ completed '%s' [%d]\n", original, retval);
        *cmd = *tok2;
    }
    return 0;
}

void process(char* cmd){
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
void pipCommand(char *cmd,int last){
    int revert;
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
        perror("execvp error\n");
    }
    else if(list){
        execlp(tok1,tok2,cmd,NULL);
        perror("execlp error\n");
    }
    if(last){
        dup2(revert,STDOUT_FILENO);
    }
    
}

int pipeline(char *cmd){
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


#endif