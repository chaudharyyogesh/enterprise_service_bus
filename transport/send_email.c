// Note: make sure "less secure app access" is turned on for sending the email

#include <curl/curl.h>
#include "../bmd.h"

int send_email(char *receiver_email,char *bmdfilepath)
{
    CURLcode ret;
    CURL *hnd;
    struct curl_slist *recipients;
    FILE *fd;
    
    recipients = NULL;
    recipients = curl_slist_append(recipients, receiver_email);

    char *payload=get_payload(bmdfilepath);//extract the payload from stored bmd file
    // motoeverest8849@gmail.com is the senders email address
    fd=fopen("mail.txt","w");
    fprintf(fd,"From: \"Sender Name\" <motoeverest8849@gmail.com>\nTo: \"Recipient Name\" <%s>\nSubject: This is your subject\n\n%s\n",receiver_email,payload);
    fclose(fd);
    fd = fopen("mail.txt", "rb");
    if (!fd) {return 1;} 
    
    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_INFILESIZE_LARGE, (curl_off_t)179);
    curl_easy_setopt(hnd, CURLOPT_URL, "smtps://smtp.gmail.com:465/mail.txt");
    curl_easy_setopt(hnd, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(hnd, CURLOPT_READDATA, fd); 
    curl_easy_setopt(hnd, CURLOPT_USERPWD, "motoeverest8849@gmail.com:171030959818");//enter your email id and password
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(hnd, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(hnd, CURLOPT_MAIL_FROM, "motoeverest8849@gmail.com");//enter your email id
    curl_easy_setopt(hnd, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);

    ret = curl_easy_perform(hnd);

    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_slist_free_all(recipients);
    recipients = NULL;

    return (int)ret;//returns 0 if success
}