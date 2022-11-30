#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "my-thread.h"

#define NUM_REINDEER 9
#define HELP_ELVES 3 // number of elves required to be able to ask Santa for help
#define SANTA_CLAUS "Santa Claus"
#define REINDEER "Reindeer"
#define ELF "Elf"

typedef enum {false, true} Boolean;

/* global variables */
int elves = 0; // elves counter protected by mutex
int reindeer = 0; // reindeer counter protected by mutex
sem_t santaSem; // this semaphore change Santa Claus state (sleep or wake up)
sem_t reindeerSem; // this semaphore notify reindeer to enter the paddock and get hitched
sem_t elfTex; // this mutex prevent additional elves from entering while three elves are being helped
sem_t mutex; // this mutex protect the countes of elves and reindeer

void getHitched(int i){
    printf("Reindeer %d is having hitch\n", i);
    sleep(2);
}

void getHelp(int i){
    printf("Elf %d asks Santa for help and waits\n", i);
    sleep(2);
}

void prepareSleigh(){
    printf("Santa Claus is preparing the sleigh\n");
    sleep(10);
}

void helpElves(){
    printf("Santa Claus is helping three elves\n");
    sleep(5);
}

void *exeSantaClaus(void *id) {
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problems with ptr allocation\n");
        exit(-1);
    }

    while(true){
        sem_wait(&santaSem);
        sem_wait(&mutex);
        if(reindeer >= NUM_REINDEER){
            prepareSleigh();
            for(int z = 0; z < NUM_REINDEER; z++)
                sem_post(&reindeerSem);
            reindeer -= NUM_REINDEER;
        } else if(elves == HELP_ELVES)
            helpElves();

        sem_post(&mutex);
    }


    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void *exeReindeer(void *id) {
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problems with ptr allocation\n");
        exit(-1);
    }

    sem_wait(&mutex);
    reindeer++;
    if(reindeer == NUM_REINDEER){
        printf("The reindeer %d is the last to arrive and takes Santa Claus with him\n", *pi);
        sem_post(&santaSem);
    } else
        printf("The reindeer %d has returned from vacation in the tropics\n", *pi);
    sem_post(&mutex);

    sem_wait(&reindeerSem);
    getHitched(*pi);

    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void *exeElf(void *id){
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problems with ptr allocation\n");
        exit(-1);
    }

    sem_wait(&elfTex);
    sem_wait(&mutex);
    elves++;
    if(elves == HELP_ELVES)
        sem_post(&santaSem);
    else
        sem_post(&elfTex);
    sem_post(&mutex);

    getHelp(*pi);

    sem_wait(&mutex);
    elves--;
    if(elves == 0)
        sem_post(&elfTex);
    sem_post(&mutex);

    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv) {
    ThreadInfo *threads;
    int i, pos;
    int *p;
    int NUM_ELVES;
    char error[250];

    // Chek number of parameters passed
    if (argc != 2) {
        sprintf(error, "Number of parameters expected = 1, number of parameters passed = %d\n", argc - 1);
        perror(error);
        exit(2);
    }

    NUM_ELVES = atoi(argv[1]);

    // Check number of reindeer passed
    if (NUM_ELVES < HELP_ELVES){
        sprintf(error, "Number of elves expected >= %d, number of elves passed =  %d\n",HELP_ELVES, NUM_ELVES);
        perror(error);
        exit(3);
    }

    threads = (ThreadInfo *) malloc((NUM_ELVES + NUM_REINDEER + 1) * sizeof(ThreadInfo));
    if(threads == NULL){
        perror("Problems with array threads allocation!\n");
        exit(4);
    }

    // Initialize Santa Claus Sempahore to 0
    if (sem_init(&santaSem, 0, 0) != 0) {
        perror("Problems with initialization of SANTA CLAUS sempahore\n");
        exit(5);
    }

    // Initialize reindeer Sempahore to 0
    if (sem_init(&reindeerSem, 0, 0) != 0) {
        perror("Problems with initialization of REINDEER sempahore\n");
        exit(6);
    }

    // Initialize elf mutex Sempahore to 1
    if (sem_init(&elfTex, 0, 1) != 0) {
        perror("Problems with initialization of ELF MUTEX sempahore\n");
        exit(7);
    }

    // Initialize mutex Sempahore to 1
    if (sem_init(&mutex, 0, 1) != 0) {
        perror("Problems with initialization of MUTEX sempahore\n");
        exit(8);
    }

    // Create Santa Claus thread
    threads[0].id = 0;
    strcpy(threads[0].tag, SANTA_CLAUS);
    threads[0].start_routine = exeSantaClaus;
    createThread(&threads[0], error);

    // Create Elves threads
    for (i = 0; i < NUM_ELVES; i++) {
        pos = i + 1; // assignment location to insert thread information
        threads[pos].id = i;
        strcpy(threads[pos].tag, ELF);
        threads[pos].start_routine = exeElf;
        createThread(&threads[pos], error);
    }

    // Create Reindeer threads
    for (i = 0; i < NUM_REINDEER; i++) {
        pos = NUM_ELVES + 1 + i; // assignment location to insert thread information
        threads[pos].id = i;
        strcpy(threads[pos].tag, REINDEER);
        threads[pos].start_routine = exeReindeer;
        createThread(&threads[pos], error);
    }

    // Wait threads termination
    for (i = 0; i < NUM_ELVES + NUM_REINDEER + 1; i++){
        if(strcmp(threads[i].tag, SANTA_CLAUS) == 0)
            printf("Since Santa Claus is an infinite loop we can NOT wait for him\n");
        else {
            int res;
            pthread_join(threads[i].thread, (void**) & p);
            res= *p;
            printf("Pthread %d-esimo returns %d\n", i, res);
        }
    }

    exit(0);
}
