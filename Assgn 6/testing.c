#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

int main() {
    char buffer[10];
    // convert bit to chars
    char check = 0b10010000;
    buffer[0] = check;
    check = 0b00000000;
    buffer[1] = check;
    buffer[2] = check;
    buffer[3] = check;
    check = 0b10010001;
    buffer[4] = check;

    char *query = buffer;

    unsigned short type = (*query & 0x80) >> 7;
    printf("type: %d\n", type);
    printf("BUFFER[0] = %d\n", buffer[0]);
    query++;
    printf("query: %d\n", *(unsigned short *)query);
    query--;
    int size = ((*query & 0x0f) << 28) | ((*(query + 1)) << 20) | (*(query + 2) << 12) | ((*(query + 3) << 4)) | ((*(query + 4) & 0xf0) >> 4);
    printf("Size: %d\n", size);

    // printf("buffer: %s\n", buffer);
    char hostname[100];
    int len;
    char *host = "www.google.com";
    printf("Size of host: %d\n", strlen(host));
    char* domain = (char *)malloc((strlen(host) + 1) * sizeof(char));
    strcpy(domain, host);
    printf("Domain: %s\n", domain);
    struct hostent *host_info = gethostbyname(domain);
    struct in_addr **p = NULL;
    p = (struct in_addr **)host_info->h_addr_list;
    inet_ntop(AF_INET, p[0], hostname, 100);
    printf("Hostname: %s\n", hostname);

    // get the hostname in 32 bit binary
    uint32_t addr = ntohl(p[0]->s_addr);
    printf("Address: %08x\n", addr);
    for (int i = 0; i < 32; i++) {
        printf("%d", (addr & (1 << (31 - i))) >> (31 - i));
        if (i % 8 == 7) {
            printf(" ");
        }
    }
    // printf("\n%d\n", ~2);
    // struct in_addr addr_t;
    // const char *ip = "192.168.1.1";

    // // Convert IP address string to binary representation
    // if (inet_pton(AF_INET, ip, &addr_t) != 1) {
    //     perror("Error converting IP address");
    //     return 1;
    // }

    // // Access the binary representation directly
    // printf("\nBinary representation: %08x\n", ntohl(addr_t.s_addr));
    // char ip_addr[32];
    // inet_ntop(AF_INET, &addr_t, ip_addr, 32);
    // printf("IP Address: %s\n", ip_addr);
}