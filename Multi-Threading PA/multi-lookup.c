#include "multi-lookup.h"
#include "array.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>

void *requesters(void *req_args){
    requesterPoolArgs *args = (requesterPoolArgs*) req_args;

    inputFiles *listFiles = args->input_files;
    writeToFile serviced = args->file_serviced;
    array *stackBuffer = args->stack_buffer;

    char name[MAX_NAME_LENGTH];
    char *hostname = name;

    int num_files_resolved = 0;
    long thread_id;

    size_t len = 0;
    while(1){
        pthread_mutex_lock(&(stackBuffer->m));

        if (listFiles->num_files_processed == listFiles->file_num_total){ 
            stackBuffer->req_done++;

            // if req_done = num_req then sem post 10 times to avoid deadlock
            if (stackBuffer->req_done == stackBuffer->num_req) {
                int i;
                //printf("All Requesters Finished\n");
                for (i=0; i<MAX_RESOLVER_THREADS; i++){
                    sem_post(&(stackBuffer->items_avail));
                }
            }

            thread_id = syscall(SYS_gettid);
            printf("thread %ld serviced %d files\n", thread_id, num_files_resolved);

            pthread_mutex_unlock(&(stackBuffer->m));
            break;
        }
        
        singleFile *processFile = &listFiles->files[listFiles->num_files_processed];
        listFiles->num_files_processed++;

        pthread_mutex_unlock(&(stackBuffer->m));

        while(getline(&hostname, &len, processFile->filepointer) != -1){

            array_push(stackBuffer, hostname);
            
            pthread_mutex_lock(&(serviced.write_service_lock));

            fprintf(serviced.servicedpointer, "%s", hostname);


            pthread_mutex_unlock(&(serviced.write_service_lock));
        }
        fclose(processFile->filepointer);
        num_files_resolved++;
    }
    free(hostname);

    return 0;
}

void *resolvers(void *res_args) {
    resolverPoolArgs *args = (resolverPoolArgs*) res_args;

    array *stackBuffer = args->stack_buffer;
    writeToFile results = args->file_results;
    
    char name[MAX_NAME_LENGTH];
    char *hostname = name;

    char IPaddress[MAX_IP_LENGTH];

    int num_hostnames_resolved = 0;
    long thread_id;

    int status;

    while(1){
        pthread_mutex_lock(&(stackBuffer->m));
        if(stackBuffer->top == -1 && (stackBuffer->req_done == stackBuffer->num_req)){
            thread_id = syscall(SYS_gettid);
            printf("thread %ld resolved %d hostnames\n", thread_id, num_hostnames_resolved);
            pthread_mutex_unlock(&(stackBuffer->m));
            break;
        }
        pthread_mutex_unlock(&(stackBuffer->m));

        status = array_pop(stackBuffer, &hostname);
        
        if (status == -1){
            thread_id = syscall(SYS_gettid);
            printf("thread %ld resolved %d hostnames\n", thread_id, num_hostnames_resolved);
            //printf("%d\n", status);
            break;
        }

        num_hostnames_resolved++;
        hostname[strlen(hostname) - 1] = '\0';
        bzero(IPaddress,MAX_IP_LENGTH);

        pthread_mutex_lock(&(results.write_service_lock));

        if (dnslookup(hostname, IPaddress, MAX_IP_LENGTH) == UTIL_SUCCESS){

            fprintf(results.servicedpointer, "%s, %s\n", hostname, IPaddress);
            
            pthread_mutex_unlock(&(results.write_service_lock));
        }

        else{

            fprintf(results.servicedpointer, "%s, NOT_RESOLVED\n", hostname);

            pthread_mutex_unlock(&(results.write_service_lock));
        }
    }
    //printf("%d\n", status);
    return 0;
}

int main(int argc, char *argv[]){
    struct timeval begin, end;
	gettimeofday(&begin, NULL);

    int requester_num = atoi(argv[1]);
    int resolver_num = atoi(argv[2]);
    char *service_file = argv[3];
    char *resolved_file = argv[4];
    int file_num = argc - 5;
    char **orig_files = argv + 5;

    if(file_num > MAX_INPUT_FILES){
        fprintf(stderr, "Too many input files\n");
        exit(0);
    }

    if(argc < 6){
        fprintf(stdout, "Missing Arguments. Usage: multi-lookup <# requester> <# resolver> <requester log> <resolver log> [ <data file> ... ]\n");
        exit(0);
    }

    if(requester_num > MAX_REQUESTER_THREADS){
        fprintf(stderr, "Too many requester threads\n");
        exit(0);
    }

    if(resolver_num > MAX_RESOLVER_THREADS){
        fprintf(stderr, "Too many resolver threads\n");
        exit(0);
    }

    array *my_stack = malloc(sizeof(array));
    array_init(my_stack);
    my_stack->num_req = requester_num;
    my_stack->req_done = 0;
 
    inputFiles *inputFilesList;
    inputFilesList = malloc(sizeof(*inputFilesList));
    inputFilesList->num_files_processed = 0;
    inputFilesList->file_num_total = 0;

    int i;
    int index=0;
    for (i = 0; i < file_num; i++){
        singleFile f;
        f.filename = orig_files[i];
        f.filepointer = fopen(f.filename, "r");
        if(f.filepointer == NULL){
            fprintf(stderr, "invalid file %s\n", f.filename);
        }
        else{
            inputFilesList->files[index] = f;
            index++;
            inputFilesList->file_num_total++;
        }
    }

    writeToFile file_to_service;
    file_to_service.fileName = service_file;
    file_to_service.servicedpointer = fopen(file_to_service.fileName, "w");
    pthread_mutex_init(&(file_to_service.write_service_lock), NULL);

    requesterPoolArgs args_requester;
    args_requester.input_files = inputFilesList;
    args_requester.stack_buffer = my_stack;
    args_requester.file_serviced = file_to_service;   

    writeToFile file_to_result;
    file_to_result.fileName = resolved_file;
    file_to_result.servicedpointer = fopen(file_to_result.fileName, "w");
    pthread_mutex_init(&(file_to_result.write_service_lock), NULL);

    resolverPoolArgs args_resolver;
    args_resolver.stack_buffer = my_stack;
    args_resolver.file_results = file_to_result;

    pthread_t *req_thread_id = malloc(sizeof(pthread_t) * requester_num);

    pthread_t *res_thread_id = malloc(sizeof(pthread_t) * resolver_num);

    for(i=0; i<requester_num; i++){
        pthread_create(&req_thread_id[i], NULL, requesters, (void *)&args_requester);
    }

    for(i=0; i<resolver_num; i++){
        pthread_create(&res_thread_id[i], NULL, resolvers, (void *)&args_resolver);
    } 

    for(i=0; i < requester_num; i++){
        pthread_join(req_thread_id[i], NULL);
    }

    for(i=0; i<resolver_num; i++){
        pthread_join(res_thread_id[i], NULL);
    }

    pthread_mutex_destroy(&(file_to_service.write_service_lock));
    pthread_mutex_destroy(&(file_to_result.write_service_lock));

    fclose(file_to_service.servicedpointer);
    fclose(file_to_result.servicedpointer);

    free(req_thread_id);
    free(res_thread_id);

    array_free(my_stack);

    free(inputFilesList);

    gettimeofday(&end, NULL);
    printf("./multilookup: total time is %f seconds\n", (double) (end.tv_usec - begin.tv_usec) / 1000000 + (double) (end.tv_sec - begin.tv_sec));

    exit(0); 
}
