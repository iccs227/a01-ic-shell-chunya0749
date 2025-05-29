// ==== job.c ====
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include "job.h"

struct job jobList[100];
int jobID = 0;

void addJob(pid_t pid, char *command, int is_stop) {
    if (jobID >= 100) {
        printf("You've reached the jobs limit\n");
        return;
    }
    jobID++;
    jobList[jobID].jobID = jobID;
    jobList[jobID].pid = pid;
    strcpy(jobList[jobID].command, command);

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
            if (pid_result > 0 && WIFEXITED(status)) {
                strcpy(jobList[i].Process_status, "Done");
                printf("[%d]+ Done   %s\n", i, jobList[i].command);
            }
        }
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
