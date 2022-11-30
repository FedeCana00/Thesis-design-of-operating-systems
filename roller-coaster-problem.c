#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "my-thread.h"

#define CAR_CAPACITY 3
#define CAR "Car"
#define PASSENGER "Passenger"

typedef enum {false, true} Boolean;

/* global variables */
int boarders = 0; // counter of passengers on board
int unboarders = 0; // counter of passengers unboard
sem_t boardQueue; // passengers wait on this sempahore before boarding
sem_t unboardQueue; // passengers wait on this semaphore before unboarding
sem_t allAboard; // indicates that the car is full
sem_t allAshore; // indicates that the car is empty
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // this mutex protects boarders, which count the number of passengers that have invoked boardCar
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER; // this mutex protects unboarders, which count the number of passengers that have invoked unboardCar

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
    sleep(5);
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

    while(true){
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

    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv) {
    ThreadInfo *threads;
    int i;
    int *p;
    int NUM_PASSENGERS;
    char error[250];

    // Chek number of parameters passed
    if (argc != 2) {
        sprintf(error, "Number of parameters expected = 1, number of parameters passed = %d\n", argc - 1);
        perror(error);
        exit(2);
    }

    NUM_PASSENGERS = atoi(argv[1]);

    // Check number of passengers passed
    if (NUM_PASSENGERS < CAR_CAPACITY){
        sprintf(error, "Number of passengers expected > %d, number of passengers passed = %d\n", CAR_CAPACITY, NUM_PASSENGERS);
        perror(error);
        exit(3);
    }

    threads = (ThreadInfo *) malloc((NUM_PASSENGERS + 1) * sizeof(ThreadInfo));
    if(threads == NULL){
        perror("Problems with array threads allocation!\n");
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

    // Create Car thread
    threads[0].id = 0;
    strcpy(threads[0].tag, CAR);
    threads[0].start_routine = exeCar;
    createThread(&threads[0], error);

    // Create Passengers threads
    for (i = 0; i < NUM_PASSENGERS; i++) {
        int pos = i + 1; // assignment location to insert thread information
        threads[pos].id = i;
        strcpy(threads[pos].tag, PASSENGER);
        threads[pos].start_routine = exePassenger;
        createThread(&threads[pos], error);
    }

    // Wait threads termination
    for (i = 0; i < NUM_PASSENGERS + 1; i++){
        if(strcmp(threads[i].tag, CAR) == 0)
            printf("Since the car is an infinite loop we can NOT wait for it\n");
        else {
            int res;
            pthread_join(threads[i].thread, (void**) & p);
            res= *p;
            printf("Pthread %d-esimo returns %d\n", i, res);
        }
    }

    exit(0);
}
