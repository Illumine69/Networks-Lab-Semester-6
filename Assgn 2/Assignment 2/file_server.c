/*
Name: Sanskar Mittal
Roll number: 21CS10057
Assignment 2: Using TCP sockets
File: file_server.c
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
#define MAX 80

int main(){
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
    serv_addr.sin_port = htons(20000);

    // Binding the socket to the server address
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("Unable to bind local address\n");
        exit(0);
    }

    // Listening for upto 5 connections
    listen(sockfd, 5);

    char* filename = (char*)malloc(MAX*sizeof(char));
    char* enc_filename = (char*)malloc(MAX*sizeof(char));
    char* buf = (char*)malloc((MAX + 1)*sizeof(char));

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

            // Create a file to store the received file
            for (int i = 0; i < MAX; i++) filename[i] = '\0';
            sprintf(filename, "%s.%d.txt", inet_ntoa(cli_addr.sin_addr),(int) ntohs(cli_addr.sin_port));
            int fp = open(filename, O_RDWR | O_CREAT);
            if(fp < 0){
                perror("Unable to create file\n");
                exit(0);
            }

            // Receive key from client
            int key;
            recv(newsockfd, &key, sizeof(key), 0);

            // // Receive file from client
            int n;
            memset(buf, '\0', MAX);
            while((n = recv(newsockfd, buf, MAX, 0)) > 0){
                buf[n] = '\0';
                if(buf[n-1] == '$'){        // End of file
                    buf[n-1] = '\0';
                    write(fp, buf, strlen(buf));
                    break;
                }
                write(fp, buf, strlen(buf));
                memset(buf, '\0', MAX);
            }

            printf("\nCLIENT sent a file.\n");

            // Store the encrypted file name
            for (int i = 0; i < MAX; i++) enc_filename[i] = '\0';
            sprintf(enc_filename, "%s.enc", filename);
            
            int enc_fp = open(enc_filename, O_RDWR | O_CREAT);
            if(enc_fp < 0){
                perror("Unable to create file\n");
                exit(0);
            }

            // Open original file in read only mode
            close(fp);
            fp = open(filename, O_RDONLY);
    
            // Encrypt the file using Caesar Cipher
            printf("Encrypting file...\n");
            while((n = read(fp, buf, MAX)) > 0){
                for(int i = 0; i < n; i++){
                    if(buf[i] >= 'a' && buf[i] <= 'z'){
                        if(buf[i] + key > 'z') buf[i] = 'a' + (buf[i] + key - 'z' - 1);
                        else buf[i] += key;
                    }
                    else if(buf[i] >= 'A' && buf[i] <= 'Z'){
                        if(buf[i] + key > 'Z') buf[i] = 'A' + (buf[i] + key - 'Z' - 1);
                        else buf[i] += key;
                    }
                }
                write(enc_fp, buf, n);
                memset(buf, '\0', MAX);
            }

            // Open encrypted file in read only mode
            close(enc_fp);
            enc_fp = open(enc_filename, O_RDONLY);

            // Send the encrypted file to the client
            while((n = read(enc_fp, buf, MAX)) > 0){
                buf[n] = '\0';
                send(newsockfd, buf, strlen(buf), 0);
            }
            printf("Encrypted file sent to CLIENT!\n");

            close(newsockfd);
            exit(0);
        }
        close(newsockfd);
    }

    close(sockfd);
    free(filename);
    free(enc_filename);
    free(buf);

    return 0;
}