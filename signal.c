// ==== signal.c ====
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include "signal.h"
#include "job.h"

extern pid_t fg_pid;
extern pid_t main_pid;

void signalHandlerSetUP() {
    struct sigaction sa_int, sa_tstp, sa_chld;

    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);
}

void sigint_handler(int sig) {
    if (fg_pid != 0) {
        kill(fg_pid, SIGINT);
    }
    tcsetpgrp(STDIN_FILENO, main_pid);
}

void sigtstp_handler(int sig) {
    if (fg_pid > 0) {
        kill(fg_pid, SIGTSTP);
        printf("\n");
    }
    tcsetpgrp(STDIN_FILENO, main_pid);
}

void sigchld_handler(int sig) {
    updateJobList();
}