#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>

extern pid_t main_pid;

void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigchld_handler(int sig);
void signalHandlerSetUP();

#endif