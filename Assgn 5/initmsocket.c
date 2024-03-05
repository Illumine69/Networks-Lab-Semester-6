#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include "msocket.h"

#define SEND_BUFFER_SIZE 10240
#define RECV_BUFFER_SIZE 5120

struct swnd{
    int window_size;
    int unack_msg[5];
};

struct rwnd{
    int window_size;
    int expected_msg[10];
};

struct shared_memory{
    int free;
    int pid;
    int sockfd;
    struct sockaddr_in *addr;
    char send_buffer[SEND_BUFFER_SIZE];
    char recv_buffer[RECV_BUFFER_SIZE];
    struct swnd swnd;
    struct rwnd rwnd;
};


void* R(void* params){}

void* S(void* params){}

int main(){
    pthread_t rid, sid;
    struct shared_memory *SM;
    int shmid;
    key_t key = KEY;
    shmid = shmget(key, N*sizeof(struct shared_memory), 0777);


    pthread_create(&rid, NULL, R, NULL);
    pthread_create(&sid, NULL, S, NULL);

    pthread_join(rid, NULL);
    pthread_join(sid, NULL);
}