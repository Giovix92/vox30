#ifndef __EMAIL_H__
#define __EMAIL_H__

#define RECIPENT_NULL_FROM_GUI "you@example.com"
int Email_alert_msg(char *msg); 

int Email_alert_msg_direct(char *email_address, char *smtp_address, int email_port, char *user_name, char *password, char *msg);
#endif
