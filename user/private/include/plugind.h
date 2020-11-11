#ifndef __PLUGIND_H
#define __PLUGIND_H

#include <dlfcn.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <json.h>

/* location of plugin libraries */
#define WEBAPI_LIBRARY_DIRECTORY	"/usr/lib/rpcd"

struct rpc_daemon_ops {
};

struct rpc_plugin {
    struct list_head list;
    int (*init)(struct ubus_context *ctx);
};

int rpc_plugin_api_init(struct ubus_context *ctx, char *dpath);

#endif
