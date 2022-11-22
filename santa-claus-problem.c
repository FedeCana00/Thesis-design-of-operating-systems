#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_REINDEER 9

typedef enum {false, true} Boolean;

/* global variables */
int activeElves; // number of total elves threads active
int activeReindeer = NUM_REINDEER; // number of total reindeer active
int elves = 0; // elves counter protected by mutex
int reindeer = 0; // reindeer counter protected by mutex
sem_t santaSem; // this semaphore change Santa Claus state (sleep or wake up)
sem_t reindeerSem; // this semaphore notify reindeer to enter the paddock and get hitched
pthread_mutex_t elfTex = PTHREAD_MUTEX_INITIALIZER; // this mutex prevent additional elves from entering while three elves are being helped
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // this mutex protect the countes of elves and reindeer

void getHitched(int i){
    printf("Reindeer %d is having hitch\n", i);
    sleep(2);
}

void getHelp(int i){
    printf("Elve %d is getting help by Santa Claus\n", i);
    sleep(2);
}

void prepareSleigh(){
    printf("Santa Claus is preparing the sleigh\n");
    sleep(2);
}

void helpElves(){
    printf("Santa Claus is helping three elves\n");
    sleep(2);
}

void *exeSantaClaus(void *id) {
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problems with ptr allocation\n");
        exit(-1);
    }

    while(activeElves > 0 && activeReindeer > 0){
        sem_wait(&santaSem);
        pthread_mutex_lock(&mutex);
        if(reindeer >= 9){
            prepareSleigh();
            for(int z = 0; z < 9; z++)
                sem_post(&reindeerSem);
            reindeer -= 9;
        } else if(elves == 3)
            helpElves();

        pthread_mutex_unlock(&mutex);
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

    pthread_mutex_lock(&mutex);
    reindeer++;
    if(reindeer == 9)
        sem_post(&santaSem);
    pthread_mutex_unlock(&mutex);

    sem_wait(&reindeerSem);
    getHitched(*pi);

    activeReindeer--;

    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void *exeElve(void *id){
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problems with ptr allocation\n");
        exit(-1);
    }

    pthread_mutex_lock(&elfTex);
    pthread_mutex_lock(&mutex);
    elves++;
    if(elves == 3)
        sem_post(&santaSem);
    else
        pthread_mutex_unlock(&elfTex);
    pthread_mutex_unlock(&mutex);

    getHelp(*pi);

    pthread_mutex_lock(&mutex);
    elves--;
    if(elves == 0)
        pthread_mutex_unlock(&elfTex);
    pthread_mutex_unlock(&mutex);

    activeElves--;

    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv) {
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    int NUM_ELVES;
    char error[250];

    // Chek number of parameters passed
    if (argc != 2) {
        sprintf(error, "Number of parameters expected = 1, number of parameters passed = %d\n", argc - 1);
        perror(error);
        exit(1);
    }

    NUM_ELVES = atoi(argv[1]);

    // Check number of reindeer passed
    if (NUM_ELVES < 3){
        sprintf(error, "Number of elves expected >= 3, number of elves passed =  %d\n", NUM_ELVES);
        perror(error);
        exit(2);
    }

    activeElves = NUM_ELVES;

    thread = (pthread_t *) malloc((NUM_ELVES + NUM_REINDEER + 1) * sizeof(pthread_t));
    if (thread == NULL) {
        perror("Problems with array thread allocation!\n");
        exit(3);
    }

    taskids = (int *) malloc((NUM_ELVES + NUM_REINDEER + 1) * sizeof(int));
    if (taskids == NULL) {
        perror("Problems with array taskids allocation!\n");
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

    // Create Santa Claus thread
    taskids[0] = 0;
    printf("I'm about to create the SANTA CLAUS thread\n");
    if (pthread_create(&thread[0], NULL, exeSantaClaus, (void *) (&taskids[0])) != 0) {
            sprintf(error,"I'm MAIN THREAD and something went wrong with creation of SANTA CLAUS THREAD\n");
            perror(error);
            exit(7);
    }
    printf("I'm MAIN THREAD and I've created SANTA CLAUS THREAD\n");

    // Create Elves threads
    for (i = 0; i < NUM_ELVES; i++) {
        int pos = i + 1;
        taskids[pos] = i;
        printf("I'm about to create the ELVE %d-esimo\n", taskids[pos]);
        if (pthread_create(&thread[pos], NULL, exeElve, (void *) (&taskids[pos])) != 0){
                sprintf(error,"I'm MAIN THREAD and something went wrong with creation of ELVE THREAD %d-esimo\n", taskids[pos]);
                perror(error);
                exit(8);
        }
        printf("I'm MAIN THREAD and I've created ELVE THREAD with id = %lu\n", thread[pos]);
    }

    // Create Reindeer threads
    for (i = 0; i < NUM_REINDEER; i++) {
        int pos = NUM_ELVES + 1 + i;
        taskids[pos] = i;
        printf("I'm about to create the REINDEER %d-esimo\n", taskids[pos]);
        if (pthread_create(&thread[pos], NULL, exeReindeer, (void *) (&taskids[pos])) != 0){
                sprintf(error,"I'm MAIN THREAD and something went wrong with creation of REINDEER THREAD %d-esimo\n", taskids[pos]);
                perror(error);
                exit(9);
        }
        printf("I'm MAIN THREAD and I've created REINDEER THREAD with id = %lu\n", thread[pos]);
    }

    // Wait threads termination
    for (i = 0; i < NUM_ELVES + NUM_REINDEER + 1; i++){
        int ris;
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo returns %d\n", i, ris);
    }

    exit(0);
}
