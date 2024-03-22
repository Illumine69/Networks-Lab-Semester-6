
#include <arpa/inet.h>
#include <errno.h>
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
    servaddr.sin_port = htons(8181);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    struct sockaddr_in destaddr;
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(10000);
    destaddr.sin_addr.s_addr = INADDR_ANY;

    if (m_bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr), (struct sockaddr *)&destaddr, sizeof(destaddr)) == -1) {
        // printf("Error in binding\n");
        perror("Error in binding");
        exit(1);
    }

    int fd = open("send.txt", O_RDONLY);
    if (fd == -1) {
        printf("Error in opening file\n");
        exit(1);
    }
    // printf("Here\n");
    char buffer[1000], main_buffer[1000];
    int n;
    while ((n = read(fd, buffer, 1000)) > 0) {
        // TODO: ensure that only 1000 bytes are sent at a time
        while (m_sendto(sockfd, buffer, n, 0, (const struct sockaddr *)&destaddr, sizeof(destaddr)) == -1) {
            if (errno == ENOBUFS) {
                // Buuder is full. Resend the same message
                continue;
            }
            printf("Error in sending\n");
            exit(1);
        }
    }
    if (m_close(sockfd) == -1) {
        printf("Error in closing socket\n");
        exit(1);
    }

    return 0;
}