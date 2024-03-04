#include "msocket.h"

int m_socket(int domain, int tyoe, int protocol){}

int m_bind(int sockfd, const struct sockaddr *src_addr, socklen_t src_addrlen, const struct sockaddr *dest_addr, socklen_t dest_addrlen){}

ssize_t m_sendto(int sockfd, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen){}

ssize_t m_recvfrom(int sockfd, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len){}

int m_close(int fd){}

int dropMessage(float probability){}