
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8181);
    servaddr.sin_addr.s_addr = inet_addr("192.7.1.1");

    // printf("IP: %s\n", inet_ntoa(servaddr.sin_addr));
    // printf("Port: %d\n", ntohs(servaddr.sin_port));
    // printf("Family: %d\n", servaddr.sin_family);
    // printf("IP: %d\n", servaddr.sin_addr.s_addr);
    // printf("Port: %d\n", servaddr.sin_port);
    // printf("%d\n", servaddr.sin_addr.s_addr == inet_addr(inet_ntoa(servaddr.sin_addr)));

    char* ack = (char*)malloc(0);
    sprintf(ack, "%s", "HILEADFKJAKsfefsefesfsef");
    printf("%s\n", ack);
}