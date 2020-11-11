#ifndef _SC_CONTACTS_H_
#define _SC_CONTACTS_H_

#include "common/list.h"

#define SCC_DATA_BASE_PATH                    "/mnt/1/contacts.csv"
#define VOIP_MAX_CONTACT_ENTRY_NUMBER          100

#define SCC_MAX_NUMBERS                       10
#define SCC_MAX_NAME_LENGTH                   256
#define SCC_MAX_NUMBER_LENGTH                 128
#define SCC_MAX_TYPE_LENGTH                   128

typedef enum
{
    SCC_RET_FAIL = -1,
    SCC_RET_OK
}SCC_RET;

#define SCC_PRIVATE_MOBILE                      "Private - Mobile"
#define SCC_PRIVATE_LANDLINE                    "Private - Landline"
#define SCC_WORK_MOBILE                         "Work - Mobile"
#define SCC_WORK_LANDLINE                       "Work - Landline"
#define SCC_OTHERS                              "Others"

#define SCC_PRIVATE_MOBILE_CODE                 "202014"
#define SCC_PRIVATE_LANDLINE_CODE               "202013"
#define SCC_WORK_MOBILE_CODE                    "202016"
#define SCC_WORK_LANDLINE_CODE                  "202015"
#define SCC_OTHERS_CODE                         "202017"


typedef struct scc_phone_entry_t
{
    struct list_head  head;
    char type[SCC_MAX_TYPE_LENGTH];
    char number[SCC_MAX_NUMBER_LENGTH];
} SCC_PHONE_ENTRY;

struct sc_contact
{
    struct list_head head;
    int id;
    char name[SCC_MAX_NAME_LENGTH];
    char initials[SCC_MAX_NAME_LENGTH];
    struct list_head phone_numbers;
};

int sc_contact_sanity_check(char* path);
int sc_contact_load_from_config(struct list_head* head);
SCC_RET sc_contact_save_to_config(struct list_head* head);
int sc_contact_load_from_file(char* path);
SCC_RET sc_contact_save_to_file(char* path);
SCC_RET sc_contact_add(struct list_head* node, struct list_head* head);
SCC_RET sc_contact_node_init(struct sc_contact* node);
SCC_RET sc_contact_free(struct list_head* head);
SCC_RET sc_contact_free_numbers(struct list_head* head);
int sc_contact_get_new_id(struct list_head* head);
struct list_head* sc_contact_delete_by_id(int id, struct list_head* head);
struct sc_contact* sc_contact_search_by_number(char* number, struct list_head* head);
int sc_contact_is_number_match(char* a, char* b);

#endif//_SC_CONTACTS_H_


