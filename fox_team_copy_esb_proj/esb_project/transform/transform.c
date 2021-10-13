#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <json-c/json.h>
#include "transform.h"


char *xmlName = NULL;
int same = 0;
//initializing the pointers

json_object *JSONroot_object = NULL;

//XML -> JSON BELOW

//checking if any node has atributes
void hasAttribute(xmlNode *currentNode,json_object *jsonRoot){
  json_object *newObj;
  xmlAttr *NodeAttribute = NULL;   //attribute as an object
  xmlChar *attribute;
  for (NodeAttribute = currentNode->properties; NodeAttribute; NodeAttribute = NodeAttribute->next) {
    if (currentNode->type == XML_ELEMENT_NODE) {
      attribute =  xmlNodeGetContent((xmlNode*)NodeAttribute);
      newObj = json_object_new_string((char*)attribute);
      json_object_object_add(jsonRoot,NodeAttribute->name,newObj);
    }
  }
}

//checking for the child element
int isChild(xmlNode *currentNode){
  for (currentNode = currentNode->children; currentNode; currentNode = currentNode->next) {
    if (currentNode->type == XML_ELEMENT_NODE) {
      return 0;
    }
  }
  return 1;
}

//parsing the xml file and storing the data into json object 
void xmlToJSON(xmlNode *a_node, json_object *jsonRoot){
  xmlNode *currentNode = NULL;     //node
  json_object *newObj;
  json_object *newArray;

  char *lastElementName = malloc(100);

  for (currentNode = a_node; currentNode; currentNode = currentNode->next) {
    if (currentNode->type == XML_ELEMENT_NODE) {
      if(isChild(currentNode)){
        newObj = json_object_new_string(currentNode->children->content);
        xmlNode *nextNode = currentNode->next;

        while(nextNode && nextNode->type != XML_ELEMENT_NODE){
          nextNode = nextNode->next;
        }

        if(nextNode && strcmp(currentNode->name,nextNode->name) == 0){
          if(same == 0){
            newArray = json_object_new_array();
            json_object_object_add(jsonRoot,currentNode->name,newArray);
            strcpy(lastElementName, currentNode->name);
          }
          same = 1;
          json_object_array_add(newArray,newObj);
          if(currentNode->properties){
            hasAttribute(currentNode,newObj);
          }
          continue;
        }

        else{
          if(strcmp(lastElementName, currentNode->name) == 0){
            json_object_array_add(newArray,newObj);
          }
          else{
            json_object_object_add(jsonRoot,currentNode->name,newObj);
          }
        }
        if(currentNode->properties){
          hasAttribute(currentNode,newObj);
        }
      }

      else{
        same = 0;
        newObj = json_object_new_object();
        json_object_object_add(jsonRoot,currentNode->name,newObj);
        if(currentNode->properties){
          hasAttribute(currentNode,newObj);
        }
        xmlToJSON(currentNode->children,newObj);

      }
    }
  }
  free(lastElementName);

}

//load and read the xml file
void readXML(char *xmlName, json_object *jsonRoot){
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  xmlNode *last_child = NULL;
  char *Filename = xmlName;
  doc = xmlReadFile(Filename, NULL, 0);

  if(doc == NULL){
    printf("Error: couldn't parse file%s\n", Filename);
  }
  else{
    root_element = xmlDocGetRootElement(doc);
    last_child=xmlLastElementChild(root_element);
    xmlToJSON(last_child,jsonRoot);
    xmlFreeDoc(doc);
  }
  xmlCleanupParser();
}
void parsing(char **parsedInput,char *string, char *parseType){//can be changed, in order of OUTPUT, INPUT, PARSING TYPE
    int i = 0;     //index of output
    char *temp = malloc(strlen(string)+1);
    strcpy(temp,string);
    char *token = strtok(temp, parseType);  //output words from input
    
    while(token != NULL){  
    parsedInput[i] = malloc(strlen(token)+1);//Ensure a memory for storing
    strcpy(parsedInput[i], token);   //indexing
    token = strtok(NULL, parseType);  //splitting
    i++; //next index
  }
}

//takes the file path as input and performs transformation and writes the json object content int a output file
char * transformToJson(char *bmdfilepath){
    char **parsedInput = malloc(100);
    char * path=bmdfilepath;
    parsing(parsedInput,path,"."); 
    char * json_file; //string to store the output filename
    const char * payload;               
    if(strcmp(parsedInput[1],"xml") == 0){  //check if the input file is xml or not
        xmlName = malloc(strlen(path)+1);
        strcpy(xmlName, path);
        JSONroot_object = json_object_new_object();
        readXML(xmlName,JSONroot_object);
        payload=json_object_get_string(JSONroot_object);   //to read json data into string
        json_object_to_file(strcat(strcat(parsedInput[1],"_to_json"),".json"), JSONroot_object);    //write the json content in the file and saving that file
        json_file=parsedInput[1];
        free(xmlName);    
    }
    printf("successfully transformed to json with data....%s....%s\n",payload,json_file);
    return json_file;   //returning json file name 
}


//transformToCSV takes the bmdfile path as the parameter and transforms it to csv format and saves it as .csv file
char* transformToCSV(char *bmdfilepath){
    char **parsedInput = malloc(100);
    char * path=bmdfilepath;
    parsing(parsedInput,path,"."); 
    char *csv_file = "xml_to_csv.csv"; //string to store the output filename
    const char *payload;   
    printf("pasred input1:%s :: %s :: %s\n",parsedInput[1],path,bmdfilepath);            
    if(strcmp(parsedInput[1],"xml") == 0) //check if the input file is xml or not
    {  
        xmlName = malloc(strlen(path)+1);
        strcpy(xmlName, path);
        xmlDoc *doc = NULL;
        xmlNode *root_element = NULL;
        xmlNode *last_child = NULL;
        char *Filename = xmlName;
        doc = xmlReadFile(Filename, NULL, 0);

        if(doc == NULL){
          printf("Error: couldn't parse file%s\n", Filename);
        }
        else{
          root_element = xmlDocGetRootElement(doc);
          last_child=xmlLastElementChild(root_element);
        }
        xmlNode *currentNode = NULL; 
        FILE *fp;
        fp = fopen(csv_file, "w");
        
        for (currentNode = last_child; currentNode; currentNode = currentNode->next) {
          if (currentNode->type == XML_ELEMENT_NODE) {
            if(isChild(currentNode)){
              fprintf(fp, "%s\n", currentNode->name);
              fprintf(fp, "%s\n", currentNode->children->content);
            }
          }
        }
      fclose(fp);
      xmlFreeDoc(doc);
      xmlCleanupParser();
      free(xmlName);
    }   
    printf("successfully transformed to CSV %s\n",csv_file);
    return csv_file;
}


char* transform_to_html(char* bmdfilepath)
{
    xmlDocPtr doc = load_xml_doc(bmdfilepath);
    char* payload ;
    strcpy( payload ,get_element_text("//Payload", doc));
    xmlFreeDoc(doc);
    xmlCleanupParser();
    FILE* fp;
    char filename[100];
    sprintf(filename,"xml_to_html.html");
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
