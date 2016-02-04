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


/*************************/
/******** HISTORY ********/        
/************************/

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
        printf("add\n");
        int j;
        for (j = 0; j < 20; j++) {
            history[nbr -1]->args[j] = args[j];
        }
        printf("%i\n", nbr);
        history[nbr -1]->nbr = nbr;
        printf("%i\n", history[0]->nbr);
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
                    printf("\n add history \n");
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
                *toBeSaved = 0;

                int nbr =  *args[0] - 48;

                // find the index of the command in the history, if it is there
                int index = searchhistory(history, nbr);
                if (index == -1){
                    printf("***ERROR  %s\n not in history\n\n", args[0]);
                    exit(0);
                }
                else {                
                    int j;
                    for (j = 0; j < 20; j++) {
                        args[j] = history[index]->args[j];
                    }
                }
            }

            // CHANGE DIRECTORY
            else if( args[0] == "cd"){

           }

            // PRESENT WORKING DIRECTORY
            else if( args[0] == "pwd") {

            }

            // JOBS
            else if( args[0] == "jobs") {

            }

            // FOREGROUND
            else if( args[0] == "fg") {

            }

            else{
                *toBeSaved = 1;
            }

            // execute the command and make sure it is valid
            if (execvp(args[0], args) == -1) {
                *toBeSaved = 0;
                printf("***ERROR  invalid command");
            }
        }
    }
}


