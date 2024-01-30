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
//#include <regex.h>

#define MAX 80

/* dont forget to check if you are sending /r/n if you dont you are gone case*/




struct sockaddr_in server, client;
int sock;

void getcrlf(char line[], char buff[])
{
    int n;

    n = recv(sock, buff, MAX, 0);
    while (1)
    {
        for (int i = 0; i < n; i++)
        {
            if (buff[i] == '\r' && (buff[i + 1] == '\n'))
            {
                buff[i] = '\0';
                strcpy(line, buff);

                break;
            }
            else
            {
                strcpy(line, buff);
                n = recv(sock, buff, MAX, 0);
            }
        }
    }
    return;
}

int main(int argc, char *argv[])
{
    int buffptr=0;
    int lineptr=0;
    int k;
    int servlen;
    char filename[100];
    char username[50];
    char password[50];
    char ip[30];
    int choice;
    char buff[MAX + 3];
    char domain[50];
    char msg[50];
    int fp;
    int n;
    int len;
    int status;
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
    char * From ="From: ";
    char * To ="To: ";
    char * Subject ="Subject: ";


    // take the filename input and start reading into buffer
    
    printf("Enter the username\n");
    scanf("%s", username);
    printf("Enter the password\n");
    scanf("%s", password);

    while (1)
    {
        buffptr=0;
        lineptr=0;

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
            printf("Enter the Message in proper format\n");
            if ((connect(sock, (struct sockaddr *)&server, sizeof(server))) < 0)
            {
                perror("Unable to conncet to server..Exiting\n");
                exit(EXIT_FAILURE);
            }
            int i = 0;
            /* write code to check correct format using i*/
            //here
                int flag=1;
                int at_therate=0;

            while (1)
            {
                flag=1;
                at_therate=0;

                gets(buff);
                len = strlen(buff) - 1;
                if (strcmp(buff, ".") == 0)
                {
                    break;
                }
                
                if(!i)
                {
                    for(int i=0;i<len;i++)
                    {
                        if(i<strlen(From))
                        {
                         if(buff[i]!=From[i])
                         {
                             flag=0;
                             break;
                         }
                        }
                        if(buff[i]=='@')at_therate=1;
                    }
                    if(!(flag&at_therate))
                    {
                        printf("Syntax Error\n");
                        break;
                    }

                }
                if (buff[len] == '\n')
                {
                    // buff[len]='\0';
                    buff[len] = '\r';
                    buff[len + 1] = '\n';
                    buff[len + 2] = '\0';
                    send(sock, buff, len + 2, 0);
                }
                else
                {
                    buff[len + 1] = '\r';
                    buff[len + 2] = '\n';
                    buff[len + 3] = '\0';
                    send(sock, buff, len + 3, 0);
                }
            }
            if(!(flag&at_therate))break;
            
            getcrlf(line, buff);

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
                    domain[i - 5] = buff[i];
                }
            }
            else {
                continue;
            }
            sprintf(buff, "HELLO %s", domain);
            len = strlen(buff);
            buff[len] = '\r';
            buff[len + 1] = '\n';
            send(sock, buff, len + 2, 0);
            getcrlf(line, buff);
            char *b = strtok(line, " ");
            status = atoi(b);


            /// here 


            if (status == 250)
            {


               
            }
            else {
                printf("Error Server Sent: %s\n",line);
                continue;
            }

        }

        else
        {
            printf("Exiting\n..");
            exit(0);
        }
    }
}