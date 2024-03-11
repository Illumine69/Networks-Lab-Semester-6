/*
Name: Sanskar Mittal
Roll number: 21CS10057
Assignment 4: Implement a Peer-to-Peer Chat Application
File: peer.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>n
#include <sys/time.h>
#include <time.h>
#define MAX 5000


int fd_set_fn(int fd, fd_set* fds, int* max_fd) {

    FD_SET(fd, fds);
    if (fd > *max_fd) {
        *max_fd = fd;
    }
    return 0;
}

// set peer info
struct peer
{
    int port;
    char name[20];
    int connected;
}peer[3] = {
    {50000, "user_1", 0},
    {50001, "user_2", 0},
    {50002, "user_3", 0}
};


int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }   

    // CREATE A SERVER SOCKET
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    struct timeval timeout;
    int clilen;
    int n;
    int port = atoi(argv[1]);

    // Create a socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Unable to create socket\n");
        exit(1);
    }

    // Set the server address to 127.0.0.1
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_aton("127.0.0.1", &serv_addr.sin_addr);

    // Bind the socket to the server address
    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Unable to bind local address\n");
        exit(1);
    }

    // listen for incoming connections
    listen(sockfd, 3);

    fd_set master;
    int max_fd = 0;
    FD_ZERO(&master);
    fd_set_fn(sockfd, &master, &max_fd);
    fd_set_fn(STDIN_FILENO, &master, &max_fd);
    timeout.tv_sec = 300;


    while(1){
        fd_set read_fds = master;
        if(select(max_fd + 1, &read_fds, NULL, NULL, &timeout) < 0){
            perror("select");
            exit(4);
        }

        for(int i = 0; i <= max_fd; i++){
            if(FD_ISSET(i, &read_fds)){
                if(i == sockfd){
                    // accept a new connection
                    clilen = sizeof(cli_addr);
                    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
                    if(newsockfd < 0){
                        perror("accept");
                        exit(1);
                    }
                    
                    // get message from the client
                    char buffer[MAX];
                    memset(buffer, 0, MAX);
                    n = recv(newsockfd, buffer, MAX, 0);

                    if(n < 0){
                        perror("recv");
                        return 1;
                    }
                    int client_num = buffer[0] - '0';
                    char* cli_name = peer[client_num].name;
                    peer[client_num].connected = 1;
                  
                    printf("Message from %s: %s\n", cli_name, buffer+1);
                    
                    fd_set_fn(newsockfd, &master, &max_fd);
                } 
                else if(i == STDIN_FILENO){
                    // read from stdin
                    char buffer[MAX];
                    memset(buffer, 0, MAX);
                    fgets(buffer, sizeof(buffer), stdin);
                    buffer[strlen(buffer) - 1] = '\0';

                    // get username from buffer
                    char user_name[20];
                    strncpy(user_name, buffer, 6);
                    user_name[6] = '\0';

                    // get message from buffer
                    char message[MAX];
                    memset(message, 0, MAX);
                    for(int i=0;i<3;i++){
                        if(port == peer[i].port){
                            message[0] = '0'+i;
                        }
                    }
                    strcat(message, buffer + 7);
                    message[strlen(message)] = '\0';

                    for(int j = 0; j < 3; j++){
                        if(strcmp(user_name, peer[j].name) == 0){
                            newsockfd = socket(AF_INET, SOCK_STREAM, 0);
                            if(newsockfd < 0){
                                perror("socket");
                                return 1;
                            }
                            struct sockaddr_in addr;
                            addr.sin_family = AF_INET;
                            addr.sin_port = htons(peer[j].port);
                            inet_aton("127.0.0.1", &addr.sin_addr);
                            if(connect(newsockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
                                perror("connect");
                                return 1;
                            }
                            fd_set_fn(newsockfd, &master, &max_fd);
                            send(newsockfd, message, strlen(message), 0);
                            break;
                        }
                    }
                }
            }
        }
    }
}

    


