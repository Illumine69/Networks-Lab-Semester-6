/*
Name: Sanskar Mittal, Karthik Reddy
Roll number: 21CS10057, 21CS30058
Assignment 6: Implementing a Custom Protocol using Raw Sockets
File: simDNSServer.c
*/

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 1000
const float p = 0.2;

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

int dropmessage(float prob) {
    srand((unsigned int)time(NULL));
    float random = (float)rand() / (float)RAND_MAX;
    if (random < prob) {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <Client IP>\n", argv[0]);
        exit(1);
    }

    in_addr_t client_ip = inet_addr(argv[1]);

    // Open a raw socket to capture all the packets till Ethernet (use ETH_P_ALL)
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket() failed");
        exit(1);
    }

    // Bind the raw socket
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(struct sockaddr_ll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = 0; // supports all interfaces

    if (bind(sockfd, (struct sockaddr *)&sll, sizeof(struct sockaddr_ll)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    // Receive packets on the raw socket
    char buffer[BUF_SIZE];
    while (1) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int len = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&client, &client_len);
        if (len < 0) {
            perror("recv() failed");
            exit(1);
        }

        // Drop Message
        if (dropmessage(p)) {
            continue;
        }

        // Extract the IP header from the received packet
        struct iphdr *cli_ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));

        // Check the protocol field and drop if not 254
        if (cli_ip->protocol != 254) {
            continue;
        }

        // Only allow specied client IP
        if (client_ip != cli_ip->saddr) {
            continue;
        }


        // Read the query header
        char *query = buffer + sizeof(struct ethhdr) + sizeof(struct iphdr);

        // Check if it is simDNS query
        // Get ID(16 bits==unsigned short)
        unsigned short id = *(unsigned short *)query;
        query += 2; // Move to message type byte

        // Get Message type(1 bit)
        unsigned short messageType = (*(query) & 0x80) >> 7;

        if (messageType != 0) { // Not a query
            continue;
        }

        // Get Number of Queries(3 bits)
        unsigned short numQueries = (*(query) & 0x70) >> 4;
        numQueries++; // Num starts from 1 so 000 means 1 and so on...

        // Get Query Strings (first 4 bytes is domain name size and then domain name)
        char *domain[numQueries];
        for (int i = 0; i < numQueries; i++) {
            // Get the size of the domain name
            int domainSize = ((*query & 0x0f) << 28) | ((*(query + 1)) << 20) | (*(query + 2) << 12) | ((*(query + 3) << 4)) | ((*(query + 4) & 0xf0) >> 4);
            
            // Get the domain name
            domain[i] = (char *)malloc((domainSize + 1) * sizeof(char));
            for (int j = 0; j < domainSize; j++) {
                domain[i][j] = ((*(query + 4 + j) & 0x0f) << 4) | ((*(query + 5 + j) & 0xf0) >> 4);
            }
            domain[domainSize] = '\0';

            query = query + 4 + domainSize;
        }

        // Generate the simDNS response
        char response[BUF_SIZE];
        char *responsePtr = response;

        // Set the ID (16 bits)
        *(unsigned short *)responsePtr = id;
        responsePtr += 2;

        // Set the message type (1 bit)
        *responsePtr = 1 << 7;

        // Set the number of responses (3 bits)
        *responsePtr |= (numQueries - 1) << 4;

        // Set the response strings
        int bit_pos = 3;
        for (int i = 0; i < numQueries; i++) {
            // Get IP address of the domain
            struct hostent *host_info = gethostbyname(domain[i]);
            if (host_info == NULL) {
                *(responsePtr) &= ~(1 << bit_pos);
                bit_pos = (bit_pos + 7) % 8;
                if (bit_pos == 7) {
                    responsePtr++;
                }
                responsePtr += 4;
                continue;
            }

            *(responsePtr) |= 1 << bit_pos;
            bit_pos = (bit_pos + 7) % 8;
            if (bit_pos == 7) {
                responsePtr++;
            }
            struct in_addr **p = (struct in_addr **)host_info->h_addr_list;
            uint32_t addr = ntohl(p[0]->s_addr);
            for (int j = 0; j < 32; j++) {
                int bit = (addr & (1 << (31 - j))) >> (31 - j);
                if (bit == 0) {
                    *(responsePtr) &= ~(1 << bit_pos);
                } else {
                    *(responsePtr) |= (1 << bit_pos);
                }
                bit_pos = (bit_pos + 7) % 8;
                if (bit_pos == 7) {
                    responsePtr++;
                }
            }
        }

        // set the IP header
        char buffer[BUF_SIZE];
        memset(buffer, 0, sizeof(buffer));
        struct iphdr *ip = (struct iphdr *)buffer;

        ip->ihl = 5;
        ip->version = 4;
        ip->tos = 0;
        ip->tot_len = sizeof(struct iphdr) + (responsePtr - response + 1);
        ip->id = htons(12345);
        ip->ttl = 64;
        ip->protocol = 254;
        ip->check = 0;
        ip->saddr = INADDR_ANY;
        ip->daddr = client_ip;

        // Add the response to the buffer
        memcpy(buffer + sizeof(struct iphdr), response, responsePtr - response + 1);

        // Open a new socket to send the response
        int respsockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
        if (respsockfd < 0) {
            perror("response socket() failed");
            exit(1);
        }

        // Set the IP_HDRINCL option
        int one = 1;
        if (setsockopt(respsockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
            perror("setsockopt() failed");
            exit(1);
        }

        // Set the destination address
        struct sockaddr_in dest;
        dest.sin_family = AF_INET;
        dest.sin_port = client.sin_port;
        dest.sin_addr.s_addr = client.sin_addr.s_addr;

        // Send the response to client
        if (sendto(respsockfd, buffer, ip->tot_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
            perror("sendto() failed");
            exit(1);
        }

        for(int i = 0; i < numQueries; i++) {
            free(domain[i]);
        }
        close(respsockfd);
    }
}