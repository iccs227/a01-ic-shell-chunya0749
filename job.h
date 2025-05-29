// ==== job.h ====
#ifndef JOB_H
#define JOB_H

#include <sys/types.h>

struct job {
    int jobID;
    pid_t pid;
    char command[1024];
    char Process_status[16];
    int is_Stop;
};

extern struct job jobList[100];
extern int jobID;

void addJob(pid_t pid, char *command, int is_stop);
void updateJobList();
void printJobList();

#endif