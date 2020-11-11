
#ifndef __EVENT_INFO_NOTIFY_H__
#define __EVENT_INFO_NOTIFY_H__

int webapi_event_connected_send(char *type, char *time, const unsigned char *mac, char *linktype, char *devicetype, char *name);
int webapi_event_newmessage_send(char *type, char *time, char *from, char *message);
int webapi_event_guestexpire_send(char *type, char *time);
int webapi_event_wpspairing_send(char *type, char *time, char *status, char *mac);
int webapi_event_notify(struct blob_attr *msg);

#endif
