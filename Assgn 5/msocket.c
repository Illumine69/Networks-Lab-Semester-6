#include "msocket.h"
#include <sys/socket.h>
#include <sys/shm.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

int m_socket(int domain, int type, int protocol){}

int m_bind(int sockfd, const struct sockaddr *src_addr, socklen_t src_addrlen, const struct sockaddr *dest_addr, socklen_t dest_addrlen){}

ssize_t m_sendto(int sockfd, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen){}

ssize_t m_recvfrom(int sockfd, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len){}

int m_close(int fd){
    int res;
    if((res = close(fd)) == 0){     // Successfully closed the socket

        // Clean the entry in shared memory
        key_t key = KEY;
        int shmid = shmget(key, N*sizeof(struct shared_memory), 0777);
        struct shared_memory *SM = (struct shared_memory *)shmat(shmid, NULL, 0);

        for(int i = 0;i < N;i++){
            if(SM[i].sockfd == fd){
                memset(&SM[i], NULL, sizeof(struct shared_memory));
                SM[i].free = 1;
            }
        }
    }
    return res;
}

int dropMessage(float probability){
    srand((unsigned int)time(NULL));
    float r = (float)rand()/(float)(RAND_MAX);
    return (r < probability ? 1 : 0);
}