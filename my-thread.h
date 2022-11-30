#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct{
    pthread_t thread;
    int id;
    void *(*start_routine)(void *);
    char tag[25];
} ThreadInfo;

void createThread(ThreadInfo *threadInfo, char error[250]){
    printf("I'm about to create the %s %d-esimo\n", threadInfo->tag, threadInfo->id);
    if (pthread_create(&threadInfo->thread, NULL, threadInfo->start_routine, (void *) (&threadInfo->id)) != 0){
            sprintf(error,"I'm MAIN THREAD and something went wrong with creation of %s THREAD %d-esimo\n", threadInfo->tag, threadInfo->id);
            perror(error);
            exit(1);
    }
    printf("I'm MAIN THREAD and I've created %s THREAD with id = %lu\n", threadInfo->tag, threadInfo->thread);
}
