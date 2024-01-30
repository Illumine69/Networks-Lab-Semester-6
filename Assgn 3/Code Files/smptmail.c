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
#define MAX 5000

void recvData(int sockfd, char* buf,char* mailBuf, int size, int flags, char* expected, char* errMsg, char* errExpected){
    int n, total_size = 0,curMailBufSize = strlen(mailBuf);
    memset(buf, '\0', MAX);
    while(n = recv(sockfd, buf, size, flags)){
        if(n == -1){
            perror(errMsg);
            exit(1);
        }
        total_size += n;
        buf[n] = '\0';
        strncat(mailBuf, buf, n);
        if(total_size >= strlen(expected)){
            if(strncmp(mailBuf + curMailBufSize, expected, strlen(expected)) != 0){
                perror(errExpected);
                exit(1);
            }
        }
        if(n > 0){
            if(mailBuf[strlen(mailBuf) - 1] == '\n' && mailBuf[strlen(mailBuf) - 2] == '\r'){
                break;
            }
        }
        memset(buf, '\0', MAX);
    }
    printf("C: %s",mailBuf+curMailBufSize);
}

void sendData(int sockfd, char* buf, int flags, char* errMsg, char* errExpected){
    if(send(sockfd, buf, strlen(buf), flags) == -1){
        if(errno == EPIPE){
            printf(errExpected);
            exit(1);
        }
        else{
            perror(errMsg);
            exit(1);
        }
    }
    printf("S: %s",buf);
}

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
            
            // Char buff used for sending and receiving messages
            char* buf = (char*)malloc((MAX + 1)*sizeof(char));
            char* mailBuf = (char*)malloc((MAX + 1)*sizeof(char));
            char* mailFrom = (char*)malloc((MAX)*sizeof(char));
            char* mailTo = (char*)malloc((MAX)*sizeof(char));

            memset(mailBuf, '\0', MAX);
            memset(buf, '\0', MAX);

            // Send SERVICE READY
            sprintf(buf, "220 <iitkgp.edu> Service ready\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at SERVICE READY\n", "Error in sending message at SERVICE READY\n");
            // if(send(newsockfd, buf, strlen(buf), 0) == -1){
            //     if(errno == EPIPE){
            //         printf("Client closed connection at SERVICE READY\n");
            //         exit(0);
            //     }
            //     else{
            //         perror("Error in sending message at SERVICE READY\n");
            //         exit(0);
            //     }
            // }
            // printf("S: %s",buf);

            // Receive HELO
            recvData(newsockfd, buf, mailBuf, MAX, 0, "HELO", "Error in receiving message at RECEIVING HELO\n", "HELO not received\n");
            // memset(buf, '\0', MAX);
            // if(recv(newsockfd, buf, MAX, 0) == -1){
            //     perror("Error in receiving message at RECEIVING HELO\n");
            //     exit(0);
            // }
            // if(strncmp(buf, "HELO", 4) != 0){
            //     perror("HELO not received\n");
            //     exit(0);
            // }
            // printf("C: %s",buf);

            // Acknowledge HELO and send OK
            memset(buf, '\0', MAX);
            sprintf(buf, "250 OK Hello iitkgp.edu\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at ACKNOWLEDGING HELO\n", "Error in sending message at ACKNOWLEDGING HELO\n");
            // if(send(newsockfd, buf, strlen(buf), 0) == -1){
            //     if(errno == EPIPE){
            //         printf("Client closed connection at ACKNOWLEDGING HELO\n");
            //         exit(0);
            //     }
            //     else{
            //         perror("Error in sending message at ACKNOWLEDGING HELO\n");
            //         exit(0);
            //     }
            // }
            // printf("S: %s",buf);

            // identify sending user
            recvData(newsockfd, buf, mailBuf, MAX, 0, "MAIL FROM:", "Error in receiving message at IDENTIFYING USER\n", "MAIL FROM not received\n");
            // memset(buf, '\0', MAX);
            // if(recv(newsockfd, buf, MAX, 0) == -1){
            //     perror("Error in receiving message at IDENTIFYING USER\n");
            //     exit(0);
            // }
            // if(strncmp(buf, "MAIL FROM:", 10) != 0){
            //     perror("MAIL FROM not received\n");
            //     exit(0);
            // }
            // printf("C: %s",buf);
            // for(int i=0;i < strlen(buf);++i){
            //     if(buf[i] == '<'){
            //         int j = i+1;
            //         while(buf[j] != '>'){
            //             mailFrom[j-i-1] = buf[j];
            //             ++j;
            //         }
            //         break;
            //     }
            // }

            // Acknowledge MAIL FROM and send OK
            memset(buf, '\0', MAX);
            sprintf(buf, "250 <%s>... Sender ok\r\n", mailFrom);
            sendData(newsockfd, buf, 0, "Client closed connection at ACKNOWLEDGING SENDING USER\n", "Error in sending message at ACKNOWLEDGING SENDING USER\n");
            // if(send(newsockfd, buf, strlen(buf), 0) == -1){
            //     if(errno == EPIPE){
            //         printf("Client closed connection at ACKNOWLEDGING SENDING USER\n");
            //         exit(0);
            //     }
            //     else{
            //         perror("Error in sending message at ACKNOWLEDGING SENDING USER\n");
            //         exit(0);
            //     }
            // }
            // printf("S: %s",buf);

            // identify target user
            int curMailBufSize = strlen(mailBuf);
            recvData(newsockfd, buf, mailBuf, MAX, 0, "RCPT TO:", "Error in receiving message at IDENTIFYING TARGET USER\n", "RCPT TO not received\n");
            // memset(buf, '\0', MAX);
            // if(recv(newsockfd, buf, MAX, 0) == -1){
            //     perror("Error in receiving message at IDENTIFYING TARGET USER\n");
            //     exit(0);
            // }
            // if(strncmp(buf, "RCPT TO:", 8) != 0){
            //     perror("RCPT TO not received\n");
            //     exit(0);
            // }
            // printf("C: %s",buf);
            for(int i=curMailBufSize;i < strlen(mailBuf);++i){
                if(mailBuf[i] == '<'){
                    int j = i+1;
                    while(mailBuf[j] != '>'){
                        mailTo[j-i-1] = mailBuf[j];
                        ++j;
                    }
                    break;
                }
            }

            // for(int i=0;i < strlen(buf);++i){
            //     if(buf[i] == '<'){
            //         int j = i+1;
            //         while(buf[j] != '>'){
            //             mailTo[j-i-1] = buf[j];
            //             ++j;
            //         }
            //         break;
            //     }
            // }

            // Acknowledge RCPT TO and send OK
            memset(buf, '\0', MAX);
            sprintf(buf, "250 root... Recipient ok\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at ACKNOWLEDGING TARGET USER\n", "Error in sending message at ACKNOWLEDGING TARGET USER\n");
            // if(send(newsockfd, buf, strlen(buf), 0) == -1){
            //     if(errno == EPIPE){
            //         printf("Client closed connection at ACKNOWLEDGING TARGET USER\n");
            //         exit(0);
            //     }
            //     else{
            //         perror("Error in sending message at ACKNOWLEDGING TARGET USER\n");
            //         exit(0);
            //     }
            // }
            // printf("S: %s",buf);

            // Client send "DATA"
            recvData(newsockfd, buf, mailBuf, MAX, 0, "DATA", "Error in receiving message at RECEIVING DATA\n", "DATA not received\n");
            // memset(buf, '\0', MAX);
            // if(recv(newsockfd, buf, MAX, 0) == -1){
            //     perror("Error in receiving message at RECEIVING DATA\n");
            //     exit(0);
            // }
            // if(strncmp(buf, "DATA", 4) != 0){
            //     perror("DATA not received\n");
            //     exit(0);
            // }
            // printf("C: %s",buf);

            // Acknowledge DATA and send OK
            memset(buf, '\0', MAX);
            sprintf(buf, "354 Enter mail, end with \".\" on a line by itself\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at ACKNOWLEDGING DATA\n", "Error in sending message at ACKNOWLEDGING DATA\n");
            // if(send(newsockfd, buf, strlen(buf), 0) == -1){
            //     if(errno == EPIPE){
            //         printf("Client closed connection at ACKNOWLEDGING DATA\n");
            //         exit(0);
            //     }
            //     else{
            //         perror("Error in sending message at ACKNOWLEDGING DATA\n");
            //         exit(0);
            //     }
            // }
            // printf("S: %s",buf);

            // Open file descriptor for storing mail



            // Receive mail
            memset(buf, '\0', MAX);
            int n;
            while(n = recv(newsockfd, buf, MAX, 0)){
                if(n == -1){
                    perror("Error in receiving message at RECEIVING MAIL\n");
                    exit(0);
                }
                
            }


        }
    }
}