#include <curl/curl.h>
#include "email.h"

int transport_through_email(char *sender, char *destination, char *path )
{
       char password[45]="171030959818";//enter your email password
       char login_credentials[90];
  	sprintf( login_credentials, "%s:%s", sender, password );
  	CURLcode ret;
    	CURL *hnd;
    	struct curl_slist *recipients;

    	FILE *fd;

		// printf("\npath:%s\n",path);
		char *payload=get_payload(path);//extract the payload from stored bmd file
    // motoeverest8849@gmail.com is the senders email address
    fd=fopen("mail.txt","w");
    fprintf(fd,"From: \"Sender Name\" <%s>\nTo: \"Recipient Name\" <%s>\nSubject: This is your subject\n\n%s\n",sender,destination,payload);
    fclose(fd);
    
    	recipients = NULL;
    	recipients = curl_slist_append(recipients, destination);
    
    	fd = fopen("email.txt", "rb");
    	if (!fd) {printf("error during opening file");} 

    	hnd = curl_easy_init();
    	curl_easy_setopt(hnd, CURLOPT_INFILESIZE_LARGE, (curl_off_t)179);
    	curl_easy_setopt(hnd, CURLOPT_URL, "smtps://smtp.gmail.com:465/email.txt");
    	curl_easy_setopt(hnd, CURLOPT_UPLOAD, 1L);
    	curl_easy_setopt(hnd, CURLOPT_READDATA, fd); 
    	curl_easy_setopt(hnd, CURLOPT_USERPWD, login_credentials );
    	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
    	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
    	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
    	curl_easy_setopt(hnd, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    	curl_easy_setopt(hnd, CURLOPT_MAIL_FROM, sender );
    	curl_easy_setopt(hnd, CURLOPT_MAIL_RCPT, recipients);
    	curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);

    	ret = curl_easy_perform(hnd);

    	curl_easy_cleanup(hnd);
    	hnd = NULL;
    	curl_slist_free_all(recipients);
        recipients = NULL;
    	return (int)ret ;
}

			
