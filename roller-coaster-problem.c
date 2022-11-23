#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define CAR_CAPACITY 3

typedef enum {false, true} Boolean;

/* global variables */
int activePassengers; // number of total passengers threads active
int boarders = 0; // counter of passengers on board
int unboarders = 0; // counter of passengers unboard
sem_t boardQueue; // passengers wait on this sempahore before boarding
sem_t unboardQueue; // passengers wait on this semaphore before unboarding
sem_t allAboard; // indicates that the car is full
sem_t allAshore; // indicates that the car is empty
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // this mutex protect boarders, which count the number of passengers that have invoked boardCar
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void board(int i){
    printf("The passenger %d is getting into the car\n", i);
    sleep(2);
}

void unboard(int i){
    printf("The passenger %d is getting out of the car\n", i);
    sleep(2);
}

void run(){
    printf("The car is doing a lap around the track\n");
    sleep(4);
}

void load(){
    printf("The car is loading the passengers on board\n");
    sleep(2);
}

void unload(){
    printf("The car is unloading the passengers from the car\n");
    sleep(2);
}

void *exeCar(void *id) {
    int *pi = (int *)id;
    int *ptr;
    int j;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problems with ptr allocation\n");
        exit(-1);
    }

    while(activePassengers > 0){
        load();
        for(j = 0; j < CAR_CAPACITY; j++)
            sem_post(&boardQueue);
        sem_wait(&allAboard);

        run();

        unload();
        for(j = 0; j < CAR_CAPACITY; j++)
            sem_post(&unboardQueue);
        sem_wait(&allAshore);
    }


    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void *exePassenger(void *id) {
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problems with ptr allocation\n");
        exit(-1);
    }

    sem_wait(&boardQueue);
    board(*pi);

    pthread_mutex_lock(&mutex);
    boarders++;
    if(boarders == CAR_CAPACITY){
        sem_post(&allAboard);
        boarders = 0;
    }
    pthread_mutex_unlock(&mutex);

    sem_wait(&unboardQueue);
    unboard(*pi);

    pthread_mutex_lock(&mutex2);
    unboarders++;
    if(unboarders == CAR_CAPACITY){
        sem_post(&allAshore);
        unboarders = 0;
    }
    pthread_mutex_unlock(&mutex2);

    activePassengers--;

    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv) {
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    int NUM_PASSENGERS;
    char error[250];

    // Chek number of parameters passed
    if (argc != 2) {
        sprintf(error, "Number of parameters expected = 1, number of parameters passed = %d\n", argc - 1);
        perror(error);
        exit(1);
    }

    NUM_PASSENGERS = atoi(argv[1]);

    // Check number of passengers passed
    if (NUM_PASSENGERS < CAR_CAPACITY){
        sprintf(error, "Number of passengers expected > %d, number of passengers passed = %d\n", CAR_CAPACITY, NUM_PASSENGERS);
        perror(error);
        exit(2);
    }

    activePassengers = NUM_PASSENGERS;

    thread = (pthread_t *) malloc((NUM_PASSENGERS + 1) * sizeof(pthread_t));
    if (thread == NULL) {
        perror("Problems with array thread allocation!\n");
        exit(3);
    }

    taskids = (int *) malloc((NUM_PASSENGERS + 1) * sizeof(int));
    if (taskids == NULL) {
        perror("Problems with array taskids allocation!\n");
        exit(4);
    }

    // Initialize board queue Sempahore to 0
    if (sem_init(&boardQueue, 0, 0) != 0) {
        perror("Problems with initialization of board queue sempahore\n");
        exit(5);
    }

    // Initialize unboard queue Sempahore to 0
    if (sem_init(&unboardQueue, 0, 0) != 0) {
        perror("Problems with initialization of unboard queue sempahore\n");
        exit(6);
    }

    // Initialize all aboard Sempahore to 0
    if (sem_init(&allAboard, 0, 0) != 0) {
        perror("Problems with initialization of all aboard sempahore\n");
        exit(7);
    }

    // Initialize all ashore Sempahore to 0
    if (sem_init(&allAshore, 0, 0) != 0) {
        perror("Problems with initialization of all ashore sempahore\n");
        exit(8);
    }

    // Create car thread
    taskids[0] = 0;
    printf("I'm about to create the CAR thread\n");
    if (pthread_create(&thread[0], NULL, exeCar, (void *) (&taskids[0])) != 0) {
            sprintf(error,"I'm MAIN THREAD and something went wrong with creation of CAR THREAD\n");
            perror(error);
            exit(9);
    }
    printf("I'm MAIN THREAD and I've created CAR THREAD\n");

    // Create passengers threads
    for (i = 0; i < NUM_PASSENGERS; i++) {
        int pos = i + 1;
        taskids[pos] = i;
        printf("I'm about to create the PASSENGER %d-esimo\n", taskids[pos]);
        if (pthread_create(&thread[pos], NULL, exePassenger, (void *) (&taskids[pos])) != 0){
                sprintf(error,"I'm MAIN THREAD and something went wrong with creation of PASSENGER THREAD %d-esimo\n", taskids[pos]);
                perror(error);
                exit(10);
        }
        printf("I'm MAIN THREAD and I've created PASSENGER THREAD with id = %lu\n", thread[pos]);
    }

    // Wait threads termination
    for (i = 0; i < NUM_PASSENGERS + 1; i++){
        int ris;
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo returns %d\n", i, ris);
    }

    exit(0);
}
