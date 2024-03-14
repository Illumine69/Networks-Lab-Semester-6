#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "msocket.h"
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>
#include <netinet/in.h>

// create a mutex (semaphore )for the whole share memory


key_t sem_sm_key;

struct shared_memory *SM;

char send_buffer[1500];

void *R(void *params)
{
    // receiver process
}

void *S(void *params)
{
    // sender process

    // should you implement a per SM semaphore or per index semaphore for SM?// choose the easy way

    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;

    while (1)
    {
        // sleep for <T/2 seconds
        sleep(T / 2 - 1);
        P(sem_sm_key);
        for (int i = 0; i < N; i++)
        {
            // checking if timed out
            if (SM[i].free == 0)
            {
                // check if the message has been acked
                // if not resend the message
                // if yes check if the time has expired
                // if yes resend the message
                // if no do nothing
                // iterate from start_index to last_sent_index


                // j is basically circular 
                // for (int j = SM[i].swnd.start_index; (j+SEND_BUFFER_SIZE)%(SEND_BUFFER_SIZE) <= SM[i].swnd.last_sent_index; j++)
                int j = SM[i].swnd.start_index;
                while(j != (SM[i].swnd.last_sent_index+1)%SEND_BUFFER_SIZE)
                {
                    // if (SM[i].swnd.unack_time[j] + T < time(NULL))
                    // {
                    //     // resend the message
                    //     // send the message
                    //     // if the message is sent successfully update the unack_time
                    //     if (sendto(SM[i].sockfd, SM[i].send_buffer[j], sizeof(SM[i].send_buffer[j]), 0, (struct sockaddr *)SM[i].addr, sizeof(struct sockaddr_in)) < 0)
                    //     //you need to add the ack number in the message
                    //     {
                    //         // SM[i].swnd.unack_time[j] = -1;

                    //         // do some error handling here
                    //         perror("Error in thread while attempting to send to the socket");
                    //         continue;
                    //     }
                    //     SM[i].swnd.unack_time[j] = time(NULL);
                    // }
                    //send all the messages from start_index to last_sent_index
                    //dont check the time

                      sprintf(send_buffer, "0$%d$%d$%d$$$", SM[i].addr->sin_addr.s_addr, SM[i].addr->sin_port,(SM[i].swnd.start_index_ack_no +(j-SM[i].swnd.start_index+SEND_BUFFER_SIZE)%(SEND_BUFFER_SIZE))% MAX_SEQ_NUM + 1);
                    

                        int len = strlen(send_buffer);
                        for(int k = 0; k < 1000; k++)
                        {
                             send_buffer[len + k] = SM[i].send_buffer[(j-SM[i].swnd.start_index+SEND_BUFFER_SIZE)%SEND_BUFFER_SIZE][k];
                        }
                        
                        if (sendto(SM[i].sockfd, send_buffer, len+1000, 0, (struct sockaddr *)SM[i].addr, sizeof(struct sockaddr_in)) < 0)
                        {
                            // do some error handling here
                            // restore the index
                            SM[i].swnd.last_sent_index = (SM[i].swnd.last_sent_index - 1 + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE;
                            
                            //include the ack number in the message
                            perror("Error in thread while attempting to send to the socket");
                            break;
                        }
                    j = (j+1)%SEND_BUFFER_SIZE;
                }
            }
        }
        // next check if there are any new messages to send

        // you need to check

        // need to add some headers  likie source adress and source port destionaion address and destination port not required??
        // also need to add the ack the number of the message
        // type of message ack or data

        // should you ntohs ??

        for (int i = 0; i < N; i++)
        {
            if (!(SM[i].free))
            {
                for (int j = 1; j <= SM[i].swnd.send_window_size; j++)
                {
                    if (((SM[i].swnd.last_sent_index + 1 - SM[i].swnd.start_index + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE) <= ((SM[i].swnd.end_index - SM[i].swnd.start_index + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE))
                    {
                        SM[i].swnd.last_sent_index = (SM[i].swnd.last_sent_index + 1) % SEND_BUFFER_SIZE;
                        sprintf(send_buffer, "0$%d$%d$%d$$$", SM[i].addr->sin_addr.s_addr, SM[i].addr->sin_port,(SM[i].swnd.last_sent_ack_no  ) % MAX_SEQ_NUM + 1);
                        int len = strlen(send_buffer);
                        for(int k = 0; k < 1000; k++)
                        {
                             send_buffer[len + k] = SM[i].send_buffer[SM[i].swnd.last_sent_index][k];
                        }
                        
                        if (sendto(SM[i].sockfd, send_buffer, len+1000, 0, (struct sockaddr *)SM[i].addr, sizeof(struct sockaddr_in)) < 0)
                        {
                            // do some error handling here
                            // restore the index
                            SM[i].swnd.last_sent_index = (SM[i].swnd.last_sent_index - 1 + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE;
                            
                            //include the ack number in the message
                            perror("Error in thread while attempting to send to the socket");
                            break;
                        }
                        SM[i].swnd.last_sent_ack_no = (SM[i].swnd.last_sent_ack_no  ) % MAX_SEQ_NUM + 1;    
                        SM[i].swnd.unack_time[SM[i].swnd.last_sent_index] = time(NULL);
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        V(sem_sm_key);
    }
}

void *G(void *params)
{

    // garbage collector process
}

int main()
{

    int sm_sem = semget(sem_sm_key, 1, 0777);
    // initialize the semaphore to 1
    semctl(sm_sem, 0, SETVAL, 1); // initially unlocked workd like mutex
    struct sembuf pop, vop;
    key_t sem1_key = SEM1_KEY;
    key_t sem2_key = SEM2_KEY;
    int sem1 = semget(sem1_key, 1, 0777);
    int sem2 = semget(sem2_key, 1, 0777);

    // initilaize both semaphores to 0
    semctl(sem1, 0, SETVAL, 0);
    semctl(sem2, 0, SETVAL, 0);
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;
    key_t sock_info_key = SOCK_INFO_KEY;
    int sock_info = shmget(sock_info_key, sizeof(struct SOCKINFO), 0777);
    struct SOCKINFO *sockinfo = (struct SOCKINFO *)shmat(sock_info, 0, 0);

    int shmid;
    key_t key = KEY;
    shmid = shmget(key, N * sizeof(struct shared_memory), 0777);

    pthread_t rid, sid, gid;
    pthread_attr_t r_attr, s_attr, g_attr;
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
    while (1)
    {
        P(sem1);
        // do stuff here

        if (!(sockinfo->sockinfo))
        {
            // call socket here

            sockinfo->sockinfo = socket(AF_INET, SOCK_STREAM, 0);
            if (sockinfo->sockinfo < 0)
            {
                sockinfo->error_no = errno;
                sockinfo->sockinfo = 0;
            }
        }
        else
        {
            // do stuff here
            // call bind here
            if (bind(sockinfo->sockinfo, (struct sockaddr *)sockinfo->addr, sizeof(struct sockaddr_in)) < 0)
            {
                sockinfo->error_no = errno;
                sockinfo->sockinfo = -1;
            }
        }

        V(sem2); // release the calling process
    }

    // when do you detach the shared memories and semaphores?
    // when user presees control c right ? use signal handler??
}
