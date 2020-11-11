#ifndef __CAL_EMAIL_H__
#define __CAL_EMAIL_H__
#define EMAIL_LOG_SOCK_NAME "/tmp/email_sock"
#define EMAIL_MAX_LOG_LENGTH 1024 
#define ALERT_EMAIL_FILE      "/tmp/alert_email_content"

char *cal_email_serveraddress_get(void);
int cal_email_serveraddress_set(char *val);
char *cal_email_EmailSMTPEncrypted_get(void);
int cal_email_EmailSMTPEncrypted_set(char *val);
char *cal_email_EmailPort_get(void);
int cal_email_EmailPort_set(char *val);
char *cal_email_Email_Username_get(void);
int cal_email_Email_Username_set(char *val);
char *cal_email_EmailPassword_get(void);
int cal_email_EmailPassword_set(char *val);
char *cal_email_EmailAddress_get(void);
int cal_email_EmailAddress_set(char *val);
char *cal_email_EmailEvent_get(void);
int cal_email_EmailEvent_set(char *val);
char *cal_email_enable_get(void);
int cal_email_enbale_set(char *val);

char *cal_email_maxitems_get(void);
int cal_email_maxitems_set(char *val);
char *cal_email_interval_get(void);
int cal_email_interval_set(char *val);
#endif
