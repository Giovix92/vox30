#ifndef __PRPL_PLUGIN_H__
#define __PRPL_PLUGIN_H__

#ifdef CONFIG_SUPPORT_PRPL_HL_API
#include <unistd.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>

#include <libubus.h>

/* location of prpl plugin libraries */
#define PRPLHLAPI_LIBRARY_DIRECTORY	"/usr/lib/prpl"

struct prpl_plugin {
    struct list_head list;
    int (*init)(struct ubus_context *ctx);
};
#endif

#endif
