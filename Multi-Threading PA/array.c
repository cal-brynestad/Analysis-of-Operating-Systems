#include "array.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int array_init(array *s) {    
    pthread_mutex_init(&(s->m), NULL);
    //pthread_mutex_init(&(s->for_req), NULL);
    sem_init(&(s->space_avail), 0, ARRAY_SIZE);
    sem_init(&(s->items_avail), 0, 0);

    int i;
    for(i=0; i < ARRAY_SIZE; i++){
        s->arr[i] = malloc(sizeof(char) * MAX_NAME_LENGTH);
    }

    s->top = -1;
    s->req_done = 0;
    s->num_req = 0;
    return 0;
}

int array_push(array *s, char *hostname) {
    sem_wait(&(s->space_avail));
    pthread_mutex_lock(&(s->m));

    s->top++;
    strncpy(s->arr[s->top], hostname, MAX_NAME_LENGTH);
    //printf("push, %s", s->arr[s->top]);

    pthread_mutex_unlock(&(s->m));
    sem_post(&(s->items_avail));

    return 0;
}

int array_pop(array *s, char **hostname) { 
    sem_wait(&(s->items_avail));

    pthread_mutex_lock(&(s->m));
    if((s->top == -1)){
        pthread_mutex_unlock(&(s->m));
        return -1;
    }
    pthread_mutex_unlock(&(s->m));

    pthread_mutex_lock(&(s->m));

    strncpy(*hostname, s->arr[s->top], MAX_NAME_LENGTH);
    //printf("pop, %s", *hostname);
    s->top--;
    
    pthread_mutex_unlock(&(s->m));
    sem_post(&(s->space_avail));

    return 0;
}

void array_free(array *s) {  
    pthread_mutex_destroy(&(s->m));
    sem_destroy(&(s->space_avail));
    sem_destroy(&(s->items_avail));

    int i;
    for(i=0; i<ARRAY_SIZE; i++){
        free(s->arr[i]);
    }

    free(s);

    return;
}
