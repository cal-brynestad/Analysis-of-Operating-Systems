#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include "array.h"

#define MAX_INPUT_FILES 100
#define MAX_REQUESTER_THREADS 10
#define MAX_RESOLVER_THREADS 10
#define MAX_IP_LENGTH INET6_ADDRSTRLEN


typedef struct {
    char *filename;
    FILE *filepointer;
} singleFile;

typedef struct{
    int num_files_processed;
    int file_num_total;
    singleFile files[MAX_INPUT_FILES];
} inputFiles;

typedef struct{
    char *fileName;
    FILE *servicedpointer;
    pthread_mutex_t write_service_lock;
} writeToFile;

typedef struct{
    inputFiles *input_files;
    array *stack_buffer;
    writeToFile file_serviced;
} requesterPoolArgs;

typedef struct{
    array *stack_buffer;
    writeToFile file_results;
} resolverPoolArgs;

void *requesters(void *args_requester);

void *resolvers(void *args_resolver);