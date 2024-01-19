/*
Name: Sanskar Mittal
Roll number: 21CS10057
Assignment 1 Part B: Simple Datagram Socket using POSIX C
File: wordserver.c
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#define MAXLEN 1024

int main(){
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    int count;

    // Creating socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8181);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server Running....\n");

    len = sizeof(cliaddr);
    char filename[150];
    count = recvfrom(sockfd, (char *)filename, 150, 0, (struct sockaddr *) &cliaddr, &len);
    filename[count] = '\0';

    printf("CLIENT: %s\n", filename);

    FILE *fp = fopen(filename, "r");

    if(fp == NULL){
        char err_find[150] = "NOTFOUND ";
        strcat(err_find, filename);
        printf("%s\n", err_find);
        sendto(sockfd, (const char *)err_find, strlen(err_find), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
        close(sockfd);
        return 0;
    }

    char* word;
    size_t word_len;
    getline(&word, &word_len, fp);
    printf("%s", word);
    sendto(sockfd, (const char *)word, strlen(word), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));

    while(strcmp(word, "END") != 0){
        printf("Message sent to client. Waiting for response...\n");

        char cli_message[MAXLEN];
        int size = recvfrom(sockfd, (char *)cli_message, MAXLEN, 0, (struct sockaddr *) &cliaddr, &len);
        cli_message[size] = '\0';

        printf("CLIENT: %s\n", cli_message);
        fscanf(fp, "%s", word);
        printf("%s\n", word);

        sendto(sockfd, (const char *)word, strlen(word), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    }

    printf("File sent successfully. Terminating...\n");
    close(sockfd);
    return 0;
}