// /* ICCS227: Project 1: icsh
//  * Name: Chunya Pattharapinya
//  * StudentID: u6581030
//  */


//  #include <stdio.h>
//  #include <string.h>
//  #include <stdlib.h>
 
//  #define MAX_CMD_BUFFER 255
 
//  void print_argument(char* input) {
//      while (input != NULL) {
//          printf("%s ", input);
//          input = strtok(NULL, " ");
//      }
//      printf("\n");
//  }

//  //check if string is all digits(for exit code)
//  int is_number(char *s) {
//     if (*s == '\0') return 0;  // empty string
//     while (*s) {
//         if (*s < '0' || *s > '9') return 0;  // not a digit
//         s++;

//     }
//     return 1;
// }

 
//  int main(int argc , char *argv[]) {
//      FILE* input_stream = stdin;

//      char buffer[MAX_CMD_BUFFER]; 
//      char last_command[MAX_CMD_BUFFER] = "";  //store last command

//      if (argc == 2) {
//         input_stream = fopen(argv[1], "r");
//         if (input_stream == NULL) {
//             perror("Error opening script file");
//             return 1;
//         }
//     }
 
//     while (1) {
//         if (input_stream == stdin) {
//             printf("icsh $ ");
//         }
    
//         if (fgets(buffer, MAX_CMD_BUFFER, input_stream) == NULL) {
//             break;
//         }
    
//         // Remove trailing newline
//         for (int i = 0; buffer[i] != '\0'; i++) {
//             if (buffer[i] == '\n') {
//                 buffer[i] = '\0';
//                 break;
//             }
//         }
    
//     // Handle '!!'
//     if (strcmp(buffer, "!!") == 0) {
//         if (strlen(last_command) == 0) {
//             printf("No commands in history.\n");
//             continue;
//         } else {
//             printf("%s\n", last_command);
//             strcpy(buffer, last_command);
//         }
//     } else if (strlen(buffer) > 0) {
//         // Save the current command as the new last_command
//         strcpy(last_command, buffer);
//     } else {
//         // If the input is empty, skip it
//         continue;
//     }

//     //command into word / argument
//     char *token = strtok(buffer, " ");
//         if (token == NULL) {
//             continue;
//         }
    
//         if (strcmp(token, "echo") == 0) {
//             token = strtok(NULL, " ");
//             print_argument(token);
//         }
//         else if (strcmp(token, "exit") == 0) {
//             token = strtok(NULL, " ");
//             int code = 0;
    
//             if (token != NULL) {
//                 if (!is_number(token)) {
//                     printf("bad command\n");
//                     continue;
//                 }
//                 code = atoi(token);
//                 code = code % 256;
//             }
    
//             printf("bye\n");
//             exit(code);
//         }
    
//         else {
//             printf("bad command\n");
//         }
//     }    
    
//     return 0;
        
// }
 
// // // Handle '!!'
// // if (strcmp(buffer, "!!") == 0) {
// //     if (strlen(last_command) == 0) {
// //         printf("No commands in history.\n");
// //         continue;
// //     } else {
// //         printf("%s\n", last_command);
// //         strcpy(buffer, last_command);
// //     }
// // } else if (strlen(buffer) > 0) {
// //     // Save the current command as the new last_command
// //     strcpy(last_command, buffer);
// // } else {
// //     // If the input is empty, skip it
// //     continue;
// // }

// // handle_command(buffer);
// // }

// // return 0;
// // }


/* ICCS227: Project 1: icsh
 * Name: Chunya Pattharapinya
 * StudentID: u6581030
 */

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <unistd.h>     // fork(), execvp()
 #include <sys/wait.h>   // waitpid()
 #include <signal.h>

 pid_t fg_pid = 0;

 
 #define MAX_CMD_BUFFER 255
 #define MAX_ARGS 64
 
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
 
 // Handle built-in and external commands
 void handle_command(char* buffer) {
     char* args[MAX_ARGS];
     int arg_count = 0;
 
     // Tokenize buffer into args array
     args[arg_count] = strtok(buffer, " ");
     while (args[arg_count] != NULL && arg_count < MAX_ARGS - 1) {
         arg_count++;
         args[arg_count] = strtok(NULL, " ");
     }
 
     if (args[0] == NULL) return;
 
     // Built-in command: echo
     if (strcmp(args[0], "echo") == 0) {
         print_argument(args);
     }
 
     // Built-in command: exit [code]
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
         pid_t pid = fork();
 
         if (pid < 0) {
             perror("fork failed");
             return;
         }
 
         if (pid == 0) {
             // Child process: try to run command
             execvp(args[0], args);
             // If exec fails
             printf("bad command\n");
             exit(1);
         } else {
            fg_pid = pid; //set foreground PID
             // Parent process: wait for child
             int status;
             waitpid(pid, &status, 0);
             fg_pid = 0; //reset to 0
         }
     }
 }
 //track foreground job
 //define two handler
 void sigint_handler(int sig){
    if(fg_pid != 0){
        kill(fg_pid, SIGINT);
    }
 }
 void sigtstp_handler(int sig){
    if(fg_pid != 0){
        kill(fg_pid, SIGTSTP);
    }
 }

 void signalHandlerSetUP(){
    struct sigaction sa_int, sa_tstp;

    // Ctrl+C (SIGINT)
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    // Ctrl+Z (SIGTSTP)
    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = 0;
    sigaction(SIGTSTP, &sa_tstp, NULL);
}


 
 int main(int argc , char *argv[]) {
     FILE* input_stream = stdin;
     char buffer[MAX_CMD_BUFFER]; 
     char last_command[MAX_CMD_BUFFER] = "";  // store last command
 
     // If a script file is passed, read from file
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
 
         // Remove trailing newline
         for (int i = 0; buffer[i] != '\0'; i++) {
             if (buffer[i] == '\n') {
                 buffer[i] = '\0';
                 break;
             }
         }
 
         // Handle '!!'
         if (strcmp(buffer, "!!") == 0) {
             if (strlen(last_command) == 0) {
                 printf("No commands in history.\n");
                 continue;
             } else {
                 printf("%s\n", last_command);
                 strcpy(buffer, last_command);
             }
         } else if (strlen(buffer) > 0) {
             // Save the current command as the new last_command
             strcpy(last_command, buffer);
         } else {
             // If the input is empty, skip it
             continue;
         }
 
         handle_command(buffer);
     }
 
     return 0;


 }
 