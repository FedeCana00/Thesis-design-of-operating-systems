#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "my-thread.h"

#define BATHROOM_CAPACITY 3
#define TYPE_MALE 1
#define MALE "Male"
#define TYPE_FEMALE 0
#define FEMALE "Female"

#define MAX_TIME 5 // maximum time of thread sleep
#define MIN_TIME 2 // minimum time of thread sleep

typedef enum {false, true} Boolean;

/* global variables */
int femaleInside = 0; // count the number of female inside the bathroom
int maleInside = 0; // count the number of male inside the bathroom
sem_t femaleSwitch; // allows women to bar men from the room
sem_t maleSwitch; // allows men to bar women from the room
sem_t empty; // equals to 1 if the room is empty, otherwise it's 0
sem_t maleMultiplex; // ensures that there are no more than three men in the room at same time
sem_t femaleMultiplex; // ensures that there are no more than three men in the room at same time

void *exeFemale(void *id) {
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problems with ptr allocation\n");
        exit(-1);
    }

    sem_wait(&femaleSwitch);
    femaleInside++;
    if(femaleInside == 1)
        sem_wait(&empty);
    sem_post(&femaleSwitch);

    sem_wait(&femaleMultiplex);
    printf("Female %d enters the bathroom\n", *pi);
    sleep(rand() % (MAX_TIME + 1 - MIN_TIME) + MIN_TIME);
    sem_post(&femaleMultiplex);

    sem_wait(&femaleSwitch);
    femaleInside--;
    if(femaleInside == 0)
        sem_post(&empty);
    sem_post(&femaleSwitch);

    printf("Female %d comes out of the bathroom\n", *pi);

    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void *exeMale(void *id) {
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problems with ptr allocation\n");
        exit(-1);
    }

    sem_wait(&maleSwitch);
    maleInside++;
    if(maleInside == 1)
        sem_wait(&empty);
    sem_post(&maleSwitch);

    sem_wait(&maleMultiplex);
    printf("Male %d enters the bathroom\n", *pi);
    sleep(rand() % (MAX_TIME + 1 - MIN_TIME) + MIN_TIME);
    sem_post(&maleMultiplex);

    sem_wait(&maleSwitch);
    maleInside--;
    if(maleInside == 0)
        sem_post(&empty);
    sem_post(&maleSwitch);

    printf("Male %d comes out of the bathroom\n", *pi);

    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv) {
    ThreadInfo *threads;
    int i;
    int *p;
    int NUM_EMPLOYEES;
    char error[250];

    // Chek number of parameters passed
    if (argc != 2) {
        sprintf(error, "Number of parameters expected = 1, number of parameters passed = %d\n", argc - 1);
        perror(error);
        exit(2);
    }

    NUM_EMPLOYEES = atoi(argv[1]);

    // Check number of male passed
    if (NUM_EMPLOYEES <= 0){
        sprintf(error, "Number of employees expected > 0, number of employees passed = %d\n", NUM_EMPLOYEES);
        perror(error);
        exit(3);
    }

    threads = (ThreadInfo *) malloc((NUM_EMPLOYEES) * sizeof(ThreadInfo));
    if(threads == NULL){
        perror("Problems with array threads allocation!\n");
        exit(4);
    }

    // Initialize male switch Sempahore to 1
    if (sem_init(&maleSwitch, 0, 1) != 0) {
        perror("Problems with initialization of male switch sempahore\n");
        exit(5);
    }

    // Initialize feamle switch Sempahore to 1
    if (sem_init(&femaleSwitch, 0, 1) != 0) {
        perror("Problems with initialization of female switch sempahore\n");
        exit(6);
    }

    // Initialize empty Sempahore to 1
    if (sem_init(&empty, 0, 1) != 0) {
        perror("Problems with initialization of empty sempahore\n");
        exit(7);
    }

    // Initialize male multiplex Sempahore to BATHROOM_CAPACITY
    if (sem_init(&maleMultiplex, 0, BATHROOM_CAPACITY) != 0) {
        perror("Problems with initialization of male multiplex sempahore\n");
        exit(8);
    }

    // Initialize female multiplex Sempahore to BATHROOM_CAPACITY
    if (sem_init(&femaleMultiplex, 0, BATHROOM_CAPACITY) != 0) {
        perror("Problems with initialization of female multiplex sempahore\n");
        exit(9);
    }

    // Create employees threads
    int randomNumber;
    for (i = 0; i < NUM_EMPLOYEES; i++) {
        threads[i].id = i;
        randomNumber = rand() % (1 + 1 - 0) + 0;

        if(randomNumber == TYPE_FEMALE){ // create female thread
            strcpy(threads[i].tag, FEMALE);
            threads[i].start_routine = exeFemale;
        } else if(randomNumber == TYPE_MALE){ // create male thread
            strcpy(threads[i].tag, MALE);
            threads[i].start_routine = exeMale;
        } else {
            perror("Something went wrong during random generation of threads\n");
            exit(10);
        }

        createThread(&threads[i], error);
    }

    // Wait threads termination
    for (i = 0; i < NUM_EMPLOYEES; i++){
        int ris;
        pthread_join(threads[i].thread, (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo returns %d\n", i, ris);
    }

    exit(0);
}
