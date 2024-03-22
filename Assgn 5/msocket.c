#include <arpa/inet.h>
#include <errno.h>
#include <msocket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <time.h>
/*
1) Avoid using strcpy just blindly copy since messages are of fixed (1000) bytes
2)(wrong and ignore) Dereferencing void * into char --> *((char *)(ptr))
2) Mutexes not included to be taken care later
3) didnt set one of the errors in m_bind

*/

int m_socket(int domain, int type, int protocol) {
    int sockfd;
    if (type != SOCK_MTP) {
        errno = EINVAL;
        return -1;
    }
    key_t key = KEY;
    int shmid = shmget(key, N * sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, NULL, 0);
    if (SM == NULL) {
        errno = ENOMEM;
        printf("Init process not called \n");
        return -1;
    }
    int free_available = 0;
    int m_sockfd;

    key_t sock_info_key = SOCK_INFO_KEY;
    int sock_info = shmget(sock_info_key, sizeof(struct SOCKINFO), 0777);
    struct SOCKINFO *sockinfo = (struct SOCKINFO *)shmat(sock_info, 0, 0);

    // get semaphores
    key_t sem1_key = SEM1_KEY;
    key_t sem2_key = SEM2_KEY;
    int sem1 = semget(sem1_key, 1, 0777);
    int sem2 = semget(sem2_key, 1, 0777);

    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;

    for (int i = 0; i < N; i++) {
        if (SM[i].free == 1) {
            V(sem1);
            P(sem2);

            if (sockinfo->sock_id == -1) {
                errno = sockinfo->error_no;
                return -1;
            }

            free_available = 1;
            SM[i].free = 0;
            SM[i].sockfd = sockinfo->sock_id;
            SM[i].pid = getpid();
            m_sockfd = i;
            break;
        }
    }
    sockinfo->sock_id = 0;
    sockinfo->error_no = 0;
    // sockinfo->addr = 0;

    if (free_available == 0) {
        errno = ENOBUFS;
        return -1;
    }
    return m_sockfd;
}

int m_bind(int m_sockfd, const struct sockaddr *src_addr, socklen_t src_addrlen, const struct sockaddr *dest_addr, socklen_t dest_addrlen) {

    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;

    // initialze the buff?

    key_t key = KEY;
    int shmid = shmget(key, N * sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, NULL, 0);
    // lock the shared memory ? not required as only this process is aceesing when m_bind is in progress

    // fill the entry in shared memory
    // before filling do error checking;
    if (m_sockfd < 0 || m_sockfd >= N) {
        errno = EBADF;
        return -1;
    }
    if (SM[m_sockfd].free == 1) {
        errno = EBADF;
        return -1;
    }
    if (SM[m_sockfd].pid != getpid()) {
        errno = EBADF;
        return -1;
    }

    // how do you initilaze this ?? can you do it this way?
    // fflush(stdout);
    // printf("Dest Addr: %d\n", (((struct sockaddr_in *)dest_addr)->sin_addr.s_addr));
    // printf("Dest Addr Ip: %s\n", inet_ntoa(((struct sockaddr_in *)dest_addr)->sin_addr));
    SM[m_sockfd].addr = (struct sockaddr_in *)dest_addr;
    // initialze the send window
    SM[m_sockfd].swnd.send_window_size = 1;

    // SM[m_sockfd].swnd.last_ack=0;// COZ numbering starrts from 1
    SM[m_sockfd].swnd.rem_buff_space = SEND_BUFFER_SIZE;
    SM[m_sockfd].swnd.start_index = 0;
    SM[m_sockfd].swnd.end_index = 0;
    SM[m_sockfd].swnd.start_index_ack_no = 0;
    SM[m_sockfd].swnd.last_sent_index = 0;

    // memset(SM[m_sockfd].swnd.unack_msg,-1,sizeof(SM[m_sockfd].swnd.unack_msg));

    memset(SM[m_sockfd].swnd.unack_time, -1, sizeof(SM[m_sockfd].swnd.unack_time));

    // initialze the receive window
    SM[m_sockfd].rwnd.receive_window_size = MAX_WINDOW_SIZE;
    SM[m_sockfd].rwnd.last_inorder_msg = 0; // Cause numbering starts from 1
    SM[m_sockfd].rwnd.last_inorder_msg_seq_num = 0;
    SM[m_sockfd].rwnd.start_index = 1;
    SM[m_sockfd].rwnd.start_seq_num = 0;
    SM[m_sockfd].rwnd.nospace = 0;

    memset(SM[m_sockfd].rwnd.recv_msg, 0, sizeof(SM[m_sockfd].rwnd.recv_msg));
    // call the system bind call
    // int res =bind(SM[m_sockfd].sockfd, src_addr, src_addrlen);
    // signal the init process
    // first attach itself to sockingo shared memory
    key_t sock_info_key = SOCK_INFO_KEY;
    int sock_info = shmget(sock_info_key, sizeof(struct SOCKINFO), 0777);
    struct SOCKINFO *sockinfo = (struct SOCKINFO *)shmat(sock_info, 0, 0);
    // error checking
    if (sockinfo == NULL) {
        errno = ENOMEM;
        printf("Init process not called \n");
        return -1;
    }
    // get semaphores
    key_t sem1_key = SEM1_KEY;
    key_t sem2_key = SEM2_KEY;
    int sem1 = semget(sem1_key, 1, 0777);
    int sem2 = semget(sem2_key, 1, 0777);
    // place its udp sock id in sockinfo
    // printf("M_sockfd: %d\n", m_sockfd);
    // printf("ADDR: %s\n",inet_ntoa(SM[m_sockfd].addr));
    sockinfo->sock_id = SM[m_sockfd].sockfd;
    sockinfo->addr = *(SM[m_sockfd].addr);
    sockinfo->error_no = 0;
    printf("Sockinfo: %d, %s, %d\n", sockinfo->sock_id, inet_ntoa(sockinfo->addr.sin_addr), sockinfo->error_no);
    // signal till the init process
    V(sem1);
    // wait till the init process signals
    P(sem2);
    printf("Semaphore complete\n");
    // error checking
    if (sockinfo->sock_id < 0) {
        errno = sockinfo->error_no;
        return -1;
    }
    // reset all the values to 0

    // can you do this ? or manually reset all the values to 0??
    memset(sockinfo, 0, sizeof(struct SOCKINFO));

    // unlock the shared memory

    // release all the semaphores// not needed

    // deatch from shared memory
    shmdt(sockinfo);
    shmdt(SM);

    return 0;
}

// set the correct errno
ssize_t m_sendto(int m_sockfd, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen) {
    // first attach to shared memory

    // need to use mutexes
    key_t key = KEY;
    // printf("M_sockfd: %d\n", m_sockfd);
    int shmid = shmget(key, N * sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, NULL, 0);
    // error checking
    if (SM == NULL) {
        errno = ENOMEM;
        printf("Init process not called \n");
        return -1;
    }
    if (m_sockfd < 0 || m_sockfd >= N) {
        errno = EBADF;
        return -1;
    }
    if (SM[m_sockfd].free == 1) {
        errno = EBADF;
        return -1;
    }
    if (SM[m_sockfd].pid != getpid()) {
        errno = EBADF;
        return -1;
    }
    // check if port and ip are same
    if ((SM[m_sockfd].addr->sin_port != ((struct sockaddr_in *)dest_addr)->sin_port) || (SM[m_sockfd].addr->sin_addr.s_addr != ((struct sockaddr_in *)dest_addr)->sin_addr.s_addr)) {
        errno = ENOTCONN;
        printf("Port and IP are not same\n");
        return -1;
    }
    if (SM[m_sockfd].swnd.start_index == (SM[m_sockfd].swnd.end_index + 1) % SEND_BUFFER_SIZE) {
        errno = ENOBUFS;
        return -1;
    }
    // increment the end index
    SM[m_sockfd].swnd.end_index = (SM[m_sockfd].swnd.end_index + 1) % SEND_BUFFER_SIZE;
    // copy the message to the buffer
    for (int i = 0; i < length; i++) {
        SM[m_sockfd].send_buffer[SM[m_sockfd].swnd.end_index][i] = *((char*)(message + i));
    }
    SM[m_sockfd].swnd.length[SM[m_sockfd].swnd.end_index] = length;

    return 0;
    // who adds the header is it S or this function ?
    // it is s who adds the header
}

ssize_t m_recvfrom(int m_sockfd, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len) {

    // first attach to shared memory
    key_t key = KEY;
    int shmid = shmget(key, N * sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, NULL, 0);

    // error checking
    if (SM == NULL) {
        errno = ENOMEM;
        printf("Init process not called \n");
        return -1;
    }

    // If the socket is free
    if (SM[m_sockfd].free == 1) {
        errno = EBADF;
        return -1;
    }

    for (int i = 0; i < RECV_BUFFER_SIZE; i++) {
        // Assuming that the R process stores only the actual message in the Receive buffer ending with <crlf>
        if (SM[m_sockfd].recv_buffer[i][0] != '\r' && SM[m_sockfd].recv_buffer[i][1] != '\n') {
            int j = 0;
            while (SM[m_sockfd].recv_buffer[i][j] != '\r' && SM[m_sockfd].recv_buffer[i][j + 1] != '\n') {
                *((char *)buffer + j) = SM[m_sockfd].recv_buffer[i][j];
                j++;
            }
            memset(SM[m_sockfd].recv_buffer[i], '\0', 1000);
            SM[m_sockfd].recv_buffer[i][0] = '\r';
            SM[m_sockfd].recv_buffer[i][1] = '\n';
            shmdt(SM);
            return j;
        }
    }
    shmdt(SM);
    errno = ENOMSG;
    return -1;
}

int m_close(int m_sockfd) {
    int res;
    key_t key = KEY;
    int shmid = shmget(key, N * sizeof(struct shared_memory), 0777);
    struct shared_memory *SM = (struct shared_memory *)shmat(shmid, NULL, 0);

    if (SM[m_sockfd].free == 0) {
        // memset(&SM[m_sockfd], NULL, sizeof(struct shared_memory));
        SM[m_sockfd].free = 1;
    } else {
        errno = EBADF;
        res = -1;
    }
    shmdt(SM);
    return res;
}

int dropMessage(float probability) {
    srand((unsigned int)time(NULL));
    float r = (float)rand() / (float)(RAND_MAX);
    return (r < probability ? 1 : 0);
}