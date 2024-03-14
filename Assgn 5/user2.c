
#include <arpa/inet.h>
#include <fcntl.h>
#include <msocket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int sockfd = m_socket(AF_INET, SOCK_MTP, 0);
    if (sockfd == -1) {
        printf("Error in creating socket\n");
        exit(1);
    }
    printf("Socket created successfully\n");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(10000);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    struct sockaddr_in destaddr;
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(8181);
    destaddr.sin_addr.s_addr = INADDR_ANY;

    if (m_bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr), NULL, 0) == -1) {
        printf("Error in binding\n");
        exit(1);
    }

    int fd = open("read.txt", O_RDWR | O_CREAT, 0777);
    if (fd == -1) {
        printf("Error in opening file\n");
        exit(1);
    }

    char buffer[1000];
    int n;
    while ((n = m_recvfrom(sockfd, buffer, 1000, 0, (struct sockaddr *)&destaddr, sizeof(destaddr)) > 0)) {
        write(fd, buffer, n);
    }

    if (m_close(sockfd) == -1) {
        printf("Error in closing socket\n");
        exit(1);
    }

    return 0;
}