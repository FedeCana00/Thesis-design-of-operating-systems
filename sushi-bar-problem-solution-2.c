#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define BAR_CAPACITY 5
#define MAX_TIME 5
#define MIN_TIME 2

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
        waiting--;
    }

    eating++;
    mustWait = (eating == 5);
    if(waiting && !mustWait)
        sem_post(&block);
    else
        pthread_mutex_unlock(&mutex);

    printf("The customer %d is eating\n", *pi);
    sleep(rand() % (MAX_TIME + 1 - MIN_TIME) + MIN_TIME);

    pthread_mutex_lock(&mutex);
    eating--;
    printf("The customer %d finished eating\n", *pi);
    if(eating == 0) {
        printf("The party finished eating!\n");
        mustWait = false;
    }

    if(waiting && !mustWait)
        sem_post(&block);
    else
        pthread_mutex_unlock(&mutex);

    // terminates the current thread and returns the integer value of the index
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv) {
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    int NUM_CUSTOMERS;
    char error[250];

    // Chek number of parameters passed
    if (argc != 2) {
        sprintf(error, "Number of parameters expected = 1, number of parameters passed = %d\n", argc - 1);
        perror(error);
        exit(1);
    }

    NUM_CUSTOMERS = atoi(argv[1]);

    // Check number of male passed
    if (NUM_CUSTOMERS <= 0){
        sprintf(error, "Number of customers expected > 0, number of customers passed = %d\n", NUM_CUSTOMERS);
        perror(error);
        exit(2);
    }

    thread = (pthread_t *) malloc((NUM_CUSTOMERS) * sizeof(pthread_t));
    if (thread == NULL) {
        perror("Problems with array thread allocation!\n");
        exit(3);
    }

    taskids = (int *) malloc((NUM_CUSTOMERS) * sizeof(int));
    if (taskids == NULL) {
        perror("Problems with array taskids allocation!\n");
        exit(4);
    }

    // Initialize block Sempahore to 0
    if (sem_init(&block, 0, 0) != 0) {
        perror("Problems with initialization of block sempahore\n");
        exit(5);
    }

    // Create customers threads
    for (i = 0; i < NUM_CUSTOMERS; i++) {
        taskids[i] = i;
        printf("I'm about to create the CUSTOMERS %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, exeCustomer, (void *) (&taskids[i])) != 0){
                sprintf(error,"I'm MAIN THREAD and something went wrong with creation of CUSTOMERS THREAD %d-esimo\n", taskids[i]);
                perror(error);
                exit(10);
        }
        printf("I'm MAIN THREAD and I've created CUSTOMER THREAD with id = %lu\n", thread[i]);
    }
    // Wait threads termination
    for (i = 0; i < NUM_CUSTOMERS; i++){
        int ris;
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo returns %d\n", i, ris);
    }

    exit(0);
}
