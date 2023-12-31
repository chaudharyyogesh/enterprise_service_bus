//compile: gcc -pthread -lxml2 -o esb.o esb.c bmd.c ./transform/transform.c ./transport/send_email.c `xml2-config --cflags --libs` `mysql_config --cflags --libs` -lcurl
// send client request: curl -X POST -d @/bmdfilepath http://localhost:8000


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<stdbool.h>
#include<pthread.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <mysql/mysql.h>
#include <curl/curl.h>
#include "bmd.h"
#include "./transport/send_email.h"
#include "./transform/transform.h"

#define SIZE 2024

/* the write_file function

1. stores the received BMD request file in the local system.
2. parses the file
3. checks if the BMD file is valid (i.e. checks for the mandatory fields, for eg. sender, destination,signature, message_id etc)
4. prints out the extracted fields from the BMD (given if the BMD request file is valid)
4. inserts the relevant data in the esb_request table

*/

void *write_file(void *new_sock)
{
    printf("[+]Creating and storing BMD File in local.\n");
    int sockfd;
    sockfd=(intptr_t)new_sock;
    int n; 
    FILE *fp;

    char *filename = "received_bmd.xml"; //file to store received bmd file locally
    char buffer[SIZE];

    fp = fopen(filename, "w");
    if(fp!=NULL)
    {
        bzero(buffer, SIZE);
  
        printf("[+]Reading BMD file into buffer.\n");
        n = read(sockfd, buffer, SIZE); //reads data into buffer
        if(n>0)
        {
            //extracting the bmd from buffer
            int start=0,end=strlen(buffer);
            for(int i=0;buffer[i]!='\0';i++)
            {
                if(buffer[i]=='<')
                {
                    start=i+1;
                    break;
                }
            }
            char xmlcontent[2000];
            int length=end-start+1,c=0;
            while (c < length) {
            xmlcontent[c] = buffer[start+c-1];
            c++;
            }
            xmlcontent[c] = '\0';
            //storing extracted bmd into file
            fprintf(fp, "%s", xmlcontent);
            printf("[+]BMD file stored Successfully.\n");
            bzero(buffer, SIZE);
            if(fclose(fp)==0){
                printf("[+]Parsing BMD File %s\n",filename);
                char bmdfilepath[] = "/home/yogesh/Downloads/nho2021/Goat/"; //local path of the bmd file received_bmd.xml
                strcat(bmdfilepath,filename);
                
                // parsing the bmd file passing bmdfilepath as parameter i.e. /home/yogesh/Downloads/nho2021/Goat/received_bmd.xml
                BMD *bmd=parse_bmd_file(bmdfilepath);
                // printf("Payload::%s\n",bmd->payload);
                
                if(is_bmd_valid(bmd))   //checks for values for parsed bmd is not empty
                {
                    printf("MessageID=%s\n", bmd->envelop.message_id);
                    printf("Sender=%s\n", bmd->envelop.sender_id);
                    printf("Destination=%s\n", bmd->envelop.destination_id);
                    printf("MessageType=%s\n",bmd->envelop.message_type);
                    printf("CreationDateTime=%s\n",bmd->envelop.creation_time);
                    printf("Signature=%s\n", bmd->envelop.signature);
                    printf("ReferenceID=%s\n", bmd->envelop.reference_id);
                    printf("Payload=%s\n", bmd->payload);

                    //connecting to database esb_db
                    MYSQL *con = mysql_init(NULL);

                    if (con != NULL)
                    {
                        // MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag);
                        if (mysql_real_connect(con, "localhost", "user", "1234",
                                "esb_db", 0, NULL, 0) != NULL)
                        {
                            printf("[+]Storing parsed values to table esb_request.\n");
                            char *status="available";
                            char sql_statement[2048];
                            sprintf(sql_statement,"INSERT INTO esb_request(sender_id , dest_id, message_type,reference_id ,message_id ,received_on ,data_location , status,status_details ,processing_attempts) VALUES('%s','%s','%s','%s','%s','%s','%s','%s','empty',0)",bmd->envelop.sender_id,bmd->envelop.destination_id,bmd->envelop.message_type,bmd->envelop.reference_id,bmd->envelop.message_id,bmd->envelop.creation_time,bmdfilepath,status);
                            if (mysql_query(con,sql_statement)==0) {
                                printf("[+]Stored Successfully in table esb_request.\n");
                            }
                            else{
                                fprintf(stderr, "%s\n", mysql_error(con));
                            }
                        
                            mysql_close(con);

                        
                        }else{
                            fprintf(stderr, "%s\n", mysql_error(con));
                            mysql_close(con);
                        }
                    
                    }
                    else{
                        fprintf(stderr, "%s\n", mysql_error(con));   
                    }
                    
                }
            }
            else{
                printf("[-]File not closed.\n");
            }
        }
        else{
            perror("[-]Error reading from socket.\n");
        }
    }
    else{
        perror("[-]Error in creating bmd file.\n");
    }
    printf("[+]Closing Connection.\n");
    close(sockfd);
    pthread_exit(NULL);
}

/* the client_handler_thread function

1. creates a client handler thread that invokes the write_file function
2. returns true if the thread was created succesfully, else returns false

*/
bool client_handler_thread(int sock_fd) {
    printf("[+]Creating a client handler thread.\n");
    pthread_t thr_id;
    int rc = pthread_create(&thr_id,
            /* Attributes of the new thread, if any. */
                            NULL,
            /* Pointer to the function which will be
             * executed in new thread. */
                            write_file,
            /* Argument to be passed to the above
             * thread function. */
                            (void *) (intptr_t) sock_fd);
    if (rc) {
        printf("[-]Failed to create thread.");
        return false;
    }
    printf("[+]Client handler thread created successfully.\n");
    return true;
}

/* the start_server_socket function
1. creates a server socket
2. bind the socket
3. starts listening for the client requests
4. invokes the client_handler_thread function

*/
void start_server_socket(){
    int port = 8000;
    int e;

    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
    {
        perror("[-]Error creating socket\n");
        exit(1);
    }
     printf("[+]Server socket created. \n");

     server_addr.sin_family = AF_INET;
     server_addr.sin_port = htons(port);
     server_addr.sin_addr.s_addr = INADDR_ANY;

     e = bind(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr));
     if(e<0)
     {
         perror("[-]Error Binding socket.\n");
         exit(1);
     }
     printf("[+]Binding Successfull.\n");

     e = listen(sockfd, 5);
     if(e==0)
     {
         printf("[+]Listening.\n"); 
     }
     else 
     {
         perror("[-]Error in Listening.\n");
         exit(1);
     }
    //  addr_size = sizeof(new_addr);
    //  new_sock = accept(sockfd,(struct sockaddr*)&new_addr, &addr_size);
    //   if (!client_handler_thread(new_sock)) {
    //         printf("[-]Failed to create client thread.\n");
    //         return;
    //     }
    int connection_number=1;
        while (1) {
        struct sockaddr_in caddr; /* client address */
        int len = sizeof(caddr);  /* address length could change */

        printf("\n[+]Waiting for incoming connection %d.\n\n",connection_number);
        connection_number++;
        int client_fd = accept(sockfd, (struct sockaddr *) &caddr, &len);  /* accept blocks */

        if (client_fd < 0) {
            printf("[-]accept() failed. Continuing to next.\n"); /* don't terminate, though there's a problem */
            continue;
        }
        /* Start a worker thread to handle the received connection. */
        if (!client_handler_thread(client_fd)) {
            printf("[-]Failed to create client thread. Continuing to next.\n");
            continue;
        }else{
            printf("[+]Continuing Listening.\n");
        }
    }  
}

/* the query_queue function
1. creates a database connection
2. queries the database for any available requests continuously
3. if request is available fetches details from transform_config table and then transforms the payload accordingly
4. fetches details from transport_config table and then transports the payload accordingly
5. marks status as 'done' if success or 'failed' if failure

*/
void *query_queue(){
    printf("[+]Quering queue started.\n");
    
         MYSQL *con = mysql_init(NULL);

                if (con != NULL)
                {
                    // MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag);
                    if (mysql_real_connect(con, "localhost", "user", "1234",
                            "esb_db", 0, NULL, 0) != NULL)
                            {
                                printf("[+]Quering Database for request.\n");
                                while(1){
                                    //continuously queries the esb-request table for available request
                                  if (mysql_query(con, "SELECT message_id,sender_id,dest_id,data_location FROM esb_request WHERE status='available'")==0)
                                    {
                                       MYSQL_RES *result = mysql_store_result(con);
                                       if (result != NULL)
                                    {
                                        char message_id[45];
                                        char sender_id[45];
                                        char dest_id[45];
                                        char bmdfilepath[50];
                                        
                                        int num_fields = mysql_num_fields(result);

                                        MYSQL_ROW row;

                                        while ((row = mysql_fetch_row(result)))
                                        {
                                            // for(int i = 0; i < num_fields; i++)
                                            // {
                                            //     printf("%s ", row[i] ? row[i] : "NULL");
                                            // }
                                            printf("[+]Processing queue started.\n");
                                            printf("MESSAGE_ID: %s\n",row[0]);
                                            printf("SENDER_ID: %s\n",row[1]);
                                            printf("DESTINATION_ID: %s\n",row[2]);
                                            strcpy(message_id,row[0]);
                                            strcpy(sender_id,row[1]);
                                            strcpy(dest_id,row[2]);
                                            strcpy(bmdfilepath,row[3]);
                                            // printf("This is the filepath::%s\n",bmdfilepath);
                                            mysql_free_result(result);
                                            char sql_query[200];
                                            sprintf(sql_query,"SELECT route_id FROM routes WHERE sender='%s' AND destination='%s'",sender_id,dest_id);
                                            bool processing_complete=true;
                                            if(mysql_query(con,sql_query)==0){
                                                MYSQL_RES *res = mysql_store_result(con);
                                                int route_id;
                                                if(res!=NULL)
                                                {
                                                    MYSQL_ROW r;
                                                    r=mysql_fetch_row(res);
                                                    route_id=atoi(r[0]);
                                                    mysql_free_result(res);

                                                    sprintf(sql_query,"SELECT config_key,config_value FROM transform_config WHERE route_id=%d",route_id);
                                                    char filepath[300];
                                                    if(mysql_query(con,sql_query)==0){
                                                        MYSQL_RES *transform_result = mysql_store_result(con);
                                                        if(transform_result!=NULL)
                                                        {
                                                            MYSQL_ROW r;
                                                            r=mysql_fetch_row(transform_result);
                                                            mysql_free_result(transform_result);
                                                            char config_key[10];
                                                            strcpy(config_key,r[0]);
                                                            strcpy(filepath,transform(config_key,bmdfilepath));//call transform function where config_key is the transformation type
                                                            printf("File stored at location:%s\n",filepath);

                                                            sprintf(sql_query,"SELECT config_key,config_value FROM transport_config WHERE route_id=%d",route_id);
                                                            if(mysql_query(con,sql_query)==0){
                                                                MYSQL_RES *transport_result = mysql_store_result(con);
                                                                if(transport_result!=NULL)
                                                                {
                                                                    MYSQL_ROW r;
                                                                    r=mysql_fetch_row(transport_result);
                                                                    mysql_free_result(transport_result);
                                                                    char config_key[10];
                                                                    strcpy(config_key,r[0]);
                                                                    if(strcmp(config_key,"email")==0) //if the transport is via email
                                                                    {
                                                                        char receiver_email[45];
                                                                        strcpy(receiver_email,r[1]);
                                                                        printf("[+]Sending email to %s.\n",receiver_email);
                                                                        if(send_email(receiver_email,filepath)==0){
                                                                            printf("[+]Email sent to %s\n",receiver_email);
                                                                        }else{
                                                                            processing_complete=false;
                                                                            printf("[-]Email not sent.\n");
                                                                        }
                                                                    }
                                                                    //work to be done:implement same as email for http and ftp
                                                                }
                                                            }else{
                                                                processing_complete=false;
                                                                fprintf(stderr, "%s\n", mysql_error(con));
                                                            }
                                                        }
                                                    }else{
                                                        processing_complete=false;
                                                        fprintf(stderr, "%s\n", mysql_error(con));
                                                    }

                                                }

                                            }else{
                                                processing_complete=false;
                                                printf("[-]Error fetching route_id.\n");
                                            }
                                            sprintf(sql_query,"UPDATE esb_request SET status='done' WHERE message_id='%s'",message_id);
                                            if(mysql_query(con,sql_query)==0 && processing_complete==true){
                                                printf("[+]Status Updated to done.\n"); //status updated from available to done indicating processing finished
                                            }else{
                                                printf("Processing Failed.\n");
                                                sprintf(sql_query,"UPDATE esb_request SET status='failed' WHERE message_id='%s'",message_id);
                                                if(mysql_query(con,sql_query)==0){
                                                    printf("[+]Status Updated to failed.\n"); //status updated from available to failed indicating processing failed
                                                }
                                            }
                                            //sleep 10 seconds
                                            sleep(10);
                                        }
                                    }
                                    }
                                    else{
                                        printf("[-]%s\n",mysql_error(con));
                                    }
                                }     
                            }else{
                                printf("[-]%s\n",mysql_error(con));
                                mysql_close(con);
                            }
                }else{
                    printf("[-]Connection error mysql_init().\n");
                    }
                    
    
}
void start_query_queue(){
    pthread_t thread;
    int rc=pthread_create(&thread,NULL,query_queue,NULL);
    if(rc){
        printf("[-]failed to create query_queue thread.\n");
        return;
    }
    printf("[+]query_queue thread created successfully.\n");
}


int main ()
{
    start_query_queue();
    start_server_socket();
}
