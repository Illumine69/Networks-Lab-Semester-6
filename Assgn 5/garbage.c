#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include "msocket.h"



void *G(void *params) {

    // garbage collector process
     int shmid;
     
    key_t key = KEY;
    shmid = shmget(key, N * sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, 0, 0);
    
    while(1)
    {
        sleep(T/2);
        int status;
        for(int i=0;i<N;i++)
        {
            if(!(SM[i].free))
            {
               int pid=SM[i].pid;
                status=kill(pid,0);
                if(status==-1)
                {
                    //process is dead
                    //free the shared memory
                    SM[i].free=1;
                    SM[i].pid=-1;
                    SM[i].swnd.send_window_size=0;
                    SM[i].swnd.start_index=0;
                    SM[i].swnd.last_sent_index=0;
                    SM[i].swnd.end_index=0;
                    SM[i].swnd.start_index_ack_no=0;
                    SM[i].swnd.last_sent_ack_no=0;
                    SM[i].swnd.rem_buff_space=SEND_BUFFER_SIZE;
                    for(int j=0;j<10;j++)
                    {
                        SM[i].swnd.unack_time[j]=0;
                    }
                }

            }

            
        }
    }


}