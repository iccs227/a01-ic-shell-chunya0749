#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include "job.h"
#include "signal.h"

pid_t main_pid;

void sigint_handler(int sig) {
    if (fg_pid != 0) {
        kill(fg_pid, SIGINT);
    }
    tcsetpgrp(STDIN_FILENO, getpgid(main_pid));
}

void sigtstp_handler(int sig) {
    if (fg_pid > 0) {
        kill(fg_pid, SIGTSTP);
        printf("\n");
    }
    tcsetpgrp(STDIN_FILENO, getpgid(main_pid));
}

void sigchld_handler(int sig) {
    updateJobList();
}

void signalHandlerSetUP() {
    struct sigaction sa_int, sa_tstp, sa_chld;

    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);

    //SIGTSTP handler
    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    //SIGCHLD handler
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);
}