#ifndef _NPC_H_
#define _NPC_H_


typedef enum
{
    CFUnavailable = 0,
    CFBusy,
    CFNoReply,
    CFUnconditional
}CFReason_E;

typedef enum 
{
    SVC_OK = 0,             /*no error*/
    SVC0001,                /*internal error*/
    SVC0002,                /*invalid input value*/
    SVC0005,                /*NE expired*/
    /*CF related*/
    SVC0601,                /*mandatory input data missed*/
    SVC0602,                /*unable to process the request*/
    SVC0603,                /*resource limitation*/
    SVC0604,                /*security policy required*/
    SVC0605,                /*HLR responds error*/
    SVC0606,                /*HLR responds unknown CLI error*/
    SVC0607,                /*HLR responds error*/
    SVC0608,                /*HLR responds error*/
    SVC0609,                /*HLR user error*/
    /*MCN related*/
    SVC0801,                /*mandatory input data missed*/
    SVC0802,                /*unable to process the request*/
    SVC0803,                /*resource limitation*/
    SVC0804,                /*security policy required*/
    SVC0805,                /*MCA entry time out*/
    SVC0806,                /*interrogateMCA failure*/
    SVC0807,                /*another update under elaboration*/
    SVC0810,                /*MCA session error*/
    SVC0811,                /*MCA validation error*/
    SVC0812,                /*MCA recoverable error*/
    SVC0813,                /*MCA purged error*/
    SVC0814                 /*MCA system exceptions*/
}NPC_Exception_E;

typedef struct ActivateCFRequest_t ActivateCFRequest;
ActivateCFRequest *ActivateCFRequest_new(void);
void ActivateCFRequest_free(ActivateCFRequest*);
void ActivateCFRequest_set_customerID(ActivateCFRequest*, const char*);
void ActivateCFRequest_set_ToNumber(ActivateCFRequest*, const char*);
void ActivateCFRequest_get_ToNumber(ActivateCFRequest *s, char *n);
void ActivateCFRequest_set_Reason(ActivateCFRequest*, const CFReason_E);
CFReason_E ActivateCFRequest_get_Reason(ActivateCFRequest *s);
void ActivateCFRequest_set_NRtime(ActivateCFRequest*, const unsigned long);
unsigned long ActivateCFRequest_get_NRtime(ActivateCFRequest *s);
NPC_Exception_E activate_CF(ActivateCFRequest*);
NPC_Exception_E deactivate_CF(ActivateCFRequest*);
NPC_Exception_E interrogate_CF(ActivateCFRequest*);

typedef struct ActivateMCNRequest_t ActivateMCNRequest;
ActivateMCNRequest *ActivateMCNRequest_new(void);
void ActivateMCNRequest_free(ActivateMCNRequest*);
void ActivateMCNRequest_set_customerID(ActivateMCNRequest*, const char*);
void ActivateMCNRequest_set_MCNStatus(ActivateMCNRequest*, const int);
int ActivateMCNRequest_get_MCNStatus(ActivateMCNRequest *s);
void ActivateMCNRequest_set_contactNumber(ActivateMCNRequest*, const char*);
void ActivateMCNRequest_get_contactNumber(ActivateMCNRequest *s, char *n);
NPC_Exception_E activate_MCN(ActivateMCNRequest*);
NPC_Exception_E deactivate_MCN(ActivateMCNRequest*);
NPC_Exception_E interrogate_MCN(ActivateMCNRequest*);
NPC_Exception_E NPC_connection_check(void);

#endif

