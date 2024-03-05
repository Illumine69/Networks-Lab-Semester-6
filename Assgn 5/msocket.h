#ifndef MSOCKET_H
#define MSOCKET_H

#include <sys/socket.h>
#include <unistd.h>
#define T 5
#define p 0.05
#define N 25
#define SOCK_MTP SOCK_DGRAM
#define KEY ftok("/tmp", 'a')

#define SEND_BUFFER_SIZE 10240
#define RECV_BUFFER_SIZE 5120

#define  MAX_WINDOW_SIZE 10

struct swnd{
    int send_window_size;//current send window size
    int last_ack;//last ack received
    int unack_msg[5];//unacknowledged messages
    time_t unack_time[5];//time at which the message was sent
};

struct rwnd{
    int receive_window_size;//current receive window size
    int last_inorder_msg;//last in-order message received
    int recv_msg[5];//received messages
};

struct shared_memory{
    int free;
    int pid;
    int sockfd;
    struct sockaddr_in *addr;
    char send_buffer[SEND_BUFFER_SIZE];
    char recv_buffer[RECV_BUFFER_SIZE];
    struct swnd swnd;
    struct rwnd rwnd;
};

int m_socket(int domain, int type, int protocol);
int m_bind(int sockfd, const struct sockaddr *src_addr, socklen_t src_addrlen, const struct sockaddr *dest_addr, socklen_t dest_addrlen);
ssize_t m_sendto(int sockfd, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen);
ssize_t m_recvfrom(int sockfd, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len);
int m_close(int fd);
int dropMessage(float probability);

#endif