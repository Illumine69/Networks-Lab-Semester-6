#include <arpa/inet.h>
#include <errno.h>
#include <msocket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/select.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/types.h>

// create a mutex (semaphore )for the whole share memory

struct shared_memory *SM;

char send_buffer[1500], recv_buffer[1500];

void update_Receive_Window_Size(int i, struct shared_memory *SM) {
    if (SM[i].rwnd.start_index == (SM[i].rwnd.last_inorder_msg + 1) % RECV_BUFFER_SIZE) {
        if (SM[i].rwnd.recv_msg[SM[i].rwnd.start_index] == 0) {
            SM[i].rwnd.receive_window_size = RECV_BUFFER_SIZE;
            return;
        }
    }
    printf("here1\n");
    int win_size = 0;
    int j = (SM[i].rwnd.last_inorder_msg + 1) % RECV_BUFFER_SIZE;
    while (j != SM[i].rwnd.start_index) {
        win_size++;
        if (SM[i].rwnd.recv_msg[j] == 1) {
            win_size = 0;
        }
        j = (j + 1) % RECV_BUFFER_SIZE;
    }
    SM[i].rwnd.receive_window_size = win_size;
}

void update_Receiver_Last_Inorder_Msg(int i, struct shared_memory *SM) {
    int j = SM[i].rwnd.last_inorder_msg;
    j = (j + 1) % RECV_BUFFER_SIZE;
    printf("Problem here\n");
    while (SM[i].rwnd.recv_msg[j] == 1 && j != SM[i].rwnd.start_index) {
        printf("J: %d\n", j);
        j = (j + 1) % RECV_BUFFER_SIZE;
    }
    fflush(stdout);
    SM[i].rwnd.last_inorder_msg = (j - 1 + RECV_BUFFER_SIZE) % RECV_BUFFER_SIZE;
}

void *R(void *params) {
    key_t key = KEY;
    int shmid = shmget(key, N * sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, 0, 0);

    key_t sem_sm_key = SEM_SM_KEY;
    int sem_sm = semget(sem_sm_key, 1, 0777);

    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;

    // receiver process
    // should use select call with timer
    fd_set master;
    FD_ZERO(&master);
    int max_fd = -1;

    // set all the file descriptors in the master set
    for (int i = 0; i < N; i++) {
        if (SM[i].free == 0) {
            FD_SET(SM[i].sockfd, &master);
            if (SM[i].sockfd > max_fd) {
                max_fd = SM[i].sockfd;
            }
        }
    }

    struct timeval timeout;
    timeout.tv_sec = T;
    timeout.tv_usec = 0;

    while (1) {
        P(sem_sm);
        // printf("Receiver\n");
        fd_set readfd;
        FD_ZERO(&readfd);
        for (int i = 0; i < N; i++) {
            if (SM[i].free == 0) {
                FD_SET(SM[i].sockfd, &readfd);
                printf("Socket setting here: %d\n", SM[i].sockfd);
                if (SM[i].sockfd > max_fd) {
                    max_fd = SM[i].sockfd;
                }
            }
        }
        timeout.tv_sec = T;
        timeout.tv_usec = 0;
        int ret = select(max_fd + 1, &readfd, NULL, NULL, &timeout);
        // printf("Max_fd: %d", max_fd);
        if (ret < 0) {
            perror("Error in select");
            V(sem_sm);
            continue;
        } else if (ret == 0) {
            // handle timeout
            //  include implementation when timeout occurs with non zero
            //  return value
            printf("Timedout\n");
            // Check if any new socket has been added
            FD_ZERO(&master);
            max_fd = 0;
            for (int i = 0; i < N; i++) {
                if (SM[i].free == 0) {
                    FD_SET(SM[i].sockfd, &master);
                    printf("Socket setting: %d\n", SM[i].sockfd);
                    if (SM[i].sockfd > max_fd) {
                        max_fd = SM[i].sockfd;
                    }
                }
            }

            // IF nospace flag is set and rwnd is not zero
            // Send ACK with the last in-order message and rwnd size
            // Reset the flag
            for (int i = 0; i < N; i++) {
                if (SM[i].rwnd.nospace == 1 && SM[i].rwnd.receive_window_size != 0) {
                    // Send duplicate ACK
                    
                    SM[i].rwnd.nospace = 0;
                }
            }
        } else {
            printf("Entered loop\n");
            for (int i = 0; i < N; i++) {
                printf("Sockfd: %d, isset=%d\n", SM[i].sockfd, FD_ISSET(SM[i].sockfd, &readfd));
                if (SM[i].free == 0 && FD_ISSET(SM[i].sockfd, &readfd)) {
                    printf("Socket: %d\n", SM[i].sockfd);
                    // handle the message
                    struct sockaddr_in sender_addr;
                    socklen_t sender_len;
                    int n = recvfrom(SM[i].sockfd, recv_buffer, 1500, 0, (struct sockaddr *)&sender_addr, &sender_len);
                    // dropMessage function
                    // if(dropMessage(p) == 1){
                    //     break;
                    // }
                    printf("Message From: %s\n", inet_ntoa(sender_addr.sin_addr));
                    printf("Port: %d\n", ntohs(sender_addr.sin_port));
                    if (n < 0) {
                        perror("Error in recvfrom");
                        continue;
                    }
                    // check if sender address is same as the address in the shared memory
                    if (sender_addr.sin_addr.s_addr != SM[i].addr.sin_addr.s_addr || sender_addr.sin_port != SM[i].addr.sin_port) {
                        // do some error handling here
                        perror("Error in sender address in RECEIVER");
                        continue;
                    }
                    printf("Message in buffer: ");
                    for (int j = 0; j < n; j++) {
                        printf("%c", recv_buffer[j]);
                    }
                    // check if the message is an ack or data
                    char main_buffer[1500];
                    for(int j=0;j<n;j++){
                        main_buffer[j] = recv_buffer[j];
                    }
                    int type = atoi(strtok(recv_buffer, "$"));
                    char *sender_ip = strtok(NULL, "$");
                    int sender_port = atoi(strtok(NULL, "$"));
                    int seq_no = atoi(strtok(NULL, "$"));
                    int size = atoi(strtok(NULL, "$"));
                    char *message;
                    message = (char *)malloc(1500);
                    printf("Type: %d\n", type);
                    printf("Sender IP: %s\n", sender_ip);
                    printf("Sender Port: %d\n", sender_port);
                    printf("Seq No: %d\n", seq_no);
                    printf("Size: %d\n", size);
                    int k = 0, msg_start = 0;
                    for (int j = 0; j < n; j++) {
                        printf("main_buffer[j]: %c\n", main_buffer[j]);
                        if (main_buffer[j] == '$') {
                            k++;
                            printf("k: %d\n", k);
                        }
                        if (k == 5) {
                            if (msg_start == 0) {
                                j++;
                            }
                            printf("recv: %c\n",main_buffer[j]);
                            message[msg_start++] = main_buffer[j];
                            printf("%c", message[msg_start - 1]);
                        }
                    }

                    if (type == 0) { // data message
                        //  If message is in-order:
                        printf("seq_no: %d", seq_no);
                        printf("Last Inorder Msg Seq No: %d\n", SM[i].rwnd.last_inorder_msg_seq_num);
                        if (seq_no == ((SM[i].rwnd.last_inorder_msg_seq_num) % MAX_SEQ_NUM + 1)) {
                            // write the message to the buffer after removing mtp header
                            SM[i].rwnd.last_inorder_msg = (SM[i].rwnd.last_inorder_msg + 1) % RECV_BUFFER_SIZE;
                            SM[i].rwnd.last_inorder_msg_seq_num = seq_no;
                            printf("Message checking: ");
                            for (int j = 0; j < size; j++) {
                                SM[i].recv_buffer[SM[i].rwnd.last_inorder_msg][j] = message[j];
                                printf("%c", SM[i].recv_buffer[SM[i].rwnd.last_inorder_msg][j]);
                            }
                            printf("\n");
                            // strncpy(SM[i].recv_buffer[SM[i].rwnd.last_inorder_msg], message, size);
                            // printf("Message: %s\n", SM[i].recv_buffer[SM[i].rwnd.last_inorder_msg]);
                            printf("Last Inorder Msg: %d\n", SM[i].rwnd.last_inorder_msg);
                            SM[i].rwnd.recv_msg[SM[i].rwnd.last_inorder_msg] = 1;
                            SM[i].rwnd.msg_size[SM[i].rwnd.last_inorder_msg] = size;
                            int old_last_msg = SM[i].rwnd.last_inorder_msg;
                            update_Receiver_Last_Inorder_Msg(i, SM);
                            printf("Here I am\n");
                            // rwnd size is changed
                            update_Receive_Window_Size(i, SM);
                            // send ACK with the new rwnd size and this in-order message
                            char *ack = (char *)malloc(1500);
                            int inc_msg_seq = (SM[i].rwnd.last_inorder_msg - old_last_msg + RECV_BUFFER_SIZE) % RECV_BUFFER_SIZE;
                            SM[i].rwnd.last_inorder_msg_seq_num = (SM[i].rwnd.last_inorder_msg_seq_num + inc_msg_seq - 1 + MAX_SEQ_NUM) % MAX_SEQ_NUM + 1;
                            char *rwnd_size = (char *)malloc(1000);
                            sprintf(rwnd_size, "%d", SM[i].rwnd.receive_window_size);
                            int size_len = strlen(rwnd_size);
                            sprintf(ack, "1$%s$%d$%d$%d$", inet_ntoa(SM[i].addr.sin_addr), ntohs(SM[i].addr.sin_port), SM[i].rwnd.last_inorder_msg_seq_num, size_len);
                            // sprintf(ack, "%d$", size_len);
                            int len = strlen(ack);
                            printf("Len: %d", len);
                            for (int k = 0; k < size_len; k++) {
                                ack[len + k] = rwnd_size[k];
                            }
                            printf("Ack: %s\n", ack);
                            if (sendto(SM[i].sockfd, ack, len + size_len, 0, (struct sockaddr *)&SM[i].addr, sizeof(struct sockaddr_in)) < 0) {
                                // do some error handling here
                                free(ack);
                                free(rwnd_size);
                                perror("Error in thread while attempting to send ACK to the socket");
                                continue;
                            }
                            free(ack);
                            free(rwnd_size);
                        } else { // out-of-order message
                            int j = seq_no - SM[i].rwnd.last_inorder_msg_seq_num + SM[i].rwnd.last_inorder_msg;
                            // TODO: If message is in rwnd AND
                            // If message is not a duplicate:
                            char *ack = (char *)malloc(1500);
                            char *rwnd_size = (char *)malloc(1000);
                            if (j >= 0 && j < RECV_BUFFER_SIZE && SM[i].rwnd.recv_msg[j] == 0) {
                                // write the message to the buffer after removing mtp header
                                for (int k = 0; k < size; k++) {
                                    SM[i].recv_buffer[j][k] = message[k];
                                }
                                SM[i].rwnd.recv_msg[j] = 1;
                                SM[i].rwnd.msg_size[j] = size;
                                // TODO: Update window size
                            }
                            // SEND the ACK
                            update_Receive_Window_Size(i, SM);
                            sprintf(rwnd_size, "%d", SM[i].rwnd.receive_window_size);
                            int size_len = strlen(rwnd_size);
                            sprintf(ack, "1$%s$%d$%d$%d$", inet_ntoa(SM[i].addr.sin_addr), ntohs(SM[i].addr.sin_port), SM[i].rwnd.last_inorder_msg_seq_num, size_len);
                            // sprintf(ack, "%d$", size_len);
                            int len = strlen(ack);
                            for (int k = 0; k < size_len; k++) {
                                ack[len + k] = rwnd_size[k];
                            }
                            if (sendto(SM[i].sockfd, ack, len + size_len, 0, (struct sockaddr *)&SM[i].addr, sizeof(struct sockaddr_in)) < 0) {
                                // do some error handling here
                                free(ack);
                                free(rwnd_size);
                                perror("Error in thread while attempting to send ACK to the socket");
                                continue;
                            }
                            free(ack);
                            free(rwnd_size);
                        }
                    } else { // ACK message
                        //  If Ack is for a previous message:

                        //      Update swnd size
                        //      Remove all message before this ACK number from the send buffer
                        //  Else if ACK is duplicate:
                        //      Update swnd size

                        int ackno = seq_no;
                        int rwnd_size = atoi(message);
                        // check if it is a valid ack no
                        if ((ackno - SM[i].swnd.start_index_ack_no + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE <= SM[i].swnd.last_sent_index - SM[i].swnd.start_index) {
                            // valid ack no
                            // update start index ack no
                            // what would be the new start index
                            int new_start_index = (SM[i].swnd.start_index + (ackno - SM[i].swnd.start_index_ack_no + MAX_SEQ_NUM + 1) % (MAX_SEQ_NUM)) % SEND_BUFFER_SIZE;
                            SM[i].swnd.start_index_ack_no = ackno;
                            SM[i].swnd.rem_buff_space += (new_start_index - SM[i].swnd.start_index + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE;
                            SM[i].swnd.start_index = new_start_index;

                            SM[i].swnd.send_window_size = rwnd_size;

                            // reset the timer

                        } else {
                            // duplicate ack
                            SM[i].swnd.send_window_size = rwnd_size;
                        }
                    }

                    // If receiver buffer is full, set nospace flag
                    update_Receive_Window_Size(i, SM);
                    if (SM[i].rwnd.receive_window_size == 0) {
                        SM[i].rwnd.nospace = 1;
                    }
                }
            }
        }
        // printf("Receiver Leaving\n");
        V(sem_sm);
    }
}

void *S(void *params) {
    // sender process

    // should you implement a per SM semaphore or per index semaphore for SM?// choose the easy way
    key_t key = KEY;
    int shmid = shmget(key, N * sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, 0, 0);

    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;

    key_t sem_sm_key = SEM_SM_KEY;
    int sem_sm = semget(sem_sm_key, 1, 0777);

    while (1) {
        // sleep for <T/2 seconds
        sleep(T / 3);
        P(sem_sm);
        printf("Sender\n");
        for (int i = 0; i < N; i++) {
            // checking if timed out
            // printf("Here\n");
            printf("n:%d, SM[i].free:%d\n", i, SM[i].free);
            if (SM[i].free == 0) {
                // check if the message has been acked
                // if not resend the message
                // if yes check if the time has expired
                // if yes resend the message
                // if no do nothing
                // iterate from start_index to last_sent_index

                // j is basically circular
                printf("Sender here: %d\n", SM[i].swnd.start_index);
                // for (int j = SM[i].swnd.start_index; (j+SEND_BUFFER_SIZE)%(SEND_BUFFER_SIZE) <= SM[i].swnd.last_sent_index; j++)
                int j = (SM[i].swnd.start_index > -1) ? SM[i].swnd.start_index : 0;
                if (SM[i].swnd.unack_time[j] != -1 && (SM[i].swnd.unack_time[j] + T < time(NULL))) {
                    printf("spamskar\n");
                    while (j != (SM[i].swnd.last_sent_index + 1) % SEND_BUFFER_SIZE) {

                        // send all the messages from start_index to last_sent_index
                        // dont check the time
                        sprintf(send_buffer, "0$%s$%d$%d$%d$", inet_ntoa(SM[i].addr.sin_addr), ntohs(SM[i].addr.sin_port), (SM[i].swnd.start_index_ack_no + (j - SM[i].swnd.start_index + SEND_BUFFER_SIZE) % (SEND_BUFFER_SIZE)) % MAX_SEQ_NUM + 1, SM[i].swnd.length[j]);
                        // printf("Here\n");
                        int msglen = SM[i].swnd.length[j];

                        int len = strlen(send_buffer);
                        printf("\nMessage from S thread(unack): %s", send_buffer);
                        for (int k = 0; k < msglen; k++) {
                            send_buffer[len + k] = SM[i].send_buffer[(j - SM[i].swnd.start_index + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE][k];
                            printf("%c", send_buffer[len + k]);
                        }
                        if (sendto(SM[i].sockfd, send_buffer, len + msglen, 0, (struct sockaddr *)&SM[i].addr, sizeof(struct sockaddr_in)) < 0) {
                            // do some error handling here
                            // restore the index
                            // SM[i].swnd.last_sent_index = (SM[i].swnd.last_sent_index - 1 + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE;

                            // include the ack number in the message
                            perror("Error in thread while attempting to send to the socket");
                            break;
                        }
                        printf("Sender: Sending message again by socket: %d to addr: %s, port: %d\n", SM[i].sockfd, inet_ntoa(SM[i].addr.sin_addr), ntohs(SM[i].addr.sin_port));
                        SM[i].swnd.unack_time[j] = time(NULL);
                        j = (j + 1) % SEND_BUFFER_SIZE;
                    }
                }
            }
        }
        // next check if there are any new messages to send
        // should you ntohs ??

        for (int i = 0; i < N; i++) {
            if (SM[i].free == 0) {
                for (int j = 1; j <= SM[i].swnd.send_window_size; j++) {
                    if (((SM[i].swnd.last_sent_index + 1 - SM[i].swnd.start_index + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE) <= ((SM[i].swnd.end_index - SM[i].swnd.start_index + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE)) {
                        printf("Here\n");
                        SM[i].swnd.last_sent_index = (SM[i].swnd.last_sent_index + 1) % SEND_BUFFER_SIZE;
                        SM[i].swnd.last_sent_ack_no = (SM[i].swnd.last_sent_ack_no) % MAX_SEQ_NUM + 1;
                        int msglen = SM[i].swnd.length[SM[i].swnd.last_sent_index];
                        printf("Inet: %s\n", inet_ntoa(SM[i].addr.sin_addr));
                        printf("Port: %d\n", ntohs(SM[i].addr.sin_port));
                        printf("Ack: %d\n", (SM[i].swnd.last_sent_ack_no) % MAX_SEQ_NUM + 1);
                        printf("Msglen: %d\n", msglen);
                        sprintf(send_buffer, "0$%s$%d$%d$%d$", inet_ntoa(SM[i].addr.sin_addr), ntohs(SM[i].addr.sin_port), (SM[i].swnd.last_sent_ack_no) % MAX_SEQ_NUM + 1, msglen);

                        printf("\nMessage from S thread(sending): %s", send_buffer);
                        int len = strlen(send_buffer);
                        for (int k = 0; k < msglen; k++) {
                            send_buffer[len + k] = SM[i].send_buffer[SM[i].swnd.last_sent_index][k];
                            printf("%c", send_buffer[len + k]);
                        }

                        if (sendto(SM[i].sockfd, send_buffer, len + msglen, 0, (struct sockaddr *)&SM[i].addr, sizeof(struct sockaddr_in)) < 0) {
                            // do some error handling here
                            // restore the index
                            SM[i].swnd.last_sent_index = (SM[i].swnd.last_sent_index - 1 + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE;

                            // -2 used here to go back to previous ack
                            SM[i].swnd.last_sent_ack_no = (SM[i].swnd.last_sent_ack_no - 2 + MAX_SEQ_NUM) % MAX_SEQ_NUM + 1;

                            // include the ack number in the message
                            perror("Error in thread while attempting to send to the socket");
                            break;
                        }

                        SM[i].swnd.unack_time[SM[i].swnd.last_sent_index] = time(NULL);
                    } else {
                        break;
                    }
                }
                // make the send window size 0
                SM[i].swnd.send_window_size = 0;
            }
        }
        printf("Sender Leaving\n");
        V(sem_sm);
    }
}

void *G(void *params) {

    // garbage collector process
    int shmid;

    key_t key = KEY;
    shmid = shmget(key, N * sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, 0, 0);

    key_t sem_sm_key = SEM_SM_KEY;
    int sm_sem = semget(sem_sm_key, 1, 0777);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;

    while (1) {
        sleep(T / 2);
        P(sm_sem);
        printf("Garbage\n");
        int status;
        for (int i = 0; i < N; i++) {
            if (SM[i].free == 0) {
                int pid = SM[i].pid;
                status = kill(pid, 0);
                if (status < 0) {
                    // process is dead
                    // free the shared memory
                    SM[i].free = 1;
                    SM[i].pid = -1;
                    SM[i].swnd.send_window_size = 0;
                    SM[i].swnd.start_index = 0;
                    SM[i].swnd.last_sent_index = 0;
                    SM[i].swnd.end_index = 0;
                    SM[i].swnd.start_index_ack_no = 0;
                    SM[i].swnd.last_sent_ack_no = 0;
                    SM[i].swnd.rem_buff_space = SEND_BUFFER_SIZE;
                    for (int j = 0; j < 10; j++) {
                        SM[i].swnd.unack_time[j] = 0;
                    }
                }
            }
        }
        printf("Garbage Leaving\n");
        V(sm_sem);
    }
}

int main() {

    key_t sem_sm_key = SEM_SM_KEY;
    int sm_sem = semget(sem_sm_key, 1, 0777 | IPC_CREAT);
    // // initialize the semaphore to 1
    semctl(sm_sem, 0, SETVAL, 1); // initially unlocked like mutex
    struct sembuf pop, vop;
    key_t sem1_key = SEM1_KEY;
    key_t sem2_key = SEM2_KEY;
    int sem1 = semget(sem1_key, 1, 0777 | IPC_CREAT);
    int sem2 = semget(sem2_key, 1, 0777 | IPC_CREAT);
    // initilaize both semaphores to 0
    semctl(sem1, 0, SETVAL, 0);
    semctl(sem2, 0, SETVAL, 0);
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;
    key_t sock_info_key = SOCK_INFO_KEY;
    int sock_info = shmget(sock_info_key, sizeof(struct SOCKINFO), 0777 | IPC_CREAT);
    struct SOCKINFO *sockinfo = (struct SOCKINFO *)shmat(sock_info, 0, 0);
    sockinfo->sock_id = 0;
    sockinfo->error_no = 0;
    // sockinfo->addr = 0;

    int shmid;
    key_t key = KEY;
    shmid = shmget(key, N * sizeof(struct shared_memory), 0777 | IPC_CREAT);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, 0, 0);
    for (int i = 0; i < N; i++) {
        SM[i].free = 1;
        // SM[i].pid = -1;
        // SM[i].sockfd = -1;
        // SM[i].addr = NULL;
        // SM[i].swnd = (struct swnd){.send_window_size = 0, .rem_buff_space = SEND_BUFFER_SIZE, .start_index = 0, .last_sent_index = -1, .end_index = SEND_BUFFER_SIZE - 1, .start_index_ack_no = 0, .last_sent_ack_no = 0};
        // SM[i].rwnd = (struct rwnd){.receive_window_size = RECV_BUFFER_SIZE, .last_inorder_msg = 0, .nospace = 0};
    }
    printf("Hi\n");
    fflush(stdout);
    pthread_t rid, sid, gid;
    pthread_attr_t r_attr, s_attr, g_attr;
    pthread_attr_init(&r_attr);
    pthread_attr_init(&s_attr);
    pthread_attr_init(&g_attr);
    pthread_create(&rid, &r_attr, R, NULL);
    pthread_detach(rid);
    pthread_create(&sid, &s_attr, S, NULL);
    pthread_detach(sid);
    // pthread_create(&gid, &g_attr, G, NULL);
    // pthread_detach(gid);

    // need to attach?

    // while(1){   // Garbage collector process

    // }
    while (1) {
        P(sem1);
        // do stuff here
        printf("Main\n");

        if ((sockinfo->sock_id) == 0) {
            // call socket here

            sockinfo->sock_id = socket(AF_INET, SOCK_MTP, 0);
            if (sockinfo->sock_id < 0) {
                perror("Socket problem\n");
                sockinfo->error_no = errno;
                sockinfo->sock_id = -1;
            }
        } else {
            // do stuff here
            // call bind here
            // printf("Socket id: %d\n", sockinfo->sock_id);
            // printf("Port: %d\n", ntohs(sockinfo->addr.sin_port));
            // printf("Family: %d\n", sockinfo->addr.sin_family);
            // printf("Sock addr: %s\n", inet_ntoa(sockinfo->addr.sin_addr));
            // printf("IP: %d\n", sockinfo->addr.sin_addr.s_addr);
            fflush(stdout);
            struct sockaddr_in serv;
            socklen_t len = sizeof(serv);
            if (bind(sockinfo->sock_id, (struct sockaddr *)(&sockinfo->addr), len) < 0) {
                perror("Bind error here\n");
                sockinfo->error_no = errno;
                sockinfo->sock_id = -1;
            }
        }

        V(sem2); // release the calling process
    }

    // when do you detach the shared memories and semaphores?
    // when user presees control c right ? use signal handler??
}
