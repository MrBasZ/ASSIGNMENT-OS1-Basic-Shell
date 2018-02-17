#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define COLOR_NONE "\033[0m"
#define COLOR_RED "\033[1;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_GRAY "\033[1;30m"

#define COMMAND_BUFSIZE 1024
#define TOKEN_BUFSIZE 64
#define TOKEN_DELIMITERS " \t\r\n\a"
#define CMD_DELIMITERS ";"

//prototype function
void mysh_init();
void mysh_loop();
void mysh_print_promt();
void sigint_handler(int signal);
char* mysh_read_command();
char** mysh_split_command(char* cmd);
char** mysh_split_concurrent_command(char* cmd);
int mysh_execute_command(char** args);
int mysh_process_command(char** args);

//builtin commands
char *builtin_str[] = {
  "shell",
  "quit"
};


int main(int argc, char **argv)
{
    mysh_init();
    mysh_loop();

    return EXIT_SUCCESS;
}

void mysh_loop()
{
    char* cmd;
    char** args;
    int state;
    char** concur;

    cmd = mysh_read_command();
    concur = mysh_split_concurrent_command(cmd);

    for (int i = 0; *(concur + i); i++)
        {
            printf("cut=[%s]\n", *(concur + i));
        }

    // do{
    //     mysh_print_promt();
    //     cmd = mysh_read_command();
    //     args = mysh_split_command(cmd);
    //     state = mysh_process_command(args);

    //     //Clear memory
    //     free(cmd);
    //     free(args);
    // }while(state);
}

char** mysh_split_concurrent_command(char* cmd){
    int posi = 0;
    char **tokens = malloc(sizeof(char*) * TOKEN_BUFSIZE);
    char *token;

    //get the first token
    token = strtok(cmd, CMD_DELIMITERS);
    
    //walk through other tokens
    while(token != NULL){
        //printf("%s\n",token);
        if(token == "quit")
            exit(0);
            
        tokens[posi] = token;
        posi++;

        //get next token with the same string
        token = strtok(NULL, CMD_DELIMITERS);
    }
    // for(token = strtok(cmd, TOKEN_DELIMITERS); token != NULL; token = strtok(NULL, TOKEN_DELIMITERS)){
        
    // }
    tokens[posi] = NULL;
    return tokens;
}

int mysh_process_command(char** args){
    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    return mysh_execute_command(args);
}

int mysh_execute_command(char** args){
    pid_t child_pid, end_pid;
    int status;
    child_pid = fork();
    if (child_pid == 0) {
        // Child processstatus
        // signal(SIGINT, SIG_DFL);
        // signal(SIGQUIT, SIG_DFL);
        // signal(SIGTSTP, SIG_DFL);
        // signal(SIGTTIN, SIG_DFL);
        // signal(SIGTTOU, SIG_DFL);
        // signal(SIGCHLD, SIG_DFL);

        //printf( "[son] pid %d from pid %d\n", getpid(), getppid());
        if (execvp(args[0], args) < 0) {
            fprintf(stderr, "mysh: %s command not found\n",args[0]);
        }
        exit(EXIT_FAILURE);
    } else if (child_pid < 0) {
        // Error forking
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
    //    printf( "[dad] pid %d\n", getpid());
       do {
            waitpid(child_pid, &status, WUNTRACED);
            //printf("\n%d\n",status);
       } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        // end_pid = waitpid(child_pid, &status, WNOHANG|WUNTRACED);
        // if (end_pid == child_pid) {  /* child ended                 */
        //       if (WIFEXITED(status))
        //          printf("Child ended normally.n");
        //       else if (WIFSIGNALED(status))
        //          printf("Child ended because of an uncaught signal.n");
        //       else if (WIFSTOPPED(status))
        //          printf("Child process has stopped.n");
        //       exit(EXIT_SUCCESS);
        // }
  }

  return 1;
}

char** mysh_split_command(char* cmd){
    int posi = 0;
    char **tokens = malloc(sizeof(char*) * TOKEN_BUFSIZE);
    char *token;

    //get the first token
    token = strtok(cmd, TOKEN_DELIMITERS);
    
    //walk through other tokens
    while(token != NULL){
        tokens[posi] = token;
        posi++;

        //get next token with the same string
        token = strtok(NULL, TOKEN_DELIMITERS);
    }
    // for(token = strtok(cmd, TOKEN_DELIMITERS); token != NULL; token = strtok(NULL, TOKEN_DELIMITERS)){
        
    // }
    tokens[posi] = NULL;
    return tokens;
}

void mysh_print_promt() {
    printf(COLOR_GREEN "mysh>" COLOR_NONE " ");
}

char* mysh_read_command(){
    int bufsize = COMMAND_BUFSIZE;
    int position = 0;
    int c;
    char *buffer = malloc(sizeof(char) * COMMAND_BUFSIZE);
    
    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
    }
}

void mysh_init(){
    system("clear");

    // Ignore signalint (Crtl+c)
    struct sigaction sigint_action = {
        // Setup the sighub handler
        .sa_handler = &sigint_handler,
        // Restart the system call, if at all possible
        .sa_flags = 0
    };

    // Block every signal during the handler
    sigemptyset(&sigint_action.sa_mask);

    // Intercept SIGINT
    if ((sigaction(SIGINT, &sigint_action, NULL)) == -1) {
        perror("Error: cannot handle SIGINT"); // Should not happen
    }

    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}

void sigint_handler(int signal) {
    //printf( "[son] pid %d from pid %d\n", getpid(), getppid());
    system("cleard");
    printf(COLOR_YELLOW "\nHint:" COLOR_NONE " You can exit program using the" COLOR_RED " 'quit' " COLOR_NONE "command.\n");
}