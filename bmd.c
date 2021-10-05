#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <string.h>

#include "bmd.h"

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

BMD* parse_bmd_file(char *bmdfilepath){
    // printf("BMDFILEPATH:%s\n",bmdfilepath);
    xmlDocPtr doc = load_xml_doc(bmdfilepath);

    BMD *parsed_data = malloc(sizeof(BMD));


    parsed_data->envelop.sender_id = get_element_text("//Sender", doc);
    parsed_data->envelop.destination_id = get_element_text("//Destination", doc);
    parsed_data->envelop.message_type = get_element_text("//MessageType", doc);
    parsed_data->envelop.reference_id = get_element_text("//ReferenceID", doc);
    parsed_data->envelop.message_id = get_element_text("//MessageID", doc);
    parsed_data->envelop.signature=get_element_text("//Signature",doc);
    parsed_data->envelop.creation_time=get_element_text("//CreationDateTime",doc);
    parsed_data->payload = get_element_text("//Payload", doc);

    // printf("PAYLOAD::%s\n",parsed_data->payload);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return parsed_data;
}

char* get_payload(char *bmdfilepath){
    xmlDocPtr doc = load_xml_doc(bmdfilepath);
    char *payload=get_element_text("//Payload", doc);
    printf("PAYLOAD:%s\n",payload);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return payload;
}