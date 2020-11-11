/*
 * Copyright (C) 2013 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <libubox/uloop.h>

#include "procd.h"
#include "watchdog.h"
#ifdef __SC_BUILD__
#include <sys/reboot.h>
#include <dirent.h>
#include <libubox/blobmsg_json.h>
#endif
static struct blob_buf b;
#ifndef __SC_BUILD__
static int notify;
#endif
static struct ubus_context *_ctx;
#ifdef __SC_BUILD__
static char name[64];
#define SYSTEM_OBJECT_PREFIX "SoftwareModules.System."
#define INITIALRUNLEVEL_PATH "InternetGatewayDevice.SoftwareModules.ExecEnv.%d.InitialRunLevel"
extern char *host;
int CurrentRunLevel = 0;
static char path[512];
struct du {
	struct list_head list;
	char *name;
	char *url;
	char *uuid;
	char *version;
    char index[4];
};
static const struct blobmsg_policy du_policy[5] = {
	[0] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[1] = { .name = "url", .type = BLOBMSG_TYPE_STRING },
	[2] = { .name = "uuid", .type = BLOBMSG_TYPE_STRING },
	[3] = { .name = "version", .type = BLOBMSG_TYPE_STRING },
	[4] = { .name = "status", .type = BLOBMSG_TYPE_STRING },
};
static LIST_HEAD(dus);
static LIST_HEAD(backup_dus);

static int system_info(struct ubus_context *ctx, struct ubus_object *obj,
                 struct ubus_request_data *req, const char *method,
                 struct blob_attr *msg)
{
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", CurrentRunLevel);
    blob_buf_init(&b ,0);
    blobmsg_add_string(&b, "type", "Linux Container");
    blobmsg_add_string(&b, "version", "1.0.6");
    blobmsg_add_string(&b, "vendor", "");
    blobmsg_add_string(&b, "runlevel", buf);
    ubus_send_reply(ctx, req, b.head);
    blob_buf_free(&b);
    return UBUS_STATUS_OK;
}
static struct uloop_process reset_proc;
static void init_dus(char *path, struct list_head *l)
{
    INIT_LIST_HEAD(l); 
    DIR *dir; 
    struct dirent *ent;
    struct du *d;
    struct blob_attr *tb[4];
    char buf[256];
    INIT_LIST_HEAD(l);
    dir = opendir(path);
    if (dir)
    {
        while (NULL != (ent = readdir(dir)))
        {
            if ('.' == ent->d_name[0])
                continue;
            memset(&b, 0, sizeof(struct blob_buf));
            snprintf(buf, sizeof(buf), "%s/%s", path, ent->d_name);
            blob_buf_init(&b, 0);
            if (false == blobmsg_add_json_from_file(&b, buf))
                continue;
            d = calloc(1, sizeof(struct du));
            if (NULL == d)
                continue;
            snprintf(d->index, sizeof(d->index), "%s", ent->d_name);
            memset(tb, 0, sizeof(tb));
	        blobmsg_parse(du_policy, sizeof(du_policy)/sizeof(struct blobmsg_policy), tb, blob_data(b.head), blob_len(b.head));
            if (tb[0])
                d->name = strdup(blobmsg_get_string(tb[0]));
            if (tb[1])
                d->url = strdup(blobmsg_get_string(tb[1]));
            if (tb[2])
                d->uuid = strdup(blobmsg_get_string(tb[2]));
            if (tb[3])
                d->version = strdup(blobmsg_get_string(tb[3]));
            list_add_tail(&d->list, l);
            blob_buf_free(&b);
        }
        closedir(dir);
    }
}
static int __du_status(struct du *d)
{
    struct blob_attr *tb[5];
    struct blob_buf b;
    char buf[256];
    DIR *dir; 
    struct dirent *ent;
    int status = 0;
   
    dir = opendir(DUINFOS);
    if (dir)
    {
        while (NULL != (ent = readdir(dir)))
        {
            if ('.' == ent->d_name[0])
                continue;
            snprintf(buf, sizeof(buf), DUINFOS"%s", ent->d_name);
            memset(&b, 0, sizeof(struct blob_buf));
            blob_buf_init(&b, 0);
            if (false == blobmsg_add_json_from_file(&b, buf))
            {
                blob_buf_free(&b);
                continue;
            }
            memset(tb, 0, sizeof(tb));
            blobmsg_parse(du_policy, sizeof(du_policy)/sizeof(struct blobmsg_policy), tb, blob_data(b.head), blob_len(b.head));
            if ((NULL == tb[0]) 
                || (0 != strcmp(blobmsg_get_string(tb[0]), d->name))
                || (NULL == tb[2]) 
                || (0 != strcmp(blobmsg_get_string(tb[2]), d->uuid))
                || (NULL == tb[3]) 
                || (0 != strcmp(blobmsg_get_string(tb[3]), d->version))
                || (NULL == tb[4]) 
                || (0 != strcmp(blobmsg_get_string(tb[4]), "Installed")))
            {
                blob_buf_free(&b);
                continue;
            }
            status = 1;
            blob_buf_free(&b);
            break;
        }
        closedir(dir);
    }
    return status;
}
static void __free_du(struct du *d)
{
    if (d)
    {
        if (d->name)
            free(d->name);
        if (d->url)
            free(d->url);
        if (d->version)
            free(d->version);
        if (d->uuid)
            free(d->uuid);
        free(d);
    }
}
static void __dump_du(struct list_head *l)
{
    struct du *p;
    list_for_each_entry(p, l, list)
    {
        printf("index;%s,%s,%s,%s,%s.\n",p->index, p->name, p->uuid, p->version, p->url);
    }
printf("\n");
    
}
static int __find_du(struct du *d, struct list_head *l)
{
    struct du *p;
    int found = 0;
    list_for_each_entry(p, l, list)
    {
        if ((0 == strcmp(d->name, p->name))
            && (0 == strcmp(d->version, p->version))
            && (0 == strcmp(d->uuid, p->uuid)))
        {
            found = 1;
            break;
        }
    }
    return found;
}
static void __operate_du(struct du *d, char *op)
{
    unsigned int id;
    char object[128];
    struct ubus_context *ctx;
	ctx = ubus_connect(NULL);
    if (NULL == ctx)
        return;
    snprintf(object, sizeof(object), "SoftwareModules.Package.%s", host);
    if (ubus_lookup_id(ctx, object, &id) != 0)
    {
        ubus_free(ctx);
        return;
    }
    memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "UUID", d->uuid);
    blobmsg_add_string(&b, "URL", d->url);
    ubus_invoke(ctx, id, op, b.head, NULL, NULL, 1000);
    blob_buf_free(&b);
}
static void reset_proc_cb(struct uloop_process *proc, int ret)
{
    reboot(RB_AUTOBOOT);
}

static int system_reset(struct ubus_context *ctx, struct ubus_object *obj,
                 struct ubus_request_data *req, const char *method,
                 struct blob_attr *msg)
{
    reset_proc.cb = reset_proc_cb; 
	reset_proc.pid = fork();
    if (0 == reset_proc.pid)
    {
        struct du *d, *n;
        LIST_HEAD(dus);
        LIST_HEAD(backup_dus);

        //retrieve DUs
        init_dus(DUINFOS, &dus); 
        __dump_du(&dus);
        init_dus(BAKDUINFOS, &backup_dus); 
        __dump_du(&backup_dus);
        //uninstall non-backup DUs
	    list_for_each_entry_safe(d, n, &dus, list)
        {
            if (0 == __find_du(d, &backup_dus)) 
            {
                __operate_du(d, "Uninstall");
                list_del(&d->list);
                __free_du(d);
            }
        }
        __dump_du(&dus);
        __dump_du(&backup_dus);
        //install DUs;
	    list_for_each_entry(d, &backup_dus, list)
        {
            if (0 == __find_du(d, &dus))
            {
                __operate_du(d, "Install");
            }
        }
        __dump_du(&dus);
        __dump_du(&backup_dus);
        //conf_reset
        system("/bin/rm -r /etc/config/* && /bin/mv /etc/config.bak /etc/config");
        // wait for package operation completes
	    list_for_each_entry(d, &backup_dus, list)
        {
            while (0 == __du_status(d))
            {
                sleep(1);
            }
        }
        list_for_each_entry_safe(d, n, &dus, list)
        {
            list_del(&d->list);
            __free_du(d);
        }
	    list_for_each_entry_safe(d, n, &backup_dus, list)
        {
            list_del(&d->list);
            __free_du(d);
        } 
        sleep(3); // wait for Installing last eu completes
        exit(0);
    }
    else if (0 > reset_proc.pid)
        return UBUS_STATUS_UNKNOWN_ERROR;
	uloop_process_add(&reset_proc);
    return UBUS_STATUS_OK;
}
static int system_backup(struct ubus_context *ctx, struct ubus_object *obj,
                 struct ubus_request_data *req, const char *method,
                 struct blob_attr *msg)
{
    //du_backup
    //eu_backup
    system("/bin/rm -r /usr/local/package.bak");

    system("/bin/cp -r /usr/local/package /usr/local/package.bak");
    //conf_backup
    system("/bin/cp -r /etc/config /etc/config.bak");
    return UBUS_STATUS_OK;
}
static int system_alive(struct ubus_context *ctx, struct ubus_object *obj,
                 struct ubus_request_data *req, const char *method,
                 struct blob_attr *msg)
{
    return UBUS_STATUS_OK;
}
static const struct blobmsg_policy set_policy[1] = {
	[0] = { .name = "runlevel", .type = BLOBMSG_TYPE_INT32 },
};
static int system_set(struct ubus_context *ctx, struct ubus_object *obj,
                 struct ubus_request_data *req, const char *method,
                 struct blob_attr *msg)
{
    struct blob_attr *tb[1];
	blobmsg_parse(set_policy, sizeof(set_policy)/sizeof(struct blobmsg_policy), tb, blob_data(msg), blob_len(msg));
    if (tb[0])
    {
        CurrentRunLevel = blobmsg_get_u32(tb[0]); 
    }
    return UBUS_STATUS_OK;
}

static const struct ubus_method system_methods[] = {
	UBUS_METHOD_NOARG("info", system_info),
	UBUS_METHOD_NOARG("reset", system_reset),
	UBUS_METHOD_NOARG("backup", system_backup),
	UBUS_METHOD_NOARG("alive", system_alive),
    UBUS_METHOD("set", system_set, set_policy),
};
#else

int upgrade_running = 0;

static int system_board(struct ubus_context *ctx, struct ubus_object *obj,
                 struct ubus_request_data *req, const char *method,
                 struct blob_attr *msg)
{
	void *c;
	char line[256];
	char *key, *val, *next;
	struct utsname utsname;
	FILE *f;

	blob_buf_init(&b, 0);

	if (uname(&utsname) >= 0)
	{
		blobmsg_add_string(&b, "kernel", utsname.release);
		blobmsg_add_string(&b, "hostname", utsname.nodename);
	}

	if ((f = fopen("/proc/cpuinfo", "r")) != NULL)
	{
		while(fgets(line, sizeof(line), f))
		{
			key = strtok(line, "\t:");
			val = strtok(NULL, "\t\n");

			if (!key || !val)
				continue;

			if (!strcasecmp(key, "system type") ||
			    !strcasecmp(key, "processor") ||
			    !strcasecmp(key, "model name"))
			{
				strtoul(val + 2, &key, 0);

				if (key == (val + 2) || *key != 0)
				{
					blobmsg_add_string(&b, "system", val + 2);
					break;
				}
			}
		}

		fclose(f);
	}

	if ((f = fopen("/tmp/sysinfo/model", "r")) != NULL ||
	    (f = fopen("/proc/device-tree/model", "r")) != NULL)
	{
		if (fgets(line, sizeof(line), f))
		{
			val = strtok(line, "\t\n");

			if (val)
				blobmsg_add_string(&b, "model", val);
		}

		fclose(f);
	}
	else if ((f = fopen("/proc/cpuinfo", "r")) != NULL)
	{
		while(fgets(line, sizeof(line), f))
		{
			key = strtok(line, "\t:");
			val = strtok(NULL, "\t\n");

			if (!key || !val)
				continue;

			if (!strcasecmp(key, "machine") ||
			    !strcasecmp(key, "hardware"))
			{
				blobmsg_add_string(&b, "model", val + 2);
				break;
			}
		}

		fclose(f);
	}

	if ((f = fopen("/etc/openwrt_release", "r")) != NULL)
	{
		c = blobmsg_open_table(&b, "release");

		while (fgets(line, sizeof(line), f))
		{
			char *dest;
			char ch;

			key = line;
			val = strchr(line, '=');
			if (!val)
				continue;

			*(val++) = 0;

			if (!strcasecmp(key, "DISTRIB_ID"))
				key = "distribution";
			else if (!strcasecmp(key, "DISTRIB_RELEASE"))
				key = "version";
			else if (!strcasecmp(key, "DISTRIB_REVISION"))
				key = "revision";
			else if (!strcasecmp(key, "DISTRIB_CODENAME"))
				key = "codename";
			else if (!strcasecmp(key, "DISTRIB_TARGET"))
				key = "target";
			else if (!strcasecmp(key, "DISTRIB_DESCRIPTION"))
				key = "description";
			else
				continue;

			dest = blobmsg_alloc_string_buffer(&b, key, strlen(val));
			if (!dest) {
				ERROR("Failed to allocate blob.\n");
				continue;
			}

			while (val && (ch = *(val++)) != 0) {
				switch (ch) {
				case '\'':
				case '"':
					next = strchr(val, ch);
					if (next)
						*next = 0;

					strcpy(dest, val);

					if (next)
						val = next + 1;

					dest += strlen(dest);
					break;
				case '\\':
					*(dest++) = *(val++);
					break;
				}
			}
			blobmsg_add_string_buffer(&b);
		}

		blobmsg_close_array(&b, c);

		fclose(f);
	}

	ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}

static int system_info(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
	void *c;
	time_t now;
	struct tm *tm;
	struct sysinfo info;

	now = time(NULL);

	if (!(tm = localtime(&now)))
		return UBUS_STATUS_UNKNOWN_ERROR;

	if (sysinfo(&info))
		return UBUS_STATUS_UNKNOWN_ERROR;

	blob_buf_init(&b, 0);

	blobmsg_add_u32(&b, "uptime",    info.uptime);
	blobmsg_add_u32(&b, "localtime", mktime(tm));

	c = blobmsg_open_array(&b, "load");
	blobmsg_add_u32(&b, NULL, info.loads[0]);
	blobmsg_add_u32(&b, NULL, info.loads[1]);
	blobmsg_add_u32(&b, NULL, info.loads[2]);
	blobmsg_close_array(&b, c);

	c = blobmsg_open_table(&b, "memory");
	blobmsg_add_u64(&b, "total",    info.mem_unit * info.totalram);
	blobmsg_add_u64(&b, "free",     info.mem_unit * info.freeram);
	blobmsg_add_u64(&b, "shared",   info.mem_unit * info.sharedram);
	blobmsg_add_u64(&b, "buffered", info.mem_unit * info.bufferram);
	blobmsg_close_table(&b, c);

	c = blobmsg_open_table(&b, "swap");
	blobmsg_add_u64(&b, "total",    info.mem_unit * info.totalswap);
	blobmsg_add_u64(&b, "free",     info.mem_unit * info.freeswap);
	blobmsg_close_table(&b, c);

	ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}

static int system_upgrade(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	upgrade_running = 1;
	return 0;
}

enum {
	WDT_FREQUENCY,
	WDT_TIMEOUT,
	WDT_STOP,
	__WDT_MAX
};

static const struct blobmsg_policy watchdog_policy[__WDT_MAX] = {
	[WDT_FREQUENCY] = { .name = "frequency", .type = BLOBMSG_TYPE_INT32 },
	[WDT_TIMEOUT] = { .name = "timeout", .type = BLOBMSG_TYPE_INT32 },
	[WDT_STOP] = { .name = "stop", .type = BLOBMSG_TYPE_BOOL },
};

static int watchdog_set(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct blob_attr *tb[__WDT_MAX];
	const char *status;

	if (!msg)
		return UBUS_STATUS_INVALID_ARGUMENT;

	blobmsg_parse(watchdog_policy, __WDT_MAX, tb, blob_data(msg), blob_len(msg));
	if (tb[WDT_FREQUENCY]) {
		unsigned int timeout = watchdog_timeout(0);
		unsigned int freq = blobmsg_get_u32(tb[WDT_FREQUENCY]);

		if (freq) {
			if (freq > timeout / 2)
				freq = timeout / 2;
			watchdog_frequency(freq);
		}
	}

	if (tb[WDT_TIMEOUT]) {
		unsigned int timeout = blobmsg_get_u32(tb[WDT_TIMEOUT]);
		unsigned int frequency = watchdog_frequency(0);

		if (timeout <= frequency)
			timeout = frequency * 2;
		 watchdog_timeout(timeout);
	}

	if (tb[WDT_STOP])
		watchdog_set_stopped(blobmsg_get_bool(tb[WDT_STOP]));

	if (watchdog_fd() < 0)
		status = "offline";
	else if (watchdog_get_stopped())
		status = "stopped";
	else
		status = "running";

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "status", status);
	blobmsg_add_u32(&b, "timeout", watchdog_timeout(0));
	blobmsg_add_u32(&b, "frequency", watchdog_frequency(0));
	ubus_send_reply(ctx, req, b.head);

	return 0;
}

enum {
	SIGNAL_PID,
	SIGNAL_NUM,
	__SIGNAL_MAX
};

static const struct blobmsg_policy signal_policy[__SIGNAL_MAX] = {
	[SIGNAL_PID] = { .name = "pid", .type = BLOBMSG_TYPE_INT32 },
	[SIGNAL_NUM] = { .name = "signum", .type = BLOBMSG_TYPE_INT32 },
};

static int proc_signal(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct blob_attr *tb[__SIGNAL_MAX];

	if (!msg)
		return UBUS_STATUS_INVALID_ARGUMENT;

	blobmsg_parse(signal_policy, __SIGNAL_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[SIGNAL_PID || !tb[SIGNAL_NUM]])
		return UBUS_STATUS_INVALID_ARGUMENT;

	kill(blobmsg_get_u32(tb[SIGNAL_PID]), blobmsg_get_u32(tb[SIGNAL_NUM]));

	return 0;
}

enum {
	NAND_PATH,
	__NAND_MAX
};

static const struct blobmsg_policy nand_policy[__NAND_MAX] = {
	[NAND_PATH] = { .name = "path", .type = BLOBMSG_TYPE_STRING },
};

static void
procd_spawn_upgraded(char *path)
{
	char *wdt_fd = watchdog_fd();
	char *argv[] = { "/tmp/upgraded", NULL, NULL};

	argv[1] = path;

	DEBUG(2, "Exec to upgraded now\n");
	if (wdt_fd) {
		watchdog_no_cloexec();
		setenv("WDTFD", wdt_fd, 1);
	}
	execvp(argv[0], argv);
}

static int nand_set(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct blob_attr *tb[__NAND_MAX];

	if (!msg)
		return UBUS_STATUS_INVALID_ARGUMENT;

	blobmsg_parse(nand_policy, __NAND_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NAND_PATH])
		return UBUS_STATUS_INVALID_ARGUMENT;

	procd_spawn_upgraded(blobmsg_get_string(tb[NAND_PATH]));
	fprintf(stderr, "Yikees, something went wrong. no /sbin/upgraded ?\n");
	return 0;
}

static void
procd_subscribe_cb(struct ubus_context *ctx, struct ubus_object *obj)
{
	notify = obj->has_subscribers;
}


static const struct ubus_method system_methods[] = {
	UBUS_METHOD_NOARG("board", system_board),
	UBUS_METHOD_NOARG("info",  system_info),
	UBUS_METHOD_NOARG("upgrade", system_upgrade),
	UBUS_METHOD("watchdog", watchdog_set, watchdog_policy),
	UBUS_METHOD("signal", proc_signal, signal_policy),

	/* must remain at the end as it ia not always loaded */
	UBUS_METHOD("nandupgrade", nand_set, nand_policy),
};
#endif

static struct ubus_object_type system_object_type =
#ifdef __SC_BUILD__
	UBUS_OBJECT_TYPE(name, system_methods);
#else
	UBUS_OBJECT_TYPE("system", system_methods);
#endif

static struct ubus_object system_object = {
#ifdef __SC_BUILD__
    .name = name,
#else
	.name = "system",
#endif
	.type = &system_object_type,
	.methods = system_methods,
	.n_methods = ARRAY_SIZE(system_methods),
#ifndef __SC_BUILD__
	.subscribe_cb = procd_subscribe_cb,
#endif
};
#ifdef __SC_BUILD__
static const struct blobmsg_policy object_policy[1] = {
	[0] = { .name = "object", .type = BLOBMSG_TYPE_TABLE }
};
static const struct blobmsg_policy path_policy[1] = {
	[0] = { .name = path, .type = BLOBMSG_TYPE_STRING }
};
static void __get_runlevel_res(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct blob_attr *t1[1],*t2[1];
    if (NULL == msg)
        return;
	blobmsg_parse(object_policy, 1, t1, blob_data(msg), blob_len(msg));
    if (t1[0])
    {
	    blobmsg_parse(path_policy, 1, t2, blobmsg_data(t1[0]), blobmsg_len(t1[0]));
        if (t2[0])
        {
            CurrentRunLevel = atoi(blobmsg_get_string(t2[0])); 
        }
    }
    return;
}
static void __get_ee_index(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct blob_attr *tb[1] = {0};
    struct blobmsg_policy ply[] = {
        [0] = { .name = "Index", .type = BLOBMSG_TYPE_INT32},
    };
    int *p=(int *)(req->priv);
    blobmsg_parse(ply, sizeof(ply)/sizeof(struct blobmsg_policy), tb, blob_data(msg), blob_len(msg));
    if (tb[0])
        *p = blobmsg_get_u32(tb[0]);
}
static void __init_runlevel()
{
    void *tbl;
    unsigned int id;
    int ee_index;
    if (ubus_lookup_id(_ctx, "Services.Management.LCM.ExecutionEnvironments", &id))
        return;
    memset(&b, 0, sizeof(struct blob_buf));
    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "Type", "ExecutionEnvironment");
    tbl = blobmsg_open_table(&b, "Filters");
    blobmsg_add_string(&b, "Name", host);
    blobmsg_close_table(&b, tbl);
    if (0 != ubus_invoke(_ctx, id, "catalog.get", b.head, __get_ee_index, &ee_index, 1000))
    {
        blob_buf_free(&b);
        return;
    }
    blob_buf_free(&b);
    if (ubus_lookup_id(_ctx, "v1.DataModel", &id)<0)
        return;
    memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init(&b, 0);
    tbl = blobmsg_open_array(&b, "object");
    snprintf(path, sizeof(path), INITIALRUNLEVEL_PATH, ee_index);
    blobmsg_add_string(&b, NULL, path);
    blobmsg_close_table(&b, tbl);
    ubus_invoke(_ctx, id, "get", b.head, __get_runlevel_res, NULL, 1000);
    blob_buf_free(&b);
}
#else
void
procd_bcast_event(char *event, struct blob_attr *msg)
{
	int ret;

	if (!notify)
		return;

	ret = ubus_notify(_ctx, &system_object, event, msg, -1);
	if (ret)
		fprintf(stderr, "Failed to notify log: %s\n", ubus_strerror(ret));
}
#endif
void ubus_init_system(struct ubus_context *ctx)
{
#ifndef __SC_BUILD__
	struct stat s;
#endif
	int ret;

#ifdef __SC_BUILD__
    if (host)
        snprintf(name, sizeof(name), SYSTEM_OBJECT_PREFIX"%s", host);
    else
        snprintf(name, sizeof(name), SYSTEM_OBJECT_PREFIX"1");
#else
	if (stat("/sbin/upgraded", &s))
		system_object.n_methods -= 1;
#endif
	_ctx = ctx;
	ret = ubus_add_object(ctx, &system_object);
	if (ret)
		ERROR("Failed to add object: %s\n", ubus_strerror(ret));
#ifdef __SC_BUILD__
    __init_runlevel();
#endif
}
