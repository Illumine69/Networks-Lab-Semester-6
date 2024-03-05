#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "msocket.h"


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