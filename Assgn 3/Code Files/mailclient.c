/*
Name: Sanskar Mittal, Karthik Reddy
Roll number: 21CS10057, 21CS30058
Assignment 3: Mail Server and Client
File: mailclient.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
// #include <regex.h>

#define MAX 80

/* dont forget to check if you are sending /r/n if you dont you are gone case*/

// end me clrf.clrf bhejdena

struct sockaddr_in server, client;
int sock;

// recving function from server side
void getcrlf(char line[], char buff[])
{
    int n = 0;
    int lineptr = 0;
    int done = 0;

    // printf("%d\n",n);
    n = recv(sock, buff, MAX, 0);
    printf("S:");
    for (int i = 0; i < n; i++)
    {
        if(buff[i]!='\r')printf("%c", buff[i]);
        
    }
    printf("\n");
    while (1)
    {

        if (n > 0)
        {
            for (int i = 0; i < n - 1; i++)
            {
                if (((buff[i] == '\r') && (buff[i + 1] == '\n')) || (line[lineptr] == '\r' && buff[0] == '\n'))
                {
                    buff[i] = '\0';
                    strcpy(line, buff);
                    // printf("line:%s ",line);
                    done = 1;
                    // printf("breaking\n");

                    break;
                }
            }
            if (done)
                break;
            strcpy(line + lineptr, buff);
            lineptr += n;
            n = recv(sock, buff, MAX, 0);
        }
    }
    return;
}
// add char sender and receiver
int validsyntax(char buff[], char From[], int *buffptr, int subj, char sender[], char receiver[], int person)
{
    int len;

    // gets(buff + buffptr);
    len = strlen(buff + *buffptr);
    if (len < strlen(From))
        return 0;
    // printf("Len:%d\n",len);
    int flag = 1;
    int at_therate = 0;
    if (person == 0)
    {
        strcpy(sender, buff + *buffptr + strlen(From));
        if (strlen(sender) < 3)
            return 0;
        sender[strlen(sender) - 1] = '\0';
        // printf("Sender is %s\n",sender);
    }
    else if (person == 1)
    {

        strcpy(receiver, buff + *buffptr + strlen(From));
        if (strlen(receiver) < 3)
            return 0;
        receiver[strlen(receiver) - 1] = '\0';
        // printf("Receiver is %s\n",receiver);
    }

    for (int i = 0; i < len; i++)
    {

        if (i < strlen(From))
        {
            if (buff[i + *buffptr] != From[i])
            {
                flag = 0;
                break;
            }
            continue;
        }

        if (buff[i + *buffptr] == '@')
            at_therate = 1;
    }
    if (buff[len + *buffptr - 1] == '\n')
    {
        // printf("yoooo\n");
        // buff[len]='\0';
        buff[len + *buffptr - 1] = '\r';
        buff[len + *buffptr] = '\n';
        buff[len + *buffptr + 1] = '\0';
        *buffptr = *buffptr + len + 1;

        // send(sock, buff, len + 2, 0);
    }
    else
    {
        printf("fat gaya\n");
    }
    // else
    // {
    //     buff[len + 1 + *buffptr] = '\r';
    //     buff[len + 2 + *buffptr] = '\n';
    //     buff[len + 3 + *buffptr] = '\0';
    //    * buffptr =*buffptr + len + 3;
    //     // send(sock, buff, len + 3, 0);
    // }
    if (!(flag & (subj | at_therate)))
    {
        // printf("returning 0\n");
        return 0;
    }
    // printf("returning 1\n");

    return 1;
}

int main(int argc, char *argv[])
{
    int buffptr = 0;
    int lineptr = 0;
    int k;
    int servlen;
    char filename[100];
    char username[50];
    char password[50];
    char ip[30];
    int choice;
    char buff[5000];
    char domain[50];
    char msg[50];
    char sender[50];
    char receiver[50];
    int fp;
    int n;
    int len;
    int status;
    char buff1[5000];

    char line[MAX + 3];
    if (argc != 4)
    {
        printf("Usage: ./client <server_ip> <smtp_port> <pop3_port>\n");
        exit(0);
    }
    strcpy(ip, argv[1]);
    int smtp_port = atoi(argv[2]);
    int pop3_port = atoi(argv[3]);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(smtp_port);
    char *From = "From: ";
    char *To = "To: ";
    char *Subject = "Subject: ";

    // take the filename input and start reading into buffer

    printf("Enter the username\n");
    scanf("%s", username);
    printf("Enter the password\n");
    scanf("%s", password);

    while (1)
    {
        buffptr = 0;
        lineptr = 0;
        int wrongsyntax = 0;

        printf("Enter a choice:\n1.Manage Mail\n2.Send Mail\n3.Quit\n");
        scanf("%d", &choice);
        getchar();

        if (choice == 2)

        {

            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("Socket creation failed\n");
                exit(EXIT_FAILURE);
            }
            if ((connect(sock, (struct sockaddr *)&server, sizeof(server))) < 0)
            {
                perror("Unable to conncet to server..Exiting\n");
                exit(EXIT_FAILURE);
            }
            // getcrlf(line, buff);
            // sleep(200);
            // exit(0);
            printf("Enter the Message in proper format\n");

            int i = 0;
            /* write code to check correct format using i*/
            // here
            int flag = 1;
            int at_therate = 0;
            int buffptr_copy;
            //

            while (1)
            {
                flag = 1;
                at_therate = 0;

                // printf("buff : %s\n",buff);
                gets(buff + buffptr);
                len = strlen(buff + buffptr);
                // printf("yee : %s\n",buff+buffptr);

                buff[len + buffptr] = '\n';
                buff[len + buffptr + 1] = '\0';
                // printf("buffptr : %d\n",buffptr);
                // printf("buff\n");
                // for(int i=0;i<buffptr;i++)
                // {
                //     printf("%c",buff[i]);
                // }
                //  printf("\n");

                // printgf("Len:%d",len);
                // printf("sdfdf:%d\n", len);
                if (!i)
                {
                    if (!validsyntax(buff, From, &buffptr, 0, sender, receiver, 0))
                    {
                        // printf("")
                        // printf("Syntax error\n");
                        wrongsyntax = 1;
                        break;
                    }
                }
                else if (i == 1)
                {
                    if (!validsyntax(buff, To, &buffptr, 0, sender, receiver, 1))
                    {
                        //  printf("Syntax error\n");
                        wrongsyntax = 1;
                        break;
                    }
                }
                else if (i == 2)
                {
                    if (!validsyntax(buff, Subject, &buffptr, 1, sender, receiver, 3))
                    {
                        // printf("Syntax error\n");
                        wrongsyntax = 1;
                        break;
                    }
                }

                // first write all checks first.

                // what if it breaks before five lines

                // append crlf.crlf

                if (strcmp(buff + buffptr, ".\n") == 0)
                {
                    // printf("breaking\n");
                    if (i < 2)
                        wrongsyntax = 1;
                    buff[buffptr] = '.';
                    buff[buffptr + 1] = '\r';
                    buff[buffptr + 2] = '\n';
                    buffptr += 2;
                    buffptr_copy = buffptr;
                    break;
                }
                else if (i > 2)
                {
                    buffptr += len + 1;
                    buff[buffptr - 1] = '\r';
                    buff[buffptr] = '\n';
                    buffptr++;
                }
                //
                i++;
            }
            if ((wrongsyntax))
            {
                printf("\n******Syntax error*******\n\n\n");
                close(sock);

                continue;
            }
            // printf("Mail entered:\n");
            // for(int i=0;i<4;i++)
            // {
            //     printf("%c",buff[i]);
            // }
            // char* mailBuf = (char*)malloc((MAX+1)*sizeof(char));
            // memset(mailBuf, '\0', MAX+1);
            // strcpy(mailBuf, buff);
            // int mailBufSize = strlen(mailBuf);
            // printf("mailBufSize: %d\n", mailBufSize);
            // for(int i=0;i<mailBufSize;i++)
            // {
            //     if(mailBuf[i] == '\r'){
            //         printf("\\r");
            //     }
            //     else{
            //         printf("%c",mailBuf[i]);
            //     }
            // }
            // printf("waiting\n");

            getcrlf(line, buff1);

            char *a = strtok(line, " ");
            status = atoi(a);

            //printf("Server Sent: %s\n", line);
           // printf("S: %s\n", line);

            if (status == 220)
            {
                for (int i = 5; i < MAX; i++)
                {
                    if (line[i] == '>')
                    {
                        domain[i - 5] = '\0';
                        break;
                    }
                    domain[i - 5] = buff1[i];
                }
            }
            else
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }

            sprintf(buff1, "HELO %s", domain);
            len = strlen(buff1);
            buff1[len] = '\r';
            buff1[len + 1] = '\n';
            send(sock, buff1, len + 2, 0);
            getcrlf(line, buff1);

            char *b = strtok(line, " ");
            status = atoi(b);

            /// here

            if (status != 250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            // else
            else
            {
                //printf("S: %s\n", line);
            }
            // printf("Sender :%ssdffsdf",sender);

            sprintf(buff1, "MAIL FROM: <%s>", sender);
            len = strlen(buff1);
            buff1[len] = '\r';
            buff1[len + 1] = '\n';
            send(sock, buff1, len + 2, 0);
            getcrlf(line, buff1);
            b = strtok(line, " ");
            status = atoi(b);
            if (status != 250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            else
            {
              //  printf("S: %s\n", line);
            }
            sprintf(buff1, "RCPT TO: <%s>", receiver);
            len = strlen(buff1);
            buff1[len] = '\r';
            buff1[len + 1] = '\n';
            send(sock, buff1, len + 2, 0);
            getcrlf(line, buff1);
            b = strtok(line, " ");
            status = atoi(b);
            if (status != 250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            else
            {
               // printf("S: %s\n", line);
            }
            sprintf(buff1, "DATA");
            len = strlen(buff1);
            buff1[len] = '\r';
            buff1[len + 1] = '\n';
            send(sock, buff1, len + 2, 0);
            getcrlf(line, buff1);
            b = strtok(line, " ");
            status = atoi(b);

            if (status != 354)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            else
            {
                //printf("S: %s\n", line);
            }
            // buff[buffptr_copy+1]='\0';
            // printf("buffptr :%d\n",buffptr_copy);
            // for(int i=0;i<buffptr_copy;i++)
            // {
            //     printf("%c",buff[i]);
            // }
            printf("\n");
            // printf("buff :%s\n",buff);
            if (send(sock, buff, buffptr + 1, 0) < 0)
            {
                perror("Unable to send data\n");
                exit(0);
            }
            //printf("sent\n");
            getcrlf(line, buff1);
            b = strtok(line, " ");
            status = atoi(b);
            if (status != 250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            else
            {
               // printf("S: %s\n", line);
            }
            sprintf(buff1, "QUIT");
            len = strlen(buff1);
            buff1[len] = '\r';
            buff1[len + 1] = '\n';
            send(sock, buff1, len + 2, 0);
            getcrlf(line, buff1);
            b = strtok(line, " ");
            status = atoi(b);
            if (status != 221)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            else
            {
               // printf("S: %s\n", line);
            }
            printf("Mail Sent Successfully\n");
            close(sock);
        }

        else
        {
            printf("Exiting\n..");
            exit(0);
        }
    }
}