/* ICCS227: Project 1: icsh
 * Name: Chunya Pattharapinya
 * StudentID: u6581030
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/wait.h>
 #include <signal.h>
 #include <fcntl.h>
 #include "job.h"
 #include "signal.h"
 
 #define MAX_CMD_BUFFER 255
 #define MAX_ARGS 64
 
 static int is_number(const char *s) {
     if (*s == '\0') return 0;
     for (; *s; s++) {
         if (*s < '0' || *s > '9') return 0;
     }
     return 1;
 }
 
 void handle_command(char *buffer) {
     char *args[MAX_ARGS];
     int arg_count = 0;
     int background = 0;
 
     args[arg_count] = strtok(buffer, " \t");
     while (args[arg_count] != NULL && arg_count < MAX_ARGS - 1) {
         arg_count++;
         args[arg_count] = strtok(NULL, " \t");
     }
 
     if (arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0) {
         background = 1;
         args[arg_count - 1] = NULL;
         arg_count--;
     }
 
     if (args[0] == NULL) return;
 
     if (strcmp(args[0], "jobs") == 0) {
         printJobList();
     } else if (strcmp(args[0], "echo") == 0) {
         int saved_stdout = dup(STDOUT_FILENO);
         for (int i = 1; args[i] != NULL; i++) {
             printf("%s ", args[i]);
         }
         printf("\n");
         dup2(saved_stdout, STDOUT_FILENO);
         close(saved_stdout);
     } else if (strcmp(args[0], "fg") == 0 && args[1] != NULL) {
         if (args[1][0] == '%') {
             int id = atoi(args[1] + 1);
             fg(id);
         } else {
             printf("Usage: fg %%<job_id>\n");
         }
     } else if (strcmp(args[0], "bg") == 0 && args[1] != NULL) {
         if (args[1][0] == '%') {
             int id = atoi(args[1] + 1);
             bg(id);
         } else {
             printf("Usage: bg %%<job_id>\n");
         }
     } else if (strcmp(args[0], "exit") == 0) {
         int code = 0;
         if (args[1] != NULL) {
             if (!is_number(args[1])) {
                 printf("bad command\n");
                 return;
             }
             code = atoi(args[1]) % 256;
         }
         printf("bye\n");
         exit(code);
     } else {
         externalRunning(args, background);
     }
 }
 
 int main(int argc, char *argv[]) {
     main_pid = getpid();
 
     setpgid(main_pid, main_pid);
     tcsetpgrp(STDIN_FILENO, main_pid);
     signal(SIGTTOU, SIG_IGN); 
     signal(SIGTTIN, SIG_IGN);
     signal(SIGINT, SIG_IGN);  // Ignore Ctrl+C
     signal(SIGTSTP, SIG_IGN); // Ignore Ctrl+Z
 
     signalHandlerSetUP();
 
     FILE *input_stream = stdin;
     char buffer[MAX_CMD_BUFFER];
     char last_command[MAX_CMD_BUFFER] = "";
 
     if (argc == 2) {
         input_stream = fopen(argv[1], "r");
         if (input_stream == NULL) {
             perror("Error opening script file");
             return 1;
         }
     }
 
     while (1) {
         updateJobList();
         if (input_stream == stdin) {
             printf("icsh $ ");
             fflush(stdout);
         }
 
         if (fgets(buffer, MAX_CMD_BUFFER, input_stream) == NULL) {
             break;
         }
 
         buffer[strcspn(buffer, "\n")] = '\0';
 
         if (strcmp(buffer, "!!") == 0) {
             if (strlen(last_command) == 0) {
                 printf("No commands in history.\n");
                 continue;
             }
             printf("%s\n", last_command);
             strcpy(buffer, last_command);
         } else if (strlen(buffer) > 0) {
             strcpy(last_command, buffer);
         } else {
             continue;
         }
 
         handle_command(buffer);
     }
 
     if (input_stream != stdin) fclose(input_stream);
     return 0;
 }
 