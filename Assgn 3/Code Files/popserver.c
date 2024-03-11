/*
Name: Sanskar Mittal, Karthik Reddy
Roll number: 21CS10057, 21CS30058
Assignment 3: Mail Server and Client
File: popserver.c
*/

/*
Instructions for the evaluator:
- As mentioned in the assignment, you need to add username and password to user.txt .
  For example: xyz 123
- Ensure that the user name and password doesn't have any spaces in between.
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

void recvData(int sockfd, char* buf, char* mainBuf, int size, int flags, char* expected, char* errMsg, char* errExpected,int checkExpected){
    int n;
    
    memset(mainBuf, '\0', MAX);
    memset(buf, '\0', MAX);

    while((n = recv(sockfd, buf, size, flags))){
        if(n == -1){
            perror(errMsg);
            exit(EXIT_FAILURE);
        }
    
        buf[n] = '\0';      // for strlen feature    
        strncat(mainBuf, buf, n);

        if(strlen(mainBuf) >= strlen(expected)){
            if(checkExpected && (strncmp(mainBuf, expected, strlen(expected)) != 0)){
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
            char* mail = (char*)malloc((MAX + 10)*sizeof(char));
            char* userFileName = (char*)malloc((MAX + 10)*sizeof(char));

            memset(mainBuf, '\0', MAX);
            memset(buf, '\0', MAX);
            memset(user, '\0', MAX);
            memset(password, '\0', MAX);
            memset(userLine, '\0', MAX);
            memset(mail, '\0', MAX);
            memset(userFileName, '\0', MAX);

            // Send SERVER READY
            sprintf(buf, "+OK POP3 server ready\r\n");
            sendData(newsockfd, buf, 0, "Client closed connection at SERVER READY\n", "Error in sending SERVER READY\n");

            /* AUTHORIZATION STATE */

            // Receive USER
            recvData(newsockfd, buf, mainBuf, MAX, 0, "USER ", "Error in receiving USER\n", "USER not received\n",1);
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
            recvData(newsockfd, buf, mainBuf, MAX, 0, "PASS ", "Error in receiving PASS\n", "PASS not received\n",1);
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
            FILE* userMailbox;

            if((userMailbox = fopen(userFileName, "r")) == NULL){
                sprintf(buf, "-ERR unable to open mailbox for %s\r\n", user);
                sendData(newsockfd, buf, 0, "Client closed connection at MAILBOX not found\n", "Error in sending MAILBOX not found\n");
                close(newsockfd);
                exit(EXIT_FAILURE);
            }

            int mailNum = 0;
            int totalMailSize = 0;
           
            memset(mail, '\0', MAX);
            while(fgets(mail, MAX, userMailbox) != NULL){
                totalMailSize += (strlen(mail) + 1);
                if(mail[0] == '.' && mail[1] == '\n'){
                    mailNum++;
                }
                memset(mail, '\0', MAX);
            }
            fclose(userMailbox);
            userMailbox = fopen(userFileName, "r");

            int mailSize[mailNum], curMailNum = 0;
            int curMailSize = 0;
            memset(mailSize, 0, mailNum*sizeof(int));
            memset(mail, '\0', MAX);
            while(fgets(mail, MAX, userMailbox) != NULL){
                curMailSize += (strlen(mail) + 1);
                if(mail[0] == '.' && mail[1] == '\n'){
                    mailSize[curMailNum] = curMailSize;
                    curMailSize = 0;
                    curMailNum++;
                }
                memset(mail, '\0', MAX);
            }
            fclose(userMailbox);
            userMailbox = fopen(userFileName, "r");

            // Send PASS OK
            sprintf(buf, "+OK %s's maildrop has %d messages (%d octets)\r\n", user, mailNum, totalMailSize);
            sendData(newsockfd, buf, 0, "Client closed connection at PASS OK\n", "Error in sending PASS OK\n");

            /* TRANSACTION STATE */
            curMailNum = mailNum;

            while(1){                

                // Receive command
                recvData(newsockfd, buf, mainBuf, MAX, 0, "STAT", "Error in receiving command\n", "Command not received\n",0);

                // if command is QUIT
                if(strncmp(mainBuf, "QUIT", 4) == 0){
                    break;
                }

                // if command is STAT
                if(strncmp(mainBuf, "STAT", 4) == 0){
                    sprintf(buf, "+OK %d %d\r\n", curMailNum, totalMailSize);
                    sendData(newsockfd, buf, 0, "Client closed connection at STAT\n", "Error in sending STAT\n");
                }

                // if command is LIST
                if(strncmp(mainBuf, "LIST", 4) == 0){
                    int num = 0;
                    // check if LIST is followed by a number
                    if(strlen(mainBuf) > 6){
                        sscanf(mainBuf, "LIST %d", &num);
                        if(num > mailNum || num < 1){
                            sprintf(buf, "-ERR no such message, only %d messages in maildrop\r\n", mailNum);
                            sendData(newsockfd, buf, 0, "Client closed connection at LIST OUTOFBOUND\n", "Error in sending LIST OUTOFBOUND\n");
                        }
                        else if(mailSize[num - 1] == 0){
                            sprintf(buf, "-ERR message %d has been deleted\r\n", num);
                            sendData(newsockfd, buf, 0, "Client closed connection at LIST DELETED\n", "Error in sending LIST DELETED\n");
                        }
                        else{
                            sprintf(buf, "+OK %d %d\r\n", num, mailSize[num - 1]);
                            sendData(newsockfd, buf, 0, "Client closed connection at LIST\n", "Error in sending LIST\n");
                        }
                    }
                    else{
                        sprintf(buf, "+OK %d messages (%d octets)\r\n", curMailNum, totalMailSize);
                        sendData(newsockfd, buf, 0, "Client closed connection at LIST ALL\n", "Error in sending LIST ALL\n");
                        for(int i = 0; i < mailNum; i++){
                            if(mailSize[i] == 0) continue;
                            sprintf(buf, "%d %d\r\n", i + 1, mailSize[i]);
                            sendData(newsockfd, buf, 0, "Client closed connection at LIST\n", "Error in sending LIST\n");
                        }
                        sprintf(buf, ".\r\n");
                        sendData(newsockfd, buf, 0, "Client closed connection at LIST COMPLETE\n", "Error in sending LIST COMPLETE\n");
                    }
                }

                // if command is RETR
                if(strncmp(mainBuf, "RETR", 4) == 0){
                    int num = 0;
                    sscanf(mainBuf, "RETR %d", &num);
                    if(num > mailNum || num < 1){
                        sprintf(buf, "-ERR no such message, only %d messages in maildrop\r\n", mailNum);
                        sendData(newsockfd, buf, 0, "Client closed connection at RETR OUTOFBOUND\n", "Error in sending RETR OUTOFBOUND\n");
                    }
                    else if(mailSize[num - 1] == 0){
                        sprintf(buf, "-ERR message %d has been deleted\r\n", num);
                        sendData(newsockfd, buf, 0, "Client closed connection at RETR DELETED\n", "Error in sending RETR DELETED\n");
                    }
                    else{
                        sprintf(buf, "+OK %d octets\r\n", mailSize[num - 1]);
                        sendData(newsockfd, buf, 0, "Client closed connection at RETR\n", "Error in sending RETR\n");

                        // Send the mail
                        int mailFound = 0;
                        memset(mail, '\0', MAX);
                        while(fgets(mail, MAX, userMailbox) != NULL){
                            if(mailFound == (num-1)){
                                mail[strlen(mail) - 1] = '\r';
                                mail[strlen(mail)] = '\n';
                                mail[strlen(mail) + 1] = '\0';
                                sendData(newsockfd, mail, 0, "Client closed connection at RETR MAIL\n", "Error in sending RETR MAIL\n");
                            }
                            if(mail[0] == '.'){
                                mailFound++;
                            }
                            if(mailFound == num){
                                break;
                            }
                            memset(mail, '\0', MAX);
                        }
                        fclose(userMailbox);
                        userMailbox = fopen(userFileName, "r");
                    }
                }

                // if command is DELE
                if(strncmp(mainBuf, "DELE", 4) == 0){
                    int num = 0;
                    sscanf(mainBuf, "DELE %d", &num);
                    if(num > mailNum || num < 1){
                        sprintf(buf, "-ERR no such message, only %d messages in maildrop\r\n", mailNum);
                        sendData(newsockfd, buf, 0, "Client closed connection at DELE OUTOFBOUND\n", "Error in sending DELE OUTOFBOUND\n");
                    }
                    else if(mailSize[num - 1] == 0){
                        sprintf(buf, "-ERR message %d already deleted\r\n", num);
                        sendData(newsockfd, buf, 0, "Client closed connection at DELE DELETED\n", "Error in sending DELE DELETED\n");
                    }
                    else{
                        totalMailSize -= mailSize[num - 1];
                        mailSize[num - 1] = 0;
                        curMailNum--;
                        sprintf(buf, "+OK message %d deleted\r\n", num);
                        sendData(newsockfd, buf, 0, "Client closed connection at DELE\n", "Error in sending DELE\n");
                    }
                }

                // if command is NOOP
                if(strncmp(mainBuf, "NOOP", 4) == 0){
                    sprintf(buf, "+OK\r\n");
                    sendData(newsockfd, buf, 0, "Client closed connection at NOOP\n", "Error in sending NOOP\n");
                }

                // if command is RSET
                if(strncmp(mainBuf, "RSET", 4) == 0){
                    memset(mail, '\0', MAX);
                    curMailSize = 0;
                    totalMailSize = 0;
                    curMailNum = 0;
                    memset(mail, '\0', MAX);
                    while(fgets(mail, MAX, userMailbox) != NULL){
                        curMailSize += (strlen(mail) + 1);
                        totalMailSize += curMailSize;
                        if(mail[0] == '.' && mail[1] == '\n'){
                            mailSize[curMailNum] = curMailSize;
                            curMailSize = 0;
                            curMailNum++;
                        }
                        memset(mail, '\0', MAX);
                    }
                    fclose(userMailbox);
                    userMailbox = fopen(userFileName, "r");
                    
                    sprintf(buf, "+OK maildrop has %d messages (%d octets)\r\n", mailNum, totalMailSize);
                    sendData(newsockfd, buf, 0, "Client closed connection at RSET\n", "Error in sending RSET\n");
                }

            }

            /* UPDATE STATE */
            mailNum = curMailNum;
            curMailNum = 0;

            // create a temporary file
            FILE* tempFile = fopen("tempfile", "w");
            fclose(userMailbox);
            userMailbox = fopen(userFileName, "r");

            memset(mail, '\0', MAX);
            while(fgets(mail, MAX, userMailbox) != NULL){
                if(mailSize[curMailNum] != 0){
                    fprintf(tempFile, "%s", mail);
                }
                if(mail[0] == '.' && mail[1] == '\n'){
                    curMailNum++;
                }
                memset(mail, '\0', MAX);
            }

            // clean the user mailbox
            fclose(tempFile);
            tempFile = fopen("tempfile", "r");
            fclose(userMailbox);
            userMailbox = fopen(userFileName, "w");
            memset(mail, '\0', MAX);
            while(fgets(mail, MAX, tempFile) != NULL){
                fprintf(userMailbox, "%s", mail);
                memset(mail, '\0', MAX);
            }

            // delete the temporary file
            if(remove("tempfile") == -1){
                perror("Error in deleting temporary file\n");
                exit(EXIT_FAILURE);
            }
            sprintf(buf, "+OK POP3 server signing off (%d messages left)\r\n", mailNum);
            sendData(newsockfd, buf, 0, "Client closed connection at UPDATE\n", "Error in sending UPDATE\n");

            fclose(tempFile);
            fclose(userMailbox);
            free(buf);
            free(mainBuf);
            free(user);
            free(password);
            free(userLine);
            free(mail);
            free(userFileName);

            close(newsockfd);
            exit(EXIT_SUCCESS);
        }
        close(newsockfd);
    }

    close(sockfd);
    return 0;
}