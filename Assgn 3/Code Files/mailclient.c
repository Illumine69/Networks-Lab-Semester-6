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
#include <sys/types.h>

#define MAX 80

struct sockaddr_in server, client;
int sock;

// recving function from server side
// transient_buff is used receive
// if \r\n is not found then it is cahced in cache until it is found
int getcrlf(char cache[], char transient_buff[], int mode)
{
    // int mode=0;//get clrf mode or get clrf . clrfl mode
    // int mssgno=1;
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
            if (!mode)
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
                for (int i = 0; i < n; i++)
                {
                    cache[cacheptr++] = transient_buff[i];
                }
                // strcpy(cache + cacheptr, transient_buff);
                // cacheptr += n;
                n = recv(sock, transient_buff, MAX, 0);
                for(int i=0;i<n;i++)
                {
                    if(transient_buff[i]!='\r')
                    printf("%c",transient_buff[i]);
                }
            }
            else
            {
                //printf("n:%d\n", n);
                //fflush(stdout);
                for (int i = 0; i < n; i++)
                {
                    cache[cacheptr++] = transient_buff[i];
                }
                if (cache[cacheptr - 1] == '\n' && cache[cacheptr - 2] == '\r' && cache[cacheptr - 3] == '.' && cache[cacheptr - 4] == '\n' && cache[cacheptr - 5] == '\r')
                {
                    done = 1;
                    break;
                }
                if (done)
                    break;
                n = recv(sock, transient_buff, MAX, 0);
            }
        }
    }
    if (mode)
    {
        // FILE *fp;
        // char filename[20]="temp.txt";
        // fp=fopen(filename,"a");
        // for(int i=0;i<cacheptr;i++)
        // {
        //     if(cache[i]!='\r')
        //     fprintf(fp,"%c",cache[i]);
        // }
        // fclose(fp);
        return cacheptr;
    }
    return 0;
}
void printsummary( int message_no,int fullmsg,int deleted_messages [])
{
    //check if messgae is present



    if(message_no<1)
    {
        printf("Mail box Empty\n");
        return;
    }

    FILE *fp;
    char filename[20];
    sprintf(filename, "./mails/%d.txt", message_no);
    fp = fopen(filename, "r");
    printf("%d\t\t",message_no);
    if(deleted_messages[message_no])
    {
        printf("Deleted\n");
        return;
    }
    if(!fullmsg)
    {
        char time[100];
        char Subject[1000];
        char line[5000];
        memset(time,0,sizeof(time));
        memset(Subject,0,sizeof(Subject));
        memset(line,0,sizeof(line));
        int i=0;
        while(fgets(line,5000,fp))
        {
             if(i==1)
            {

               // printf("To: %s\n",line);
               char * a;
                a=strtok(line," ");
                a=strtok(NULL," ");
                int len=strlen(a);
                a[len-1]='\0';
                printf("%s",a);
                //printf("%s",a);


            }
            else if(i==3)
            {
                strcpy(Subject,line);
            }
            else if(i==4)
            {
                // printf("\t\t%[^ \n]",line);
                // printf("\t\t%[^ \n]",Subject);
                    int len = strlen(line);
                    line[len-1]='\0';

                printf("\t\t%s",line);
                // len=strlen(Subject);
                // Subject[len-1]='\0';

                printf("\t\t%s",Subject);
                //printf("\t\t");

              //  printf("\t\t");
                //puts(time);

              //  printf("\n");
            }
            i++;
            if(i>4)
            {
                break;
            }
        }

    }
    else {
        char line[5000];
        //  memset(time,0,sizeof(time));
        //memset(Subject,0,sizeof(Subject));
        memset(line,0,sizeof(line));
        int i=0;
        while(fgets(line,5000,fp))
        {
            if(i==1)
            {

                printf("%s",line);
            }
            else if(i>1)
            {
                printf("\t\t%s",line);

            }
            i++;
            
        }
    
    }


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
        // fflush(stdout);

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
// function to get al the messgaes from the server

int main(int argc, char *argv[])
{
    int buffptr = 0;
    char username[50];
    char password[50];
    char ip[30];
    int choice;
    char mainbuff[5000];
    char domain[50];
    char sender[50];
    char receiver[50];
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
    printf("Enter the username: ");
    scanf("%s", username);
    printf("Enter the password: ");
    scanf("%s", password);

    while (1)
    {
        fflush(stdin);
        buffptr = 0;
        int wrongsyntax = 0;

        printf("\nEnter a choice:\n1.Manage Mail\n2.Send Mail\n3.Quit\n\nChoice: ");
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
                // gets does not add \n at the end
                // so manually adding it
                mainbuff[len + buffptr] = '\n';
                mainbuff[len + buffptr + 1] = '\0';
                if (!Lineno) // checking syntax of from
                {
                    if (!validsyntax(mainbuff, From, &buffptr, 0, sender, receiver, 0))
                    {
                        wrongsyntax = 1;
                        break;
                    }
                }
                else if (Lineno == 1) // checking syntax of to
                {
                    if (!validsyntax(mainbuff, To, &buffptr, 0, sender, receiver, 1))
                    {
                        wrongsyntax = 1;
                        break;
                    }
                }
                else if (Lineno == 2) // checking syntax of subject
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
            getcrlf(line, transient_buff, 0);

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
            getcrlf(line, transient_buff, 0);

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
            getcrlf(line, transient_buff, 0);
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
            getcrlf(line, transient_buff, 0);
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
            getcrlf(line, transient_buff, 0);
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

            getcrlf(line, transient_buff, 0);
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
            getcrlf(line, transient_buff, 0);
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
        else if (choice == 1)
        {

            //create a directory to store the messages
            struct stat st = {0};

        if (stat("./mails", &st) == -1) {
    if(mkdir("./mails", 0777)<0)
    {
        perror("Error in creating directory\n");
        exit(0);
    
    };
        }

            int n_messages = 0;
            int total_size = 0;

            //      strcpy(ip, argv[1]);
            // int smtp_port = atoi(argv[2]);
            // int pop3_port = atoi(argv[3]);
            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = inet_addr(ip);
            server.sin_port = htons(pop3_port);

            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("Socket creation for pop3 server failed\n");
                exit(EXIT_FAILURE);
            }
            if ((connect(sock, (struct sockaddr *)&server, sizeof(server))) < 0)
            {
                perror("Unable to conncet to pop3 server..Exiting\n");
                exit(EXIT_FAILURE);
            }

            // first recevie the hello message from the server
            getcrlf(line, transient_buff, 0);
            char *a = strtok(line, " ");
            if (strcmp(a, "+OK") != 0)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }

            sprintf(transient_buff, "USER %s", username);
            len = strlen(transient_buff);
            transient_buff[len] = '\r';
            transient_buff[len + 1] = '\n';
            send(sock, transient_buff, len + 2, 0);
            getcrlf(line, transient_buff, 0);
            a = strtok(line, " ");
            if (strcmp(a, "+OK") != 0)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            sprintf(transient_buff, "PASS %s", password);
            len = strlen(transient_buff);
            transient_buff[len] = '\r';
            transient_buff[len + 1] = '\n';
            send(sock, transient_buff, len + 2, 0);
            getcrlf(line, transient_buff, 0);
            a = strtok(line, " ");
            if (strcmp(a, "+OK") != 0)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            sprintf(transient_buff, "STAT");
            len = strlen(transient_buff);
            transient_buff[len] = '\r';
            transient_buff[len + 1] = '\n';
            send(sock, transient_buff, len + 2, 0);
            getcrlf(line, transient_buff, 0);
            a = strtok(line, " ");
            if (strcmp(a, "+OK") != 0)
            {
                printf("Error Server Sent: %s\n", line);
                continue;
            }
            a = strtok(NULL, " ");
            n_messages = atoi(a);
            a = strtok(NULL, " ");
            total_size = atoi(a);

            int is_deleted_message[n_messages + 1];
            memset(is_deleted_message, 0, sizeof(is_deleted_message));

            int individual_mssg_size[n_messages + 1];
            int sum = 0;

            for (int i = 1; i <= n_messages; i++)
            {
                sprintf(transient_buff, "LIST %d", i);
                len = strlen(transient_buff);
                transient_buff[len] = '\r';
                transient_buff[len + 1] = '\n';
                send(sock, transient_buff, len + 2, 0);
                getcrlf(line, transient_buff, 0);
                a = strtok(line, " ");
                if (strcmp(a, "+OK") != 0)
                {
                    printf("Error Server Sent: %s\n", line);
                    continue;
                }
                
                a = strtok(NULL, " ");
                a= strtok(NULL, " ");
                //printf("%d\n",atoi(a));
                sum += atoi(a);
                individual_mssg_size[i] = atoi(a);
            }
            if (sum != total_size)
            {
                printf("Error in the total size of the messages from server side\n");
                sprintf(transient_buff, "QUIT");
                len = strlen(transient_buff);
                transient_buff[len] = '\r';
                transient_buff[len + 1] = '\n';
                send(sock, transient_buff, len + 2, 0);

                continue;
            }
            // get all the meesages and store them in an file.
            for (int i = 1; i <= n_messages; i++)
            {
                sprintf(transient_buff, "RETR %d", i);
                len = strlen(transient_buff);
                transient_buff[len] = '\r';
                transient_buff[len + 1] = '\n';
                send(sock, transient_buff, len + 2, 0);
                int cacheptr = getcrlf(line, transient_buff, 1);
               // printf("Hello\n");
                char *a;
                a = strtok(line, " ");
                if (strcmp(a, "+OK") != 0)
                {
                    printf("Error Server Sent: %s\n", line);
                    continue;
                }
                // store them in a file
                FILE *fp;
                char filename[20];
                sprintf(filename, "./mails/%d.txt", i);
                fp = fopen(filename, "w");
                for (int i = 0; i < cacheptr; i++)
                {
                    if (line[i] != '\r')
                        fprintf(fp, "%c", line[i]);
                }
                fclose(fp);
            }
            int print_main_menu=0;
            while(1)
            {
              for(int i=1;i<=n_messages;i++)
              {
                    printsummary(i,0,is_deleted_message);
                   
              }
              printf("Enter mail no: ");
                int mailno;
                scanf("%d",&mailno);
                while(mailno!=-1&&mailno>n_messages)
                {
                    printf("mail out of range give again:");
                    //printf("Enter mail no: ");
                    scanf("%d",&mailno);
                }
                if(mailno==-1)
                {
                    // clear the mail directory
                    for(int i=1;i<=n_messages;i++)
                    {
                        char filename[20];
                        sprintf(filename, "./mails/%d.txt", i);
                        remove(filename);
                    }
                    rmdir("./mails");
                    //send quit to the server
                    sprintf(transient_buff, "QUIT");
                    len = strlen(transient_buff);
                    transient_buff[len] = '\r';
                    transient_buff[len + 1] = '\n';
                    send(sock, transient_buff, len + 2, 0);
                    getcrlf(line, transient_buff, 0);
                    a = strtok(line, " ");
                    if (strcmp(a, "+OK") != 0)
                    {
                        printf("Error Server Sent: %s\n", line);
                        
                    }

                    print_main_menu=1;
                    break;
                }
                else{
                    printsummary(mailno,1,is_deleted_message);
                    fflush(stdin);
                    char delete=getchar();
                    if(delete=='d')
                    {
                        sprintf(transient_buff, "DELE %d", mailno);
                        len = strlen(transient_buff);
                        transient_buff[len] = '\r';
                        transient_buff[len + 1] = '\n';
                        send(sock, transient_buff, len + 2, 0);
                        getcrlf(line, transient_buff, 0);
                        char *a;
                        a = strtok(line, " ");
                        if (strcmp(a, "+OK") != 0)
                        {
                            printf("Error Server Sent: %s\n", line);
                            continue;
                        }
                        is_deleted_message[mailno]=1;
                    }
                    else 
                    {
                      //  break;
                    }
                }


            }
            if(print_main_menu)
            {
                continue;
            }
        }
        else if (choice == 3)
        {
            printf("\nExiting...\n");
            exit(0);
        }
        else
        {
            printf("\nInvalid Choice\n");
        }
    }
}