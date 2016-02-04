/***********************************************************************/
// File: oscode.c
// Description: Implement a simple shell
// Features: History of the last 10 commands, 
// Date: February 2016
// Author: Felix Dube 260533620
/***********************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <limits.h>


/*********************/
/******** JOB ********/        
/*********************/
struct job
{
    char *args[20];
    int nbr;
};

void printJobs(struct cmd *job[]) {
    int i = 0;
    if (jobs[0]->nbr == 0) {
        printf("No job in background.\n");
    }
    else{
        printf("Jobs ID\t\tJobs\n----------------------------------------------------------\n");
    }
    while (jobs[i]->nbr != 0) {
        printf("%i\t\t\t", jobs[i]->nbr);
        int j = 0;
        while(jobs[i]->args[j] !=  NULL){
            printf("%s", history[i]->args[j]);
            j++;
        }
        printf("\n");
        i++;
    }
}

/*************************/
/******** HISTORY ********/        
/*************************/

struct cmd
{
    char *args[20];
    int nbr;
};

void addhistory(struct cmd *history[], char *args[], int nbr){

    // delete the first element of history and add the new command at position 9
    if (nbr > 10){
        int j;
        for (j = 0; j< 9; j++){
            history[j] = history[j+1];
        }
        for (j = 0; j < 20; j++) {
            strcpy(history[9]->args[j], args[j]);
        }
        history[9]->nbr = nbr;
    }

    // add the new command at the first empty spot in the history
    else{
        int j;
        for (j = 0; j < 20; j++) {
            history[nbr -1]->args[j] = args[j];
        }
        history[nbr -1]->nbr = nbr;
    }
    return;
}

int searchhistory(struct cmd *history[], int nbr) {
    int i;
    for (i = 0; i < 10; ++i)
    {
        if (history[i]->nbr == nbr){
            return i;
        }
    }

    return -1;
}

int isNumber (char *string) {
    int num;
    num = atoi( string );
 
    if (num == 0 && string[0] != '0')
       return 0;    //false
    else
       return 1;    //true
}

void printHistory(struct cmd *history[]) {
    int i = 0;
    if (history[0]->nbr == 0) {
        printf("Nothing in history.\n");
    }
    else{
        printf("History ID\t\tCommand\n----------------------------------------------------------\n");
    }
    while (history[i]->nbr != 0) {
        printf("%i\t\t\t", history[i]->nbr);
        int j = 0;
        while(history[i]->args[j] !=  NULL){
            printf("%s", history[i]->args[j]);
            j++;
        }
        printf("\n");
        i++;
    }
}



/*************************/
/******** COMMAND ********/
/*************************/

int getcmd(char *prompt, char *args[], int *background)
{
    int length, i = 0;
    char *token, *loc;
    char *line = NULL;
    size_t linecap = 0;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);

    if (length <= 0) {
        exit(-1);
    }


    // Check if background is specified..
    if ((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else
        *background = 0;

    while ((token = strsep(&line, " \t\n")) != NULL) {
        int j;
        for (j = 0; j < strlen(token); j++)
            if (token[j] <= 32)
                token[j] = '\0';
        if (strlen(token) > 0)
            args[i++] = token;
    }
    return i;
}

void freecmd(char *args[]) {
    int i;
    for (i = 0; i < 20; ++i)
    {
        args[i] = NULL;
    }
    return;
}



/**********************/
/******** Main ********/
/**********************/

int main()
{
    char *args[20];
    int i;
    for (i = 0; i < 20; ++i)
    {
        args[i] = NULL;
    }
    int bg;
    int status;
    int historynbr = 1;
    int *toBeSaved = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    // allocate memory
    struct cmd **history =  mmap(NULL, sizeof(struct cmd), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int notInitialized = 1;
    while(notInitialized) {
        int i;
        for (i = 0; i < 10; i++) {
            history[i] = (struct cmd*) malloc(sizeof(struct cmd));
        }
        notInitialized = 0;
    }
    
    printf("\n-----------------------------------------\nWelcome!\nThis is a simple shell. \nEnter 'help' for more info.\n-----------------------------------------\n\n");

    while (1) {
        int cnt = getcmd("\n>> ", args, &bg); 

        int i;
        for (i = 0; i < cnt; i++)
            printf("\nArg[%d] = %s", i, args[i]);

        if (bg)
            printf("\nBackground enabled..\n");
        else
            printf("\nBackground not enabled \n");

        printf("\n\n");

        // HELP
        if (strcmp(args[0], "help") == 0){
            printf("This is a simple shell brought to you by Felix Dube\n\nIt keeps the last 10 commands in HISTORY.\nEnter 'history' to see the list of commands in history.\n\nProcess can be run un BACKGROUND using the '&' argument.\nEnter 'jobs' to see the list of process running in background\nEnter 'fg' and the process job number to bring a process in the forground.\n\n");
        }

        // PRINT HISTORY
        else if (strcmp(args[0], "history") == 0) {
            printHistory(history);
        }
         // PRESENT WORKING DIRECTORY
        else if( strcmp(args[0], "pwd") == 0) {
            *toBeSaved = 1;
            char* cwd;
            char buff[PATH_MAX + 1];

            cwd = getcwd( buff, PATH_MAX + 1 );
            if( cwd != NULL ) {
                printf( "My working directory is %s.\n", cwd );
            }
            addhistory(history, args, historynbr);
            historynbr++;
            freecmd(args);
        }
        
        // CHANGE DIRECTORY
        else if( strcmp(args[0], "cd") == 0 ){
            chdir(args[1]);
            addhistory(history, args, historynbr);
            historynbr++;
            freecmd(args);
       }

        // JOBS
        else if( strcmp(args[0], "jobs") == 0) {
            //printJobs(jobs);
            addhistory(history, args, historynbr);
            historynbr++;
            freecmd(args);
        }

        // FOREGROUND
        else if( strcmp(args[0], "fg") == 0) {
            *toBeSaved = 1;

        }

        // EXIT
        else if ( strcmp(args[0], "exit") == 0){
            exit(0);

        }

        else{
            *toBeSaved = 1;

            pid_t pid = fork();
            
            if ( pid != 0 ) {

                /**** PARENT ****/

                if (bg) {
                    main();
                }
                else {
                    waitpid(pid, &status, 0);
                    // save the cmd if it was valid and not already in the history
                    if(*toBeSaved){
                        addhistory(history, args, historynbr);
                        historynbr++;
                    }
                    freecmd(args);
                }
            }
            else {

                /**** CHILD ****/
                // HISTORY
                if (isNumber(args[0])) {
                        int nbr =  *args[0] - 48;
                        *toBeSaved = 0;
                        // find the index of the command in the history, if it is there
                        int index = searchhistory(history, nbr);
                        if (index == -1){
                            printf("***ERROR  command ID '%s' does not exist\n\n", args[0]);
                            exit(0);
                        }
                        else {                
                            int j;
                            for (j = 0; j < 20; j++) {
                                args[j] = history[index]->args[j];
                            }
                        }
                }
                // execute the command and make sure it is valid
                if (execvp(args[0], args) == -1) {
                    *toBeSaved = 0;
                    printf("***ERROR  invalid command");
                }
            }
        }
    }
}


