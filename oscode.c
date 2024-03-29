/**
    ******************************************************************************
    * File              : oscode.c
    * Description       : Implementation of a simple shell
    * Features          : History of the last 10 commands,
                            background processes, list of jobs in background
                            output redirection
    * Author            : Felix Dube 260533620
    * Version           : 1.0.0
    * Date              : February 2016
    ******************************************************************************
    */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <limits.h>
#include <math.h>
#include "oscode.h"


int main()
{
    int status;
    // command variable
    char *args[20];
    int i;
    for (i = 0; i < 20; ++i)
    {
        args[i] = NULL;
    }

    // background variable
    int bg;

    // history variables
    int historynbr = 1;
    int *toBeSaved = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    struct cmd **history =  mmap(NULL, sizeof(struct cmd), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int notInitialized = 1;
    while(notInitialized) {
        int i;
        for (i = 0; i < 10; i++) {
            history[i] = (struct cmd*) malloc(sizeof(struct cmd));
        }
        notInitialized = 0;
    }

    // job variable
    struct job * head = NULL;

    
    printf("\n-----------------------------------------\nWelcome!\nThis is a simple shell. \nEnter 'help' for more info.\n-----------------------------------------\n\n");

    while (1) {
        int cnt = getcmd("\n>> ", args, &bg); 

        // int i;
        // for (i = 0; i < cnt; i++)
        //     printf("\nArg[%d] = %s", i, args[i]);

        // if (bg)
        //     printf("\nBackground enabled..\n");
        // else
        //     printf("\nBackground not enabled \n");

        printf("\n");



        /**********************************/
        /******** INTERNAL COMMAND ********/
        /**********************************/

        // HISTORY
        if (cnt != 0 && isnumber(args[0])) {
                int nbr =  stringtoint(args[0]);
                *toBeSaved = 0;
                // find the index of the command in the history, if it is there
                int index = searchhistory(history, nbr);
                if (index == -1){
                    printf("***ERROR  command ID '%s' does not exist\n\n", args[0]);
                }
                else {                
                    int j;
                    for (j = 0; j < 20; j++) {
                        args[j] = history[index]->args[j];
                    }

                    //check is it was in backgorund
                    int bgIndex;
                    if ((bgIndex = isbg(args)) > 0) {
                        bg = 1;
                        args[bgIndex] = NULL;
                    }
                }
        }

        // if nothing is entered
        if(cnt == 0){
            // do nothing
        }

        // HELP
        else if (strcmp(args[0], "help") == 0){
            printf("This is a simple shell brought to you by Felix Dube\n\nIt keeps the last 10 commands in HISTORY.\nEnter 'history' to see the list of commands in history.\n\nProcess can be run un BACKGROUND using the '&' argument.\nEnter 'jobs' to see the list of process running in background\nEnter 'fg' and the process job number to bring a process in the forground.\n\n");
        }

        // PRINT HISTORY
        else if (strcmp(args[0], "history") == 0) {
            printhistory(history);
            initargs(args);
        }
         // PRESENT WORKING DIRECTORY
        else if( strcmp(args[0], "pwd") == 0) {
            char* cwd;
            char buff[PATH_MAX + 1];

            cwd = getcwd( buff, PATH_MAX + 1 );
            if( cwd != NULL ) {
                printf( "My working directory is %s.\n", cwd );
            }
            addhistory(history, args, historynbr);
            historynbr++;
            initargs(args);
        }
        
        // CHANGE DIRECTORY
        else if( strcmp(args[0], "cd") == 0 ){
            chdir(args[1]);
            addhistory(history, args, historynbr);
            historynbr++;
            initargs(args);
       }

        // JOBS
        else if( strcmp(args[0], "jobs") == 0) {
            printjobs(&head);
            addhistory(history, args, historynbr);
            historynbr++;
            initargs(args);
        }

        // FOREGROUND
        else if( strcmp(args[0], "fg") == 0) {
            addhistory(history, args, historynbr);
            historynbr++;
            if (jobexist(head, stringtoint(args[1]))){
                waitpid(stringtoint(args[1]), &status, 0);
            }
            else{
                printf("***ERROR  job PID '%s' does not exist\n\n", args[1]);
            }
            initargs(args);
        }

        // EXIT
        else if ( strcmp(args[0], "exit") == 0){
            exit(0);

        }



        /**********************************/
        /******** EXTERNAL COMMAND ********/
        /**********************************/

        else if (cnt != 0) {
            *toBeSaved = 1;

            pid_t pid = fork();
            


            /**** PARENT ****/
            // the parent process either wait for the child process or not 
            // depending if the command is executed in background or not

            if ( pid != 0 ) {
                if (bg) {
                    pushjob(&head, pid, args);
                    // save the cmd if it was valid and not already in the history
                    if(*toBeSaved){
                        int i = 0;
                        while(args[i] != NULL){
                            i++;
                        }
                        args[i] = "&";
                        addhistory(history, args, historynbr);
                        historynbr++;
                    }
                    initargs(args);
                }
                else {
                    waitpid(pid, &status, 0);
                    // save the cmd if it was valid and not already in the history
                    if(*toBeSaved){
                        addhistory(history, args, historynbr);
                        historynbr++;
                    }
                    initargs(args);
                }
            }



            /**** CHILD ****/
            // the command is exucuted in the child process

            else {

                if (cnt != 0 && isnumber(args[0])){
                    exit(0);
                }

                // change the output of the process when specified (eg. ls > out.txt)
                int argNbr;
                if ((argNbr = isredirected(args)) > 0){
                    freopen(args[argNbr+1], "w", stdout);
                    args[argNbr] = NULL;
                    args[argNbr+1] = NULL;
                }

                // execute the command and make sure it is valid
                if (execvp(args[0], args) == -1) {
                    *toBeSaved = 0;
                    printf("***ERROR  invalid command\n");
                }
            }
        }
    }
}


/* JOB -------------------------------------------------------------------------*/

void removejob(struct job ** head, int pid){
    struct job * current = *head;
    struct job * temp_job = NULL;

    // if the job to be removed is the head
    if (current->pid == pid){
        temp_job = *head;
        *head = (*head)->next;
        free(temp_job);
        return;
    }

    // if not we need to find where is the job
    while(current->next != NULL && current->next->pid != pid){
        current = current->next;
    }
    if(current->next != NULL){
        temp_job = current->next;
        current->next = temp_job->next;
        free(temp_job);
    }

}

void printjobs(struct job ** head) {
    int i = 0;
    int status;
    struct job * current = *head;

    // check if any job need to be printed
    if (current == NULL) {
        printf("No job in background.\n");
    }
    else{
        printf("Jobs PID\tStatus\t\tJobs\n----------------------------------------------------------\n");
    }

    while (current != NULL) {

        // print pid
        printf("%i\t\t", current->pid);

        // print status
        int pid = current->pid;
        waitpid(pid, &status, WNOHANG);
        if(waitpid(pid, &status, WNOHANG) == -1){
            printf("Done\t\t");
        }
        else {
            printf("Working\t\t");
        }

        // print cmd
        int j = 0;
        while(current->args[j] !=  NULL){
            printf("%s\t", current->args[j]);
            j++;
        }
        printf("\n");

        // check if the process is terminated
        // if it is it is removed from the jobs list
        current = current->next;
        if(waitpid(pid, &status, WNOHANG) == -1){
            removejob(head, pid);
        }
    }
}

void pushjob(struct job ** head, int pid, char *args[]) {
    struct job * new_job;
    new_job = malloc(sizeof(struct job));

    new_job->pid = pid;
    new_job->next = *head;
    int j = 0 ;
    for (j = 0; j < 20; j++) {
        new_job->args[j] = args[j];
    }
    *head = new_job;
}

int jobexist(struct job * head, int pid){
    struct job * current = head;

    while(current != NULL){
        if (current->pid == pid){
            return 1;
        }
    }
    return 0;
}


/* HISTORY -----------------------------------------------------------------------*/

void addhistory(struct cmd *history[], char *args[], int nbr){

    // delete the first element of history and add the new command at position 9
    if (nbr > 10){
        int j;
        for (j = 0; j< 9; j++){
            history[j] = history[j+1];
        }
        history[9] = (struct cmd*) malloc(sizeof(struct cmd));
        for (j = 0; j < 20; j++) {
            history[9]->args[j] = args[j];
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

int isnumber (char *string) {
    int num;
    num = atoi( string );
 
    if (num == 0 && string[0] != '0')
       return 0;    //false
    else
       return 1;    //true
}

void printhistory(struct cmd *history[]) {
    int i = 0;

    // check if there is any history to be printed
    if (history[0]->nbr == 0) {
        printf("Nothing in history.\n");
    }
    else{
        printf("History ID\t\tCommand\n----------------------------------------------------------\n");
    }
    while (i < 10 && history[i]->nbr != 0) {
        printf("%i\t\t\t", history[i]->nbr);
        int j = 0;
        while(history[i]->args[j] !=  NULL){
            printf("%s\t", history[i]->args[j]);
            j++;
        }
        printf("\n");
        i++;
    }
}

int stringtoint (char* a) {
    int i;
    int index = 0;
    int add;
    for (i = 0; i < strlen(a); i++){

        // from the ascci of the char get the number
        add = a[i] -48;

        // multiply by some power of 10 if required and add to the final number
        index +=  (add)*( pow( 10, (strlen(a) -i -1)));
    }
    return index;
}

int isbg(char * args[]){
    int i = 0;
    while(args[i] != NULL){
        if (strcmp(args[i], "&") == 0){
            return i;
        }
        i++;
    }
    return -1;
}


/* COMMAND -----------------------------------------------------------------------*/

void freecmd(char *line) {
    free(line);
    return;
}

int getcmd(char *prompt, char *args[], int *background) {
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
    freecmd(line);
    return i;
}

void initargs(char *args[]){
    int i;
    for (i = 0; i < 20; ++i)
    {
        args[i] = NULL;
    }
    return;
}

int isredirected(char * args[]){
    int i = 0;
    while(args[i] != NULL){
        if (strcmp(args[i],  ">") == 0){
            return i;
        }
        i++;
    }
    return -1;
}

