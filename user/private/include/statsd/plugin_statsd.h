#ifndef __PLUGIN_STATSD_H
#define __PLUGIN_STATSD_H

#include <dlfcn.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "common/list.h"

#ifdef CONFIG_SUPPORT_UBUS
#include <libubox/blobmsg.h>
#include <libubus.h>
#include <libubox/blob.h>
#endif

#define STATSD_LIBRARY_DIRECTORY        "/usr/lib/statsd"

typedef struct day_time_t{
    int hour;
    int min;
}day_time_s;

struct statsd {
    day_time_s start;
    day_time_s end;
    int interval;
    int next;
    int is_register;
    int index;
    int (*collect_cb)(struct statsd *ctx_s);
#ifdef CONFIG_SUPPORT_UBUS
    int (*register_cb)(struct statsd *ctx_s, struct ubus_context *ctx_u);
#endif
    int (*update_cb)(struct statsd *ctx_s);
    int (*clear_cb)(struct statsd *ctx_s);
    void *private_p;
    struct list_head list;
};

struct statsd_plugin{
    int (*init)(struct statsd *ctx);
};

int statsd_plugin_api_init(char *dpath);
int cp_wancfg_to_flash();
int cp_sysinfo_to_flash();

#endif
