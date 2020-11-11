
#ifndef __SMS_XML__H_
#define __SMS_XML_H_
#define SMS_ENV_NODE "feed"
#define SMS_LINK_NODE "link"
#define SMS_UPDATED_NODE "updated"
#define SMS_ENTRY_NODE "entry"
#define SMS_ENTRY_TITLE_NODE "title"
#define SMS_ENTRY_MD5_NODE "md5"
#define SMS_ENTRY_LINK_NODE "link"
#define SMS_ENTRY_AUTHOR_NODE "author"
#define SMS_ENTRY_AUTHOR_NAME_NODE "name"
#define SMS_ENTRY_ID_NODE "id"
#define SMS_ENTRY_UPDATED_NODE "updated"
#define SMS_ENTRY_SUMMARY_NODE "summary"
 enum sms_xml_ret
{
    SMS_XML_ERR = -1,
    SMS_XML_SUCC
};
int sms_output_data_xml_init(char *xmlPath);//init xml file
int sms_xml_add_entry(SMS_Info *p_sms_info,char*xmlPath);//add new message in xml file
int sms_xml_dele_entry(int msg_index,char* xmlPath);//delete msg form xml by id
int sms_xml_dele_entry_by_time(char *pdu,char*xmlPath);//delete msg form xml by time
int sms_xml_set_uri(char * access_for_feed,char * access_for_sms,char*xmlPath);//set the URI for accessing the feed and set the uri to access the SMS handling page
int sms_xml_set_msg_uri(int id,char * uri,char *xmlPath);//set the URI for link to the message page
int sms_find_msg(int id ,char *time,char *xmlPath);//if found out  return 1 else return 0
int sms_read_msg_to_list(struct list_head *list,char *xmlPath);
int sms_store_output_data_xml_init(char *xmlPath);
int sms_store_xml_add_entry(SMS_Info *p_sms_info,char *xmlPath);
int sms_store_xml_get_time_by_md5(char *time,int time_len,char *md5,char*xmlPath);
int sms_store_xml_dele_entry(int id,char*xmlPath);//delete msg form xml by id

#endif // __SMS_XML_H_
