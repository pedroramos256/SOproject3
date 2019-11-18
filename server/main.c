/*
*   File System made by
*       Pedro Ramos 92539 and Sancha Barroso 92557
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>

#include "fs.h"
#include "sync.h"

#include <sys/time.h>




pthread_t *tid;



int numberThreads;   
tecnicofs* fs;                  

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
/* index that determines where to insert and remove commands from the buffer */
int prodptr = 0, consptr = 0;

FILE *input;
FILE *output;

/******************************************************************************
 *                         FUNCTIONS USED IN MAIN
 *****************************************************************************/


static void displayUsage (const char* appName){
    printf("Usage: %s\n", appName);
    exit(EXIT_FAILURE);
}


static void parseArgs (long argc, char* const argv[]){
    if (argc != 5) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }
}

void insertCommand(char * line) {
    strcpy(inputCommands[prodptr], line);
     /* Incrementing a variable in a cyle */
    prodptr = (prodptr+1) % MAX_COMMANDS;
}

void removeCommand(char * command) {
    strcpy(command, inputCommands[consptr]);
    consptr = (consptr+1) % MAX_COMMANDS;
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    //exit(EXIT_FAILURE);
}

void * processInput(){
    char line[MAX_INPUT_SIZE];
    /* instead of stdin, it is used a file as input */
    while (fgets(line, sizeof(line)/sizeof(char), input)) {
        char token;
        char name1[MAX_INPUT_SIZE];
        char name2[MAX_INPUT_SIZE];
        int numTokens = sscanf(line, "%c %s %s", &token, name1, name2);
        /* locks producer semaphore with validation of value */
        SEM_WAIT(produce);
        /* perform minimal validation */
        if (numTokens < 1)
            continue;

        switch (token) {
            case 'c':
            case 'l':
            case 'd':
            case 'r':
                if(numTokens != 2 && numTokens != 3) {
                    errorParse();
                    return NULL;
                }
                LOCK1();
                insertCommand(line);
                UNLOCK1();
                /* unlocks consumer semaphore with validation of value */
                SEM_POST(consume);
                break;
            case '#':
                break;
            default: { /* error */
                errorParse();
            }
        }
    }
    LOCK1();
    strcpy(line, "\0");
     /* inserts "\0" in the buffer when file is out of commands */
    insertCommand(line);
    UNLOCK1();
    SEM_POST(consume);
    return NULL;
}


void * applyCommands(){
    char token;
    char name1[MAX_INPUT_SIZE];
    char name2[MAX_INPUT_SIZE];
    int numTokens;
    char command[MAX_INPUT_SIZE];
    int searchResult1;
    int searchResult2;
    int iNumber;

    while(1) {
        /* locks consumer semaphore with validation of value */
        SEM_WAIT(consume);
        LOCK2();
        /* removes command from buffer to execute */
        removeCommand(command);
        if (!strcmp(command, "\0")) {
            /* makes consumer's index stop incrementing */
            consptr = (consptr - 1) % MAX_COMMANDS;
            UNLOCK2();
            SEM_POST(consume);
            return NULL;
        }
        UNLOCK2();
        LOCK2();
        numTokens = sscanf(command, "%c %s %s", &token, name1, name2);
        if ((numTokens == 2 && token == 'r') || (numTokens == 3 && token != 'r')) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }
        /*If command is 'c', obtaining new iNumber will need to stay sequencial*/
        if (token != 'c') {
            UNLOCK2();
            /* unlocks producer semaphore with validation of value */
            SEM_POST(produce);
        }
        switch (token) {
            case 'c':
                iNumber = obtainNewInumber(fs);
                UNLOCK2();
                SEM_POST(produce);
                /* Writing lock because bst will change */
                create(fs, name1, iNumber);
                break;
            case 'l':
                /* Reading lock because bst can be searched in parallel */
                searchResult1 = lookup(fs, name1);
                if(!searchResult1)
                    printf("%s not found\n", name1);
                else
                    printf("%s found with inumber %d\n", name1, searchResult1);
                break;
            case 'd':
                /* Writing lock, same reason as 'c' */
                delete(fs, name1);
                break;
            case 'r':
                searchResult1 = lookup(fs, name1);
                searchResult2 = lookup(fs, name2);
                if(searchResult1 && !searchResult2)
                    exchange(fs, name1, name2, searchResult1);
                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return NULL;
}

void execute() {
    int i;
    for(i = 1; i < numberThreads+1; i++) {
        /* Creates threads in order to execute applyCommands in parallel*/
        if(pthread_create(&tid[i], NULL, applyCommands, NULL) == 0) {
            printf("Criada a tarefa %ld\n", tid[i]);
        }
        else {
            printf("Erro na criacao da tarefa\n");
            exit(1);
        }
    }
}

/*prints time*/
void execution_time(struct timeval start, struct timeval end) {
    double time = (end.tv_sec - start.tv_sec) * 1e6; 
    time = (time + (end.tv_usec - start.tv_usec)) * 1e-6; 

	printf("TecnicoFS completed in %.4f seconds.\n", time);
}


/******************************************************************************
 *                                  MAIN
 *****************************************************************************/


int main(int argc, char* argv[]) {
    struct timeval start, end;
    int i;

    parseArgs(argc, argv);

    /*using files given in arguments*/
    input = fopen(argv[1], "r");
    if (input == NULL) {
        printf("input file open failure\n");
        exit(EXIT_FAILURE);
    }
    output = fopen(argv[2], "w");
    if (output == NULL) {
        printf("output file open failure\n");
        exit(EXIT_FAILURE);
    }
    if(argc == 5) {
        #if defined(MUTEX) || defined(RWLOCK)
            numberThreads = atoi(argv[3]);
        /* forces numberThreads to be one when there is no syncronization */
        #else
            numberThreads = 1;
        #endif
        if (numberThreads <= 0) {
            printf("invalid number of threads\n");
            exit(EXIT_FAILURE);
        }
        numberBuckets = atoi(argv[4]);
        if(numberBuckets <= 0){
            printf("invalid number of buckets\n");
            exit(EXIT_FAILURE);
        }
    }

    fs = new_tecnicofs();

    /*using malloc to give more freedom in the amount of threads possible*/
    tid = (pthread_t*) malloc((numberThreads+1)*sizeof(pthread_t));

    /*verify if malloc was successful*/
    if (!tid) {
        printf("thread malloc failure\n");  
        exit(EXIT_FAILURE);
    }

    /*initializes the thread lockers and semaphores needed with validation */
    init();
    
    /*counting just the execution time*/
    gettimeofday(&start, NULL);

    if(pthread_create(&tid[0], NULL, processInput, NULL) == 0)
        printf("Criada a tarefa produtora %ld\n", tid[0]);
    else {
        printf("Erro na criacao da tarefa\n");
        exit(1);
    }
    execute();
    /* Ensure that threads wait for each other */
    for(i = 0; i < numberThreads+1; i++) {
        if(pthread_join(tid[i], NULL) == 0)
            printf("Join da tarefa %ld\n", tid[i]);
        else {
            printf("Erro no join da tarefa\n");
            exit(1);
        }
    }
    gettimeofday(&end, NULL);

    /*destroys the lockers*/
    destroy();

    /* instead of stdout, it is used a file as output */
    print_tecnicofs_tree(output, fs);

    /*closing files*/
    fclose(input);
    fclose(output);

    execution_time(start, end);

    free_tecnicofs(fs);
    free(tid);

    exit(EXIT_SUCCESS);
}


