// Name: Sanskar Mittal
// Roll No: 21CS10057

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int donate_main = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(40000);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(donate_main, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error in binding!");
        exit(EXIT_FAILURE);
    }

    // Donation Table
    struct donation_table_ {
        int donation;
        char charity_name[200];
    } donation_table[20];

    for (int i = 0; i < 20; i++) {
        donation_table[i].donation = -1;
        memset(donation_table[i].charity_name, '\0', 200);
    }

    listen(donate_main, 7);

    int clients[7];
    for (int i = 0; i < 7; i++) {
        clients[i] = -1;
    }

    printf("\n\nChoose:\nAdd charity name(Press 1), Delete Charity(Press 2), Display Charities(Press 3)->\n");
    int done = 0;

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(donate_main, &readfds);
        int maxfd = donate_main;
        for (int i = 0; i < 7; i++) {
            if (clients[i] != -1) {
                FD_SET(clients[i], &readfds);
                if (clients[i] > maxfd) {
                    maxfd = clients[i];
                }
            }
        }
        int ret = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("Error in select call!");
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(0, &readfds)) {

            char ans[10];
            fgets(ans, 2, stdin);
            int response = ans[0] - '0';
            if (response == 1) {
                printf("Charity Name: ");
                for (int i = 0; i < 20; i++) {
                    if (donation_table[i].donation == -1) {
                        donation_table[i].donation = 0;
                        scanf("%s", donation_table[i].charity_name);
                        break;
                    }
                }
                done = 1;
            } else if (response == 2) {
                printf("Enter charity number to delete: ");
                int charity_num;
                scanf("%d", &charity_num);
                if (charity_num < 1 || charity_num > 20) {
                    printf("Wrong Charity Num!\n");
                } else {
                    if (donation_table[charity_num - 1].donation == -1) {
                        printf("No such charity exists!\n");
                    } else {
                        donation_table[charity_num - 1].donation = -1;
                        memset(donation_table[charity_num - 1].charity_name, '\0', 200);
                    }
                }
                done = 1;
            } else if (response == 3) {
                printf("\t\tCharities\n");
                for (int i = 0; i < 20; i++) {
                    if (donation_table[i].donation != -1) {
                        printf("%d: Charity: %s, Donation: %d\n", i + 1, donation_table[i].charity_name, donation_table[i].donation);
                    }
                }
                done = 1;
            } else {
                done = 1;
            }
            if (done) {
                printf("\n\nChoose:\nAdd charity name(Press 1), Delete Charity(Press 2), Display Charities(Press 3)->\n");
                done = 0;
            }
        }
        if (FD_ISSET(donate_main, &readfds)) {
            struct sockaddr_in cli_addr;
            socklen_t sock_len;
            int clientsockfd = accept(donate_main, (struct sockaddr *)&cli_addr, &sock_len);
            for (int i = 0; i < 7; i++) {
                if (clients[i] == -1) {
                    clients[i] = clientsockfd;
                    char message[10000];
                    int msg_len = 0;
                    for (int k = 0; k < 20; k++) {
                        if (donation_table[k].donation != -1) {
                            for (int j = 0; j < 200; j++) {
                                message[msg_len++] = donation_table[k].charity_name[j];
                                if (donation_table[k].charity_name[j] == '\0') {
                                    break;
                                }
                            }
                        }
                    }
                    message[msg_len] = '\0';
                    send(clients[i], message, msg_len, 0);
                    // printf("Mesg sent!");
                    break;
                }
            }
        }
        for (int i = 0; i < 7; i++) {
            if (clients[i] != -1) {
                if (FD_ISSET(clients[i], &readfds)) {
                    char message[10000];
                    // int msg_len = 0;
                    // for(int k=0;k<20;k++){
                    //     if(donation_table[k].donation != -1){
                    //         for(int j=0;j<200;j++){
                    //             message[msg_len++] = donation_table[k].charity_name[j];
                    //             if(donation_table[k].charity_name[j] == '\0'){
                    //                 break;
                    //             }
                    //         }
                    //     }
                    // }
                    // message[msg_len] = '\0';
                    // send(clients[i],message,msg_len,0);

                    memset(message, '\0', 10000);
                    if (recv(clients[i], message, 10000, 0) == 0) {
                        close(clients[i]);
                        clients[i] = -1;
                        break;
                    }
                    int donated = 0;
                    for (int j = 0; j < 20; j++) {
                        if (strcmp(donation_table[j].charity_name, message) == 0) {
                            donation_table[j].donation += 100;
                            donated = 1;
                            break;
                        }
                    }
                    if (donated) {
                        char response[100] = "Donation made and appreciated";
                        send(clients[i], response, 30, 0);
                    } else {
                        char response[100] = "Name does not exist";
                        send(clients[i], response, 20, 0);
                    }
                    if (recv(clients[i], message, 10000, 0) == 0) {
                        close(clients[i]);
                        clients[i] = -1;
                    }
                }
            }
        }
    }
}