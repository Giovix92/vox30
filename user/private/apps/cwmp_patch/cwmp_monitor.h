#ifndef __CWMP_MONITOR_H__
#define __CWMP_MONITOR_H__
/**
 * @file  cwmp_monitor.h
 * @author Martin_Huang@sdc.sercomm.com
 * @date   2011-06-10
 * @brief  monitor cwmp server & client
 *
 * Copyright - 2011 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifically
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */
#include <sys/un.h>
#include <libubus.h>
#include <libubox/blobmsg.h>
#include <libubox/blob.h>
#include <libubox/blobmsg_json.h>

#define CWMP_MONITOR_SOCKETNAME "/tmp/cwmp_monitor.sock"
#define CWMP_MONITOR_MAGIC 	0x33229911
#define OBJECT_WATCHDOG "v1.Watchdog"

enum{
    TRIGGER_BASIC = 1,
    TRIGGER_SPECIAL,
    TRIGGER_MAX
};

enum{
	POLICY_MAGIC,
	POLICY_SOURCE,
	POLICY_PID,
    POLICY_EVENT_TYPE,
    POLICY_EVENT_CODE,
    POLICY_TRIGGER_TYPE,
    POLICY_COMMAND,
    POLICY_MAX
};

enum{
    PID,
    MAX
};

enum {
	CWMP_CLIENT = 1,
	CWMP_HTTPS,
    CWMP_VOICE,
    CWMP_DNS,
	OTHERS,
};
enum {
	CWMP_REG = 1,
	CWMP_CR,
	CWMP_DOWNLOAD,
	CWMP_UPLOAD,
    VOICE_REG,
    VOICE_ASSERT,
	VOICE_HEART_BEAT,
    DNS_REG,
};
enum {
	CWMP_REG_IN = 1,
	CWMP_REG_OUT,
	CWMP_CR_IN,
	CWMP_CR_OUT,
	CWMP_DOWNLOAD_IN,
	CWMP_DOWNLOAD_OUT,
	CWMP_UPLOAD_IN,
	CWMP_UPLOAD_OUT,
    VOICE_REG_IN,
    VOICE_REG_OUT,
    VOICE_ASSERT_FAIL,
    VOICE_INIT_FAIL,
    DNS_REG_IN,
    DNS_REG_OUT,
};
typedef struct tag_cwmp_monitor_message{
    struct list_head list;
	int magic;
	int source;
	pid_t pid;
    int event_type;
	int event_code;
	int trigger_type; /* 1: set up /proc/pid/cmdline  2: exec command 3: special handle*/
	char command[1024]; /*user command or /proc/pid/cmdline*/
	char cmdline[256];
}cwmp_monitor_message;


/*static int notify_cwmp_monitor_unregister(pid_t pid)
{
    struct ubus_context *ctx;
    struct blob_buf b = {0};
    int ret = 0;
    uint32_t id = 0;
    
    ctx = ubus_connect(NULL);
    if(!ctx)
    {
        return 0;
    }
    
    if(ubus_lookup_id(ctx,OBJECT_WATCHDOG,&id))
    {
        ubus_free(ctx);
        return 0;
    }
    
    blob_buf_init(&b, 0);
    if (pid)
    {
        blobmsg_add_u32(&b,"PID",pid);
    }
    
    ubus_invoke(ctx, id, "unregister", b.head, NULL, &ret, 3000);
    blob_buf_free(&b);
    ubus_free(ctx);

	return 0;
}
*/
static int notify_cwmp_monitor_register(cwmp_monitor_message *message)
{
#ifdef CONFIG_SUPPORT_UBUS
    struct ubus_context *ctx = NULL;
    struct blob_buf b = {0};
    int ret = 0;
    uint32_t id = 0;
    int count = 3 ;
    ctx = ubus_connect(NULL);
    if(!ctx)
    {
        return 0;
    }
    
    while(ubus_lookup_id(ctx,OBJECT_WATCHDOG,&id))
    {
        sleep(1);
        count --;
        if(count <= 0)
        {
            ubus_free(ctx);
            return 0;
        }
    }
    
    blob_buf_init(&b, 0);
    if (message->magic)
    {
        blobmsg_add_u32(&b,"Magic",message->magic);
    }
    if (message->source)
    {
		blobmsg_add_u32(&b,"Source",message->source);
    }
    if(message->pid)
    {
        blobmsg_add_u32(&b,"PID",message->pid);
    }
    if(message->event_type)
    {
        blobmsg_add_u32(&b,"EventType",message->event_type);
    }
    if(message->event_code)
    {
        blobmsg_add_u32(&b,"EventCode",message->event_code);
    }
    if(message->trigger_type)
    {
        blobmsg_add_u32(&b,"Type",message->trigger_type);
    }
    if(message->command)
    {
        blobmsg_add_string(&b,"Command",message->command);
    }
	
	ubus_invoke(ctx, id, "register", b.head, NULL, &ret, 3000);
    blob_buf_free(&b);
	ubus_free(ctx);

#else
	int socket_fd;
	struct sockaddr_un name;
	int i;
	int ret;
    int fd_flag;

	if(access(CWMP_MONITOR_SOCKETNAME, F_OK) != 0)
	{
		return -1;
	}
	/* Create the socket.  */
	socket_fd = socket (PF_LOCAL, SOCK_DGRAM, 0);
	if(socket_fd < 0)
		return -1;
	name.sun_family = AF_LOCAL;
	strcpy (name.sun_path, CWMP_MONITOR_SOCKETNAME);
	if ((fd_flag = fcntl(socket_fd, F_GETFL, 0)) >= 0) {
		fcntl(socket_fd, F_SETFL, fd_flag | O_NONBLOCK);
	}
	for(i = 0; i < 3; i++)
	{
		ret = sendto(socket_fd, message, sizeof(cwmp_monitor_message), MSG_DONTWAIT, (struct sockaddr*)&name, SUN_LEN(&name));
		if(sizeof(cwmp_monitor_message) == ret)
			break;
	}
	close(socket_fd);
#endif
	return 0;
}

#define CWMPC_NOTIFY_REG_IN(info) \
do{\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_CLIENT;\
	messgage.event_type = CWMP_REG;\
	messgage.event_code = CWMP_REG_IN;\
	messgage.trigger_type = TRIGGER_SPECIAL;\
	messgage.pid = getpid();\
    notify_cwmp_monitor_register(&messgage);\
}while(0)

#define CWMPC_NOTIFY_REG_OUT(info) \
do{\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_CLIENT;\
	messgage.event_type = CWMP_REG;\
	messgage.event_code = CWMP_REG_OUT;\
	messgage.pid = getpid();\
	messgage.trigger_type = TRIGGER_SPECIAL;\
    notify_cwmp_monitor_register(&messgage);\
}while(0)

#define CWMPC_NOTIFY_DOWNLOAD_START(info) \
do{\
	int ret = 0;\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_CLIENT;\
	messgage.event_type = CWMP_DOWNLOAD;\
	messgage.event_code = CWMP_DOWNLOAD_IN;\
	messgage.trigger_type = TRIGGER_SPECIAL;\
	messgage.pid = getpid();\
    ret = notify_cwmp_monitor_register(&messgage);\
}while(0)

#define CWMPC_NOTIFY_DOWNLOAD_STOP(info) \
do{\
	int ret = 0;\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_CLIENT;\
	messgage.event_type = CWMP_DOWNLOAD;\
	messgage.event_code = CWMP_DOWNLOAD_OUT;\
	messgage.trigger_type = TRIGGER_SPECIAL;\
	messgage.pid = getpid();\
    ret = notify_cwmp_monitor_register(&messgage);\
}while(0)

#define CWMPC_NOTIFY_UPLOAD_START(info) \
do{\
	int ret = 0;\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_CLIENT;\
	messgage.event_type = CWMP_UPLOAD;\
	messgage.event_code = CWMP_UPLOAD_IN;\
	messgage.trigger_type = TRIGGER_SPECIAL;\
	messgage.pid = getpid();\
    ret = notify_cwmp_monitor_register(&messgage);\
}while(0)

#define CWMPC_NOTIFY_UPLOAD_STOP(info) \
do{\
	int ret = 0;\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_CLIENT;\
	messgage.event_type = CWMP_UPLOAD;\
	messgage.event_code = CWMP_UPLOAD_OUT;\
	messgage.trigger_type = TRIGGER_SPECIAL;\
	messgage.pid = getpid();\
    ret = notify_cwmp_monitor_register(&messgage);\
}while(0)

#define CWMPC_NOTIFY_CR_OUT(info) \
do{\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_CLIENT;\
	messgage.event_type = CWMP_CR;\
	messgage.event_code = CWMP_CR_OUT;\
	messgage.trigger_type = TRIGGER_SPECIAL;\
	messgage.pid = getpid();\
    notify_cwmp_monitor_register(&messgage);\
}while(0)

#define CWMPS_NOTIFY_REG_IN(info) \
do{\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_HTTPS;\
	messgage.event_type = CWMP_REG;\
	messgage.event_code = CWMP_REG_IN;\
	messgage.trigger_type = TRIGGER_SPECIAL;\
	messgage.pid = getpid();\
    notify_cwmp_monitor_register(&messgage);\
}while(0)

#define CWMPS_NOTIFY_REG_OUT(info) \
do{\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_HTTPS;\
	messgage.event_type = CWMP_REG;\
	messgage.event_code = CWMP_REG_OUT;\
	messgage.trigger_type = TRIGGER_SPECIAL;\
	messgage.pid = getpid();\
    notify_cwmp_monitor_register(&messgage);\
}while(0)

#define CWMPS_NOTIFY_CR_IN(info) \
do{\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_HTTPS;\
	messgage.event_type = CWMP_CR;\
	messgage.event_code = CWMP_CR_IN;\
	messgage.trigger_type = TRIGGER_SPECIAL;\
	messgage.pid = getppid();\
    notify_cwmp_monitor_register(&messgage);\
}while(0)

#define VOICE_NOTIFY_REG_IN(info) \
do{\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_VOICE;\
	messgage.event_type = VOICE_REG;\
	messgage.event_code = VOICE_REG_IN;\
	messgage.trigger_type = TRIGGER_SPECIAL;\
	messgage.pid = getpid();\
    notify_cwmp_monitor_register(&messgage);\
}while(0)

#define VOICE_NOTIFY_REG_OUT(info) \
do{\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_VOICE;\
	messgage.event_type = VOICE_REG;\
	messgage.event_code = VOICE_REG_OUT;\
	messgage.trigger_type = TRIGGER_SPECIAL;\
	messgage.pid = getpid();\
     notify_cwmp_monitor_register(&messgage);\
}while(0)

#define VOICE_NOTIFY_ASSERT_FAIL(info) \
do{\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_VOICE;\
	messgage.event_type = VOICE_ASSERT;\
	messgage.event_code = VOICE_ASSERT_FAIL;\
	messgage.pid = getpid();\
	messgage.trigger_type = TRIGGER_SPECIAL;\
    notify_cwmp_monitor_register(&messgage);\
}while(0)

#define VOICE_NOTIFY_INIT_FAIL(info) \
do{\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_VOICE;\
	messgage.event_type = VOICE_ASSERT;\
    messgage.event_code = VOICE_INIT_FAIL;\
    messgage.pid = getpid();\
	messgage.trigger_type = TRIGGER_SPECIAL;\
    notify_cwmp_monitor_register(&messgage);\
}while(0)

#define DNS_NOTIFY_REG_IN(info) \
do{\
	int ret = 0;\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_DNS;\
	messgage.event_type = DNS_REG;\
	messgage.event_code = DNS_REG_IN;\
	messgage.pid = getpid();\
	messgage.trigger_type = TRIGGER_SPECIAL;\
    ret = notify_cwmp_monitor_register(&messgage);\
}while(0)
#define VOICE_NOTIFY_HEART_BEAT(info) \
do{\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_VOICE;\
	messgage.event_type = VOICE_HEART_BEAT;\
    messgage.pid = getpid();\
	messgage.trigger_type = TRIGGER_SPECIAL;\
    notify_cwmp_monitor_register(&messgage);\
}while(0)

#define DNS_NOTIFY_REG_OUT(info) \
do{\
	int ret = 0;\
	cwmp_monitor_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = CWMP_MONITOR_MAGIC;\
	messgage.source = CWMP_DNS;\
	messgage.event_type = DNS_REG;\
	messgage.event_code = DNS_REG_OUT;\
	messgage.pid = getpid();\
	messgage.trigger_type = TRIGGER_SPECIAL;\
    ret = notify_cwmp_monitor_register(&messgage);\
}while(0)

#define BASIC_WATCHDOG_REGISTER(trigger_type,command) \
do{\
	cwmp_monitor_message message;\
	memset(&message, 0, sizeof(message));\
	message.magic = CWMP_MONITOR_MAGIC;\
	message.pid = getpid();\
	message.trigger_type = trigger_type;\
    if(command != NULL)\
    {\
        snprintf(message.command,sizeof(message.command),"%s",command);\
    }\
    notify_cwmp_monitor_register(&message);\
}while(0)

#define BASIC_WATCHDOG_REGISTER_WITHPID(trigger_type,command,pid) \
do{\
	int ret = 0;\
	cwmp_monitor_message message;\
	memset(&message, 0, sizeof(message));\
	message.magic = CWMP_MONITOR_MAGIC;\
	message.pid = pid;\
	message.trigger_type = trigger_type;\
    if(command != NULL)\
    {\
        snprintf(message.command,sizeof(message.command),"%s",command);\
    }\
    ret = notify_cwmp_monitor_register(&message);\
}while(0)

/*#define BASIC_WATCHDOG_UNREGISTER(info) \
do{\
	int ret = 0;\
    pid_t pid = 0;\
	pid = getpid();\
    ret = notify_cwmp_monitor_unregister(pid);\
}while(0)*/
#endif

