#ifndef _CAL_CONTACTS_H_
#define _CAL_CONTACTS_H_

#define CAL_CONTACTS_VALUE_LEN_MAX  512

int cal_contacts_get_index_list(int** index_array);

int cal_contacts_new_entry(int *p_index);
int cal_contacts_del_entry(int index);
char *cal_contacts_get_id(int index);
int cal_contacts_set_id(int index, char *value);
char *cal_contacts_get_name(int index);
int cal_contacts_set_name(int index, char *value);
char *cal_contacts_get_initials(int index);
int cal_contacts_set_initials(int index, char *value);
int cal_contacts_get_numbers_list(int index, int** index_array);
int cal_contacts_new_numbers(int index, int *p_numberIndex);
int cal_contacts_del_numbers(int index, int numberIndex);
char *cal_contacts_get_number(int index, int numberIndex);
int cal_contacts_set_number(int index, int numberIndex,  char *value);
char *cal_contacts_get_type(int index, int numberIndex);
int cal_contacts_set_type(int index, int numberIndex,  char *value);
#endif//_CAL_CONTACTS_H_

