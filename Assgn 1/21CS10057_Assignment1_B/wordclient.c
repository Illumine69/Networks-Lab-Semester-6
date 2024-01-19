/*
Name: Sanskar Mittal
Roll number: 21CS10057
Assignment 1 Part B: Simple Datagram Socket using POSIX C
File: wordclient.c
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
    int sockfd, err;
    struct sockaddr_in servaddr;
    
    // Creating socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8181);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // file name to read from
    char *filename = "word.txt";
    
    sendto(sockfd, (const char *)filename, strlen(filename), 0, (const struct sockaddr*) &servaddr, sizeof(servaddr));

    // receive the file contents
    char serv_message[MAXLEN];
    socklen_t len;
    int count = recvfrom(sockfd, (char *)serv_message, MAXLEN, 0, (struct sockaddr *) &servaddr, &len);
    serv_message[count] = '\0';
    char err_find[150] = "NOTFOUND ";
    strcat(err_find, filename);   

    if(strcmp(serv_message, err_find) == 0){
        close(sockfd);
        printf("File %s Not Found\n", filename);
        return 0;
    }
    printf("SERVER: %s", serv_message);

    // write the contents to a file
    FILE* fp = fopen("out.txt", "w");

    for(int i = 1;;++i){
        char word[10] = "WORD";
        char num[10];
        sprintf(num, "%d", i);
        strcat(word, num);
        printf("%s\n", word);

        sendto(sockfd, (const char *)word, strlen(word), 0, (const struct sockaddr*) &servaddr, sizeof(servaddr));
        int size = recvfrom(sockfd, (char *)serv_message, MAXLEN, 0, (struct sockaddr *)&servaddr, &len);
        serv_message[size] = '\0';
        printf("SERVER: %s\n", serv_message);
        
        if(strcmp(serv_message, "END") == 0){
            break;
        }
        fprintf(fp, "%s\n", serv_message);
    }
    printf("File received successfully\n");
    fclose(fp);
    close(sockfd);
    return 0;
}