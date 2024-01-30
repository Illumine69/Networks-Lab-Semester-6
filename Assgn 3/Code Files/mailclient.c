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

void getcrlf(char line[], char buff[])
{
    int n;
    int lineptr=0;
    int done=0;

    n = recv(sock, buff, MAX, 0);
    while (1)
    {
        for (int i = 0; i < n; i++)
        {
            if (((buff[i] == '\r') && (buff[i + 1] == '\n'))||(line[lineptr-1]=='\r'&&buff[0]=='\n'))
            {
                buff[i] = '\0';
                strcpy(line, buff);
                done=1;

                break;
            }
            
          
        }
        if(done)break;
        strcpy(line+lineptr, buff);
                lineptr+=n;
                n = recv(sock, buff, MAX, 0);

        

    }
    return;
}
//add char sender and receiver
int validsyntax(char buff[], char From[], int * buffptr,int subj,char sender [],char receiver[],int person)
{
    int len;
    
    //gets(buff + buffptr);
    len = strlen(buff + *buffptr) - 1;
    int flag = 1;
    int at_therate = 0;
    if(person==0)
    {
        strcpy(sender,buff+*buffptr+strlen(From));
    }
    else if(person==1)
    {
        strcpy(receiver,buff+*buffptr+strlen(sender));
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
     if (buff[len + *buffptr] == '\n')
                {
                    // buff[len]='\0';
                    buff[len + *buffptr] = '\r';
                    buff[len + 1 + *buffptr] = '\n';
                    buff[len + 2 + *buffptr] = '\0';
                    *buffptr = *buffptr+len + 2;

                    // send(sock, buff, len + 2, 0);
                }
                else
                {
                    buff[len + 1 + *buffptr] = '\r';
                    buff[len + 2 + *buffptr] = '\n';
                    buff[len + 3 + *buffptr] = '\0';
                   * buffptr =*buffptr + len + 3;
                    // send(sock, buff, len + 3, 0);
                }
    if (!(flag & (subj|at_therate)))
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
    char buff[MAX + 3];
    char domain[50];
    char msg[50];
    char sender [50];
    char receiver[50];
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
        int wrongsyntax=0;

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
           
            int i = 0;
            /* write code to check correct format using i*/
            // here
            int flag = 1;
            int at_therate = 0;


            while (1)
            {
                flag = 1;
                at_therate = 0;
                 gets(buff + buffptr);
                len = strlen(buff + buffptr) - 1;
                if (!i)
                {
                    if(!validsyntax(buff,From,&buffptr,0,sender,receiver,0))
                    {
                        //printf("Syntax error\n");
                        wrongsyntax=1;
                        break;


                    }
                }
                else if (i==1)
                {
                    if(!validsyntax(buff,From,&buffptr,0,sender,receiver,1)) {
                      //  printf("Syntax error\n");
                        wrongsyntax=1;
                        break;

                    }


                }
                else if(i==2)
                {
                    if(!validsyntax(buff,Subject,&buffptr,1,sender,receiver,3))
                    {
                        //printf("Syntax error\n");
                        wrongsyntax=1;
                    }
                   
                }
               

                // first write all checks first.

               
                // what if it breaks before five lines

                //append crlf.crlf
                if (strcmp(buff, ".") == 0)
                {
                    if(i<2)wrongsyntax=1;
                    buff[buffptr] = '\r';
                    buff[buffptr + 1] = '\n';
                    buff[buffptr + 2] = '.';
                    buff[buffptr + 3] = '\r';
                    buff[buffptr + 4] = '\n';
                    buffptr+=5;
                    break;
                }
                else {
                    buffptr+=len;

                }

                i++;
            }
            if (!(wrongsyntax))
            {
                printf("Syntax error\n");
                continue;
            }
            

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
            else
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            sprintf(buff, "HELLO %s", domain);
            len = strlen(buff);
            buff[len] = '\r';
            buff[len + 1] = '\n';
            send(sock, buff, len + 2, 0);
            getcrlf(line, buff);
            char * b = strtok(line, " ");
            status = atoi(b);

            /// here

            if (status != 250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            //else
            
            sprintf(buff,"MAIL FROM: <%s>",sender); 
            len = strlen(buff);
            buff[len] = '\r';
            buff[len + 1] = '\n';
            send(sock, buff, len + 2, 0);
            getcrlf(line, buff);
            b = strtok(line, " ");
            status = atoi(b);
            if(status!=250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            sprintf(buff,"RCPT TO: <%s>",receiver);
            len = strlen(buff);
            buff[len] = '\r';
            buff[len + 1] = '\n';
            send(sock, buff, len + 2, 0);
            getcrlf(line, buff);
            b = strtok(line, " ");
            status = atoi(b);
            if(status!=250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            sprintf(buff,"DATA");
             len = strlen(buff);
            buff[len] = '\r';
            buff[len + 1] = '\n';
            send(sock, buff, len + 2, 0);
            getcrlf(line, buff);
             b = strtok(line, " ");
              status = atoi(b);
            if(status!=354)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            send(sock, buff, buffptr, 0);
             getcrlf(line, buff);
            b = strtok(line, " ");
            status = atoi(b);
            if(status!=250)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            sprintf(buff,"QUIT");
             len = strlen(buff);
              buff[len] = '\r';
            buff[len + 1] = '\n';
            send(sock, buff, len + 2, 0);
            getcrlf(line, buff);
            b = strtok(line, " ");
             status = atoi(b);
            if(status!=221)
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