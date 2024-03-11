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
    pthread_t rid , sid;
    pthread_attr_t r_attr, s_attr;
    pthread_attr_init(&r_attr);
    pthread_attr_init(&s_attr);
    struct shared_memory *SM;
    int shmid;
    key_t key = KEY;
    shmid = shmget(key, N*sizeof(struct shared_memory), 0777);


    pthread_create(&rid, NULL, R, NULL);
    pthread_detach(rid);
    pthread_create(&sid, NULL, S, NULL);
    pthread_detach(sid);

    while(1){   // Garbage collector process

    }
}