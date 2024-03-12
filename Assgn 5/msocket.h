#ifndef MSOCKET_H
#define MSOCKET_H

#include <sys/socket.h>
#include <unistd.h>
#define T 5
#define p 0.05
#define N 25
#define SOCK_MTP SOCK_DGRAM
#define KEY ftok("/tmp", 'a')
#define SEM1_KEY ftok("/sem", 'a')
#define SEM2_KEY ftok("/sem", 'b')
#define SOCK_INFO_KEY ftok("/sockinfo", 'a')

#define SEND_BUFFER_SIZE 10
#define RECV_BUFFER_SIZE 5

#define  MAX_WINDOW_SIZE 10

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