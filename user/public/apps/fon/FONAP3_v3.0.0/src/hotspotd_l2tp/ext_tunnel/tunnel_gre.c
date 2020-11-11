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
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <unistd.h>
#include <net/if.h>

#include "lib/config.h"
#include "lib/urandom.h"
#include "lib/event.h"
#include "lib/insmod.h"
#include "lib/str.h"
#include "core/hotspotd.h"
#include "core/client.h"
#include "gre.h"

#define TUNNEL_LISTCOUNT 4
#define TUNNEL_SERVERCOUNT 2
static int global_tunnel_status = -1;
struct tunnel {
	struct trigger_handle *trigger;
	int handle;
	int32_t timeout;
	bool connected;
    int status;
};

static enum tunnel_type {
	TUNNEL_NONE,
	TUNNEL_L2TP,
    TUNNEL_GRE
} tunnel_type = TUNNEL_NONE;

#include "gre.h"
static int gre_tries = 0;
static int tunnel_gre_up(int tunnel_id);

static void tunnel_gre_cb(enum gre_state state, struct gre_lac* lac);
static struct gre_cfg gre_cfg[TUNNEL_LISTCOUNT] = {{0},{0},{0},{0}};
static struct gre_lac *gre_lac[TUNNEL_LISTCOUNT] = {NULL,NULL,NULL,NULL};
static struct gre_lac *gre_lac_backup[TUNNEL_LISTCOUNT] = {NULL,NULL,NULL,NULL};
static int tunnel_status[TUNNEL_LISTCOUNT] = {0,0,0,0};
static int tunnel_open(void);


struct tunnel_list 
{
    char* name;
    char* server[TUNNEL_SERVERCOUNT];
    int server_count;
};

static struct tunnel_list tunnel_servers[TUNNEL_LISTCOUNT] = {{0}};
static int tunnel_server_index[TUNNEL_LISTCOUNT] = {0,0,0,0};

static const struct client_backend *eap_handler = NULL;

static void tunnel_shutdown(int tunnelid);
static void tunnel_close();

struct tunnel_apply_args {
	int idx;
	int listidx;
    int is_eap;
};
struct tunnel_list_args {
	int idx;
};

static void tunnel_apply_server(const char *server, void *ctx) {
	struct tunnel_apply_args *args = (struct tunnel_apply_args *)ctx;

	char *__server = NULL;
	__server = (char *)malloc(128);
    snprintf(__server, 128, server);

	if (args->idx < TUNNEL_SERVERCOUNT)
		tunnel_servers[(args->listidx)].server[(args->idx)++] = __server;
}

static void tunnel_apply_ssid(const char *interface, void *ctx) {
	struct tunnel_apply_args *args = (struct tunnel_apply_args *)ctx;
    
    struct SSID* ssid = (struct SSID*) malloc(sizeof(struct SSID));
	char *__interface = NULL;
    char buf[128] = {0};
    const char* val = NULL;
	__interface = (char *)malloc(128);
    char* name = (char *)malloc(128);
    snprintf(__interface, 128, interface);
    ssid->interface = __interface;
    snprintf(buf,sizeof(buf),"%s_vlan",__interface);
    val = config_get_string(tunnel_servers[(args->listidx)].name, buf, NULL);
    if(val && *val && (*val >= '0'))
    {
        ssid->vlan = atoi(val); 
    }
    else
    {
        ssid->vlan = -1;
    }
    snprintf(buf,sizeof(buf),"%s_ssid",__interface);
    val = config_get_string(tunnel_servers[(args->listidx)].name, buf, "");
    snprintf(name, 128, "%s", val);
    ssid->name = name; 
    snprintf(buf,sizeof(buf),"%s_block",__interface);
    ssid->block_boardcast = config_get_bool(tunnel_servers[(args->listidx)].name, buf, false);
    ssid->is_eap = args->is_eap;
#ifdef CONFIG_SUPPORT_WIFI_5G
    snprintf(buf,sizeof(buf),"%s_is_5g",__interface);
    ssid->is_5g = config_get_bool(tunnel_servers[(args->listidx)].name, buf, false);
#endif
    struct list_head *c;
    for (c = gre_cfg[args->listidx].ssid.next;c != &(gre_cfg[args->listidx].ssid);c = c->next);
    list_add_tail(&(ssid->_head), c);

}

static void tunnel_apply_list(const char *name, void *ctx) {
	struct tunnel_list_args *args = (struct tunnel_list_args *)ctx;

    char *__server = NULL;
    __server = (char *)malloc(128);
    snprintf(__server, 128, name);
	if (args->idx < TUNNEL_LISTCOUNT)
    {
		tunnel_servers[(args->idx)++].name = __server;
    }
}

static void free_tunnelservers() 
{
	for(int ii=0; ii<TUNNEL_LISTCOUNT; ii++) {
        if(tunnel_servers[ii].name)
        {
            free(tunnel_servers[ii].name);
            tunnel_servers[ii].name = NULL;
        }
        for (int i = 0; i < TUNNEL_SERVERCOUNT; i++) 
        {
            if(tunnel_servers[ii].server[i])
            {
                free(tunnel_servers[ii].server[i]);
                tunnel_servers[ii].server[i] = NULL;
            }
        }
        if(gre_cfg[ii].ssid.next)
        {
            if(gre_cfg[ii].ssid.next != &gre_cfg[ii].ssid)
            {
                struct SSID *c,*n;
                list_for_each_entry_safe(c, n, &gre_cfg[ii].ssid, _head)
                {
                    list_del(&(c->_head));
                    free(c->interface);
                    free(c->name);
                    free(c);
                }
            }
        }
        memset(&gre_cfg[ii],0,sizeof(gre_cfg[ii]));
	}
}

static int tunnel_apply() {
    DPRINTF("free tunnel\n");
	free_tunnelservers();

	if (!tunnel_type)
		return 0;
    
    DPRINTF("tunnel apply\n");
    int i = 0;
    struct tunnel_list_args args;
    args.idx = 0;
    memset(tunnel_servers,0,sizeof(tunnel_servers));
    config_foreach_list("main","tunnel",tunnel_apply_list, &args);
    if (tunnel_type == TUNNEL_GRE) {
        for(i = 0; i < args.idx && i < TUNNEL_LISTCOUNT; i++)
        {
            memset(&gre_cfg[i],0,sizeof(gre_cfg[i]));
            DPRINTF("tunnel apply %s %s\n",tunnel_servers[i].name, config_get_string(tunnel_servers[i].name, "enable", "cc"));

            if(!strcmp(config_get_string(tunnel_servers[i].name, "enable", "false"), "true"))
            {
                DPRINTF("tunnel apply %s start\n",tunnel_servers[i].name);
                gre_cfg[i].enable = true;
                config_set_string(tunnel_servers[i].name, "eap_handler", "radius");
                const char *eaphndl = config_get_string(tunnel_servers[i].name, "eap_handler", NULL);
                eap_handler = client_get_backend(eaphndl);
                gre_cfg[i].cb = tunnel_gre_cb;
                gre_cfg[i].keepalive_interval = config_get_int(tunnel_servers[i].name, "keepalive_interval", 0);
                gre_cfg[i].timeout_interval = config_get_int(tunnel_servers[i].name, "timeout_interval", 30);
                gre_cfg[i].timeout = config_get_int(tunnel_servers[i].name, "timeout", 10);
                gre_cfg[i].timeout_num = config_get_int(tunnel_servers[i].name, "timeout_num", 10);
                gre_cfg[i].ip_fragment = config_get_int(tunnel_servers[i].name, "ip_fragment", 1);
                /* parse tunnel servers  */
                struct tunnel_apply_args server_args;
                server_args.idx = 0;
                server_args.listidx = i;
                config_foreach_list(tunnel_servers[i].name, "server", tunnel_apply_server, &server_args);
                tunnel_servers[i].server_count = server_args.idx;
                /* parse tunnel SSIDs  */
                struct tunnel_apply_args ssid_args;
                ssid_args.idx = 0;
                ssid_args.listidx = i;
                ssid_args.is_eap = 0;
                INIT_LIST_HEAD(&gre_cfg[i].ssid);
                config_foreach_list(tunnel_servers[i].name, "ssid", tunnel_apply_ssid, &ssid_args);
                ssid_args.is_eap = 1;
                config_foreach_list(tunnel_servers[i].name, "eap_ssid", tunnel_apply_ssid, &ssid_args);
                tunnel_server_index[i] = 0; // always  start with the first server.
                DPRINTF("tunnel apply %s end\n",tunnel_servers[i].name);

            }
            else
            {
                gre_cfg[i].enable = false;
                log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "tunnel %d is not enabled", i);
                continue;
            }
        }
    }

	return tunnel_open();
}


int tunnel_init() {

    int i = 0;
    struct tunnel_list_args args;
    memset(tunnel_servers,0,sizeof(tunnel_servers));
    args.idx = 0;
    config_foreach_list("main","tunnel",tunnel_apply_list, &args);
    for(i = 0; i < args.idx; i++)
    {

        const char *val = config_get_string(tunnel_servers[i].name, "type", "none");
        if (!strcmp(val, "none")) {
            tunnel_type = TUNNEL_NONE;
            log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "can not parse tunnel type");
            return 0;
        } else if (!strcmp(val, "gre")){
            tunnel_type = TUNNEL_GRE;
            global_tunnel_status = GRE_NONE;
        } else {

#ifdef __SC_BUILD__
            log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Invalid tunnel type: %s", val);
#else
            syslog(LOG_ERR, "Invalid tunnel type: %s", val);
#endif
            return -1;
        }
    }
	return tunnel_apply();
}


void tunnel_deinit() {
	if (!tunnel_type)
		return;


	if (tunnel_type == TUNNEL_GRE) {
		tunnel_close();
        global_tunnel_status = GRE_NONE;
	}
	free_tunnelservers();
}


static int tunnel_open(void) {
	int tunnelid = -1;

    for (int i = 0; i < TUNNEL_LISTCOUNT; ++i) {
        tunnel_status[i] = 0;
        if(!(gre_cfg[i].enable))
            continue;

        tunnelid = i;

        if (tunnel_type == TUNNEL_GRE) {
            DPRINTF("tunnel %d up\n",tunnelid);
            if(tunnel_gre_up(tunnelid) < 0)
            {
                tunnel_status[tunnelid] = 0;
                log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "tunnel %d can not setup \n", tunnelid);
                return -1;
            }
            else
            {
                tunnel_status[tunnelid] = 1;
            }
        } 

    }
    return 0;

}

static void tunnel_close()
{
    int tunnelid = -1;
    for (int i = 0; i < TUNNEL_LISTCOUNT; ++i) {
        if(!(gre_cfg[i].enable))
            continue;
        tunnelid = i;
        if (tunnel_type == TUNNEL_GRE) {
            tunnel_shutdown(tunnelid);
        }
    }
}



static void tunnel_shutdown(int tunnel_id) {
	if (tunnel_id < 0 || tunnel_id >= TUNNEL_LISTCOUNT)
		return;

#ifdef __SC_BUILD__
    log_fon(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Tunnel shutdown %i ",
				tunnel_id );
#else
	syslog(LOG_INFO, "Tunnel shutdown %i (code %i)",
				tunnel_id + 1);
#endif

    if(gre_state(gre_lac[tunnel_id]) != GRE_NONE)
    {
        gre_destroy(gre_lac[tunnel_id]);
        gre_lac[tunnel_id] = NULL;
    }
    if(gre_state(gre_lac_backup[tunnel_id]) != GRE_NONE)
    {
        gre_destroy(gre_lac_backup[tunnel_id]);
        gre_lac_backup[tunnel_id] = NULL;
    }
    tunnel_server_index[tunnel_id] = 0;
    return;
}


static int tunnel_gre_up(int tunnel_id) {
    gre_tries = 0;

    if(tunnel_id < 0 || tunnel_id >= TUNNEL_LISTCOUNT)
        return -1;

	if (tunnel_servers[tunnel_id].server_count < 1)
		return -1;

    if(!(gre_cfg[tunnel_id].enable))
        return -1;
#ifdef __SC_BUILD__
    log_fon(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "tunnel: Connecting to server %s",
            tunnel_servers[tunnel_id].server[tunnel_server_index[tunnel_id]]);
#else
    syslog(LOG_INFO, "tunnel: Connecting to LNS %s",
            tunnel_servers[tunnel_server_index[tunnel_id]]);
#endif
    struct gre_lac* lac = NULL;
    while(gre_state(lac) == GRE_NONE)
    {
        if(++gre_tries > tunnel_servers[tunnel_id].server_count)
        {
            gre_tries = 0;
            return -1;
        }
        if(tunnel_server_index[tunnel_id] == 0)   // primary server
        {
            gre_lac[tunnel_id] = gre_create(tunnel_id, tunnel_servers[tunnel_id].server[tunnel_server_index[tunnel_id]],
                    &gre_cfg[tunnel_id], tunnel_server_index[tunnel_id]);
            lac = gre_lac[tunnel_id];
        }
        else
        {
            gre_lac_backup[tunnel_id] = gre_create(tunnel_id, tunnel_servers[tunnel_id].server[tunnel_server_index[tunnel_id]],
                    &gre_cfg[tunnel_id], tunnel_server_index[tunnel_id]);
            lac = gre_lac_backup[tunnel_id];
        }
        tunnel_server_index[tunnel_id] = (tunnel_server_index[tunnel_id]+1) % tunnel_servers[tunnel_id].server_count;
    }
	return 0;
}
static void tunnel_gre_turn_down_ssid(int tunnel_id)
{
    struct SSID* p;
    list_for_each_entry(p, &(gre_cfg[tunnel_id].ssid), _head)
    {
        if(p->is_up == 1)
        {
            SYSTEM("ip link set %s down", p->interface);
#ifdef CONFIG_SUPPORT_5G_QD
            if(p->is_5g)
                SYSTEM("qcsapi_sockraw br0 00:26:86:00:00:00 enable_interface %s 0", ethif_mapping_to_qd_if(p->interface));
            else
#endif
            {
                SYSTEM("wlctl -i %s down", p->interface);
                SYSTEM("wlctl -i %s bss down", p->interface);
            }
            p->is_up = 0;
            log_fon(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "tunnel: %d disable ssid %s ",
                    tunnel_id, p->interface);
        }
    }
    return;
}

static void tunnel_gre_turn_up_ssid(int tunnel_id)
{
    struct SSID* p;
	int main_wifi_enable = config_get_int("main","main_wifi_enable",0); 
#ifdef CONFIG_SUPPORT_WIFI_5G
	int main_5g_wifi_enable = config_get_int("main","main_5g_wifi_enable",0);
#endif
    list_for_each_entry(p, &(gre_cfg[tunnel_id].ssid), _head)
    {
#ifdef CONFIG_SUPPORT_WIFI_5G
        if(p->is_5g)
        {
            if(!main_5g_wifi_enable)
                continue;
        }
        else
#endif
        {
            if(!main_wifi_enable)
                continue;
        }
        if(p->is_up == 0)
        {
            SYSTEM("ip link set %s up", p->interface);
#ifdef CONFIG_SUPPORT_5G_QD
            if(p->is_5g)
                SYSTEM("qcsapi_sockraw br0 00:26:86:00:00:00 enable_interface %s 1", ethif_mapping_to_qd_if(p->interface));
            else
#endif
            {
                SYSTEM("wlctl -i %s up", p->interface);
                SYSTEM("wlctl -i %s bss up", p->interface);
            }
            p->is_up = 1;
        }
    }
    return;
}
static int tunnel_gre_up_by_server_index(int tunnel_id, int server_index) 
{
    struct gre_lac* lac = NULL;
    if(server_index)
        lac = gre_lac_backup[tunnel_id];
    else
        lac = gre_lac[tunnel_id];
    if(lac)
    {
        gre_destroy(lac);
        lac = NULL;
    }
    if(server_index > 0 && server_index < tunnel_servers[tunnel_id].server_count)
    {
        if(gre_state(lac) == GRE_NONE)
        {
            if(server_index)
            {
                gre_lac_backup[tunnel_id] = gre_create(tunnel_id, tunnel_servers[tunnel_id].server[server_index],
                        &gre_cfg[tunnel_id], server_index);
            }
            else
            {
                gre_lac[tunnel_id] = gre_create(tunnel_id, tunnel_servers[tunnel_id].server[server_index],
                        &gre_cfg[tunnel_id], server_index);
            }
        }
    }
    else
    {
        log_fon(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "tunnel: there is no server %d for tunnel", server_index, tunnel_id);
    }
    return 0;
}
extern int getIFMac(char *if_name, char* mac);
static void gre_ifindex_init(struct gre_lac *lac)
{
    struct SSID* p;
    list_for_each_entry(p, &(lac->cfg->ssid), _head)
    {
        if(p->is_eap)
        {
#ifdef CONFIG_SUPPORT_WIFI_5G
            if(p->is_5g)
            {
                strncpy(routing_cfg.iface_eap_5g ,p->interface,sizeof(routing_cfg.iface_eap_5g));
                getIFMac(p->interface, (char *)routing_cfg.iface_addr_5g);
                routing_cfg.iface_eap_5g_index = if_nametoindex(p->bridge);
                snprintf(routing_cfg.eap_5g_ssid,sizeof(routing_cfg.eap_5g_ssid), "%s", p->name);
            }
            else
#endif
            {
                strncpy(routing_cfg.iface_eap,p->interface,sizeof(routing_cfg.iface_eap));
                getIFMac(p->interface, (char *)routing_cfg.iface_addr);
                routing_cfg.iface_eap_index = if_nametoindex(p->bridge);
                snprintf(routing_cfg.eap_ssid,sizeof(routing_cfg.eap_ssid), "%s", p->name);
            }
        }
        else
        {
#ifdef CONFIG_SUPPORT_WIFI_5G
            if(p->is_5g)
            {
                strncpy(routing_cfg.iface_5g,p->interface,sizeof(routing_cfg.iface_5g));
                routing_cfg.iface_5g_index = if_nametoindex(p->bridge);
            }
            else
#endif
            {
                strncpy(routing_cfg.iface,p->interface,sizeof(routing_cfg.iface));
                routing_cfg.iface_index = if_nametoindex(p->bridge);
            }
        }
    }
    
}
static void tunnel_gre_cb(enum gre_state state, struct gre_lac* lac)
{
    int tunnel_id = lac->tunnel_id;
    if(state == GRE_ESTABLISHED)
    {
        if(lac->is_up == 0)
        {
            chmod(get_setup_script(lac->wan_id, lac->tunnel_id, lac->is_backup),755);
            SYSTEM(get_setup_script(lac->wan_id, lac->tunnel_id, lac->is_backup));
            gre_ifindex_init(lac);
            lac->is_up = 1;
        }
        tunnel_gre_turn_up_ssid(tunnel_id);
        if(lac->is_backup)
        {
            gre_destroy(gre_lac[tunnel_id]);
            gre_lac[tunnel_id]= NULL;
        }
        else
        {
            gre_destroy(gre_lac_backup[tunnel_id]);
            gre_lac_backup[tunnel_id] = NULL;
        }
        global_tunnel_status = GRE_ESTABLISHED;
    }
    else if(state == GRE_UNREACHABLE)
    {
        gre_cleanup(lac);
        if(lac->is_backup)
        {
            if(gre_state(gre_lac[tunnel_id]) == GRE_NONE)
            {
                tunnel_gre_up_by_server_index(tunnel_id, 0); // setup primary server
            }
            else if(gre_state(gre_lac[tunnel_id]) == GRE_UNREACHABLE)
            {
                tunnel_gre_turn_down_ssid(tunnel_id);
            }
        }
        else
        {
            if(tunnel_servers[tunnel_id].server_count > 1)
            {
                if(gre_state(gre_lac_backup[tunnel_id]) == GRE_NONE)
                {
                    tunnel_gre_up_by_server_index(tunnel_id, 1); // setup backup server if have
                }
                else if(gre_state(gre_lac_backup[tunnel_id]) == GRE_UNREACHABLE)
                {
                    tunnel_gre_turn_down_ssid(tunnel_id);
                }
            }
            else
            {
                tunnel_gre_turn_down_ssid(tunnel_id);
            }
        }
        global_tunnel_status = GRE_UNREACHABLE;
    }
    return ;
}
int tunnel_gre_get_status(void)
{
    return global_tunnel_status;
}
//RESIDENT_REGISTER(tunnel, 105)
