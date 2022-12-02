#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "my-thread.h"

#define BAR_CAPACITY 5
#define CUSTOMER "Customer"

#define MAX_TIME 5 // maximum time of thread sleep
#define MIN_TIME 2 // minimum time of thread sleep

typedef enum {false, true} Boolean;

/* global variables */
int eating = 0; // count the number of customers eating
int waiting = 0; // count the number of customers waiting
sem_t block; // blocks customer if the bar is full
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // protects the counters eating and waiting
Boolean mustWait = false; // indicates that the bar is (or has been) full

void *exeCustomer(void *id) {
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problems with ptr allocation\n");
        exit(-1);
    }

    pthread_mutex_lock(&mutex);
    if(mustWait){
        waiting++;
        pthread_mutex_unlock(&mutex);
        sem_wait(&block);

        pthread_mutex_lock(&mutex); // reacquire mutex
        waiting--;
    }

    eating++;
    mustWait = (eating == BAR_CAPACITY);
    pthread_mutex_unlock(&mutex);


    printf("The customer %d is eating\n", *pi);
    sleep(rand() % (MAX_TIME + 1 - MIN_TIME) + MIN_TIME);

    pthread_mutex_lock(&mutex);
    eating--;
    printf("The customer %d finished eating\n", *pi);
    if(eating == 0) {
        printf("The party finished eating!\n");
        int n = waiting < BAR_CAPACITY ? waiting : BAR_CAPACITY;
        for(int j = 0; j < n; j++)
            sem_post(&block);
        mustWait = false;
    }
    pthread_mutex_unlock(&mutex);

    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv) {
    ThreadInfo *threads;
    int i;
    int *p;
    int NUM_CUSTOMERS;
    char error[250];

    // Chek number of parameters passed
    if (argc != 2) {
        sprintf(error, "Number of parameters expected = 1, number of parameters passed = %d\n", argc - 1);
        perror(error);
        exit(2);
    }

    NUM_CUSTOMERS = atoi(argv[1]);

    // Check number of male passed
    if (NUM_CUSTOMERS <= 0){
        sprintf(error, "Number of customers expected > 0, number of customers passed = %d\n", NUM_CUSTOMERS);
        perror(error);
        exit(3);
    }

    threads = (ThreadInfo *) malloc((NUM_CUSTOMERS) * sizeof(ThreadInfo));
    if(threads == NULL){
        perror("Problems with array threads allocation!\n");
        exit(4);
    }

    // Initialize block Sempahore to 0
    if (sem_init(&block, 0, 0) != 0) {
        perror("Problems with initialization of block sempahore\n");
        exit(5);
    }

    // Create customers threads
    for (i = 0; i < NUM_CUSTOMERS; i++) {
        threads[i].id = i;
        strcpy(threads[i].tag, CUSTOMER);
        threads[i].start_routine = exeCustomer;
        createThread(&threads[i], error);
    }

    // Wait threads termination
    for (i = 0; i < NUM_CUSTOMERS; i++){
        int ris;
        pthread_join(threads[i].thread, (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo returns %d\n", i, ris);
    }

    exit(0);
}
