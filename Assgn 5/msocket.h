#ifndef MSOCKET_H
#define MSOCKET_H

#include <sys/socket.h>
#include <unistd.h>
#include <sys/shm.h>
#include<sys/sem.h>
#define T 5
#define p 0.05
#define N 25
#define SOCK_MTP SOCK_DGRAM
#define KEY ftok("/tmp", 'a')
#define SEM1_KEY ftok("/sem", 'a')
#define SEM2_KEY ftok("/sem", 'b')
#define SOCK_INFO_KEY ftok("/sockinfo", 'a')
#define SEM_SM_KEY ftok("/sem_sm", 'a')
#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing
				   the V(s) operation */

#define SEND_BUFFER_SIZE 10
#define RECV_BUFFER_SIZE 5

#define  MAX_WINDOW_SIZE 10 //define this properly what should be the max window size ?? this should be less than than the buffer size and also the no of bits in the sequence number

// let say your sequence numbers vary from 01 to 15 

#define MAX_SEQ_NUM 15 // you need to use %15 +1 to get the next sequence number


//header syntax ::

// Type field (0/1)$Destip$port$Sequence Number$Ack Number$Window Size$Data

// 0 for data and 1 for ack



struct SOCKINFO{
    int sockinfo;
    // ip address
    struct sockaddr_in *addr;
    int error_no;
};

struct swnd{
    // chose to implemet circular buffer
    int send_window_size;           //current send window size
    //int last_ack;                   //last ack received
   // int unack_msg[5];               //unacknowledged messages
    time_t unack_time[10]; 
    int rem_buff_space;
    //int last_sent;                  //last sent message
  //time at which the message was sent
  int start_index;//start index in the buffer 
  int last_sent_index;//last sent messages index in the buffer
    int end_index;//end index in the buffer
    int start_index_ack_no;//ack no of the start index
    int last_sent_ack_no;//ack no of the last sent index
};

struct rwnd{
    int receive_window_size;        //current receive window size
    int last_inorder_msg;           //last in-order message received
    int recv_msg[5];                //received messages
};

struct shared_memory{
    int free;
    int pid;
    int sockfd;
    struct sockaddr_in *addr;
    char send_buffer[SEND_BUFFER_SIZE][1000];
    char recv_buffer[RECV_BUFFER_SIZE][1000];
    struct swnd swnd;
    struct rwnd rwnd;
};

int m_socket(int domain, int type, int protocol);
int m_bind(int m_sockfd, const struct sockaddr *src_addr, socklen_t src_addrlen, const struct sockaddr *dest_addr, socklen_t dest_addrlen);
ssize_t m_sendto(int m_sockfd, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen);
ssize_t m_recvfrom(int m_sockfd, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len);
int m_close(int m_sockfd);
int dropMessage(float probability);

#endif