/*
Name: Sanskar Mittal, Karthik Reddy
Roll number: 21CS10057, 21CS30058
Assignment 3: Mail Server and Client
File: popserver.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#define MAX 5000

int main(int argc, char* argv[]){
    if(argc != 2){
        perror("Usage: ./popserver <pop3_port>\n");
        exit(1);
    }
}