#ifndef __SLOG_P_H__
#define __SLOG_P_H__

enum
{
    LOG_EVENT_GET_CONTENT = 0,
    LOG_EVENT_CLEAR_ALL,
    LOG_EVENT_UPDATE_TIMESTAMP,
    LOG_EVENT_SYNC_CRIT,
    LOG_EVENT_SYNC_CRIT_RESPONSE,
    LOG_EVENT_GET_CONTENT_STRUCT,
};

typedef struct event_content_s
{
    int length;
    int reverse;
    int access_user;
    int has_device_info;
}event_content_t;

typedef struct
{
    int cmd;
    int ret; /* -1: fail 0: ok */
    union 
    {
        event_content_t ec;
    }data;
}log_event_t;

typedef struct
{
    unsigned int id;  /* log category id */
    int  warning;
    char date[16];
    char time[12];
    char details[240];
}log_data_t;
#define LOG_DIR_PATH "/tmp/lxxd/"
#define LOG_EVENT_SOCK_NAME LOG_DIR_PATH"event_sock"
#ifdef  __cplusplus
extern "C" {
#endif
/****************** event api **********************************************/

int log_get_content(char *buf, int buf_len, int content_len, int reverse, int has_device_info);
int log_get_content_struct(char **buf,int access_user);
int log_clear_all(void);
int log_update_timestamp(void);
int log_sync_crit(void);
int log_store_file(char *type, char *path);

#ifdef  __cplusplus
}
#endif

#endif

