/* ICCS227: Project 1: icsh
 * Name: Chunya Pattharapinya
 * StudentID: u6581030
 */

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 
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
 
 void handle_command(char *buffer) {
     char *args[MAX_ARGS];
     int arg_count = 0;
 
     args[arg_count] = strtok(buffer, " ");
     while (args[arg_count] != NULL && arg_count < MAX_ARGS - 1) {
         arg_count++;
         args[arg_count] = strtok(NULL, " ");
     }
 
     if (args[0] == NULL) return;
 
     if (strcmp(args[0], "echo") == 0) {
         print_argument(args);
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
         printf("bad command\n");
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
 