/* ICCS227: Project 1: icsh
 * Name: Chunya Pattharapinya
 * StudentID: u6581030
 */

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <unistd.h>     // fork(), execvp(), setpgid()
 #include <sys/wait.h>   // waitpid()
 #include <signal.h>
 #include <fcntl.h>      // open()
 
 #define MAX_CMD_BUFFER 255
 #define MAX_ARGS 64
 
 struct job {
     int jobID;
     pid_t pid;
     char command[1024];
     char Process_status[16];
     int is_Stop;
 };
 
 struct job jobList[100];
 int jobID = 0;
 int fgJob = 0;
 pid_t fg_pid = 0;
 
 // Print remaining tokens (used for echo)
 void print_argument(char** input) {
     int i = 1; // Start from 1 to skip "echo"
     while (input[i] != NULL) {
         printf("%s ", input[i]);
         i++;
     }
     printf("\n");
 }
 
 // Check if string is all digits (used for exit code)
 int is_number(char *s) {
     if (*s == '\0') return 0;  // empty string
     while (*s) {
         if (*s < '0' || *s > '9') return 0;  // not a digit
         s++;
     }
     return 1;
 }
 
 // Flatten args array into one string
 char *flattenArgs(char **args) {
     static char cmd[1024];
     cmd[0] = '\0';
     for (int i = 0; args[i] != NULL; i++) {
         strcat(cmd, args[i]);
         if (args[i+1] != NULL) strcat(cmd, " "); // only add space if not the last word
     }
     return cmd;
 }
 
 
 // I/O redirect
 void redirect(char** args) {
     int in = -1;
     int out = -1;
     char buffer[1024];
     size_t got;
     char* input_file = NULL;
     char* output_file = NULL;
 
     for (int i = 0; args[i] != NULL; i++) {
         if (strcmp(args[i], "<") == 0 && args[i+1] != NULL) {
             input_file = args[i+1];
             args[i] = NULL;
             args[i+1] = NULL;
         } else if (strcmp(args[i], ">") == 0 && args[i+1] != NULL) {
             output_file = args[i+1];
             args[i] = NULL;
             args[i+1] = NULL;
         }
     }
 
     if (input_file) {
         in = open(input_file, O_RDONLY);
         if (in <= 0) {
             fprintf(stderr, "Couldn't open input file\n");
             exit(1);
         }
         dup2(in, 0);
         close(in);
     }
 
     if (output_file) {
         out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
         if (out <= 0) {
             fprintf(stderr, "Couldn't open output file\n");
             exit(1);
         }
         dup2(out, 1);
         close(out);
     }
 
     if (args[0] == NULL && input_file && output_file) {
         while ((got = fread(buffer, 1, 1024, stdin)) > 0) {
             fwrite(buffer, 1, got, stdout);
         }
         fflush(stdout);
         exit(0);
     }
 }
 
 void addJob(pid_t pid, char* command, int is_stop) {
     if (jobID >= 100) {
         printf("You've reached the jobs limit\n");
         return;
     }
 
     jobID++;
     jobList[jobID].jobID = jobID;
     jobList[jobID].pid = pid;
     strcpy(jobList[jobID].command, command);
 
     if (is_stop == 1) {
         strcpy(jobList[jobID].Process_status, "Stopped");
         jobList[jobID].is_Stop = 1;
         printf("[%d]+  Stopped                 %s\n", jobID, command);
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
                 printf("[%d]+ Done                 %s\n", i, jobList[i].command);
             }
         }
     }
 }
 
 
 void externalRunning(char **args, int background) {
     int status;
     pid_t pid = fork();
 
     if (pid < 0) {
         perror("Fork failed");
         exit(1);
     } else if (pid == 0) {
         setpgid(0, 0);
         execvp(args[0], args);
         perror("exec failed");
         exit(1);
     } else {
         if (background == 1) {
             addJob(pid, flattenArgs(args), 0);  // Running in background
             
         } else {
             fg_pid = pid;
             fgJob = 1;
             waitpid(pid, &status, WUNTRACED);
 
             if (WIFSTOPPED(status)) {
                jobList[id].is_Stop = 1;
                strcpy(jobList[id].Process_status, "Stopped");
                printf("[%d]+  Stopped   %s\n", jobList[id].jobID, jobList[id].command);
            }
            
 
            fg_pid = 0;
         }
     }
 }
 
 
 void fg(int id) {
     if (id <= jobID && id > 0 && strcmp(jobList[id].Process_status, "Done") != 0) {
         int status;
         jobList[id].is_Stop = 0;
         strcpy(jobList[id].Process_status, "Running");
         fgJob = 1;
         printf("%s\n", jobList[id].command);
         kill(jobList[id].pid, SIGCONT);
         waitpid(jobList[id].pid, &status, 0);
     } else {
         printf("Invalid job ID\n");
     }
 }
 
 void bg(int id) {
    if (id <= jobID && id > 0 && jobList[id].is_Stop == 1) {
        jobList[id].is_Stop = 0;
        strcpy(jobList[id].Process_status, "Running");
        kill(jobList[id].pid, SIGCONT);
        printf("[%d]+ %s &\n", jobList[id].jobID, jobList[id].command);
    } else {
        printf("Invalid job ID\n");
    }
}


 void printJobList(){ //only print running and stopped processes
     char sign = '-';
     for (int i = 1; i <= jobID; i++)
     { 
         if(strcasecmp(jobList[i].Process_status,"Running") == 0) {
             printf("[%d]%c %s                 %s\n",i,sign,jobList[i].Process_status,jobList[i].command); 
             sign = '+';
         } else if (strcasecmp(jobList[i].Process_status, "Stopped") == 0) {
             printf("[%d]+ %s                 %s\n",i,jobList[i].Process_status,jobList[i].command); 
         }
     }
 }
 
 
 // Handle Ctrl+C
 void sigint_handler(int sig){
     if(fg_pid != 0){
         kill(fg_pid, SIGINT);
         printf("Foreground job killed\n");
     }
 }
  
 // Handle Ctrl+Z
 void sigtstp_handler(int sig) {
    if (fg_pid > 0) {
        kill(fg_pid, SIGTSTP);  // send stop to child
    }
}

 
 void sigchld_handler(int sig) {
     updateJobList();  // Tell shell to check finished jobs immediately
 }
 
 void signalHandlerSetUP() {
     struct sigaction sa_int, sa_tstp, sa_chld;
 
     // Ctrl+C
     sa_int.sa_handler = sigint_handler;
     sigemptyset(&sa_int.sa_mask);
     sa_int.sa_flags = 0;
     sigaction(SIGINT, &sa_int, NULL);
 
     // Ctrl+Z
     
     sa_tstp.sa_handler = sigtstp_handler;
     sigemptyset(&sa_tstp.sa_mask);
     sa_tstp.sa_flags = SA_RESTART;
     sigaction(SIGTSTP, &sa_tstp, NULL);
     
 
     // SIGCHLD — handle finished background jobs immediately
     sa_chld.sa_handler = sigchld_handler;
     sigemptyset(&sa_chld.sa_mask);
     sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
     sigaction(SIGCHLD, &sa_chld, NULL);
 }
 
 
 void handle_command(char* buffer) {
     char* args[MAX_ARGS];
     int arg_count = 0;
     int background = 0; // flag to check if command ends with '&'
 
     // Tokenize input
     args[arg_count] = strtok(buffer, " ");
     while (args[arg_count] != NULL && arg_count < MAX_ARGS - 1) {
         arg_count++;
         args[arg_count] = strtok(NULL, " ");
     }
 
     if (arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0) {
         background = 1;
         args[arg_count - 1] = NULL; // Remove '&' from command
         arg_count--;
     }
 
     if (args[0] == NULL) return;
 
     // Built-in: jobs
     if (strcmp(args[0], "jobs") == 0) {
         printJobList();
         return;
     }
 
     // Built-in: echo
     else if (strcmp(args[0], "echo") == 0) {
         int saved_stdout = dup(STDOUT_FILENO);
         redirect(args);
         print_argument(args);
         dup2(saved_stdout, STDOUT_FILENO);
         close(saved_stdout);
         return;
     }
 
     // Built-in: fg
     else if (strcmp(args[0], "fg") == 0 && args[1] != NULL) {
         if (args[1][0] == '%') {
             int id = atoi(args[1] + 1);  // skip the '%'
             fg(id);
         } else {
             printf("Usage: fg %%<job_id>\n");
         }
         return;
     }
 
     // Built-in: bg
     else if (strcmp(args[0], "bg") == 0 && args[1] != NULL) {
         if (args[1][0] == '%') {
             int id = atoi(args[1] + 1);  // skip the '%'
             bg(id);
         } else {
             printf("Usage: bg %%<job_id>\n");
         }
         return;
     }
 
     // Built-in: exit
     else if (strcmp(args[0], "exit") == 0) {
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
     }
 
     // External command
     else {
         externalRunning(args, background);
     }
 }
 
 int main(int argc , char *argv[]) {
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
     tcsetpgrp(STDIN_FILENO, getpgrp());
     signalHandlerSetUP();
     FILE* input_stream = stdin;
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
         }
 
         if (fgets(buffer, MAX_CMD_BUFFER, input_stream) == NULL) {
             break;
         }
 
         for (int i = 0; buffer[i] != '\0'; i++) {
             if (buffer[i] == '\n') {
                 buffer[i] = '\0';
                 break;
             }
         }
 
         if (strcmp(buffer, "!!") == 0) {
             if (strlen(last_command) == 0) {
                 printf("No commands in history.\n");
                 continue;
             } else {
                 printf("%s\n", last_command);
                 strcpy(buffer, last_command);
             }
         } else if (strlen(buffer) > 0) {
             strcpy(last_command, buffer);
         } else {
             continue;
         }
 
         handle_command(buffer);
     }
 
     return 0;
 }
 