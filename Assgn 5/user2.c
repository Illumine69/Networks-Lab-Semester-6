
#include <arpa/inet.h>
#include <fcntl.h>
#include <msocket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
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
    inet_aton("127.0.0.1", &servaddr.sin_addr);
    struct sockaddr_in destaddr;
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(8181);
    inet_aton("127.0.0.1", &destaddr.sin_addr);

    if (m_bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr), (struct sockaddr *)&destaddr, sizeof(destaddr)) == -1) {
        printf("Error in binding\n");
        exit(1);
    }
    printf("Here bind done\n");
    fflush(stdout);
    int fd = open("read.txt", O_RDWR | O_CREAT, 0777);
    if (fd == -1) {
        printf("Error in opening file\n");
        exit(1);
    }

    char buffer[1000];
    int n;
    while (1) {
        n = m_recvfrom(sockfd, buffer, 1000, 0, NULL, NULL);
        if (n == -1) {
            perror("Error in receiving\n");
            continue;
        }
        if (n == 0) {
            break;
        }
        int val = write(fd, buffer, n);
        if (val == -1) {
            printf("Error in writing to file\n");
            exit(1);
        }
        printf("Writing %d bytes\n", val);
    }
    // while ((n = m_recvfrom(sockfd, buffer, 1000, 0, (struct sockaddr *)&destaddr, sizeof(destaddr)) > 0)) {

    //     write(fd, buffer, n);
    // }

    // if (m_close(sockfd) == -1) {
    //     printf("Error in closing socket\n");
    //     exit(1);
    // }

    return 0;
}