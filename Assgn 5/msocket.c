#include "msocket.h"
#include <sys/socket.h>
#include <sys/shm.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int m_socket(int domain, int type, int protocol){
    int sockfd;
    if(type != SOCK_MTP){
        errno = EINVAL;
        return -1;
    }
    if((sockfd = socket(domain, type, protocol)) != -1){     // Successfully created the socket

        key_t key = KEY;
        int shmid = shmget(key, N*sizeof(struct shared_memory), 0777);
        struct shared_memory *SM = (struct shared_memory *)shmat(shmid, NULL, 0);
        int free_available = 0;
        int m_sockfd;
        for(int i=0; i<N; i++){
            if(SM[i].free = 1){
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
            close(sockfd);
            return -1;
        }
        return m_sockfd;
    }
    else{
        return -1;
    }
}

int m_bind(int m_sockfd, const struct sockaddr *src_addr, socklen_t src_addrlen, const struct sockaddr *dest_addr, socklen_t dest_addrlen){


    //initialze the buff?
    key_t key = KEY;
    int shmid = shmget(key, N*sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, NULL, 0);
   

    //lock the shared memory ? not required as only this process is aceesing when m_bind is in progress


    //fill the entry in shared memory
    SM[m_sockfd].addr = (struct sockaddr_in *)dest_addr;
    // initialze   the send window
    SM[m_sockfd].swnd.send_window_size=1;
    SM[m_sockfd].swnd.last_ack=0;// COZ numbering starrts from 1
    memset(SM[m_sockfd].swnd.unack_msg,-1,sizeof(SM[m_sockfd].swnd.unack_msg));
    memset(SM[m_sockfd].swnd.unack_time,-1,sizeof(SM[m_sockfd].swnd.unack_time));
    // initialze   the receive window
    SM[m_sockfd].rwnd.receive_window_size=MAX_WINDOW_SIZE;
    SM[m_sockfd].rwnd.last_inorder_msg=0;// COZ numbering starrts from 1
    memset(SM[m_sockfd].rwnd.recv_msg,-1,sizeof(SM[m_sockfd].rwnd.recv_msg));
    // call the system bind call
    
    int res =bind(SM[m_sockfd].sockfd, src_addr, src_addrlen);



    //unlock the shared memory
   

    //deatch from shared memory
    shmdt(SM);



     return res;

    




}

ssize_t m_sendto(int m_sockfd, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen){}

ssize_t m_recvfrom(int m_sockfd, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len){}

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