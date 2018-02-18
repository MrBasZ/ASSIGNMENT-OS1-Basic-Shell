#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#define COLOR_NONE "\033[0m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN "\033[1;36m"
#define COLOR_GREEN "\033[1;32m"

#define COMMAND_BUFSIZE 1024
#define TOKEN_BUFSIZE 64
#define TOKEN_DELIMITERS " \t\r\n\a"
#define CMD_DELIMITERS ";"

//prototype function
void mysh_init();
void mysh_batch(char** argv);
void mysh_loop();
void mysh_print_promt();
void mysh_sigint_handler(int signal);
char* mysh_read_command();
char** mysh_split_command(char* cmd);
char** mysh_split_concurrent_command(char* cmd);
int mysh_execute_command(char** args);
int mysh_process_command(char** args);
int mysh_check_concurrent(char* cmd);

//global variable
pid_t child_pid, kill_pid;
int is_concurrent = 0;
int idx = 0;

int main(int argc, char **argv)
{
    //initail signal
    mysh_init();

    //batch mode
    mysh_batch(argv);

    //main process
    mysh_loop();
    
    return EXIT_SUCCESS;
}

void mysh_loop(){
    char* cmd;
    char** args;
    int status;
    char** concur;

    do{
        mysh_print_promt();
        cmd = mysh_read_command(); //input command by user

        if(mysh_check_concurrent(cmd)){
            //concurrent
            is_concurrent = 1;
            concur = mysh_split_concurrent_command(cmd);
            status = mysh_process_command(concur);
            idx = 0;
        }else {
            //no concurrent
            is_concurrent = 0;
            args = mysh_split_command(cmd);
            status = mysh_process_command(args);
        }

        //Clear memory
        free(cmd);
        free(args);
    }while(status);
}

/*--------------intitial terminate signal---------------*/

void mysh_init(){
    system("clear");

    // Ignore signalint (Crtl+c)
    struct sigaction sigint_action = {
        // Setup the sighub handler
        .sa_handler = &mysh_sigint_handler,
        // Restart the system call
        .sa_flags = 0
    };

    // Block every signal during the handler
    sigemptyset(&sigint_action.sa_mask);

    // Intercept SIGINT
    sigaction(SIGINT, &sigint_action, NULL);

    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}

/*--------------Step 1 read command----------------------*/

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

/*--------------Step 2 parsing & process -----------------*/

//check concurrent
int mysh_check_concurrent(char* cmd){
    int i = 0;
    while(cmd[i] != '\0'){
        if(cmd[i] == ';')
            return 1;
        i++;
    }
    return 0;
}

//no concurret (split string with ' \t\r\n\a')
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

    tokens[posi] = NULL;
    return tokens;
}

//concurret (split string with ';')
char** mysh_split_concurrent_command(char* cmd){
    int posi = 0;
    char **tokens = malloc(sizeof(char*) * TOKEN_BUFSIZE);
    char *token;

    //get the first tokens
    token = strtok(cmd, CMD_DELIMITERS);
    
    //walk through other tokens
    while(token != NULL){
        tokens[posi] = token;
        posi++;

        //get next token with the same string
        token = strtok(NULL, CMD_DELIMITERS);
    }
    tokens[posi] = NULL;
    return tokens;
}

//process befor execute
int mysh_process_command(char** args){
    //no concurrent
    if(!(is_concurrent)){
        if (args[0] == NULL) {
            // An empty command was entered.
            return 1;
        }

        //terminate program when received 'quit' command
        if(strcmp(args[0], "quit") == 0){
            printf(COLOR_CYAN "Goodbye :)\nTerminated\n");
            exit(0);
        }
    }
    //run command
    return mysh_execute_command(args);
}

/*--------------------Step 3 execute---------------------*/

int mysh_execute_command(char** args){
    int status;
    char** concur;

    fflush(NULL);
    child_pid = fork();
    if (child_pid == 0) {
        // Child process
        
        if(is_concurrent){
            kill_pid = child_pid;

            //split for concurrent command
            concur = mysh_split_command(*(args + idx));
            
            //terminate program
            if(!strcmp(concur[0], "quit")){
                printf(COLOR_CYAN "Goodbye :)\n");
                kill(kill_pid, SIGTERM);
                exit(0);
            }

            printf(COLOR_NONE);
            if (execvp(concur[0], concur) < 0) {
                fprintf(stderr, COLOR_RED "mysh: %s command not found\n",concur[0]);
            }
            free(concur);
        }else{
            kill_pid = child_pid;

            printf(COLOR_NONE);
            if (execvp(args[0], args) < 0) {
                fprintf(stderr, COLOR_RED "mysh: %s command not found\n",args[0]);
            }
        }

        exit(EXIT_FAILURE);
    } else if (child_pid < 0) {
        // Error forking
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    } else {
        // Parent process

        //Recursive execute concurrent command
        if(is_concurrent && *(args + idx)){
            idx++;
            mysh_execute_command(args);
        }
        //wait child process
        wait(0);
  }

  return 1;
}

/*--------------read command with batch file------------*/

void mysh_batch(char** argv){
    //No argv Files
    if(argv[1] == NULL)
        return;

    int input_fd, end_fd;
    int status;
    char* batch_cmd;
    char* batch_line = malloc(sizeof(char*) * TOKEN_BUFSIZE);
    char** batch_parsing;
    char** batch_concur;

    //Open file descriptors read only
    input_fd = open (argv[1], O_RDONLY);
    if (input_fd == -1) {
        // open failed
        fprintf(stderr, COLOR_RED "%s: No such file\n" COLOR_NONE,argv[1]);
        return;
    }
    
    //read file into string
    end_fd = read(input_fd, &batch_line[0], COMMAND_BUFSIZE);
    batch_line[end_fd] = '\0';

    //Close file descriptors
    close (input_fd);

    //split each command with '\n'
    while ((batch_cmd = strsep(&batch_line, "\n")) != NULL){
        if(mysh_check_concurrent(batch_cmd)){
            //concurrent
            is_concurrent = 1;
            batch_concur = mysh_split_concurrent_command(batch_cmd);
            status = mysh_process_command(batch_concur);
            idx = 0;
        }else {
            //no concurrent
            is_concurrent = 0;
            batch_parsing = mysh_split_command(batch_cmd);
            status = mysh_process_command(batch_parsing);
        }
        //if(!status) return;
    }

    //clear memory
    free(batch_line);
    return;
}

/*--------------------MISC---------------------*/

//printf peompt mysh>
void mysh_print_promt() {
    printf(COLOR_GREEN "mysh>" COLOR_NONE " ");
}

//catch signal when press key (Ctrl-c)
void mysh_sigint_handler(int signal) {
    //Kill zombie process
    waitpid(kill_pid, NULL, 0);
    //system("clear");
    printf(COLOR_YELLOW "\nHint:" COLOR_NONE " You can exit program using the" COLOR_RED " 'quit' " COLOR_NONE "command.\n");
}