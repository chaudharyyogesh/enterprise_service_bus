// gcc -pthread -lxml2 -o esb_app esb_app.c `xml2-config --cflags --libs` `mysql_config --cflags --libs` -ljson-c -lcurl
//curl -X POST -d@/home/yogesh/Downloads/nho2021/Goat/bmd/bmd_email.xml http://localhost:8001



/*
Corrections made on fox team code by goat team
1.In queue.c changed the key size to store the message id
2. In worker thread, messageID in where clause was not taking a varchar value
3. email not sending payload instead sending predefined body
4. testing not done by fox team
5. no proper documentation provided

*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<stdbool.h>
#include<arpa/inet.h>
#include<libxml/parser.h>
#include<libxml/xpath.h>
#include<mysql.h>
#include <curl/curl.h>

#include "./queue/queue.c"
#include "./xml_parser/xml_parser.c"
#include "./transform/transform.c"
#include "./email/email.c"
#include "./http/http.c"
#include "./ftp/ftp.c"

#define SIZE 16384
int i=0;

/* the finish_with_error function prints a desired
    error message 
*/

void finish_with_error(MYSQL *con){
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}

/* the request_handler function

1. creates a file to store the contents of the received BMD request file.

2. fetches the file from the local system, parses the file and stores the mandatory
   credentials of the BMD request file.
    
3. performs an authentication check on the received BMD file.

4. inserts the relevant attributes into the esb_request table, given if the 
    request is valid.
    
5. appends the messageID in a queue

*/

void *request_handler(void *socket){
    int sockfd; 
    sockfd = (intptr_t)socket;
    int n;
    
    //Creating local file to store the received bmd file from client
    FILE *fp;
    char str[200];
    i++;
    sprintf(str, "/home/yogesh/Downloads/nho2021/Goat/fox_team_copy_esb_proj/esb_project/received_bmd_files/received%d.xml", i); //location to store the bmd file
    char buffer[SIZE];

    fp = fopen(str, "w");
    if(fp==NULL){ perror(">> Error in creating file."); exit(1); }
    bzero(buffer, SIZE);
  
    printf(">>> Reading and Creating File.\n");
    n = read(sockfd, buffer, SIZE);//read the data sent from client and store it in the buffer
    if(n<=0){
        perror("[-]error in reading");
    }
    // seperating the bmd xml portion from buffer 
    int start=0,end=strlen(buffer);
    for(int i=0;buffer[i]!='\0';i++){
        if(buffer[i]=='<'){
            start=i+1;
            break;
        }
    }
    char xmlcontent[SIZE];
    int length=end-start+1,c=0;
    while (c < length) {
      xmlcontent[c] = buffer[start+c-1];
      c++;
    }
    xmlcontent[c] = '\0';
        
    fprintf(fp, "%s", xmlcontent);//writing the bmd xml in the file
    printf(">>> File Created Successfully.\n");
    bzero(buffer, SIZE);
    if(fclose(fp)!=0){
   	printf("[-]File not closed.");
        exit(-1);
    }

    // parsing the received bmd file
    printf(">>> Parsing the File %s\n\n",str);
    //char docname[] = "/home/rahul/Desktop/Programming/esb_proj/";
    //strcat(docname,filename);
    
    //putting various elements of xml document in variables
    printf("**PARSED BMD VALUES**\n");
    xmlDocPtr doc = load_xml_doc(str);
    xmlChar* MessageID = get_element_text("//MessageID", doc);
    xmlChar* SenderID = get_element_text("//Sender", doc);
    xmlChar* DestinationID = get_element_text("//Destination", doc);
    xmlChar* MessageType = get_element_text("//MessageType", doc);
    xmlChar* CreationDateTime = get_element_text("//CreationDateTime", doc);
    xmlChar* Signature = get_element_text("//Signature", doc);
    xmlChar* ReferenceID = get_element_text("//ReferenceID", doc);
    xmlChar* Key1 = get_element_text("//key1", doc);
    xmlChar* Payload = get_element_text("//Payload", doc);
    
    //Printing xml content on screen
    printf("MessageID: %s\n", MessageID);
    printf("SenderID: %s\n", SenderID);
    printf("DestinationID: %s \n", DestinationID);
    printf("MessageType: %s \n", MessageType);
    printf("CreationDateTime: %s \n", CreationDateTime);
    printf("Signature: %s \n", Signature);
    printf("ReferenceID: %s \n", ReferenceID);
    printf("key1: %s\n", Key1);
    printf("Payload: %s\n", Payload );
    
    if( is_bmd_valid(MessageID,SenderID,DestinationID,MessageType)==0 )//checks if the parsed bmd is valid or not
    {
      printf(">> Invalid bmd input request.\n");
    } 
    else
    {
        //Storing message ID of xml file in queue
        char message_id[45];
    	strcpy( message_id,MessageID );
    	enQueue(q,message_id);
    	printf(">>> MessageID %s ",q->front->key);
    	printf("is inserted into queue.\n");
    	printf(">>> Status of request: %s\n", q->rear->status );
     	printf(">>> Number_of_attempts: %d\n", q->rear->number_of_attempts );

     	//Inializing mysql server
     	MYSQL *con = mysql_init(NULL);
    	if (con == NULL){
      		fprintf(stderr, "%s\n", mysql_error(con));
      		exit(1);
    	}
        printf(">>> Database connection created.\n");
    
    	//connecting to mysql server
    	if (mysql_real_connect(con, "localhost", "user", "1234","esb_db", 0, NULL, 0) == NULL){
      		finish_with_error(con);
    	}
    	printf(">>> Storing xml file content in table esb_request.\n");

    	char status[]="available";
    	char sql_statement[2048];
    
    	//Inserting xml file values into SQL database into esb_request table
    	sprintf(sql_statement,"INSERT INTO esb_request(sender_id , dest_id, message_type,reference_id ,message_id ,received_on ,data_location , status,status_details) VALUES('%s','%s','%s','%s','%s','%s','%s','%s','empty')",SenderID,DestinationID,MessageType,ReferenceID,MessageID,CreationDateTime,str,status);
    	if (mysql_query(con,sql_statement)) {
      		finish_with_error(con);
    	}
    	xmlFreeDoc(doc);
    	xmlCleanupParser();
    	mysql_close(con);
    	printf(">>> Stored Successfully into database.\n");
    
    }
    // Connection closed
    close(sockfd);
    printf(">>> Client connection closed successfully.\n");
}

/* the worker_thread function

1. Checks for an active(pending) BMD request by searching the messageID
    in the queue.

2. in the case of a valid pending request,pulls the relevant contents 
    from the esb_request table

3. retrieves the tranform and tranport key from the transform_config and
    tranport_config table respectively.

4. applies the necessary transformation and send it to the desired destination
5. sleeps for 10 secs, given if there is no pending request in the queue.

*/
void *worker_thread(){
    while(1){
        // sleeps for 10 secs
    	sleep(10);
 	printf(">>> Worker thread is working independentely.\n");
 		
 	while( q->front != NULL ){
 	    strcpy( q->front->status, "Processing" );
            printf(">>> Status: %s\n",q->front->status);
	    char sender_info[45], destination_info[45], transport_key[45], transform_key[45], data_location[145],transport_value[45];
		
		//Inializing mysql server
     		MYSQL *con = mysql_init(NULL);
    		if (con == NULL){
      			fprintf(stderr, "%s\n", mysql_error(con));
      			exit(1);
    		}
    		
		//connecting with database esb_proj
		if( mysql_real_connect(con, "localhost", "user", "1234","esb_db", 0, NULL, 0) == NULL){
      		finish_with_error( con );
      		}
      		
      		//Query to retrieve sender_id, destination_id from esb_request table
              printf("\nMessageID: %s\n\n",q->front->key);
      		char sql_query1[250] = "select sender_id, dest_id, data_location from esb_request where message_id='";
      		strcat( sql_query1, q->front->key );
      		strcat( sql_query1, "';" );
            //   printf("\n%s\n\n",sql_query1);
      		if( mysql_query( con, sql_query1 )){ finish_with_error( con ); }
      		MYSQL_RES *result = mysql_store_result(con);
  		if( result == NULL ){ finish_with_error(con); }
  		int num_fields = mysql_num_fields(result);
  		MYSQL_ROW row;
  		while( row = mysql_fetch_row(result) ){
 			strcpy( sender_info, row[0] );
 			strcpy( destination_info, row[1] );
 			strcpy( data_location, row[2] );
  		}
  		mysql_free_result(result);
      		
      		//Query to retrieve transform_key, transport_key from tables transport_config, transform_config using routes
      		char sql_query2[1000] = "SELECT transport_config.config_key,transport_config.config_value, transform_config.config_key FROM routes INNER JOIN transport_config ON transport_config.route_id=routes.route_id INNER JOIN transform_config ON transform_config.route_id = routes.route_id WHERE routes.sender='";
      		strcat( sql_query2, sender_info );
      		strcat( sql_query2, "' AND routes.destination='");
      		strcat( sql_query2, destination_info );
      		strcat( sql_query2, "';" );
      		if( mysql_query( con, sql_query2 )){ finish_with_error( con ); }
      		result = mysql_store_result(con);
  		if( result == NULL ){ finish_with_error(con); }
  		while( row = mysql_fetch_row(result) ){
 			strcpy( transport_key, row[0] );
             strcpy( transport_value, row[1] );
 			strcpy( transform_key, row[2] );
  		} 
  		printf( "SenderID:%s\nDestinationID:%s\nTransportKey:%s\nTransformKey:%s\nTransport_value:%s\n", sender_info, destination_info, transport_key, transform_key, transport_value );
  		mysql_free_result(result);
  		
  		
  		//Transform the payload to desired format
  		char transform_status[20] = "FAILURE" ;
        char transformed_file[30];//the file that stores the transformed format i.e. csv,json,html

              printf("\n>>>Transforming.\n");
  		if( strcmp(transform_key, "json") == 0 ){
            strcpy(transformed_file, transformToJson( data_location));
  			if( strcmp(transformed_file,"xml_to_json.json")==0 ) strcpy( transform_status, "SUCCESS" );
  		} 
          else if( strcmp(transform_key, "csv") == 0 ){
            strcpy(transformed_file, transformToCSV( data_location));
  			if(strcmp(transformed_file,"xml_to_csv.csv")==0 ) strcpy( transform_status, "SUCCESS" );

  		} 
        else if( strcmp(transform_key, "html") == 0 ){
  			strcpy(transformed_file, transform_to_html(data_location)); 
            if(strcmp(transformed_file,"xml_to_html.html")==0)
            strcpy( transform_status, "SUCCESS" );
        }
        else if( strcmp(transform_key, "xml") == 0 ){
  			printf( ">>> Transformation of desired file is not needed.\n" );strcpy( transform_status, "SUCCESS" );
        }
        else{
  			printf( ">> Transformation of desired file is not supported.\n" );
  		}
  		
  		//Transport the payload to desired destination
  		char transport_status[20] = "FAILURE";
  		if( strcmp(transform_status, "SUCCESS") == 0 ){ 
              printf("\n>>> Transporting.\n");
  			if( strcmp(transport_key, "email") == 0 ){
                  printf("Sending email to %s\n",transport_value);
  				if( transport_through_email( "motoeverest8849@gmail.com", transport_value,data_location ) == 0 ) strcpy( transport_status, "SUCCESS");
  			 }
            else if(strcmp(transport_key,"http")==0)
            {
               if(http(transport_value, "xml_to_csv.csv")) strcpy( transport_status, "SUCCESS");;
            }
            else if(strcmp(transport_key,"ftp")==0)
            {
               if(send_ftp_file(transport_value, "xml_to_json.json")) strcpy( transport_status, "SUCCESS");;
            }
            else{
  				printf("No destination\n");
  			}
  		}
  		
  		if( strcmp(transform_status, "SUCCESS") == 0 && strcmp(transport_status, "SUCCESS") == 0 ){
            char sql_query3[1000];
            sprintf(sql_query3,"UPDATE esb_request SET status='done' WHERE message_id='%s'",q->front->key);
            if(mysql_query(con,sql_query3)==0){
                printf(">>> SUCCESS\n");
                printf(">>> Status Updated to done.\n"); //status updated from available to done indicating processing finished
            }else
            {
                printf("Processing Failed.\n");
                sprintf(sql_query3,"UPDATE esb_request SET status='failed' WHERE message_id='%s'",q->front->key);
                if(mysql_query(con,sql_query3)==0){
                    printf(">>> Status Updated to failed.\n"); //status updated from available to failed indicating processing failed
                }
            }
  			
  		}
        mysql_close(con);
  		deQueue(q);
	    }
	printf("\n");
	}
}

/*  main function
 1. entry point of the esb
 2. creates a server socket(port no 8001)
 4. listens for client request continuously
 3. starts the request handler and worker thread 
*/

int main ()
{
    q = createQueue();
    int result;
    
    //Create a socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket<0)
    {
        perror(">>Error in socket");
        exit(1);
    }
     printf(">>> Server socket created. \n");
     
     //Define the address
     struct sockaddr_in server_addr;
     server_addr.sin_family = AF_INET;
     server_addr.sin_port = htons(8001);
     server_addr.sin_addr.s_addr = INADDR_ANY;

     //Bind the server to a specified address and port
     result = bind( server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr) );
     if(result<0){
         perror(">> Error in Binding");
         exit(1);
     }printf(">>> Binding Successfull.\n");

     //Listening
     result = listen(server_socket, 5);
     if(result==0){
         printf(">>> Server listening at port 8001.\n"); 
     }else {
         perror(">> Error occur during listening.\n");
         exit(1);
     }
     
     int client_socket;
     bool flag = false;
     while(true){
       //Accepting client request
       client_socket = accept(server_socket,NULL,NULL);
       pthread_t thread1,thread2;
       int rc = pthread_create( &thread1, NULL, request_handler, (void *) (intptr_t) client_socket );
       if (rc) printf(">> Failed to create thread1."); else printf(">>> Request handler thread created successfully.\n");
       if(!flag){
       int rc1 = pthread_create( &thread2, NULL, worker_thread, NULL);
       if (rc1) printf(">> Failed to create thread2."); else printf(">>> Worker thread created successfully.\n");
       flag=true;
       }
       //pthread_exit(NULL); 
     }
     
    return 0;
}
