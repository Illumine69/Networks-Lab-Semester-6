/*
Name: Sanskar Mittal, Karthik Reddy
Roll number: 21CS10057, 21CS30058
Assignment 3: Mail Server and Client
File: smtpmail.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#define MAX 80

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Usage: ./smtpmail <my_port>\n");
        exit(0);
    }

    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in cli_addr, serv_addr;

    // Opening a socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Cannot create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    // Binding the socket to the server address
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("Unable to bind local address\n");
        exit(0);
    }

    // Listening for upto 5 connections
    listen(sockfd, 5);

    // Server started message
    printf("Server started...\n");

    while(1){

        // Accepting a connection
        clilen = sizeof(cli_addr);
        memset(&cli_addr, 0, sizeof(cli_addr));
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if(newsockfd < 0){
            perror("Unable to accept connection\n");
            exit(0);
        }

        if(fork() == 0){        // Child Process

            close(sockfd);      // Close the listening socket
            
            char* buf = (char*)malloc((MAX + 1)*sizeof(char));
            sprintf(buf, "220 <iitkgp.edu> Service ready\r\n");
            if(send(newsockfd, buf, strlen(buf), 0) == -1){
                if(errno == EPIPE){
                    printf("Client closed connection\n");
                    exit(0);
                }
                else{
                    perror("Error in sending message\n");
                    exit(0);
                }
            }
        }
    }
}