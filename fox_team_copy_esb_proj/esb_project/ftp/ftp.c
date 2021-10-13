#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define DATASIZE 1024

void send_file(FILE *fp, int sockfd)
{
    char data[DATASIZE] = {0};

    while(fgets(data, DATASIZE, fp)!=NULL)
    {
        if(send(sockfd, data, sizeof(data), 0)== -1)
        {
            perror("[-] Error in sendung data");
            exit(1);
        }
        bzero(data, DATASIZE);
    }
}

int send_ftp_file(char *ip,char *filename)
{
  printf("address:%s filename:%s\n\n",ip,filename);
    // char *ip = "127.0.0.1";
    int port = 8080;
    int e;

    int status=1;
    int sockfd;
    struct sockaddr_in server_addr;
    FILE *fp;
    // char *filename = "abc.xml";
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
    {
        perror("[-]Error in socket");
        status=0;
        // exit(1);
    }
     printf("[+]Server socket created. \n");

     server_addr.sin_family = AF_INET;
     server_addr.sin_port = htons(port);
     server_addr.sin_addr.s_addr = inet_addr(ip);

     e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
     if(e == -1)
     {
         perror("[-]Error in Connecting");
         status=0;
        //  exit(1);
     }
     printf("[+]Connected to server.\n");
     fp = fopen(filename, "r");
     if(fp == NULL)
     {
         perror("[-]Error in reading file.");
         status=0;
        //  exit(1);
     }
     send_file(fp,sockfd);
     printf("[+] File data send successfully. \n");
     close(sockfd);
     printf("[+]Disconnected from the server. \n");
     return status;

}