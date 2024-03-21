
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

    // char* ack = (char*)malloc(0);
    // sprintf(ack, "%s", "HILEADFKJAKsfefsefesfsef");
    // printf("%s\n", ack);
    char recv_buffer[1500] = "awdwad$adwa\r\n$ad4w$\rDA$WAD$\na4wefw";
    // strcpy(recv_buffer, "awdwad$adwa$ad4w$DA$WAD$a4\0wefw");
    char *string = strtok(recv_buffer, "\r\n");
    while (string != NULL) {
        printf("%s\n", string);
        string = strtok(NULL, "\r\n");
    }
}