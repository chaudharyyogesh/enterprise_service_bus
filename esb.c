//compile: gcc -pthread -lxml2 -o esb.o esb.c `xml2-config --cflags --libs` `mysql_config --cflags --libs`

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

#define SIZE 2024

// typedef struct Queue
// {
//         int capacity;
//         int size;
//         int front;
//         int rear;
//         char **elements;
// }Queue;
// Queue *Q;
// Queue * createQueue(int maxElements)
// {
//         /* Create a Queue */
//         Queue *Q;
//         Q = (Queue *)malloc(sizeof(Queue));
//         /* Initialise its properties */
//         Q->elements = (char**)malloc(sizeof(char*)*maxElements);
//         Q->size = 0;
//         Q->capacity = maxElements;
//         Q->front = 0;
//         Q->rear = -1;
//         /* Return the pointer */
//         return Q;
// }

// void Dequeue(Queue *Q)
// {
//         if(Q->size!=0)
//         {
//                 Q->size--;
//                 Q->front++;
//                 /* As we fill elements in circular fashion */
//                 if(Q->front==Q->capacity)
//                 {
//                         Q->front=0;
//                 }
//         }
//         return;
// }

// char* front(Queue *Q)
// {
//         if(Q->size!=0)
//         {
//                 /* Return the element which is at the front*/
//                 return Q->elements[Q->front];
//         }
//         return NULL;
// }


// void Enqueue(Queue *Q , char *element)
// {
//         //char *p = (char *) malloc(strlen(element)+1);

//         /* If the Queue is full, we cannot push an element into it as there is no space for it.*/
//         if(Q->size == Q->capacity)
//         {
//                 printf("Queue is Full\n");
//         }
//         else
//         {
//                 Q->size++;
//                 Q->rear = Q->rear + 1;
//                 /* As we fill the queue in circular fashion */
//                 if(Q->rear == Q->capacity)
//                 {
//                         Q->rear = 0;
//                 }
//                 /* Insert the element in its rear side */ 

//                 //printf("testing\n");

//                 Q->elements[Q->rear] = (char *) malloc((sizeof element + 1)* sizeof(char));

//                 strcpy(Q->elements[Q->rear], element);
//         }
//         return;
// }

xmlDocPtr load_xml_doc(char *xml_file_path) {
    xmlDocPtr doc = xmlParseFile(xml_file_path);
    if (doc == NULL) {
        fprintf(stderr, "ERROR: Document not parsed successfully. \n");
        return NULL;
    }
    return doc;
}

/**
 * Extract the nodes matching the given xpath from the supplied
 * XML document object.
 */
xmlXPathObjectPtr get_nodes_at_xpath(xmlDocPtr doc, xmlChar *xpath) {

    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    if (context == NULL) {
        printf("ERROR: Failed to create xpath context from the XML document.\n");
        return NULL;
    }
    xmlXPathObjectPtr result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);
    if (result == NULL) {
        printf("ERROR: Failed to evaluate xpath expression.\n");
        return NULL;
    }
    if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        xmlXPathFreeObject(result);
        printf("No matching nodes found at the xpath.\n");
        return NULL;
    }
    return result;
}

/**
 * Returns the text value of an XML element. It is expected that
 * there is only one XML element at the given xpath in the XML.
 */
xmlChar* get_element_text(char *node_xpath, xmlDocPtr doc) {
    xmlChar *node_text;
    xmlXPathObjectPtr result = get_nodes_at_xpath(doc, 
        (xmlChar*)node_xpath);
    if (result) {
        xmlNodeSetPtr nodeset = result->nodesetval;
        if (nodeset->nodeNr == 1) {
            node_text = xmlNodeListGetString(doc,
                nodeset->nodeTab[0]->xmlChildrenNode, 1);
        } else {
            printf("ERROR: Expected one %s node, found %d\n", node_xpath, nodeset->nodeNr);
        }
        xmlXPathFreeObject(result);
    } else {
        printf("ERROR: Node not found at xpath %s\n", node_xpath);
    }
    return node_text;
}

void *write_file(void *new_sock)
{
    printf("[+]Creating and storing BMD File in local.\n");
    int sockfd;
    sockfd=(intptr_t)new_sock;
    int n; 
    FILE *fp;

    char *filename = "received_bmd.xml";
    char buffer[SIZE];

    fp = fopen(filename, "w");
    if(fp!=NULL)
    {
        printf("hi\n");
        bzero(buffer, SIZE);
  
        printf("[+]Reading File.\n");
        n = read(sockfd, buffer, SIZE);
        if(n>0)
        {
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
        fprintf(fp, "%s", xmlcontent);
        printf("[+]File Created Successfully.\n");
        bzero(buffer, SIZE);
        if(fclose(fp)==0){
            printf("[+]Parsing File %s\n",filename);
            char docname[] = "/home/yogesh/Downloads/c_programs/";
            strcat(docname,filename);
            printf("%s\n",docname);
            xmlDocPtr doc = load_xml_doc(docname);
            char *message_id=get_element_text("//MessageID", doc);
            char *sender=get_element_text("//Sender", doc);
            char *destination=get_element_text("//Destination", doc);
            if(message_id!=""&& sender!=""&& destination!="")
            {
                printf("MessageID=%s\n", message_id);
                printf("Sender=%s\n", sender);
                printf("Destination=%s\n", destination);
                printf("MessageType=%s\n", get_element_text("//MessageType", doc));
                printf("CreationDateTime=%s\n", get_element_text("//CreationDateTime", doc));
                printf("Signature=%s\n", get_element_text("//Signature", doc));
                printf("ReferenceID=%s\n", get_element_text("//ReferenceID", doc));
                printf("key1=%s\n", get_element_text("//key1", doc));
                printf("Payload=%s\n", get_element_text("//Payload", doc));

                    //connecting to database esb_db
                    MYSQL *con = mysql_init(NULL);

                if (con != NULL)
                {
                    // MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag);
                if (mysql_real_connect(con, "localhost", "user", "1234",
                        "esb_db", 0, NULL, 0) != NULL)
                {
                    printf("[+]Storing To Table esb_request.\n");
                    char *status="available";
                    char sql_statement[2048];
                    sprintf(sql_statement,"INSERT INTO esb_request(sender_id , dest_id, message_type,reference_id ,message_id ,received_on ,data_location , status,status_details ,processing_attempts) VALUES('%s','%s','%s','%s','%s','%s','%s','%s','empty',0)",sender,destination,get_element_text("//MessageType", doc),get_element_text("//ReferenceID", doc),message_id,get_element_text("//CreationDateTime", doc),docname,status);
                if (mysql_query(con,sql_statement)==0) {
                    printf("[+]Stored Successfully.\n");
                    printf("[+]Inserting into Queue.\n");
                    // Enqueue(Q,message_id);
                    // printf("Front element is %s\n",front(Q));
                    printf("[+]MessageID inserted into queue.\n");
                    
                }
                else{
                    fprintf(stderr, "%s\n", mysql_error(con));
                    mysql_close(con);
                }
                    xmlFreeDoc(doc);
                    xmlCleanupParser();
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
            else{
                perror("[-]Empty bmd parameters.");
            }
            
        }
        else{
            printf("[-]File not closed.");
        }

        
        }
        else{
            perror("[-]Error reading from socket.");
        }
    }
    else{
        perror("[-]Error in creating file.");
    }
    printf("[+]Closing Connection.\n");
    close(sockfd);
    pthread_exit(NULL);
}
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
    printf("[+]Thread created successfully.\n");
    // pthread_exit(NULL);
    return true;
}
void start_server_socket(){
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
         printf("[+]Listening.\n"); 
     }
     else 
     {
         perror("[-]Error in Binding");
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

        printf("[+]Waiting for incoming connection %d.\n",connection_number);
        connection_number++;
        int client_fd = accept(sockfd, (struct sockaddr *) &caddr, &len);  /* accept blocks */

        if (client_fd < 0) {
            printf("[-]accept() failed. Continuing to next.\n"); /* don't terminate, though there's a problem */
            continue;
        }
        /* Start a worker thread to handle the received connection. */
        if (!client_handler_thread(client_fd)) {
            printf("[-]Failed to create thread. Continuing to next.\n");
            continue;
        }else{
            printf("[+]Continuing while.\n");
        }

    }  
}


int main ()
{
    // Q=createQueue(5);
    start_server_socket();
}
