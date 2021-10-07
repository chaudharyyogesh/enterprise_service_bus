#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"transform.h"
#include"../bmd.h"

char* transform_xml_to_json(char* bmdfilepath)
{

    char* payload = get_payload(bmdfilepath);
    char filename[100];
    sprintf(filename,"transformedjson.json");
    FILE *fp;
    fp = fopen(filename,"w");
    if(fp == NULL) 
    {
       printf("[-]File opening failed\n");
       exit(0);
    }
    fprintf(fp,"{\n \"Payload\":\"%s\"\n}",payload);
    
    fclose(fp);
    return strdup(filename);
}

char* transform_xml_to_csv(char* bmdfilepath)
{
    printf("%s\n",bmdfilepath);
    char* payload = get_payload(bmdfilepath);
    FILE * fp;
    char filename[100];
    sprintf(filename,"transformedcsv.csv");
    fp = fopen(filename,"w"); 
     if(fp == NULL) 
    {
       printf("[-]File opening failed\n");
       exit(0);
    }
    fprintf(fp,"Payload\n");
    fprintf(fp,"%s\n",payload);
    fclose(fp);
    return strdup(filename);
}


char* transform_xml_to_html(char* bmdfilepath)
{
    char* payload = get_payload(bmdfilepath);
    FILE* fp;
    char filename[100];
    sprintf(filename,"transformedhtml.html");
    fp = fopen(filename,"w");
     if(fp == NULL) 
    {
       printf("[-]File opening failed\n");
       exit(0);
    }
    fprintf(fp,"<!DOCTYPE html>\n");
    fprintf(fp,"    <html>\n");
    fprintf(fp,"        <head>\n");
    fprintf(fp,"            <meta charset='UTF-8'>\n");
    fprintf(fp,"                <title>XML To HTML Conversion</title>\n");
    fprintf(fp,"        </head>\n");
    fprintf(fp,"    <body>\n");
    fprintf(fp,"        <table border=1>\n");
    fprintf(fp,"            <thead>\n");
    fprintf(fp,"                <tr>\n");
    fprintf(fp,"                    <th>Payload</th>\n");
    fprintf(fp,"                </tr>\n");
    fprintf(fp,"            </thead>\n");
    fprintf(fp,"            <tbody>\n");
    fprintf(fp,"                <tr>\n");
    fprintf(fp,"                    <td>%s</td>\n",payload);
    fprintf(fp,"                </tr>\n");
    fprintf(fp,"            </tbody>\n");
    fprintf(fp,"        </table>\n");
    fprintf(fp,"    </body>\n");
    fprintf(fp,"</html>\n");
    fclose(fp);
    return strdup(filename);
}




char* transform(char *key,char* bmdfilepath)
{
  char filepath[300]= "/home/yogesh/Downloads/nho2021/Goat/"; //working directory path where all files will be stored(change with your working directory)
  char *filename;
  if(strcmp(key,"xml")==0)
  {
      printf("[+]No transformation needed\n");
      return bmdfilepath;
  }

  else if(strcmp(key,"json")==0)
  {
    printf("[+]Transforming XML to JSON\n");
    filename = transform_xml_to_json(bmdfilepath);
  }

  else if(strcmp(key,"html")==0)
  {
    printf("[+]Transforming XML TO HTML\n");
    filename = transform_xml_to_html(bmdfilepath);   
  }
  
  else if(strcmp(key,"csv")==0)
  {
    printf("[+]Transforming XML to CSV\n");
    filename = transform_xml_to_csv(bmdfilepath);
    
  }
    //concat the path with the filename created
  strcat(filepath,filename); 
  return strdup(filepath);
}