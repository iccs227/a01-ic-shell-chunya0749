#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "job.h"

struct job jobList[MAX_JOBS];
int jobID = 0;
pid_t fg_pid = 0;

static char *flattenArgs(char **args) {
    static char cmd[1024];
    cmd[0] = '\0';
    for (int i = 0; args[i] != NULL; i++) {
        strcat(cmd, args[i]);
        if (args[i + 1] != NULL) strcat(cmd, " ");
    }
    return cmd;
}

static void redirect(char **args) {
    int in = -1, out = -1;
    char *input_file = NULL, *output_file = NULL;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0 && args[i + 1] != NULL) {
            input_file = args[i + 1];
            args[i] = NULL;
            args[i + 1] = NULL;
        } else if (strcmp(args[i], ">") == 0 && args[i + 1] != NULL) {
            output_file = args[i + 1];
            args[i] = NULL;
            args[i + 1] = NULL;
        }
    }

    if (input_file) {
        in = open(input_file, O_RDONLY);
        if (in < 0) perror("Couldn't open input file");
        else dup2(in, STDIN_FILENO);
        if (in >= 0) close(in);
    }

    if (output_file) {
        out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (out < 0) perror("Couldn't open output file");
        else dup2(out, STDOUT_FILENO);
        if (out >= 0) close(out);
    }
}

void addJob(pid_t pid, char *command, int is_stop) {
    if (jobID >= MAX_JOBS) {
        printf("You've reached the jobs limit\n");
        return;
    }
    jobID++;
    jobList[jobID].jobID = jobID;
    jobList[jobID].pid = pid;
    strncpy(jobList[jobID].command, command, 1023);
    jobList[jobID].command[1023] = '\0';

    if (is_stop) {
        strcpy(jobList[jobID].Process_status, "Stopped");
        jobList[jobID].is_Stop = 1;
        printf("[%d]+  Stopped   %s\n", jobID, command);
    } else {
        strcpy(jobList[jobID].Process_status, "Running");
        jobList[jobID].is_Stop = 0;
        printf("[%d] %d\n", jobID, pid);
    }
}

void updateJobList() {
    int status;
    pid_t pid_result;
    
    for (int i = 1; i <= jobID; i++) {
        if (strcmp(jobList[i].Process_status, "Running") == 0) {
            pid_result = waitpid(jobList[i].pid, &status, WNOHANG);
            
            if (pid_result > 0) {
                if (WIFEXITED(status)) {
                    strcpy(jobList[i].Process_status, "Done");
                    printf("[%d]+ Done   %s\n", i, jobList[i].command);
                }
                else if (WIFSIGNALED(status)) {
                    strcpy(jobList[i].Process_status, "Killed");
                    printf("[%d]+ Killed   %s\n", i, jobList[i].command);
                }
            }
        }
    }
}

void externalRunning(char **args, int background) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        setpgid(0, 0);  
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        
        redirect(args);
        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    } else { // Parent Process
        if (background) {
            addJob(pid, flattenArgs(args), 0);
        } else {
            int status;
            
            // Put child in foreground process group
            setpgid(pid, pid);
            tcsetpgrp(STDIN_FILENO, pid);
            
            waitpid(pid, &status, WUNTRACED);
            
            // Restore shell to foreground
            tcsetpgrp(STDIN_FILENO, getpgrp());
            
            if (WIFSTOPPED(status)) {
                addJob(pid, flattenArgs(args), 1);
            }
        }
    }
}

void fg(int id) {
    if (id <= jobID && id > 0 && strcmp(jobList[id].Process_status, "Done") != 0) {
        int status;
        pid_t pid = jobList[id].pid;

        fg_pid = pid;
        if (jobList[id].is_Stop) {
            jobList[id].is_Stop = 0;
            strcpy(jobList[id].Process_status, "Running");
            kill(pid, SIGCONT);
        }

        tcsetpgrp(STDIN_FILENO, pid);
        waitpid(pid, &status, WUNTRACED);
        tcsetpgrp(STDIN_FILENO, getpgrp());

        if (WIFSTOPPED(status)) {
            jobList[id].is_Stop = 1;
            strcpy(jobList[id].Process_status, "Stopped");
            printf("\n[%d]+  Stopped   %s\n", id, jobList[id].command);
        } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
            strcpy(jobList[id].Process_status, "Done");
        }
        fg_pid = 0;
    } else {
        printf("Invalid job ID\n");
    }
}

void bg(int id) {
    if (id <= jobID && id > 0 && jobList[id].is_Stop == 1) {
        jobList[id].is_Stop = 0;
        strcpy(jobList[id].Process_status, "Running");
        kill(jobList[id].pid, SIGCONT);
        printf("[%d]+ %s &\n", id, jobList[id].command);
    } else {
        printf("Invalid job ID\n");
    }
}

void printJobList() {
    char sign = '-';
    for (int i = 1; i <= jobID; i++) {
        if (strcasecmp(jobList[i].Process_status, "Running") == 0) {
            printf("[%d]%c %s   %s\n", i, sign, jobList[i].Process_status, jobList[i].command);
            sign = '+';
        } else if (strcasecmp(jobList[i].Process_status, "Stopped") == 0) {
            printf("[%d]+ %s   %s\n", i, jobList[i].Process_status, jobList[i].command);
        }
    }
}
