
typedef struct {
    char* sender_id;
    char* destination_id;
    char* message_type;
    char* reference_id;
    char* message_id;
    char* signature;
    char* creation_time;    
} bmd_envelop;

typedef struct {
    bmd_envelop envelop;
    char* payload;
} BMD;

 BMD* parse_bmd_file(char *bmdfilepath);
 char* get_payload(char *bmdfilepath);