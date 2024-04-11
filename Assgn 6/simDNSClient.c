/*
Name: Sanskar Mittal, Karthik Reddy
Roll number: 21CS10057, 21CS30058
Assignment 6: Implementing a Custom Protocol using Raw Sockets
File: simDNSClient.c
*/

#include <arpa/inet.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_SIZE 500
#define BUF_SIZE 1000
const int timeout = 5;
/*
simDNS Query Packet
1) ID: 16 bits
2) Message type: 1 bit
3) Number of Queries: 3 bits
4) Query Strings: Variable

simDNS Response Packet
1) ID: 16 bits
2) Message type: 1 bit
3) Number of Responses: 3 bits
4) Response Strings: Multiple of 33 bits
*/
                                                                                                                                                                                            
int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <Server IP>\n", argv[0]);
        exit(1);
    }

    // Get the destination IP address
    in_addr_t dest_addr = inet_addr(argv[1]);

    // Open a raw socket to capture all the packets till Ethernet (use ETH_P_ALL)
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket() failed");
        exit(1);
    }

    int msg_num = 0; // Message number

    // Query Table
    typedef struct querytable {
        int valid;
        int retry;
        char domain[8][32];
        char query[BUF_SIZE];
    } querytable;

    querytable query_table[MAX_SIZE];

    // Initialize the query table
    for (int i = 0; i < MAX_SIZE; i++) {
        query_table[i].valid = 0;
        query_table[i].retry = 0;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(0, &master);
    FD_SET(sockfd, &master);
    int maxfd = sockfd;
    struct timeval timer;
    timer.tv_sec = timeout;
    timer.tv_usec = 0;

    while (1) {
        fd_set readfds = master;

        int result = select(maxfd + 1, &readfds, NULL, NULL, &timer);
        if (result < 0) {
            perror("select() failed");
            exit(1);
        }
        if (result == 0) {
            for (int i = 0; i < MAX_SIZE; i++) {
                if (query_table[i].valid == 1) {
                    if (query_table[i].retry < 3) {

                        // Open a raw socket to send the packets
                        int sendsockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
                        if (sendsockfd < 0) {
                            perror("repeat send socket() failed");
                            exit(1);
                        }

                        // Set the IP_HDRINCL option
                        int one = 1;
                        if (setsockopt(sendsockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
                            perror("repeat setsockopt() failed");
                            exit(1);
                        }

                        struct iphdr *ip = (struct iphdr *)query_table[i].query;

                        // Send the query packet
                        struct sockaddr_in dest;
                        dest.sin_family = AF_INET;
                        dest.sin_port = htons(10000);
                        dest.sin_addr.s_addr = ip->daddr;

                        if (sendto(sendsockfd, query_table[i].query, ip->tot_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
                            perror("sendto() failed");
                            exit(1);
                        }
                        close(sendsockfd);
                        query_table[i].retry++;
                    } else {
                        // Delete the ID from pending query table
                        query_table[i].valid = 0;
                        printf("\nNo response for Query ID %d detected even after 3 tries!\n", i);
                    }
                }
            }

            timer.tv_sec = timeout;
            timer.tv_usec = 0;
            continue;
        }
        if (FD_ISSET(0, &readfds)) {

            // Get query string from user
            char query[100];
            fgets(query, 100, stdin);

            if (strncmp(query, "EXIT", 4) == 0) {
                break;
            }

            char *word = strtok(query, " ");
            if (strcmp(word, "getIP") != 0) {
                printf("\nError: Start with 'getIP'!\n");
                continue;
            }

            word = strtok(NULL, " ");
            int n = atoi(word);
            if (n > 8) {
                printf("\nError: Number of domains should be less than 8!\n");
                continue;
            }
            if (n <= 0) {
                printf("\nError: Number of domains should be greater than 0!\n");
                continue;
            }

            char domain[n][32];
            int error_val = 0;
            int cnt = 0;
            while ((word = strtok(NULL, " ")) != NULL) {
                cnt++;
                if (cnt > n) {
                    printf("\nError: Number of domains doesn't match domains entered!\n");
                    error_val = 1;
                    break;
                }
                int len = strlen(word);
                if (cnt == n) {
                    len--;
                }
                if (len > 31) {
                    printf("\nError: Domain name should be less than 32 characters!\n");
                    error_val = 1;
                    break;
                }
                if (len < 3) {
                    printf("\nError: Domain name should be more than 2 characters!\n");
                    error_val = 1;
                    break;
                }
                int hyphen = 0;
                for (int i = 0; i < len; i++) {
                    if (word[i] == '-') {
                        if (i == 0 || i == len - 1) {
                            printf("\nError: Domain name should not start or end with '-'!\n");
                            error_val = 1;
                            break;
                        } else if (hyphen) {
                            printf("\nError: Domain name should not contain consecutive '-'!\n");
                            error_val = 1;
                            break;
                        }
                        hyphen = 1;
                    } else {
                        hyphen = 0;
                        if (!((word[i] >= 'a' && word[i] <= 'z') || (word[i] >= 'A' && word[i] <= 'Z') || (word[i] >= '0' && word[i] <= '9') || word[i] == '.')) {
                            printf("\nError: Domain name should contain only alphanumeric characters and '.'!\n");
                            error_val = 1;
                            break;
                        }
                    }
                }
                strcpy(domain[cnt - 1], word);
            }
            if (error_val) {
                continue;
            }
            if (cnt != n) {
                printf("\nError: Number of domains doesn't match domains entered!\n");
                continue;
            }

            // Create the query packet
            char message[BUF_SIZE];

            char *messagePtr = message;

            // Set the ID (16 bits)
            *(unsigned short *)messagePtr = msg_num;
            query_table[msg_num].valid = 1;
            messagePtr += 2;

            // Set the message type (1 bit)
            *messagePtr = 0;

            // Set the number of queries (3 bits)
            *messagePtr |= (((n - 1) << 4) & 0xf0);

            // Store the domain names
            for (int i = 0; i < n; i++) {
                // Set the size of the domain name
                int domainSize = strlen(domain[i]);
                if (i == n - 1) {
                    domainSize--; // Remove the newline character
                }

                *messagePtr |= (domainSize >> 28) & 0x0f;
                *(messagePtr + 1) = (domainSize >> 20) & 0xff;
                *(messagePtr + 2) = (domainSize >> 12) & 0xff;
                *(messagePtr + 3) = (domainSize >> 4) & 0xff;
                *(messagePtr + 4) = ((domainSize << 4) & 0xf0);
                messagePtr += 4;

                // Set the domain name
                for (int j = 0; j < domainSize; j++) {
                    *messagePtr |= (domain[i][j] >> 4) & 0x0f;
                    *(messagePtr + 1) = (domain[i][j] << 4) & 0xf0;
                    messagePtr += 1;
                }

                // Store domain name in the query table
                domain[i][domainSize] = '\0';
                strncpy(query_table[msg_num].domain[i], domain[i], domainSize);
            }

            // Set the IP header
            char buffer[BUF_SIZE];
            memset(buffer, 0, sizeof(buffer));
            struct iphdr *ip = (struct iphdr *)buffer;

            ip->ihl = 5;
            ip->version = 4;
            ip->tos = 0;
            ip->tot_len = sizeof(struct iphdr) + (messagePtr - message + 1);
            ip->id = htons(12345);
            ip->ttl = 64;
            ip->protocol = 254;
            ip->check = 0;
            ip->saddr = INADDR_ANY;
            ip->daddr = dest_addr;

            // Add the query packet to the buffer
            memcpy(buffer + sizeof(struct iphdr), message, messagePtr - message + 1);

            // Open a raw socket to send the packets
            int sendsockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
            if (sendsockfd < 0) {
                perror("send socket() failed");
                exit(1);
            }

            // Set the IP_HDRINCL option
            int one = 1;
            if (setsockopt(sendsockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
                perror("setsockopt() failed");
                exit(1);
            }

            // Send the query packet
            struct sockaddr_in dest;
            dest.sin_family = AF_INET;
            dest.sin_port = htons(10000);
            dest.sin_addr.s_addr = ip->daddr;

            if (sendto(sendsockfd, buffer, ip->tot_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
                perror("sendto() failed");
                exit(1);
            }

            // Store the buffer in the query table
            memcpy(query_table[msg_num].query, buffer, ip->tot_len);

            close(sendsockfd);
            // Increment the message number
            msg_num = (msg_num + 1) % MAX_SIZE;
        }
        if (FD_ISSET(sockfd, &readfds)) {
            // Receive the response packet
            char buffer[BUF_SIZE];
            int len = recv(sockfd, buffer, BUF_SIZE, 0);
            if (len < 0) {
                perror("recv() failed");
                exit(1);
            }

            // Extract the IP header from the received packet
            struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));

            // Check the protocol field and drop if not 254
            if (ip->protocol != 254) {
                continue;
            }

            // Check the sender IP address and drop if not the client
            if (ip->saddr != dest_addr) {
                continue;
            }

            // Read the response header
            char *responsePtr = buffer + sizeof(struct ethhdr) + sizeof(struct iphdr);

            // Get ID(16 bits==unsigned short)
            unsigned short id = *(unsigned short *)responsePtr;
            responsePtr += 2;

            // Check if query ID is present in the query table
            if (query_table[id].valid == 0) {
                continue;
            }

            // Check if it is simDNS response
            // Get Message type(1 bit)
            unsigned short messageType = (*(responsePtr) & 0x80) >> 7;

            if (messageType != 1) { // Not a response
                continue;
            }

            printf("\nQuery ID: %d\n", id);

            // Get the number of responses(3 bits)
            unsigned short numResponses = (*(responsePtr) & 0x70) >> 4;
            numResponses++; // Num starts from 1 so 000 means 1 and so on...
            printf("Total query strings: %d\n", numResponses);

            // Get the response strings
            int bit_pos = 3;
            for (int i = 0; i < numResponses; i++) {
                printf("<%s> ", query_table[id].domain[i]);
                // Check if the response is valid
                int valid = (*(responsePtr) & (1 << bit_pos)) >> bit_pos;
                bit_pos = (bit_pos + 7) % 8;
                if (bit_pos == 7) {
                    responsePtr++;
                }
                if (valid == 0) {
                    printf("NO IP ADDRESS FOUND\n");
                    responsePtr += 4;
                    continue;
                }

                // Get the IP address
                unsigned int ip = 0;
                for (int j = 0; j < 32; j++) {
                    ip |= ((*(responsePtr) & (1 << bit_pos)) >> bit_pos) << (31 - j);
                    bit_pos = (bit_pos + 7) % 8;
                    if (bit_pos == 7) {
                        responsePtr++;
                    }
                }
                printf("%d.%d.%d.%d\n", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
            }

            // Delete the ID from pending query table
            query_table[id].valid = 0;

            // reset the timer
            timer.tv_sec = timeout;
            timer.tv_usec = 0;
        }
    }
    close(sockfd);
    return 0;
}