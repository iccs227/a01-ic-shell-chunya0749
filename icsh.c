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
 #include <fcntl.h>
 
 #define MAX_CMD_BUFFER 255
 #define MAX_ARGS 64
 
 void print_argument(char **input) {
     int i = 1;
     while (input[i] != NULL) {
         printf("%s ", input[i]);
         i++;
     }
     printf("\n");
 }
 
 int is_number(char *s) {
     if (*s == '\0') return 0;
     while (*s) {
         if (*s < '0' || *s > '9') return 0;
         s++;
     }
     return 1;
 }
 
 char *flattenArgs(char **args) {
     static char cmd[1024];
     cmd[0] = '\0';
     for (int i = 0; args[i] != NULL; i++) {
         strcat(cmd, args[i]);
         if (args[i + 1] != NULL) strcat(cmd, " ");
     }
     return cmd;
 }
 
 void redirect(char **args) {
     int in = -1, out = -1;
     char buffer[1024];
     size_t got;
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
         if (in < 0) {
             perror("Couldn't open input file");
             exit(1);
         }
         dup2(in, STDIN_FILENO);
         close(in);
     }
 
     if (output_file) {
         out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
         if (out < 0) {
             perror("Couldn't open output file");
             exit(1);
         }
         dup2(out, STDOUT_FILENO);
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
 
 void externalRunning(char **args) {
     int status;
     pid_t pid = fork();
 
     if (pid < 0) {
         perror("Fork failed");
         exit(1);
     } else if (pid == 0) {
         redirect(args);
         execvp(args[0], args);
         perror("exec failed");
         exit(1);
     } else {
         waitpid(pid, &status, 0);
     }
 }
 
 void handle_command(char *buffer) {
     char *args[MAX_ARGS];
     int arg_count = 0;
     int background = 0;
 
     args[arg_count] = strtok(buffer, " ");
     while (args[arg_count] != NULL && arg_count < MAX_ARGS - 1) {
         arg_count++;
         args[arg_count] = strtok(NULL, " ");
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
         redirect(args);
         print_argument(args);
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
         externalRunning(args);
     }
 }
 
 int main(int argc, char *argv[]) {
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