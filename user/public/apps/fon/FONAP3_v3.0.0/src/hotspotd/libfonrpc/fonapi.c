/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include "fonrpc-local.h"
#include "fonrpc.h"
#include <errno.h>
#include "021-radconf.h"
#include "014-client.h"
#include "012-whitelist.h"
#include "011-firewall.h"
#include "010-core.h"
#include "fonapi.h"

// RPC connection and state
static int sock = -1;
static uint32_t seq = 0;
static uint8_t buffer[8192] = {0};
static struct frmsg *next = NULL;
static size_t buffered = 0;

// Send an RPC request
static int request(struct frmsg *frm) {
	frm->frm_seq = fswap32(++seq);
	frm->frm_flags |= fswap16(FRM_F_REQUEST);
	if (send(sock, frm, frm_length(frm), 0) > 0)
		return seq;
	return -1;
}

// Receive reply
static struct frmsg* receive(int seq) {
	struct frmsg *frm;
	// Test for valid messages that match our sequence ID
	while (!(frm = frm_load(next, buffered)) || frm->frm_seq != fswap32(seq)) {
		ssize_t len = recv(sock, buffer, sizeof(buffer), 0);
		if (len < 0) {
			if (errno == EINTR)
				continue;
			return NULL; // Socket error
		}

		// Add content to buffer
		buffered = len;
		next = (void*)buffer;
	}
	// Move buffer to next position
	next = FRM_NEXT(next, buffered);
	return frm;
}

// Send a message, expecting only a status reply
static int call(struct frmsg *frm) {
	int s = request(frm);
	if (s < 0 || !(frm = receive(s)))
		return errno;
	if (frm_type(frm) != FRMSG_ERROR)
		return 0;
	struct frattr *attrs[_FRE_SIZE];
	frm_parse(frm, attrs, _FRE_SIZE);
	return fra_to_u32(attrs[FRE_CODE], 0);
}

static int get_client_info_by_type(char* value, int id, int type) {
    uint8_t buffer[64];
    int cnt = 0;
    struct frmsg *frm = frm_init(buffer, FRT_CL_GET, 0);
    frm->frm_flags |= fswap32(FRM_F_DUMP);

    int seq = request(frm);
    if (seq < 0)
        return errno;

    while ((frm = receive(seq)) && !frm_status(frm)
            && frm_type(frm) != FRMSG_DONE) {
        unsigned char *pData = (unsigned char *)frm;
        cnt++;
        if(id != cnt)
            continue;
        struct frattr *fra[_FRA_CL_SIZE];
        frm_parse(frm, fra, _FRA_CL_SIZE);

        // Preformat data
        char mac[24] = "", *stat;
        if (fra_length(fra[FRA_CL_HWADDR]) >= 6) {
            const uint8_t *data = fra_data(fra[FRA_CL_HWADDR]);
            snprintf(mac, sizeof(mac), "%02X-%02X-%02X-%02X-%02X-%02X",
                    data[0], data[1], data[2], data[3], data[4], data[5]);
        }

        const char *user = fra_to_string(fra[FRA_CL_USERNAME]);
        if (!user)
            user = "-";

        switch (fra_to_u32(fra[FRA_CL_STATUS], 0)) {
            case CL_CLSTAT_NOAUTH:
                stat = "noauth";
                break;
            case CL_CLSTAT_LOGOUT:
                stat = "logout";
                break;
            case CL_CLSTAT_LOGIN:
                stat = "login";
                break;
            case CL_CLSTAT_AUTHED:
                stat = "authed";
                break;
            default:
                stat = "unknown";
                break;
        }
        if(type == TYPE_HOST_NAME)
        {
            sprintf(value, "%s", mac);
        }
        else if(type == TYPE_STATUS)
        {
            sprintf(value, "%s", stat);
        }
        else if(type == TYPE_SESSION)
        {
            sprintf(value, "%u", fra_to_u32(fra[FRA_CL_TIME], 0));
        }
        else
        {
            break;
        }

        if (!(frm_flags(frm) & FRM_F_MULTI))
            break;
    }

    return frm_status(frm);
}

char *fon_rpc_get_client_info(int id, int type) {
    static char value[512] = {0};
    const char *path = "/var/run/hotspotd.sock";
    uint32_t timeout = 1000;
    // Open RPC socket
    struct sockaddr_un sa = { .sun_family = AF_UNIX };

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct timeval tv = {
        .tv_sec = timeout / 1000,
        .tv_usec = (timeout % 1000) * 1000,
    };
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sprintf(sa.sun_path + 1, "fonctl%i", (int)getpid());
    bind(sock, (void*)&sa, sizeof(sa));

    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
    if (connect(sock, (void*)&sa, sizeof(sa))) {
        close(sock);
        return "";
    }
    seq = time(NULL);

    int status;
    status = get_client_info_by_type(value, id, type);

    close(sock);
    return value;
}

char *fon_rpc_get_client_count(int type) {
    static char value[512] = {0};
    const char *path = "/var/run/hotspotd.sock";
    uint32_t timeout = 1000;
    // Open RPC socket
    struct sockaddr_un sa = { .sun_family = AF_UNIX };

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct timeval tv = {
        .tv_sec = timeout / 1000,
        .tv_usec = (timeout % 1000) * 1000,
    };
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sprintf(sa.sun_path + 1, "fonctl%i", (int)getpid());
    bind(sock, (void*)&sa, sizeof(sa));

    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
    if (connect(sock, (void*)&sa, sizeof(sa))) {
        close(sock);
        return "";
    }
    seq = time(NULL);
    uint8_t buffer[64];
    struct frmsg *frm = frm_init(buffer, FRT_CL_GET_INFO, 0);
    int seq = request(frm);
    if (seq < 0)
    {
        close(sock);
        return "";
    }

    if (!(frm = receive(seq)))
    {
        close(sock);
        return "";
    }

    if ((seq = frm_status(frm)))
    {
        close(sock);
        return "";
    }
    struct frattr *fra[_FRA_CL_SIZE];
    frm_parse(frm, fra, _FRA_CL_SIZE);
    if(type == TYPE_AUTH_CLIENT)
    {
        sprintf(value, "%u", fra_to_u32(fra[FRA_CL_AUTH_COUNT], 0));
    }
    else if(type == TYPE_CMAX)
    {
        sprintf(value, "%u", fra_to_u32(fra[FRA_CL_TOTAL_COUNT], 0));
    }
    else if(type == TYPE_AUTH_SUM)
    {
        sprintf(value, "%u", fra_to_u32(fra[FRA_CL_AUTH_SUM], 0));
    }

    close(sock);
    return value;
}
#ifdef CONFIG_SUPPORT_DSL  
int fon_rpc_wan_action(int wan_id, char* wan_if, char* wan_mac, int ulink_speed, int dlink_speed, int action, int dsl_if )
#else
int fon_rpc_wan_action(int wan_id, char* wan_if, char* wan_mac, int dlink_speed, int action )
#endif

 {
    const char *path = "/var/run/hotspotd.sock";
    uint32_t timeout = 1000;
    // Open RPC socket
    struct sockaddr_un sa = { .sun_family = AF_UNIX };

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct timeval tv = {
        .tv_sec = timeout / 1000,
        .tv_usec = (timeout % 1000) * 1000,
    };
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sprintf(sa.sun_path + 1, "fonctl%i", (int)getpid());
    bind(sock, (void*)&sa, sizeof(sa));

    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
    if (connect(sock, (void*)&sa, sizeof(sa))) {
        close(sock);
        return -1;
    }
    seq = time(NULL);
    uint8_t buffer[1024];
    if(action == ACTION_START)
    {
        char wan[16] = {0};
        char dspeed[64] = {0};

        snprintf(dspeed,sizeof(dspeed),"%d", dlink_speed);
        snprintf(wan, sizeof(wan), "%d", wan_id);
#ifdef CONFIG_SUPPORT_DSL
		char uspeed[64] = {0};
		char dsl[16] = {0};
        snprintf(uspeed,sizeof(uspeed),"%d", ulink_speed);
		snprintf(dsl, sizeof(dsl), "%d", dsl_if);
#endif
        struct frmsg *frm = frm_init(buffer, FRT_CFG_SET, 0);
        frm_put_string(frm, FRA_CFG_SECTION, "main");
        frm_put_string(frm, FRA_CFG_OPTION, "wan_id");
        frm_put_string(frm, FRA_CFG_VALUE, wan);
        call(frm);
        memset(buffer,0,sizeof(buffer));
        frm = frm_init(buffer, FRT_CFG_SET, 0);
        frm_put_string(frm, FRA_CFG_SECTION, "main");
        frm_put_string(frm, FRA_CFG_OPTION, "wan_iface");
        frm_put_string(frm, FRA_CFG_VALUE, wan_if);
        call(frm);
        memset(buffer,0,sizeof(buffer));
        frm = frm_init(buffer, FRT_CFG_SET, 0);
        frm_put_string(frm, FRA_CFG_SECTION, "main");
        frm_put_string(frm, FRA_CFG_OPTION, "nasid");
        frm_put_string(frm, FRA_CFG_VALUE, wan_mac);
        call(frm);
        memset(buffer,0,sizeof(buffer));
        frm = frm_init(buffer, FRT_CFG_SET, 0);
        frm_put_string(frm, FRA_CFG_SECTION, "main");
        frm_put_string(frm, FRA_CFG_OPTION, "wan_bandwidth");
        frm_put_string(frm, FRA_CFG_VALUE, dspeed);
        call(frm);
#ifdef CONFIG_SUPPORT_DSL  
        memset(buffer,0,sizeof(buffer));
        frm = frm_init(buffer, FRT_CFG_SET, 0);
        frm_put_string(frm, FRA_CFG_SECTION, "main");
        frm_put_string(frm, FRA_CFG_OPTION, "wan_ubandwidth");
        frm_put_string(frm, FRA_CFG_VALUE, uspeed);
        call(frm);
        memset(buffer,0,sizeof(buffer));
        frm = frm_init(buffer, FRT_CFG_SET, 0);
        frm_put_string(frm, FRA_CFG_SECTION, "main");
        frm_put_string(frm, FRA_CFG_OPTION, "cmpresult_wan_type_ptm");
        frm_put_string(frm, FRA_CFG_VALUE, dsl);
        call(frm);
#endif
		memset(buffer,0,sizeof(buffer));
        frm = frm_init(buffer, FRT_SYS_START, 0);
        call(frm);
		
		

    }
    else if(action == ACTION_STOP)
    {
        memset(buffer,0,sizeof(buffer));
        struct frmsg *frm = frm_init(buffer, FRT_SYS_STOP, 0);
        call(frm);
    }
    close(sock);
    return 0;
}
int fon_rpc_wlan_action(int action, char* main_wifi_enable, char* main_5g_wifi_enable) {
    const char *path = "/var/run/hotspotd.sock";
    uint32_t timeout = 1000;
    // Open RPC socket
    struct sockaddr_un sa = { .sun_family = AF_UNIX };

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct timeval tv = {
        .tv_sec = timeout / 1000,
        .tv_usec = (timeout % 1000) * 1000,
    };
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sprintf(sa.sun_path + 1, "fonctl%i", (int)getpid());
    bind(sock, (void*)&sa, sizeof(sa));

    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
    if (connect(sock, (void*)&sa, sizeof(sa))) {
        close(sock);
        return -1;
    }
    seq = time(NULL);
    uint8_t buffer[1024];
    if(action == ACTION_START)
    {
        struct frmsg *frm = frm_init(buffer, FRT_CFG_SET, 0);
        frm_put_string(frm, FRA_CFG_SECTION, "main");
        frm_put_string(frm, FRA_CFG_OPTION, "main_wifi_enable");
        frm_put_string(frm, FRA_CFG_VALUE, main_wifi_enable);
        call(frm);
#ifdef CONFIG_SUPPORT_WIFI_5G
        memset(buffer,0,sizeof(buffer));
        frm = frm_init(buffer, FRT_CFG_SET, 0);
        frm_put_string(frm, FRA_CFG_SECTION, "main");
        frm_put_string(frm, FRA_CFG_OPTION, "main_5g_wifi_enable");
        frm_put_string(frm, FRA_CFG_VALUE, main_5g_wifi_enable);
        call(frm);
#endif   
        memset(buffer,0,sizeof(buffer));
        frm = frm_init(buffer, FRT_SYS_START, 0);
        call(frm);

    }
    else if(action == ACTION_STOP)
    {
        struct frmsg *frm = frm_init(buffer, FRT_SYS_STOP, 0);
        call(frm);
    }
    else if(action == ACTION_RELOAD)
    {
        struct frmsg *frm = frm_init(buffer, FRT_SYS_RELOAD, 0);
        call(frm);
    }

    close(sock);
    return 0;
}
char *fon_rpc_get_server_status(void) {
    static char value[512] = {0};
    const char *path = "/var/run/hotspotd.sock";
    uint32_t timeout = 1000;
    // Open RPC socket
    struct sockaddr_un sa = { .sun_family = AF_UNIX };

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct timeval tv = {
        .tv_sec = timeout / 1000,
        .tv_usec = (timeout % 1000) * 1000,
    };
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sprintf(sa.sun_path + 1, "fonctl%i", (int)getpid());
    bind(sock, (void*)&sa, sizeof(sa));

    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
    if (connect(sock, (void*)&sa, sizeof(sa))) {
        close(sock);
        return "";
    }
    seq = time(NULL);
    uint8_t buffer[64];
    struct frmsg *frm = frm_init(buffer, FRT_SYS_STATUS, 0);
    int seq = request(frm);
    if (seq < 0)
    {
        close(sock);
        return "";
    }

    if (!(frm = receive(seq)))
    {
        close(sock);
        return "";
    }

    if ((seq = frm_status(frm)))
    {
        close(sock);
        return "";
    }
    struct frattr *fra[_FRA_SYS_SIZE];
    frm_parse(frm, fra, _FRA_SYS_SIZE);
    sprintf(value, "%s", (fra_to_string(fra[FRA_SYS_STATUS]))?:"");

    close(sock);
    return value;
}
