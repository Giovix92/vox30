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

#ifndef __PROCD_SERVICE_H
#define __PROCD_SERVICE_H

#include <libubox/avl.h>
#include <libubox/vlist.h>
#include <libubox/list.h>

extern struct avl_tree services;

struct vrule {
	struct avl_node avl;
	char *option;
	char *rule;
};

struct validate {
	struct avl_node avl;
	struct list_head list;

	char *package;
	char *type;

	struct avl_tree rules;
};

struct service {
	struct avl_node avl;
	const char *name;

	struct blob_attr *trigger;
	struct vlist_tree instances;
	struct list_head validators;
};
#ifdef __SC_BUILD__
struct procs {
	struct list_head list;
    int pid;
    unsigned long int cputime1;
    unsigned long int cputime2;
    unsigned int mem;
};
struct xservice {
	struct avl_node avl;
	const char *name;
    char *list;
    int index;
    bool autostart;
    int runlevel;
    char euid[64];
    char execenvlabel[64];
    int status;
    int faultcode;
    char faultmessage[256];
    char vendor[128];
    char version[32];
    char description[256];
    char executionenvref[256];
    char package[256];
    struct ubus_object_type obj_type;
    struct ubus_object obj;
    struct list_head procs;
};
#endif

void service_validate_add(struct service *s, struct blob_attr *attr);
void service_validate_dump(struct blob_buf *b, struct service *s);
void service_validate_dump_all(struct blob_buf *b, char *p, char *s);
int service_start_early(char *name, char *cmdline);
void service_validate_del(struct service *s);
void service_validate_init(void);
void service_init(void);
void service_event(const char *type, const char *service, const char *instance);
#endif
