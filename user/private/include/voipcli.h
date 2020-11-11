#ifndef _VOIP_CLI_H_
#define _VOIP_CLI_H_
#include <sys/un.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "voip.h"
#include <unistd.h>

#define VOIP_CLI_SOCKETNAME VGW_ROOT_PATH"voip_cli.sock"
#define VOIP_CLI_MAGIC 	0x33229922
#define CLIMSG_LEN		128
/*The value of split_number means nothing, just using to split two line's callerie name*/
#define SPLIT_NUMBER		20

enum {
    VOIP_CLI_SPY_CLEAR,
    VOIP_CLI_SPY_UPDATE,
    VOIP_CLI_SIPTRACE_UPDATE,
    VOIP_CLI_SIPTRACE_CLEAR,
    VOIP_CLI_SPY_LOGLEVEL,
    VOIP_CLI_INNER_STATUS,
    VOIP_CLI_DIAGNOSE_ALL,
    VOIP_CLI_WAKE_UP_CALL_INIT,
    VOIP_CLI_RINGING_SCHEDULE_INIT,
    VOIP_CLI_RESET_VOICE_STATISTICS,
    VOIP_CLI_FOREIGN_NUMBER_BLOCK_ENABLE,
    VOIP_CLI_NUMBER_BLOCK_ENABLE,
    VOIP_CLI_WAN_DOWN,
    VOIP_CLI_WAN_UP,
    VOIP_CLI_LINE_UPDATE,
    VOIP_CLI_STOP_VGW,
    VOIP_CLI_DIAG_START,
    VOIP_CLI_DIAG_STOP,
    VOIP_CLI_SAVE_NUMBER,
    VOIP_CLI_DIAL_TONE,
    VOIP_CLI_DELETE_CALL_LOG,
    VOIP_CLI_UPDATE_CALL_LOG,
    VOIP_CLI_UPDATE_DIAL_PLAN,
    VOIP_CLI_BLOCK_COUNT_CHANGED,
    VOIP_CLI_WHITE_COUNT_CHANGED,
    VOIP_CLI_CHANGE_TO_WHITE_LIST,
    VOIP_CLI_CHANGE_TO_BLACK_LIST,
    VOIP_CLI_CMDS,
    VOIP_CLI_RINGING_SCHEDULE_CHANGED,
    VOIP_CLI_CALLER_ID_NAME_CHANGED,
    VOIP_CLI_UPDATE_CALL_WAITING,
    VOIP_CLI_SILENT_CALL,
    VOIP_CLI_FWA_MODE_CS_CALL,
};


typedef struct tag_voip_cli_message{
	int  magic;
	int  cmd_code;
	char extral_info[CLIMSG_LEN];
}voip_cli_message;

static inline int cli_send_cmd_to_vgw(voip_cli_message *message)
{
	int socket_fd;
	struct sockaddr_un name;
	int i;
	int ret;
    int fd_flag;

	if(access(VOIP_CLI_SOCKETNAME, F_OK) != 0)
	{
		return -1;
	}
	/* Create the socket.  */
	socket_fd = socket (PF_LOCAL, SOCK_DGRAM, 0);
	if(socket_fd < 0)
		return -1;
	name.sun_family = AF_LOCAL;
	strcpy (name.sun_path, VOIP_CLI_SOCKETNAME);
	if ((fd_flag = fcntl(socket_fd, F_GETFL, 0)) >= 0) {
		fcntl(socket_fd, F_SETFL, fd_flag | O_NONBLOCK);
	}
	for(i = 0; i < 3; i++)
	{
		ret = sendto(socket_fd, message, sizeof(voip_cli_message), MSG_DONTWAIT, (struct sockaddr*)&name, SUN_LEN(&name));
		if(sizeof(voip_cli_message) == ret)
			break;
	}
	close(socket_fd);
	return 0;
}

#define VOIP_CLI_SEND_COMMAND(cmd) \
do{\
	voip_cli_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = VOIP_CLI_MAGIC;\
	messgage.cmd_code = cmd;\
    cli_send_cmd_to_vgw(&messgage);\
}while(0)

#define VOIP_CLI_SEND_COMMAND_WITH_DATA(cmd, data) \
do{\
	voip_cli_message messgage;\
	memset(&messgage, 0, sizeof(messgage));\
	messgage.magic = VOIP_CLI_MAGIC;\
	messgage.cmd_code = cmd;\
    snprintf(messgage.extral_info, (CLIMSG_LEN - 1), "%s", data);\
    cli_send_cmd_to_vgw(&messgage);\
}while(0)


#endif//_VOIP_CLI_H_
