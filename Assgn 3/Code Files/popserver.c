/*
Name: Sanskar Mittal, Karthik Reddy
Roll number: 21CS10057, 21CS30058
Assignment 3: Mail Server and Client
File: popserver.c
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
#include <time.h>
#define MAX 5000

void recvData(int sockfd, char* buf, char* mainBuf, int size, int flags, char* expected, char* errMsg, char* errExpected){
    int n;
    
    memset(mainBuf, '\0', MAX);
    memset(buf, '\0', MAX);

    while(n = recv(sockfd, buf, size, flags)){
        if(n == -1){
            perror(errMsg);
            exit(EXIT_FAILURE);
        }
    
        buf[n] = '\0';      // for strlen feature    
        strncat(mainBuf, buf, n);

        if(strlen(mainBuf) >= strlen(expected)){
            if(strncmp(mainBuf, expected, strlen(expected)) != 0){
                perror(errExpected);
                exit(EXIT_FAILURE);
            }
        }
        if(strlen(mainBuf) > 1){
            if(mainBuf[strlen(mainBuf) - 1] == '\n' && mainBuf[strlen(mainBuf) - 2] == '\r'){
                break;
            }
        }
        memset(buf, '\0', MAX);
    }
    printf("C: %s",mainBuf);
}

void sendData(int sockfd, char* buf, int flags, char* errConnection, char* errMsg){
    if(send(sockfd, buf, strlen(buf), flags) == -1){
        if(errno == EPIPE){
            perror(errConnection);
            exit(EXIT_FAILURE);
        }
        else{
            perror(errMsg);
            exit(EXIT_FAILURE);
        }
    }
    printf("S: %s",buf);
}

int main(int argc, char* argv[]){
    if(argc != 2){
        perror("Usage: ./popserver <pop3_port>\n");
        exit(1);
    }

    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in cli_addr, serv_addr;

    // Opening a socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Cannot create socket\n");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    // Binding the socket to the server address
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("Unable to bind local address\n");
        exit(1);
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
            exit(EXIT_FAILURE);
        }

        if(fork() == 0){        // Child Process

            close(sockfd);      // Close the listening socket

            // Char buff used for sending and receiving messages
            char* buf = (char*)malloc((MAX + 10)*sizeof(char));
            char* mainBuf = (char*)malloc((MAX + 10)*sizeof(char));
            char* user = (char*)malloc((MAX)*sizeof(char));
            char* password = (char*)malloc((MAX)*sizeof(char));
            char* userLine = (char*)malloc(MAX*sizeof(char));
            char* mailFrom = (char*)malloc((MAX)*sizeof(char));     // This also includes sender's domain name
            char* mailTo = (char*)malloc((MAX)*sizeof(char));
            char* mail = (char*)malloc((MAX + 10)*sizeof(char));
            char* recvTime = (char*)malloc((100)*sizeof(char));
            char* userFileName = (char*)malloc((MAX + 10)*sizeof(char));
            char* domain = (char*)malloc((MAX + 10)*sizeof(char));

            memset(mainBuf, '\0', MAX);
            memset(buf, '\0', MAX);
            memset(user, '\0', MAX);
            memset(password, '\0', MAX);
            memset(userLine, '\0', MAX);
            memset(mail, '\0', MAX);
            memset(mailFrom, '\0', MAX);
            memset(mailTo, '\0', MAX);
            memset(recvTime, '\0', 100);
            memset(userFileName, '\0', MAX);
            memset(domain, '\0', MAX);

            // Send SERVER READY
            sprintf(buf, "+OK POP3 server ready\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at SERVER READY\n", "Error in sending SERVER READY\n");

            /* AUTHORIZATION STATE */

            // Receive USER
            recvData(newsockfd, buf, mainBuf, MAX, 0, "USER ", "Error in receiving USER\n", "USER not received\n");
            sscanf(mainBuf, "USER %s", user);

            // check if mailbox of such user exists
            if(access(user, F_OK) == -1){
                sprintf(buf, "-ERR sorry, no mailbox for %s here\r\n", user);
                sendData(newsockfd, buf, 0, "Client closed connection at USER not found\n", "Error in sending USER not found\n");
                close(newsockfd);
                
                // Enter free message here

                exit(EXIT_SUCCESS);
            }

            // Send USER OK
            sprintf(buf, "+OK %s is a valid mailbox\r\n", user);
            sendData(newsockfd, buf, 0, "Client closed connection at USER OK\n", "Error in sending USER OK\n");

            // Receive PASS
            recvData(newsockfd, buf, mainBuf, MAX, 0, "PASS ", "Error in receiving PASS\n", "PASS not received\n");
            sscanf(mainBuf, "PASS %s", password);

            // check if user and password is correct in user.txt
            FILE* userFile = fopen("user.txt", "r");
            int userFound = 0;
            char* tempUser = (char*)malloc(MAX*sizeof(char));   memset(tempUser, '\0', MAX);
            char* tempPass = (char*)malloc(MAX*sizeof(char));   memset(tempPass, '\0', MAX);

            while(fgets(userLine, MAX, userFile) != NULL){
                sscanf(userLine, "%s %s", tempUser, tempPass);
                if(strcmp(tempUser, user) == 0 && strcmp(tempPass, password) == 0){
                    userFound = 1;
                    break;
                }
            }
            free(tempUser);
            free(tempPass);
            fclose(userFile);
            
            if(userFound == 0){
                sprintf(buf, "-ERR invalid password\r\n");
                sendData(newsockfd, buf, 0, "Client closed connection at PASS not found\n", "Error in sending PASS not found\n");
                close(newsockfd);
                exit(EXIT_SUCCESS);
            }

            // Find message number and size in user's mailbox(to be used in STAT)
            sprintf(userFileName, "%s/mymailbox", user);
            FILE* userMailbox = fopen(userFileName, "r");

            int mailNum = 0;
            int mailSize = 0;
           
            // increase mailNum on every <.\n> line
            while(fgets(mail, MAX, userMailbox) != NULL){
                mailSize += strlen(mail);
                if(mail[0] == '.' && mail[1] == '\n'){
                    mailNum++;
                }
            }
            fclose(userMailbox);

            // Send PASS OK
            sprintf(buf, "+OK %s's maildrop has %d messages (%d octets)\r\n", user, mailNum, mailSize);

            /* TRANSACTION STATE */

            close(newsockfd);
            exit(EXIT_SUCCESS);
        }
        close(newsockfd);
    }

    close(sockfd);
    return 0;
}