// Name: Sanskar Mittal
// Roll No: 21CS10057

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(40000);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Unable to connect!");
    }
    char charities[20][200];
    char message[10000];
    int ret = recv(sockfd, message, 10000, 0);
    printf("\t\tCharities\n");
    int j = 0, start = 0, k = 0;
    for (int i = 0; i < ret; i++) {
        if (start == 0) {
            start = 1;
            printf("%d: ", j + 1);
        }
        charities[j][k++] = message[i];
        if (message[i] == '\0') {
            printf("\n");
            j++;
            start = 0;
            k = 0;
            if (message[i + 1] == '\0') {
                break;
            }
            continue;
        }
        printf("%c", message[i]);
    }

    printf("Want to donate? (Press 1 if Yes, Else 0): ");
    int donate;
    scanf("%d", &donate);
    if (donate == 0) {
        close(sockfd);
        return 0;
    } else {
        printf("Enter charity num: ");
        int charity_num;
        scanf("%d", &charity_num);
        if (charity_num < 1 || charity_num > j) {
            printf("Wrong number!\n");
            exit(EXIT_FAILURE);
        }
        send(sockfd, charities[charity_num - 1], strlen(charities[charity_num - 1]), 0);
        char answer[10000];
        recv(sockfd, answer, 10000, 0);
        printf("%s", answer);
        close(sockfd);
        return 0;
    }
}