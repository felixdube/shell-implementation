/**
    *******************************************************************************
    * File              : oscode.h
    * Description       : Implementation of a simple shell
    * Features          : History of the last 10 commands,
                            background processes, list of jobs in background
                            output redirection
    * Author            : Felix Dube 260533620
    * Version           : 1.0.0
    * Date              : February 2016
    *******************************************************************************
    */


/* JOB ---------------------------------------------------------------------------*/

// store in a linked list all the jobs in background, along with their PID
struct job
{
    char *args[20];
    int pid;
    struct job * next;
};

/**
    * @brief    remove a job with a particuliar PID
    * @param    head:   pointer to a pointer to the head of linked list of jobs
    * @param    pid:    pid of the job to be removed from the linked list
    * @retval   None
    */
void removejob(struct job ** head, int pid);

/**
    * @brief    print all the jobs in back ground and remove the job that are done
    * @param    head:   linked list of job to be printed and updated
    * @retval   None
    */
void printjobs(struct job ** head);

/**
    * @brief    add a job to the linked list
    * @param    head:   pointer to a pointer to the head of linked list of jobs
    * @param    pid:    pid of the job to be added
    * @param    args:   command that the job is doing
    * @retval   None
    */
void pushjob(struct job ** head, int pid, char *args[]);


/* HISTORY -----------------------------------------------------------------------*/

// store the command allong with its history ID
struct cmd
{
    char *args[20];
    int nbr;
};

/**
    * @brief    add a specific cmd to the history    
    * @param    history:    array in which the command need to be added
    * @param    args:       command to be added
    * @param    nbr:        history ID
    * @retval   void
    */
void addhistory(struct cmd *history[], char *args[], int nbr);

/**
    * @brief    search for a specific history ID in the history
    * @param    history:    history array to be search in
    * @param    nbr:        history ID being search for
    * @retval   if history ID found it returns the index of the cmd in the history array otherwise return -1        
    */
int searchhistory(struct cmd *history[], int nbr);

/**
    * @brief    verify if a string is a number
    * @param    string:    pointer to the string to be verified
    * @retval   1 if the string is a number, 0 otherwise
    */
int isnumber (char *string);

/**
    * @brief    print all commands in the history
    * @param    history:    pointer to the history array
    * @retval   None
    */
void printhistory(struct cmd *history[]);

/**
    * @brief    cast a string to int
    * @param    a:     pointer to the string to be cast to int
    * @retval   number that was in the string
    */
int stringtoint (char* a);


/* COMMAND -----------------------------------------------------------------------*/

/**
    * @brief    free the line allocated by getline()
    * @param    line:   pointer to the line
    * @retval   None
    */
void freecmd(char *line);

/**
    * @brief    get a command from the user 
    * @param    prompt:     prompt string to be displayed to the user
    * @param    args:       pointer to the array in which the command is saved
    * @param    background: pointer to the value of background
    * @retval   the number of argument in the command  
    */
int getcmd(char *prompt, char *args[], int *background);

/**
    * @brief    initialize all the arguments of the command to null
    * @param    args:   pointer to the arguments of the command
    * @retval   None
    */
void initargs(char *args[]);

/**
    * @brief    check if a command output is redirected
    * @param    args:   pointer to the arguments of the command
    * @retval   the position of the sign ">" in the array, -1 if not found
    */
int isredirected(char * args[]);

