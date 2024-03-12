#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "msocket.h"
#include <sys/sem.h>

#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing
				   the V(s) operation */


/*
creating struct sockinfo
*/

struct SOCKINFO{
    int sockinfo;
    // ip address
    struct sockaddr_in *addr;
    int errno;
};


void* R(void* params){}

void* S(void* params){}

void* G(void* params){}

int main(){
    struct sembuf pop, vop ;
     key_t sem1_key = SEM1_KEY;
    key_t sem2_key = SEM2_KEY;
    int sem1 = semget(sem1_key, 1, 0777);
    int sem2 = semget(sem2_key, 1, 0777);

    //initilaize both semaphores to 0
    semctl(sem1, 0, SETVAL, 0);
    semctl(sem2, 0, SETVAL, 0);
    pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;
    key_t sock_info_key = SOCK_INFO_KEY;
    int sock_info = shmget(sock_info_key, sizeof(struct SOCKINFO), 0777);
    struct SOCKINFO *sockinfo = (struct SOCKINFO *)shmat(sock_info, 0, 0);



    struct shared_memory *SM;
    int shmid;
    key_t key = KEY;
    shmid = shmget(key, N*sizeof(struct shared_memory), 0777);

     pthread_t rid , sid, gid;
    pthread_attr_t r_attr, s_attr,g_attr;
    pthread_attr_init(&r_attr);
    pthread_attr_init(&s_attr);
    pthread_attr_init(&g_attr);
    pthread_create(&rid, NULL, R, NULL);
    pthread_detach(rid);
    pthread_create(&sid, NULL, S, NULL);
    pthread_detach(sid);
    pthread_create(&gid, NULL, G, NULL);
    pthread_detach(gid);

   
    // need to attach?



    // while(1){   // Garbage collector process

    // }
    while(1)
    {
        P(sem1);
        // do stuff here 

        if(!(sockinfo->sockinfo))
        {
            //call socket here 
            

        }
        else {
            // do stuff here
            //call bind here

        }

        V(sem2);// release the calling process

    }

    // when do you detach the shared memories and semaphores?
    //when user presees control c right ? use signal handler??
}