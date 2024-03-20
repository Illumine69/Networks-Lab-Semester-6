#include <errno.h>
#include <msocket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/select.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/types.h>

// create a mutex (semaphore )for the whole share memory

key_t sem_sm_key;

struct shared_memory *SM;

char send_buffer[1500],recv_buffer[1500];

void *R(void *params) {
    // receiver process
    // should use select call with timer
    fd_set master;
    FD_ZERO(&master);
    int max_fd = 0;

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
        fd_set readfd = master;
        int ret = select(max_fd + 1, &readfd, NULL, NULL, &timeout);
        if (ret < 0) {
            perror("Error in select");
            continue;
        } else if (ret == 0) {
            // handle timeout
            //  include implementation when timeout occurs with non zero
            //  return value

            // Check if any new socket has been added
            for(int i=0;i < N;i++){
                if(SM[i].free == 0){
                    FD_SET(SM[i].sockfd, &master);
                    if(SM[i].sockfd > max_fd){
                        max_fd = SM[i].sockfd;
                    }
                }
            }

            // IF nospace flag is set and rwnd is not zero
            // Send ACK with the last in-order message and rwnd size
            // Reset the flag

        } else {
            for (int i = 0; i < N; i++) {
                if (SM[i].free == 0 && FD_ISSET(SM[i].sockfd, &readfd)) {
                    // handle the message
                    struct sockaddr_in sender_addr;
                    int n = recvfrom(SM[i].sockfd, recv_buffer, 1500, 0, (struct sockaddr *)&sender_addr, sizeof(struct sockaddr_in));
                    if (n < 0) {
                        perror("Error in recvfrom");
                        continue;
                    }
                    // check if sender address is same as the address in the shared memory
                    if (sender_addr.sin_addr.s_addr != SM[i].addr->sin_addr.s_addr || sender_addr.sin_port != SM[i].addr->sin_port) {
                        // do some error handling here
                        perror("Error in sender address in RECEIVER");
                        continue;
                    }

                    // check if the message is an ack or data
                    if(recv_buffer[0] == '0'){  //data message
                        //  If message is in-order:
                        //      write the message to the buffer after removing mtp header
                        //      rwnd size is changed
                        //      send ACK with the new rwnd size and this in-order message
                        //  Else if message is out-of-order:
                        //      If message is in rwnd:
                        //          If message is not a duplicate:
                        //              write the message to the buffer after removing mtp header
                        //              send ACK with the new rwnd size and last in-order message
                        //          Else:
                        //              Drop the message
                        //              Send ACK of last in-order message and rwnd size
                    }
                    else{       //ACK message 
                        //  If Ack is for a previous message:
                        //      Update swnd size
                        //      Remove all message before this ACK number from the send buffer
                        //  Else if ACK is duplicate:
                        //      Update swnd size
                    }

                    // If receiver buffer is full, set nospcae flag

                }
            }
        }
    }
}

void *S(void *params) {
    // sender process

    // should you implement a per SM semaphore or per index semaphore for SM?// choose the easy way

    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;

    while (1) {
        // sleep for <T/2 seconds
        sleep(T / 3);
        P(sem_sm_key);
        for (int i = 0; i < N; i++) {
            // checking if timed out
            if (SM[i].free == 0) {
                // check if the message has been acked
                // if not resend the message
                // if yes check if the time has expired
                // if yes resend the message
                // if no do nothing
                // iterate from start_index to last_sent_index

                // j is basically circular
                // for (int j = SM[i].swnd.start_index; (j+SEND_BUFFER_SIZE)%(SEND_BUFFER_SIZE) <= SM[i].swnd.last_sent_index; j++)
                int j = SM[i].swnd.start_index;
                while (j != (SM[i].swnd.last_sent_index + 1) % SEND_BUFFER_SIZE) {
                    // if (SM[i].swnd.unack_time[j] + T < time(NULL))
                    // {
                    //     // resend the message
                    //     // send the message
                    //     // if the message is sent successfully update the unack_time
                    //     if (sendto(SM[i].sockfd, SM[i].send_buffer[j], sizeof(SM[i].send_buffer[j]), 0, (struct sockaddr *)SM[i].addr, sizeof(struct sockaddr_in)) < 0)
                    //     //you need to add the ack number in the message
                    //     {
                    //         // SM[i].swnd.unack_time[j] = -1;

                    //         // do some error handling here
                    //         perror("Error in thread while attempting to send to the socket");
                    //         continue;
                    //     }
                    //     SM[i].swnd.unack_time[j] = time(NULL);
                    // }
                    // send all the messages from start_index to last_sent_index
                    // dont check the time

                    sprintf(send_buffer, "0$%d$%d$%d$$$", SM[i].addr->sin_addr.s_addr, SM[i].addr->sin_port, (SM[i].swnd.start_index_ack_no + (j - SM[i].swnd.start_index + SEND_BUFFER_SIZE) % (SEND_BUFFER_SIZE)) % MAX_SEQ_NUM + 1);

                    int len = strlen(send_buffer);
                    for (int k = 0; k < 1000; k++) {
                        send_buffer[len + k] = SM[i].send_buffer[(j - SM[i].swnd.start_index + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE][k];
                    }

                    if (sendto(SM[i].sockfd, send_buffer, len + 1000, 0, (struct sockaddr *)SM[i].addr, sizeof(struct sockaddr_in)) < 0) {
                        // do some error handling here
                        // restore the index
                        // SM[i].swnd.last_sent_index = (SM[i].swnd.last_sent_index - 1 + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE;

                        // include the ack number in the message
                        perror("Error in thread while attempting to send to the socket");
                        break;
                    }
                    j = (j + 1) % SEND_BUFFER_SIZE;
                }
            }
        }
        // next check if there are any new messages to send

        // you need to check

        // need to add some headers  likie source adress and source port destionaion address and destination port not required??
        // also need to add the ack the number of the message
        // type of message ack or data

        // should you ntohs ??

        for (int i = 0; i < N; i++) {
            if (!(SM[i].free)) {
                for (int j = 1; j <= SM[i].swnd.send_window_size; j++) {
                    if (((SM[i].swnd.last_sent_index + 1 - SM[i].swnd.start_index + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE) <= ((SM[i].swnd.end_index - SM[i].swnd.start_index + SEND_BUFFER_SIZE) % SEND_BUFFER_SIZE)) {
                        SM[i].swnd.last_sent_index = (SM[i].swnd.last_sent_index + 1) % SEND_BUFFER_SIZE;
                        SM[i].swnd.last_sent_ack_no = (SM[i].swnd.last_sent_ack_no) % MAX_SEQ_NUM + 1;
                        sprintf(send_buffer, "0$%d$%d$%d$$$", SM[i].addr->sin_addr.s_addr, SM[i].addr->sin_port, (SM[i].swnd.last_sent_ack_no) % MAX_SEQ_NUM + 1);

                        int len = strlen(send_buffer);
                        for (int k = 0; k < 1000; k++) {
                            send_buffer[len + k] = SM[i].send_buffer[SM[i].swnd.last_sent_index][k];
                        }

                        if (sendto(SM[i].sockfd, send_buffer, len + 1000, 0, (struct sockaddr *)SM[i].addr, sizeof(struct sockaddr_in)) < 0) {
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

        V(sem_sm_key);
    }
}

void *G(void *params) {

    // garbage collector process
}

int main() {

    int sm_sem = semget(sem_sm_key, 1, 0777);
    // initialize the semaphore to 1
    semctl(sm_sem, 0, SETVAL, 1); // initially unlocked workd like mutex
    struct sembuf pop, vop;
    key_t sem1_key = SEM1_KEY;
    key_t sem2_key = SEM2_KEY;
    int sem1 = semget(sem1_key, 1, 0777);
    int sem2 = semget(sem2_key, 1, 0777);

    // initilaize both semaphores to 0
    semctl(sem1, 0, SETVAL, 0);
    semctl(sem2, 0, SETVAL, 0);
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;
    key_t sock_info_key = SOCK_INFO_KEY;
    int sock_info = shmget(sock_info_key, sizeof(struct SOCKINFO), 0777);
    struct SOCKINFO *sockinfo = (struct SOCKINFO *)shmat(sock_info, 0, 0);
    sockinfo->sock_id = 0;
    sockinfo->error_no = 0;
    sockinfo->addr = 0;

    int shmid;
    key_t key = KEY;
    shmid = shmget(key, N * sizeof(struct shared_memory), 0777);

    pthread_t rid, sid, gid;
    pthread_attr_t r_attr, s_attr, g_attr;
    pthread_attr_init(&r_attr);
    pthread_attr_init(&s_attr);
    pthread_attr_init(&g_attr);
    pthread_create(&rid, NULL, R, NULL);
    pthread_detach(rid);
    pthread_create(&sid, NULL, S, NULL);
    pthread_detach(sid);
    pthread_create(&gid, NULL, G, NULL);
    pthread_detach(gid);

    // need to attach?

    // while(1){   // Garbage collector process

    // }
    while (1) {
        P(sem1);
        // do stuff here

        if (!(sockinfo->sock_id)) {
            // call socket here

            sockinfo->sock_id = socket(AF_INET, SOCK_STREAM, 0);
            if (sockinfo->sock_id < 0) {
                sockinfo->error_no = errno;
                sockinfo->sock_id = -1;
            }
        } else {
            // do stuff here
            // call bind here
            if (bind(sockinfo->sock_id, (struct sockaddr *)sockinfo->addr, sizeof(struct sockaddr_in)) < 0) {
                sockinfo->error_no = errno;
                sockinfo->sock_id = -1;
            }
        }

        V(sem2); // release the calling process
    }

    // when do you detach the shared memories and semaphores?
    // when user presees control c right ? use signal handler??
}
