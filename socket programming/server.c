/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#define SIZE 1024
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
// void write_file(int sockfd)
// {
//     int n; 
//     FILE *fp;
//     char *filename = "file2.xml";
//     char buffer[SIZE];

//     fp = fopen(filename, "w");
//     if(fp==NULL)
//     {
//         perror("[-]Error in creating file.");
//         exit(1);
//     }
//     while(1)
//     {
//     printf("Writing..");
//         n = recv(sockfd, buffer, SIZE, 0);
//         if(n<=0)
//         {
//             printf("end");
//             break;
//             return;
//         }
//         fprintf(fp, "%s", buffer);
//         bzero(buffer, SIZE);
//     }
//     return;
    
// }

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     //This structure is defined in netinet/in.h.
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        {
            error("ERROR opening socket");
        }
    printf("Server socket created.");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              
              {error("ERROR on binding");}
         
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
                 
     if (newsockfd < 0) 
          error("ERROR on accept");
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
    //  write_file(newsockfd);
     close(newsockfd);
     close(sockfd);
     return 0; 
}
