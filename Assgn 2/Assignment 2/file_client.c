/*
Name: Sanskar Mittal
Roll number: 21CS10057
Assignment 2: Using TCP sockets
File: file_client.c
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
#define MAX 100

int main(){
    int sockfd;
    struct sockaddr_in serv_addr;

    // Setting up the server address
    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(20000);

    char* filename = (char*)malloc(MAX*sizeof(char));
    char* enc_filename = (char*)malloc(MAX*sizeof(char));
    char* buf = (char*)malloc((MAX + 1)*sizeof(char));

    // Server started
    printf("Server started...\n");

    while(1){

        // Opening a socket
        if((sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("Unable to create socket\n");
            exit(0);
        }

        // Take filename from user
        for (int i = 0; i < MAX; i++) filename[i] = '\0';
        printf("\nEnter filename(-1 to exit): ");
        scanf("%s", filename);

        // Exit condition
        if(strcmp(filename, "-1") == 0) break;

        // Check if file exists in the current directory
        if(access(filename, F_OK) == -1){
            printf("File does not exist! Try again.\n");
            continue;
        }
        else if(access(filename, R_OK) == -1){
            printf("Give read permission for the file.\n");
            continue;
        }

        // Take key from user and convert it to modulo 26
        int key;
        printf("Enter key: ");
        scanf("%d", &key);
        key = key%26;
        if(key < 0){
            key *= -1;
            key = 26 - key%26;
        }
        printf("Key(in modulo 26): %d\n", key);

        // Connect to the server
        if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
            perror("Unable to connect to server\n");
            exit(0);
        }

        // Send key to the server
        send(sockfd, &key, sizeof(key), 0);

        // Open the file
        int fp = open(filename, O_RDONLY);

        // Scan file contents until EOF and send it
        printf("Reading file...\n");
        int n;
        while((n = read(fp, buf, MAX)) > 0){
            buf[n] = '\0';
            send(sockfd, buf, strlen(buf), 0);
        }
        memset(buf, '\0', MAX);
        buf[0] = '$';
        send(sockfd, buf, strlen(buf), 0);      // EOF
        printf("File sent to SERVER!\n");

        // Store the encrypted file name
        for (int i = 0; i < MAX; i++) enc_filename[i] = '\0';
        sprintf(enc_filename, "%s.enc", filename);

        // Create the encrypted file
        if((fp = open(enc_filename, O_RDWR | O_CREAT, 0666)) < 0){
            perror("Unable to create file\n");
            exit(0);
        }

        // Receive the encrypted file contents from the server
        memset(buf, '\0', MAX);
        while((n = recv(sockfd, buf, sizeof(buf), 0)) > 0){
            buf[n] = '\0';
            write(fp, buf, strlen(buf));
            memset(buf, '\0', MAX);
        }
        printf("Encrypted file received from SERVER!\n");

        // Prints successful encryption message
        printf("File \"%s\" successfully encrypted as \"%s\"\n", filename, enc_filename);

        close(fp);
    }

    close(sockfd);
    free(filename);
    free(enc_filename);
    free(buf);

    return 0;
}