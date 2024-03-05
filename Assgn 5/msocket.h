#ifndef MSOCKET_H
#define MSOCKET_H

#include <sys/socket.h>
#include <unistd.h>
#define T 5
#define p 0.05
#define N 25
#define SOCK_MTP SOCK_DGRAM
#define KEY ftok("/tmp", 'a')

int m_socket(int domain, int type, int protocol);
int m_bind(int sockfd, const struct sockaddr *src_addr, socklen_t src_addrlen, const struct sockaddr *dest_addr, socklen_t dest_addrlen);
ssize_t m_sendto(int sockfd, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen);
ssize_t m_recvfrom(int sockfd, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len);
int m_close(int fd);
int dropMessage(float probability);

#endif