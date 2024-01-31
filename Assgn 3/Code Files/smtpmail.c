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

void sendData(int sockfd, char* buf, int flags, char* errMsg, char* errExpected){
    if(send(sockfd, buf, strlen(buf), flags) == -1){
        if(errno == EPIPE){
            perror(errExpected);
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
        perror("Usage: ./smtpmail <my_port>\n");
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
            char* mailFrom = (char*)malloc((MAX)*sizeof(char));
            char* mailTo = (char*)malloc((MAX)*sizeof(char));
            char* mail = (char*)malloc((MAX + 10)*sizeof(char));
            char* recvTime = (char*)malloc((100)*sizeof(char));

            memset(mainBuf, '\0', MAX);
            memset(buf, '\0', MAX);
            memset(mail, '\0', MAX);
            memset(mailFrom, '\0', MAX);
            memset(mailTo, '\0', MAX);
            memset(recvTime, '\0', 100);

            // Send SERVICE READY
            sprintf(buf, "\n220 <iitkgp.edu> Service ready\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at SERVICE READY\n", "Error in sending message at SERVICE READY\n");

            // Receive HELO
            recvData(newsockfd, buf, mainBuf, MAX, 0, "HELO", "Error in receiving message at RECEIVING HELO\n", "HELO not received\n");

            // Acknowledge HELO and send OK
            memset(buf, '\0', MAX);
            sprintf(buf, "250 OK Hello iitkgp.edu\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at ACKNOWLEDGING HELO\n", "Error in sending message at ACKNOWLEDGING HELO\n");

            // identify sending user
            recvData(newsockfd, buf, mainBuf, MAX, 0, "MAIL FROM:", "Error in receiving message at IDENTIFYING USER\n", "MAIL FROM not received\n");
    
            for(int i=0;i < strlen(mainBuf);++i){
                if(mainBuf[i] == '<'){
                    int j = i+1;
                    while(mainBuf[j] != '@'){
                        mailFrom[j-i-1] = mainBuf[j];
                        ++j;
                    }
                    break;
                }
            }

            // Acknowledge MAIL FROM and send OK
            memset(buf, '\0', MAX);
            sprintf(buf, "250 <%s@iitkgp.edu>... Sender ok\r\n", mailFrom);
            sendData(newsockfd, buf, 0, "Client closed connection at ACKNOWLEDGING SENDING USER\n", "Error in sending message at ACKNOWLEDGING SENDING USER\n");

            // identify target user
            recvData(newsockfd, buf, mainBuf, MAX, 0, "RCPT TO:", "Error in receiving message at IDENTIFYING TARGET USER\n", "RCPT TO not received\n");

            for(int i=0;i < strlen(mainBuf);++i){
                if(mainBuf[i] == '<'){
                    int j = i+1;
                    while(mainBuf[j] != '@'){
                        mailTo[j-i-1] = mainBuf[j];
                        ++j;
                    }
                    break;
                }
            }

            // Acknowledge RCPT TO and send OK
            memset(buf, '\0', MAX);
            sprintf(buf, "250 root... Recipient ok\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at ACKNOWLEDGING TARGET USER\n", "Error in sending message at ACKNOWLEDGING TARGET USER\n");

            // Client send "DATA"
            recvData(newsockfd, buf, mainBuf, MAX, 0, "DATA", "Error in receiving message at RECEIVING DATA\n", "DATA not received\n");

            // Acknowledge DATA and send OK
            memset(buf, '\0', MAX);
            sprintf(buf, "354 Enter mail, end with \".\" on a line by itself\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at ACKNOWLEDGING DATA\n", "Error in sending message at ACKNOWLEDGING DATA\n");

            // Open file descriptor for storing mail
            char* userFileName = (char*)malloc((MAX + 10)*sizeof(char));
            memset(userFileName, '\0', MAX);
            sprintf(userFileName, "%s/mymailbox", mailTo);
            int mymailbox = open(userFileName, O_RDWR | O_CREAT);
            if(mymailbox < 0){
                perror("Unable to create file\n");
                exit(1);
            }

            // Receive mail
            memset(buf, '\0', MAX);
            memset(mainBuf, '\0', MAX);
            int n, total_len = 0;
            while(n = recv(newsockfd, buf, MAX, 0)){
                if(n == -1){
                    perror("Error in receiving message at RECEIVING MAIL\n");
                    exit(0);
                }
                buf[n] = '\0';
                if((total_len  > 0) && (mainBuf[total_len - 1] == '\r') && (buf[0] == '\n')){
                    mainBuf[total_len - 1] = '\n';
                    buf[0] = '\0';
                }
                for(int i=0;i < n;i++){
                    if(buf[i] == '\r' && buf[i+1] == '\n'){
                        buf[i] = '\n';
                        buf[i+1] = '\0';
                    }
                }
                strncpy(mainBuf, buf, n);
                total_len += n;
                if( total_len > 5 
                    && mainBuf[total_len - 1] == '\0'
                    && mainBuf[total_len - 2] == '\n'
                    && mainBuf[total_len - 3] == '.'
                    && mainBuf[total_len - 4] == '\0'
                    && mainBuf[total_len - 5] == '\n'){
                    break;
                }
                memset(buf, '\0', MAX);
            }

            // Get current time of the system
            time_t currentTime;
            time(&currentTime);

            struct tm *localTime = localtime(&currentTime);

            // Format the time as a string in "Date:Hour:Minute" format
            char formattedTime[21];  
            strftime(formattedTime, sizeof(formattedTime), "%d-%m-%Y : %H : %M", localTime);

            sprintf(recvTime, "Received: %s\n", formattedTime);
            // Cleaning mailBuf content to match with expected output
            int j = 0;
            int newlineLeft = 3;
            for(int i=0;i < total_len;i++){
                if(newlineLeft == 0){
                    int len = strlen(recvTime);
                    strncat(mail, recvTime, len);
                    newlineLeft--;
                    j += len;
                }
                if(mainBuf[i] == '\0'){
                    newlineLeft--;
                    continue;
                }
                mail[j++] = mainBuf[i];
            }

            write(mymailbox, mail, j);

            // Acknowledge mail and send OK
            memset(buf, '\0', MAX);
            sprintf(buf, "250 OK Message accepted for delivery\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at ACKNOWLEDGING MAIL\n", "Error in sending message at ACKNOWLEDGING MAIL\n");

            // Sender close connection
            memset(buf, '\0', MAX);
            recvData(newsockfd, buf, mainBuf, MAX, 0, "QUIT", "Error in receiving message at CLOSING CONNECTION\n", "QUIT not received\n");

            // Close connection
            memset(buf, '\0', MAX);
            sprintf(buf, "221 iitkgp.edu closing connection\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at CLOSING CONNECTION\n", "Error in sending message at CLOSING CONNECTION\n");

            close(newsockfd);
            printf("Client closed connection\n");
            free(buf);
            free(mainBuf);
            free(mailFrom);
            free(mailTo);
            free(userFileName);
            exit(0);
        }
        close(newsockfd);
    }

    close(sockfd);
    return 0;
}