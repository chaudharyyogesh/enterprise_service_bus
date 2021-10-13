// gcc -o server.o server.c
// ./server.o 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SIZE 1024

void write_file(int sockfd)
{
    int n; 
    FILE *fp;
    char *filename = "received_json_file.json";
    char buffer[SIZE];

    fp = fopen(filename, "w");
    if(fp==NULL)
    {
        perror("[-]Error in creating file.");
        exit(1);
    }
    bzero(buffer, SIZE);
  
        printf("[+]Receiving File.");
        n = read(sockfd, buffer, SIZE);
        if(n<=0)
        {
            perror("error reading");
        }
        
        fprintf(fp, "%s", buffer);
        bzero(buffer, SIZE);
    return;
    
}

int main ()
{
    // char *ip = "127.0.0.1";
    int port = 8080;
    int e;

    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
    {
        perror("[-]Error in socket");
        exit(1);
    }
     printf("[+]Server socket created. \n");

     server_addr.sin_family = AF_INET;
     server_addr.sin_port = htons(port);
     server_addr.sin_addr.s_addr = INADDR_ANY;

     e = bind(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr));
     if(e<0)
     {
         perror("[-]Error in Binding");
         exit(1);
     }
     printf("[+]Binding Successfull.\n");

     e = listen(sockfd, 5);
     if(e==0)
     {
         printf("[+]Listening at port %d\n",port); 
     }
     else 
     {
         perror("[-]Error Listening");
         exit(1);
     }
     addr_size = sizeof(new_addr);
     new_sock = accept(sockfd,(struct sockaddr*)&new_addr, &addr_size);
     write_file(new_sock);
     printf("[+]Data written in the text file.\n");
     
}