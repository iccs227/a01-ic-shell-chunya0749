#ifndef JOB_H
#define JOB_H

#include <sys/types.h>

#define MAX_JOBS 100

struct job {
    int jobID;
    pid_t pid;
    char command[1024];
    char Process_status[16];
    int is_Stop;
};

extern struct job jobList[MAX_JOBS];
extern int jobID;
extern pid_t fg_pid;

void addJob(pid_t pid, char *command, int is_stop);
void updateJobList();
void printJobList();
void externalRunning(char **args, int background);
void fg(int id);
void bg(int id);

#endif