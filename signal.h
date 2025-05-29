// ==== signal.h ====
#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

void signalHandlerSetUP();
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigchld_handler(int sig);

#endif
