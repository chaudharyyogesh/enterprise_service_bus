#include <stdio.h>
#include "http.h"
#include <curl/curl.h>
#include <json-c/json.h>
#include <string.h>


struct memory{
   char * memory;
   size_t size;
};

//writing the callback function
static size_t writecallback(char * contents,size_t size,size_t nmemb,void *userp){
    size_t realsize=size * nmemb;
    struct memory *mem=(struct memory *)userp;
    char *ptr=realloc(mem->memory,mem->size + realsize +1);
    if(ptr==NULL) return 0;
    mem->memory=ptr;
    memcpy(&mem->memory[mem->size],contents,realsize);
    mem->size+=realsize;
    mem->memory[mem->size]=0;
    printf("%s\n",mem->memory);
    return realsize;
}



int http(char *http_url,char *text)
{   FILE * fp,*f;
    fp=fopen(text,"r");
    int c;
    int i=0;
    //reading one by one character from a file and to count the size of buffer determine to stored the content of the file
    while ( (c=fgetc(fp)) != EOF ) {
        i++;
    }
    fclose(fp);   //close the file after determining the size of buffer needed

    //now we know the size of buffer, so create the buffer of that size and store the content of the file after reading from the file
    f=fopen(text,"r");
    char buffer[i];
    int j=0;
    while ( (c=fgetc(f)) != EOF ) {
        buffer[j]=c;  
        j++;
    }
    char * string=buffer;   //now string contains the content of the file to be uploaded
    fclose(f);   //closing the local file
    
    int status = 0;
    CURL * curl;
    CURLcode res;
    struct memory chunk;
    chunk.memory=NULL;
    chunk.size=0;
    
    curl_global_init(CURL_GLOBAL_ALL); 
    curl = curl_easy_init();
    if (curl) {  
        printf("Response Received.\n");

        curl_easy_setopt(curl, CURLOPT_URL, http_url);    //server's url
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, string);  //post the string
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback); //write the callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
        res = curl_easy_perform(curl);
        
        //checking the Status Code if it is 200 then request is send successfully
        if(res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code >= 200 && response_code <= 299 && response_code != CURLE_ABORTED_BY_CALLBACK)
            {
                   printf("Request sent successfully, http response code is %ld.\n",response_code);
                   status=1;
            }
        }
        
        //error checking
        if(res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
        }
        else{
            printf("we got %d bytes to our callback.\n",(int)chunk.size);
        }
        
        free(chunk.memory);     //free the pointer 
    
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    return status;

}