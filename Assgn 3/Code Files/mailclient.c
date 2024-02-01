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

#define MAX 80

struct sockaddr_in server, client;
int sock;

// recving function from server side
// transient_buff is used receive
// if \r\n is not found then it is cahced in cache until it is found
void getcrlf(char cache[], char transient_buff[])
{
    int n = 0;
    int cacheptr = 0;
    int done = 0;

    n = recv(sock, transient_buff, MAX, 0);
    printf("S:");
    for (int i = 0; i < n; i++)
    {
        if (transient_buff[i] != '\r')
            printf("%c", transient_buff[i]);
    }
    printf("\n");
    while (1)
    {

        if (n > 0)
        {
            for (int i = 0; i < n - 1; i++)
            {
                if (((transient_buff[i] == '\r') && (transient_buff[i + 1] == '\n')) || (cache[cacheptr] == '\r' && transient_buff[0] == '\n'))
                {
                    transient_buff[i] = '\0';
                    strcpy(cache, transient_buff);
                    done = 1;

                    break;
                }
            }
            if (done)
                break;
            strcpy(cache + cacheptr, transient_buff);
            cacheptr += n;
            n = recv(sock, transient_buff, MAX, 0);
        }
    }
    return;
}
// add char sender and receiver
// checks if the syntax is correct
// if issubj is 1 then it checks for subject and ignores @
// sender stores the sender name
// receiver stores the receiver name
// person is 0 for sender and 1 for receiver

int validsyntax(char mainbuff[], char String[], int *buffptr, int issubj, char sender[], char receiver[], int person)
{
    int currlinelen;
    currlinelen = strlen(mainbuff + *buffptr);
    if (currlinelen < strlen(String))
    {
        return 0;
    }

    int stringmatched = 1;
    int at_therate = 0;
    if (person == 0)
    {
        strcpy(sender, mainbuff + *buffptr + strlen(String));
        if (strlen(sender) < 3)
        {
            return 0;
        }

        sender[strlen(sender) - 1] = '\0';
    }
    else if (person == 1)
    {

        strcpy(receiver, mainbuff + *buffptr + strlen(String));
        if (strlen(receiver) < 3)
        {
            return 0;
        }

        receiver[strlen(receiver) - 1] = '\0';
    }

    for (int i = 0; i < currlinelen; i++)
    {

        if (i < strlen(String))
        {
            if (mainbuff[i + *buffptr] != String[i])
            {
                stringmatched = 0;
                break;
            }
            continue;
        }
        fflush(stdout);

        if (mainbuff[i + *buffptr] == '@')
            at_therate = 1;
    }
    if (mainbuff[currlinelen + *buffptr - 1] == '\n')
    {

        mainbuff[currlinelen + *buffptr - 1] = '\r';
        mainbuff[currlinelen + *buffptr] = '\n';
        mainbuff[currlinelen + *buffptr + 1] = '\0';
        *buffptr = *buffptr + currlinelen + 1;
    }
    if (!(stringmatched & (issubj | at_therate)))
    {

        return 0;
    }

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
    char mainbuff[5000];
    char domain[50];
    char msg[50];
    char sender[50];
    char receiver[50];
    int fp;
    int n;
    int len;
    int status;
    char transient_buff[5000];

    char line[5000];
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
    printf("Enter the username\n");
    scanf("%s", username);
    printf("Enter the password\n");
    scanf("%s", password);

    while (1)
    {
        fflush(stdin);
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
            printf("Enter the Message in proper format\n");

            int Lineno = 0;

            while (1)
            {

                gets(mainbuff + buffptr);
                len = strlen(mainbuff + buffptr);
                //gets does not add \n at the end
                //so manually adding it
                mainbuff[len + buffptr] = '\n';
                mainbuff[len + buffptr + 1] = '\0';
                if (!Lineno)//checking syntax of from
                {
                    if (!validsyntax(mainbuff, From, &buffptr, 0, sender, receiver, 0))
                    {
                        wrongsyntax = 1;
                        break;
                    }
                }
                else if (Lineno == 1)//checking syntax of to
                {
                    if (!validsyntax(mainbuff, To, &buffptr, 0, sender, receiver, 1))
                    {
                        wrongsyntax = 1;
                        break;
                    }
                }
                else if (Lineno == 2)//checking syntax of subject
                {
                    if (!validsyntax(mainbuff, Subject, &buffptr, 1, sender, receiver, 3))
                    {

                        wrongsyntax = 1;
                        break;
                    }
                }

                if (strcmp(mainbuff + buffptr, ".\n") == 0)
                {

                    if (Lineno < 2)
                        wrongsyntax = 1;
                    mainbuff[buffptr] = '.';
                    mainbuff[buffptr + 1] = '\r';
                    mainbuff[buffptr + 2] = '\n';
                    buffptr += 2;
                    break;
                }
                else if (Lineno > 2)
                {
                    buffptr += len + 1;
                    mainbuff[buffptr - 1] = '\r';
                    mainbuff[buffptr] = '\n';
                    buffptr++;
                }
                
                Lineno++;
            }
            if ((wrongsyntax))
            {
                printf("\n******Syntax error*******\n\n\n");
                close(sock);

                continue;
            }
            getcrlf(line, transient_buff);

            char *a = strtok(line, " ");
            status = atoi(a);

            if (status == 220)
            {
                for (int i = 5; i < MAX; i++)
                {
                    if (line[i] == '>')
                    {
                        domain[i - 5] = '\0';
                        break;
                    }
                    domain[i - 5] = transient_buff[i];
                }
            }
            else
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }

            sprintf(transient_buff, "HELO %s", domain);
            len = strlen(transient_buff);
            transient_buff[len] = '\r';
            transient_buff[len + 1] = '\n';
            send(sock, transient_buff, len + 2, 0);
            getcrlf(line, transient_buff);

            char *b = strtok(line, " ");
            status = atoi(b);

            if (status != 250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }

            sprintf(transient_buff, "MAIL FROM: <%s>", sender);
            len = strlen(transient_buff);
            transient_buff[len] = '\r';
            transient_buff[len + 1] = '\n';
            send(sock, transient_buff, len + 2, 0);
            getcrlf(line, transient_buff);
            b = strtok(line, " ");
            status = atoi(b);
            if (status != 250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }

            sprintf(transient_buff, "RCPT TO: <%s>", receiver);
            len = strlen(transient_buff);
            transient_buff[len] = '\r';
            transient_buff[len + 1] = '\n';
            send(sock, transient_buff, len + 2, 0);
            getcrlf(line, transient_buff);
            b = strtok(line, " ");
            status = atoi(b);
            if (status != 250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }

            sprintf(transient_buff, "DATA");
            len = strlen(transient_buff);
            transient_buff[len] = '\r';
            transient_buff[len + 1] = '\n';
            send(sock, transient_buff, len + 2, 0);
            getcrlf(line, transient_buff);
            b = strtok(line, " ");
            status = atoi(b);

            if (status != 354)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }

            printf("\n");

            if (send(sock, mainbuff, buffptr + 1, 0) < 0)
            {
                perror("Unable to send data\n");
                exit(0);
            }

            getcrlf(line, transient_buff);
            b = strtok(line, " ");
            status = atoi(b);
            if (status != 250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }

            sprintf(transient_buff, "QUIT");
            len = strlen(transient_buff);
            transient_buff[len] = '\r';
            transient_buff[len + 1] = '\n';
            send(sock, transient_buff, len + 2, 0);
            getcrlf(line, transient_buff);
            b = strtok(line, " ");
            status = atoi(b);
            if (status != 221)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
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