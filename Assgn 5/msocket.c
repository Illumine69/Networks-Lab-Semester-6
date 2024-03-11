#include "msocket.h"
#include <sys/socket.h>
#include <sys/shm.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
//#include<netinet/in.h>
//#include<arpa/inet.h>
#include<netdb.h>
/*
1) Avoid using strcpy just blindly copy since messages are of fixed (1000) bytes 
2)(wrong and ignore) Dereferencing void * into char --> *((char *)(ptr)) 
2) Mutexes not included to be taken care later 
3) didnt set one of the errors in m_bind

*/

int m_socket(int domain, int type, int protocol){
    int sockfd;
    if(type != SOCK_MTP){
        errno = EINVAL;
        return -1;
    }
    key_t key = KEY;
    int shmid = shmget(key, N*sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, NULL, 0);
    if(SM==NULL){
        errno = ENOMEM;
        printf("Init process not called \n");
        close(sockfd);
        return -1;
    }
    int free_available = 0;
    int m_sockfd;
    for(int i=0; i<N; i++){
        if(SM[i].free = 1){
            if((sockfd = socket(domain, type, protocol)) == -1){
                shmdt(SM);
                return -1;
            }
            free_available = 1;
            SM[i].free = 0;
            SM[i].sockfd = sockfd;
            SM[i].pid = getpid();
            m_sockfd = i;
            break;
        }
    }
    if(free_available == 0){
        errno = ENOBUFS;
        return -1;
    }
    return m_sockfd;
}

int m_bind(int m_sockfd, const struct sockaddr *src_addr, socklen_t src_addrlen, const struct sockaddr *dest_addr, socklen_t dest_addrlen){

    //initialze the buff?
    key_t key = KEY;
    int shmid = shmget(key, N*sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, NULL, 0);
   
    //lock the shared memory ? not required as only this process is aceesing when m_bind is in progress


    //fill the entry in shared memory
    //before filling do error checking;
    if(m_sockfd<0 || m_sockfd>=N){
        errno = EBADF;
        return -1;
    }
    if(SM[m_sockfd].free == 1){
        errno = EBADF;
        return -1;
    }
    if(SM[m_sockfd].pid!=getpid())
    {
        errno =EBADF;
        return -1;
    }
    SM[m_sockfd].addr = (struct sockaddr_in *)dest_addr;
    // initialze   the send window
    SM[m_sockfd].swnd.send_window_size=1;

    SM[m_sockfd].swnd.last_ack=0;// COZ numbering starrts from 1

    memset(SM[m_sockfd].swnd.unack_msg,-1,sizeof(SM[m_sockfd].swnd.unack_msg));

    memset(SM[m_sockfd].swnd.unack_time,-1,sizeof(SM[m_sockfd].swnd.unack_time));

    // initialze   the receive window
    SM[m_sockfd].rwnd.receive_window_size=MAX_WINDOW_SIZE;

    SM[m_sockfd].rwnd.last_inorder_msg=0;// COZ numbering starts from 1

    memset(SM[m_sockfd].rwnd.recv_msg,-1,sizeof(SM[m_sockfd].rwnd.recv_msg));
    // call the system bind call
    
    int res =bind(SM[m_sockfd].sockfd, src_addr, src_addrlen);

    //unlock the shared memory


    //deatch from shared memory
    shmdt(SM);



     return res;
}


// set the correct errno
ssize_t m_sendto(int m_sockfd, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen){
    //first attach to shared memory

    //need to use mutexes
    key_t key = KEY;

    int shmid=shmget(key,N*sizeof(struct shared_memory),0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid,NULL,0);
    //error checking
    if(SM==NULL){
        errno = ENOMEM;
        printf("Init process not called \n");
        return -1;
    }
    if(m_sockfd<0 || m_sockfd>=N){
        errno = EBADF;
        return -1;
    }
    if(SM[m_sockfd].free == 1){
        errno = EBADF;
        return -1;
    }
    if(SM[m_sockfd].pid!=getpid())
    {
        errno =EBADF;
        return -1;
    }
    //check if port and ip are same
    if( (SM[m_sockfd].addr->sin_port != ((struct sockaddr_in *)dest_addr)->sin_port) || (SM[m_sockfd].addr->sin_addr.s_addr != ((struct sockaddr_in *)dest_addr)->sin_addr.s_addr) ){
        //errno = ENOTBOUND;
        //set the correct errno;
        printf("Port and IP are not same\n");
        return -1;
    }
    // bruh send error if buf is not available not if window size is 0
    // if(SM[m_sockfd].swnd.send_window_size == 0){
    //     errno = ENOBUFS;
    //     return -1;
    // }
    if(!SM[m_sockfd].swnd.rem_buff_space)
    {
        errno = ENOBUFS;
        return -1;
    }
    //you would need to initiliaze the buffer with charecters that are not used in usual messages
    for(int i=0;i<SEND_BUFFER_SIZE;i++)
    {
        if(SM[m_sockfd].send_buffer[i][0]=='\r' && SM[m_sockfd].send_buffer[i][1]=='\n' )
        {
            // you are assuming each message is of length 1000
            // blindly copy
            for(int j=0;j<1000;j++)
            {
                SM[m_sockfd].send_buffer[i][j]= *((char *)message+j);
            }
            //Karthik left work here 


            // you shouldnt decreasing the window size here
            // you should decrease the window size only after the message is sent




            SM[m_sockfd].swnd.unack_msg[SM[m_sockfd].swnd.last_sent]=i;
            SM[m_sockfd].swnd.unack_time[SM[m_sockfd].swnd.last_sent]=time(NULL);
            SM[m_sockfd].swnd.last_sent=(SM[m_sockfd].swnd.last_sent+1)%5;
            //SM[m_sockfd].swnd.send_window_size--;
            SM[m_sockfd].swnd.rem_buff_space--;
            break;
        }
    }


    // who adds the header is it S or this function ?







}

ssize_t m_recvfrom(int m_sockfd, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len){

    //first attach to shared memory
    key_t key = KEY;
    int shmid=shmget(key,N*sizeof(struct shared_memory),0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid,NULL,0);

    //error checking
    if(SM==NULL){
        errno = ENOMEM;
        printf("Init process not called \n");
        return -1;
    }

    // If the socket is free
    if(SM[m_sockfd].free == 1){
        errno = EBADF;
        return -1;
    }

    for(int i=0;i<RECV_BUFFER_SIZE;i++){
        // Assuming that the R process stores only the actual message in the Receive buffer ending with <crlf>
        if(SM[m_sockfd].recv_buffer[i][0]!='\r' && SM[m_sockfd].recv_buffer[i][1]!='\n'){
            int j = 0;
            while(SM[m_sockfd].recv_buffer[i][j]!='\r' && SM[m_sockfd].recv_buffer[i][j+1]!='\n'){
                *((char *)buffer+j) = SM[m_sockfd].recv_buffer[i][j];
                j++;
            }
            memset(SM[m_sockfd].recv_buffer[i],'\0',1000);
            SM[m_sockfd].recv_buffer[i][0]='\r';
            SM[m_sockfd].recv_buffer[i][1]='\n';
            shmdt(SM);
            return j;
        }
    }
    shmdt(SM);
    errno = ENOMSG;
    return -1;

}

int m_close(int m_sockfd){
    int res;
    key_t key = KEY;
    int shmid = shmget(key, N*sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, NULL, 0);

    if(SM[m_sockfd].free == 0){
        int sockfd = SM[m_sockfd].sockfd;
        if((res = close(sockfd)) != 0){
            shmdt(SM);
            return -1;
        }
        memset(&SM[m_sockfd], NULL, sizeof(struct shared_memory));
        SM[m_sockfd].free = 1;
    }
    else{
        errno = EBADF;
        res = -1;
    }
    shmdt(SM);
    return res;
}

int dropMessage(float probability){
    srand((unsigned int)time(NULL));
    float r = (float)rand()/(float)(RAND_MAX);
    return (r < probability ? 1 : 0);
}