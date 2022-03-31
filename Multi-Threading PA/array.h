#ifndef ARRAY_H
#define ARRAY_H

#include <semaphore.h>
#include <pthread.h>

#define ARRAY_SIZE 8                      // max elements in stack
#define MAX_NAME_LENGTH 32                     

typedef struct{
    char *arr[ARRAY_SIZE];                  // storage array for integers
    int top;
    int req_done;
    int num_req;

    pthread_mutex_t m;
    sem_t space_avail;
    sem_t items_avail;                                // array index indicating where the top is
} array;

int  array_init(array *s);                  // init the stack
int  array_push(array *s, char *hostname);     // place element on the top of the stack
int  array_pop(array *s, char **hostname);    // remove element from the top of the stack
void array_free(array *s);  // free the stack's resources               

#endif
