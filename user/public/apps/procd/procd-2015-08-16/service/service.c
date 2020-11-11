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

#include <libubox/blobmsg_json.h>
#include <libubox/avl-cmp.h>

#include "../procd.h"

#include "service.h"
#include "instance.h"

#include "../rcS.h"
#ifdef __SC_BUILD__
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libubus.h>
#endif
struct avl_tree services;
static struct blob_buf b;
static struct ubus_context *ctx;
#ifdef __SC_BUILD__
struct avl_tree xservices;
static char name[64];
static char servs[128];
struct ubus_event_handler ev;
#define SERVICE_OBJECT_PREFIX "SoftwareModules.Service."
#define SERVICES_OBJECT "Services.Management.LCM.ExecutionEnvironments.%s.Services"
extern char *host;
static const struct blobmsg_policy eu_policy[9] = {
	[0] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[1] = { .name = "euid", .type = BLOBMSG_TYPE_STRING },
	[2] = { .name = "execenvlabel", .type = BLOBMSG_TYPE_STRING },
	[3] = { .name = "vendor", .type = BLOBMSG_TYPE_STRING },
	[4] = { .name = "version", .type = BLOBMSG_TYPE_STRING },
	[5] = { .name = "description", .type = BLOBMSG_TYPE_STRING },
	[6] = { .name = "executionenvref", .type = BLOBMSG_TYPE_STRING },
	[7] = { .name = "Package", .type = BLOBMSG_TYPE_STRING },
    [8] = { .name = "List", .type = BLOBMSG_TYPE_STRING},
};
static const struct blobmsg_policy ee_policy[3] = {
	[0] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[1] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
	[2] = { .name = "EE", .type = BLOBMSG_TYPE_STRING },
};
extern int CurrentRunLevel;
static struct blobmsg_policy servs_operate_policy[] = {
    [0] = { .name = "Operation", .type = BLOBMSG_TYPE_STRING},
    [1] = { .name = "name", .type = BLOBMSG_TYPE_ARRAY}
}; 
static struct uloop_timeout t;
static LIST_HEAD(remove_objs);
struct __remove_obj {
    struct list_head list;
    struct xservice *s;
};
static struct uloop_timeout monitor;
static int reserve_monitors = 0;
static unsigned long int totalcpu1=0, totalcpu2=0;

static unsigned long int __get_totalcputime()
{
    char buf[256] = "";
    unsigned long int a=0,b=0,c=0,d=0,e=0,f=0,g=0,h=0,i=0;
    FILE *fp = fopen("/proc/stat","r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        fclose(fp);
        sscanf(buf,"cpu  %lu %lu %lu %lu %lu %lu %lu %lu %lu", &a,&b,&c,&d,&e,&f,&g,&h,&i);
    }
    return a+b+c+d+e+f+g+h+i;
}
static  unsigned long int __get_cputime(pid_t pid)
{
    char buf[256] = "", file[64];
    char s[64];
    char p;
    int a,b,c,d,e,o;
    unsigned int f;
    unsigned long int g,i,j,h,k=0,l=0;
    long int m=0,n=0;
    FILE *fp;
    snprintf(file, sizeof(file), "/proc/%d/stat", pid);
    fp = fopen(file,"r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        fclose(fp);
        sscanf(buf,"%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld ",&o,s,&p, &a,&b,&c,&d,&e,&f,&g,&h,&i,&j,&k,&l,&m,&n);
    }
    return k+l+m+n;
}
static unsigned int __get_mem(pid_t pid)
{
    FILE *fp;
    char buf[256]="", f[64];
    int size=0,rss=0;
    snprintf(f, sizeof(f), "/proc/%d/statm", pid);
    fp = fopen(f, "r");
    if (fp)
    {
        if (fgets(buf, sizeof(buf), fp))
        {
            sscanf(buf, "%d %d", &size, &rss);
        }
        fclose(fp);
    }
    return rss*4;
}
static void __check_procs(struct xservice *s)
{
    char *f,*l;
    struct stat sa, sb;
	DIR *procdir;
	struct dirent *entry;
	pid_t pid;
    char buf[32];
    struct procs *p, *tmp;
    int new;
    list_for_each_entry_safe(p, tmp, &s->procs, list)
    {
        sprintf(buf, "/proc/%d/exe", p->pid);  
        if (0 != access(buf, F_OK))
        {
            list_del(&p->list);
            free(p); 
        }
    }
    if (NULL == s->list)
        return;
    l = strdup(s->list);
    if (NULL == l)
        return;
    f = strtok(l, ";");  
    while (NULL != f)
    {
	    if (stat(f, &sa) == 0)
        {
            procdir = opendir("/proc");
            if (procdir)
            {
                while ((entry = readdir(procdir)) != NULL)
                {
                    if (sscanf(entry->d_name, "%d", &pid) == 1)
                    {
                        sprintf(buf, "/proc/%d/exe", pid);  
                        if (stat(buf, &sb) == 0)
                        {
                            if (sb.st_dev == sa.st_dev && sb.st_ino == sa.st_ino)
                            {
                                new = 1;
                                list_for_each_entry(p, &s->procs,list)
                                {
                                    if (pid == p->pid)
                                    {
                                        p->cputime1 = p->cputime2;
                                        p->cputime2 = __get_cputime(pid);
                                        p->mem = __get_mem(pid);
                                        new = 0;
                                        break;
                                    }
                                }
                                if (1 == new)
                                {
                                    p = (struct procs *)calloc(1, sizeof(struct procs));
                                    if (p)
                                    {
                                        p->pid = pid; 
                                        p->cputime2 = __get_cputime(pid);
                                        list_add(&p->list, &s->procs);
                                    }
                                }
                            }
                        }
                    }

                }
                closedir(procdir);
            }
        }
        f = strtok(NULL, ";");
    }
    free(l);
}
static void __free_procs(struct xservice *s)
{
    struct procs *p, *tmp;
    list_for_each_entry_safe(p, tmp, &s->procs, list)
    {
        list_del(&p->list);
        free(p);
    }
}
static void __monitor_procs_timer_handler(struct uloop_timeout *t)
{
    struct xservice *s;
    if (0 == (reserve_monitors--))
    {
        totalcpu1 = 0;
        totalcpu2 = 0;
        avl_for_each_element(&xservices, s, avl)
        {
            __free_procs(s); 
        }
        return;
    }
    else
    {
        totalcpu1 = totalcpu2;
        totalcpu2 = __get_totalcputime();
        avl_for_each_element(&xservices, s, avl)
        {
            __check_procs(s);
        }
        uloop_timeout_set(t, 1000);
    }
    return;
}

static void __remove_ubus_obj(struct uloop_timeout *t)
{
    struct __remove_obj *obj,*tmp;
    list_for_each_entry_safe(obj, tmp, &remove_objs, list)
    {
        ubus_remove_object(ctx, &(obj->s->obj));
        list_del(&obj->list);
        free(obj);
        avl_delete(&xservices, &obj->s->avl);
        if (obj->s->list)
            free(obj->s->list);
        free((char *)obj->s->name);
        free(obj->s);
    }
}
static int __servs_operate(struct ubus_context *ctx, struct ubus_object *obj,
		 struct ubus_request_data *req, const char *method,
		 struct blob_attr *msg)
{
    struct blob_attr *tb[2] = {0}, *entry;
    char *ope;
    int rem;
	blobmsg_parse(servs_operate_policy, sizeof(servs_operate_policy)/sizeof(struct blobmsg_policy), tb, blobmsg_data(msg), blobmsg_len(msg));
    if (tb[0])
    {
        ope = blobmsg_get_string(tb[0]);
        if (tb[1])
        {
            blobmsg_for_each_attr(entry, tb[1], rem)
            {
                if (BLOBMSG_TYPE_STRING == blob_id(entry))
                {
                    if (0 == strcmp(ope, "start"))
                        rc(blobmsg_get_string(entry), "start");
                    else if (0 == strcmp(ope, "stop"))
                        rc(blobmsg_get_string(entry), "stop");
                }
            }      
        }
    }
    return UBUS_STATUS_OK;
}
static struct ubus_method servs_object_methods[] = 
{
    UBUS_METHOD("operate", __servs_operate, servs_operate_policy),
};
static struct ubus_object_type servs_object_type = UBUS_OBJECT_TYPE(servs, servs_object_methods);
static struct ubus_object servs_object = {
    .name = servs,
	.type = &servs_object_type,
	.methods = servs_object_methods,
	.n_methods = ARRAY_SIZE(servs_object_methods),
};

#endif
static void
service_instance_add(struct service *s, struct blob_attr *attr)
{
	struct service_instance *in;

	if (blobmsg_type(attr) != BLOBMSG_TYPE_TABLE)
		return;

	in = calloc(1, sizeof(*in));
	if (!in)
		return;

	instance_init(in, s, attr);
	vlist_add(&s->instances, &in->node, (void *) in->name);
}

static void
service_instance_update(struct vlist_tree *tree, struct vlist_node *node_new,
			struct vlist_node *node_old)
{
	struct service_instance *in_o = NULL, *in_n = NULL;

	if (node_old)
		in_o = container_of(node_old, struct service_instance, node);

	if (node_new)
		in_n = container_of(node_new, struct service_instance, node);

	if (in_o && in_n) {
		DEBUG(2, "Update instance %s::%s\n", in_o->srv->name, in_o->name);
		instance_update(in_o, in_n);
		instance_free(in_n);
	} else if (in_o) {
		DEBUG(2, "Free instance %s::%s\n", in_o->srv->name, in_o->name);
		instance_stop(in_o);
		instance_free(in_o);
	} else if (in_n) {
		DEBUG(2, "Create instance %s::%s\n", in_n->srv->name, in_n->name);
		instance_start(in_n);
	}
	blob_buf_init(&b, 0);
	trigger_event("instance.update", b.head);
}

static struct service *
service_alloc(const char *name)
{
	struct service *s;
	char *new_name;

	s = calloc_a(sizeof(*s), &new_name, strlen(name) + 1);
	strcpy(new_name, name);

	vlist_init(&s->instances, avl_strcmp, service_instance_update);
	s->instances.keep_old = true;
	s->name = new_name;
	s->avl.key = s->name;
	INIT_LIST_HEAD(&s->validators);

	return s;
}

enum {
	SERVICE_SET_NAME,
	SERVICE_SET_SCRIPT,
	SERVICE_SET_INSTANCES,
	SERVICE_SET_TRIGGER,
	SERVICE_SET_VALIDATE,
	__SERVICE_SET_MAX
};

static const struct blobmsg_policy service_set_attrs[__SERVICE_SET_MAX] = {
	[SERVICE_SET_NAME] = { "name", BLOBMSG_TYPE_STRING },
	[SERVICE_SET_SCRIPT] = { "script", BLOBMSG_TYPE_STRING },
	[SERVICE_SET_INSTANCES] = { "instances", BLOBMSG_TYPE_TABLE },
	[SERVICE_SET_TRIGGER] = { "triggers", BLOBMSG_TYPE_ARRAY },
	[SERVICE_SET_VALIDATE] = { "validate", BLOBMSG_TYPE_ARRAY },
};

static int
service_update(struct service *s, struct blob_attr **tb, bool add)
{
	struct blob_attr *cur;
	int rem;

	if (s->trigger) {
		trigger_del(s);
		free(s->trigger);
		s->trigger = NULL;
	}

	service_validate_del(s);

	if (tb[SERVICE_SET_TRIGGER] && blobmsg_data_len(tb[SERVICE_SET_TRIGGER])) {
		s->trigger = blob_memdup(tb[SERVICE_SET_TRIGGER]);
		if (!s->trigger)
			return -1;
		trigger_add(s->trigger, s);
	}

	if (tb[SERVICE_SET_VALIDATE] && blobmsg_data_len(tb[SERVICE_SET_VALIDATE])) {
		blobmsg_for_each_attr(cur, tb[SERVICE_SET_VALIDATE], rem)
			service_validate_add(s, cur);
	}

	if (tb[SERVICE_SET_INSTANCES]) {
		if (!add)
			vlist_update(&s->instances);
		blobmsg_for_each_attr(cur, tb[SERVICE_SET_INSTANCES], rem) {
			service_instance_add(s, cur);
		}
		if (!add)
			vlist_flush(&s->instances);
	}

	rc(s->name, "running");

	return 0;
}

static void
service_delete(struct service *s)
{
	service_event("service.stop", s->name, NULL);
	vlist_flush_all(&s->instances);
	avl_delete(&services, &s->avl);
	trigger_del(s);
	free(s->trigger);
	free(s);
	service_validate_del(s);
}

static void instance_delete(struct service *s)
{
	vlist_flush_all(&s->instances);
}
enum {
	SERVICE_ATTR_NAME,
	__SERVICE_ATTR_MAX,
};

static const struct blobmsg_policy service_attrs[__SERVICE_ATTR_MAX] = {
	[SERVICE_ATTR_NAME] = { "name", BLOBMSG_TYPE_STRING },
};

enum {
	SERVICE_DEL_ATTR_NAME,
	SERVICE_DEL_ATTR_INSTANCE,
	__SERVICE_DEL_ATTR_MAX,
};

static const struct blobmsg_policy service_del_attrs[__SERVICE_DEL_ATTR_MAX] = {
	[SERVICE_DEL_ATTR_NAME] = { "name", BLOBMSG_TYPE_STRING },
	[SERVICE_DEL_ATTR_INSTANCE] = { "instance", BLOBMSG_TYPE_STRING },
};

enum {
	SERVICE_LIST_ATTR_NAME,
	SERVICE_LIST_ATTR_VERBOSE,
	__SERVICE_LIST_ATTR_MAX,
};

static const struct blobmsg_policy service_list_attrs[__SERVICE_LIST_ATTR_MAX] = {
	[SERVICE_LIST_ATTR_NAME] = { "name", BLOBMSG_TYPE_STRING },
	[SERVICE_LIST_ATTR_VERBOSE] = { "verbose", BLOBMSG_TYPE_BOOL },
};

enum {
	EVENT_TYPE,
	EVENT_DATA,
	__EVENT_MAX
};

static const struct blobmsg_policy event_policy[__EVENT_MAX] = {
	[EVENT_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
	[EVENT_DATA] = { .name = "data", .type = BLOBMSG_TYPE_TABLE },
};

enum {
	VALIDATE_PACKAGE,
	VALIDATE_TYPE,
	VALIDATE_SERVICE,
	__VALIDATE_MAX
};

static const struct blobmsg_policy validate_policy[__VALIDATE_MAX] = {
	[VALIDATE_PACKAGE] = { .name = "package", .type = BLOBMSG_TYPE_STRING },
	[VALIDATE_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
	[VALIDATE_SERVICE] = { .name = "service", .type = BLOBMSG_TYPE_STRING },
};

enum {
	DATA_NAME,
	DATA_INSTANCE,
	DATA_TYPE,
	__DATA_MAX
};

static const struct blobmsg_policy get_data_policy[] = {
	[DATA_NAME] = { "name", BLOBMSG_TYPE_STRING },
	[DATA_INSTANCE] = { "instance", BLOBMSG_TYPE_STRING },
	[DATA_TYPE] = { "type", BLOBMSG_TYPE_STRING },
};

static int
service_handle_set(struct ubus_context *ctx, struct ubus_object *obj,
		   struct ubus_request_data *req, const char *method,
		   struct blob_attr *msg)
{
	struct blob_attr *tb[__SERVICE_SET_MAX], *cur;
	struct service *s = NULL;
	const char *name;
	bool add = !strcmp(method, "add");
	int ret;

	blobmsg_parse(service_set_attrs, __SERVICE_SET_MAX, tb, blob_data(msg), blob_len(msg));
	cur = tb[SERVICE_ATTR_NAME];
	if (!cur)
		return UBUS_STATUS_INVALID_ARGUMENT;

	name = blobmsg_data(cur);

	s = avl_find_element(&services, name, s, avl);
	if (s) {
		DEBUG(2, "Update service %s\n", name);
		return service_update(s, tb, add);
	}

	DEBUG(2, "Create service %s\n", name);
	s = service_alloc(name);
	if (!s)
		return UBUS_STATUS_UNKNOWN_ERROR;

	ret = service_update(s, tb, add);
	if (ret)
		return ret;

	avl_insert(&services, &s->avl);

	service_event("service.start", s->name, NULL);

	return 0;
}

static void
service_dump(struct service *s, bool verbose)
{
	struct service_instance *in;
	void *c, *i;

	c = blobmsg_open_table(&b, s->name);
#if 0//def __SC_BUILD__
    blobmsg_add_u32(&b, "index", s->index); 
    blobmsg_add_u8(&b, "autostart", s->autostart); 
    blobmsg_add_u32(&b, "runlevel", s->runlevel); 
    blobmsg_add_string(&b, "runlevel", s->euid); 
#endif

	if (!avl_is_empty(&s->instances.avl)) {
		i = blobmsg_open_table(&b, "instances");
		vlist_for_each_element(&s->instances, in, node)
			instance_dump(&b, in, verbose);
		blobmsg_close_table(&b, i);
	}
	if (verbose && s->trigger)
		blobmsg_add_blob(&b, s->trigger);
	if (verbose && !list_empty(&s->validators))
		service_validate_dump(&b, s);
	blobmsg_close_table(&b, c);
}

static int
service_handle_list(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method,
		    struct blob_attr *msg)
{
	struct blob_attr *tb[__SERVICE_LIST_ATTR_MAX];
	struct service *s;
	const char *name = NULL;
	bool verbose = false;

	blobmsg_parse(service_list_attrs, __SERVICE_LIST_ATTR_MAX, tb, blob_data(msg), blob_len(msg));

	if (tb[SERVICE_LIST_ATTR_VERBOSE])
		verbose = blobmsg_get_bool(tb[SERVICE_LIST_ATTR_VERBOSE]);
	if (tb[SERVICE_LIST_ATTR_NAME])
		name = blobmsg_get_string(tb[SERVICE_LIST_ATTR_NAME]);

	blob_buf_init(&b, 0);
	avl_for_each_element(&services, s, avl) {
		if (name && strcmp(s->name, name) != 0)
			continue;

		service_dump(s, verbose);
	}

	ubus_send_reply(ctx, req, b.head);

	return 0;
}

static int
service_handle_delete(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method,
		    struct blob_attr *msg)
{
	struct blob_attr *tb[__SERVICE_DEL_ATTR_MAX], *cur;
	struct service *s;
	struct service_instance *in;

	blobmsg_parse(service_del_attrs, __SERVICE_DEL_ATTR_MAX, tb, blob_data(msg), blob_len(msg));

	cur = tb[SERVICE_DEL_ATTR_NAME];
	if (!cur)
		return UBUS_STATUS_NOT_FOUND;

	s = avl_find_element(&services, blobmsg_data(cur), s, avl);
	if (!s)
		return UBUS_STATUS_NOT_FOUND;

	cur = tb[SERVICE_DEL_ATTR_INSTANCE];
	if (!cur) {
		service_delete(s);
		return 0;
	}

	in = vlist_find(&s->instances, blobmsg_data(cur), in, node);
	if (!in) {
		ERROR("instance %s not found\n", (char *) blobmsg_data(cur));
		return UBUS_STATUS_NOT_FOUND;
	}

	vlist_delete(&s->instances, &in->node);

	return 0;
}
static int
service_handle_delete_instance(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method,
		    struct blob_attr *msg)
{
	struct blob_attr *tb[__SERVICE_DEL_ATTR_MAX], *cur;
	struct service *s;

	blobmsg_parse(service_del_attrs, __SERVICE_DEL_ATTR_MAX, tb, blob_data(msg), blob_len(msg));

	cur = tb[SERVICE_DEL_ATTR_NAME];
	if (!cur)
		return UBUS_STATUS_NOT_FOUND;

	s = avl_find_element(&services, blobmsg_data(cur), s, avl);
	if (!s)
		return UBUS_STATUS_NOT_FOUND;
    instance_delete(s);
    return UBUS_STATUS_OK;
}

static int
service_handle_update(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__SERVICE_ATTR_MAX], *cur;
	struct service *s;

	blobmsg_parse(service_attrs, __SERVICE_ATTR_MAX, tb, blob_data(msg), blob_len(msg));

	cur = tb[SERVICE_ATTR_NAME];
	if (!cur)
		return UBUS_STATUS_INVALID_ARGUMENT;

	s = avl_find_element(&services, blobmsg_data(cur), s, avl);
	if (!s)
		return UBUS_STATUS_NOT_FOUND;

	if (!strcmp(method, "update_start"))
		vlist_update(&s->instances);
	else
		vlist_flush(&s->instances);

	return 0;
}

static int
service_handle_event(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct blob_attr *tb[__EVENT_MAX];

	if (!msg)
		return UBUS_STATUS_INVALID_ARGUMENT;

	blobmsg_parse(event_policy, __EVENT_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[EVENT_TYPE] || !tb[EVENT_DATA])
		return UBUS_STATUS_INVALID_ARGUMENT;

	trigger_event(blobmsg_get_string(tb[EVENT_TYPE]), tb[EVENT_DATA]);

	return 0;
}

static int
service_handle_validate(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct blob_attr *tb[__VALIDATE_MAX];
	char *p = NULL, *t = NULL;

	if (!msg)
		return UBUS_STATUS_INVALID_ARGUMENT;

	blobmsg_parse(validate_policy, __VALIDATE_MAX, tb, blob_data(msg), blob_len(msg));
	if (tb[VALIDATE_SERVICE]) {
		return 0;
	}
	if (tb[VALIDATE_PACKAGE])
		p = blobmsg_get_string(tb[VALIDATE_PACKAGE]);

	if (tb[VALIDATE_TYPE])
		t = blobmsg_get_string(tb[VALIDATE_TYPE]);

	blob_buf_init(&b, 0);
	service_validate_dump_all(&b, p, t);
	ubus_send_reply(ctx, req, b.head);

	return 0;
}

static int
service_get_data(struct ubus_context *ctx, struct ubus_object *obj,
		 struct ubus_request_data *req, const char *method,
		 struct blob_attr *msg)
{
	struct service_instance *in;
	struct service *s;
	struct blob_attr *tb[__DATA_MAX];
	const char *name = NULL;
	const char *instance = NULL;
	const char *type = NULL;

	blobmsg_parse(get_data_policy, __DATA_MAX, tb, blob_data(msg), blob_len(msg));
	if (tb[DATA_NAME])
		name = blobmsg_data(tb[DATA_NAME]);
	if (tb[DATA_INSTANCE])
		instance = blobmsg_data(tb[DATA_INSTANCE]);
	if (tb[DATA_TYPE])
		type = blobmsg_data(tb[DATA_TYPE]);

	blob_buf_init(&b, 0);
	avl_for_each_element(&services, s, avl) {
		void *cs = NULL;

		if (name && strcmp(name, s->name))
			continue;

		vlist_for_each_element(&s->instances, in, node) {
			struct blobmsg_list_node *var;
			void *ci = NULL;

			if (instance && strcmp(instance, in->name))
				continue;

			blobmsg_list_for_each(&in->data, var) {
				if (type &&
				    strcmp(blobmsg_name(var->data), type))
					continue;

				if (!cs)
					cs = blobmsg_open_table(&b, s->name);
				if (!ci)
					ci = blobmsg_open_table(&b, in->name);

				blobmsg_add_blob(&b, var->data);
			}

			if (ci)
				blobmsg_close_table(&b, ci);
		}

		if (cs)
			blobmsg_close_table(&b, cs);
	}

	ubus_send_reply(ctx, req, b.head);
	return 0;
}
#ifdef __SC_BUILD__
static const struct blobmsg_policy get_policy[] = {
	[0] = { "index", BLOBMSG_TYPE_INT32 },
};
static char faultcode[][32]=
{
    [NoFault] = "NoFault",
    [FailureOnStart] = "FailureOnStart",
    [FailureOnAutoStart] = "FailureOnAutoStart",
    [FailureOnStop] = "FailureOnStop",
    [FailureWhileActive] = "FailureWhileActive",
    [DependencyFailure] = "DependencyFailure",
    [UnStartable] = "UnStartable",
};
static int
service_handle_get(struct ubus_context *ctx, struct ubus_object *obj,
		 struct ubus_request_data *req, const char *method,
		 struct blob_attr *msg)
{
	struct blob_attr *tb[1];
    int index, found = 0;
	struct xservice *s;
	blobmsg_parse(get_policy, 1, tb, blob_data(msg), blob_len(msg));
    if (NULL == tb[0])
        return UBUS_STATUS_INVALID_ARGUMENT;
    index = blobmsg_get_u32(tb[0]);
	avl_for_each_element(&xservices, s, avl) 
    {
        if (index == s->index)
        {
            blob_buf_init(&b, 0);
            blobmsg_add_string(&b, "name", s->name?:"");
            blobmsg_add_string(&b, "euid", s->euid?:"");
            blobmsg_add_string(&b, "execenvlable", s->execenvlabel?:"");
#if 0
            if (0 == __process_running(s->name))
                blobmsg_add_string(&b, "status", "Idle");
            else
                blobmsg_add_string(&b, "status", "Active");
#endif
            blobmsg_add_string(&b, "executionfaultcode", faultcode[s->faultcode]);
            blobmsg_add_string(&b, "vendor", s->vendor?:"");
            blobmsg_add_string(&b, "version", s->version?:"");
            blobmsg_add_string(&b, "description", s->description?:"");
            blobmsg_add_string(&b, "executionenvref", s->executionenvref?:"");
            found = 1;
            break;
        }
    }
    if (1 == found)
    {
	    ubus_send_reply(ctx, req, b.head);
        blob_buf_free(&b);
	    return UBUS_STATUS_OK;
    }
    else
	    return UBUS_STATUS_NOT_FOUND;
}
static const struct blobmsg_policy state_policy[] = {
	[0] = { "index", BLOBMSG_TYPE_INT32 },
	[1] = { "state", BLOBMSG_TYPE_STRING },
};
static int
service_handle_state(struct ubus_context *ctx, struct ubus_object *obj,
		 struct ubus_request_data *req, const char *method,
		 struct blob_attr *msg)
{
	struct blob_attr *tb[2], *t2[9];
    int index;
	struct xservice *s;
    char *state = NULL;
    int found = 0;
    char path[1024];
    blobmsg_parse(state_policy, 2, tb, blob_data(msg), blob_len(msg));
    if (NULL == tb[0] ||  NULL == tb[1])
        return UBUS_STATUS_INVALID_ARGUMENT;
    index = blobmsg_get_u32(tb[0]);
    state = blobmsg_get_string(tb[1]);
	avl_for_each_element(&xservices, s, avl) 
    {
        if (index == s->index)
        {
            if (0 == strcmp(state, "Idle"))
                rc(s->name, "stop");
            else if (0 == strcmp(state, "Active"))
            {
                rc(s->name, "start");
            }
            found = 1;
            break;
        }
    }
    if (0 == found)
    {
        snprintf(path, sizeof(path), EUINFOS"%d", index);
        blob_buf_init(&b, 0);
        blobmsg_add_json_from_file(&b, path);
	    blobmsg_parse(eu_policy, sizeof(eu_policy)/sizeof(struct blobmsg_policy), t2, blob_data(b.head), blob_len(b.head));
        if (t2[0])
        {
            snprintf(path, sizeof(path), "%s", blobmsg_get_string(t2[0]));
            if (0 == strcmp(state, "Idle"))
            {
                rc(path, "stop");
                blob_buf_free(&b);
                return UBUS_STATUS_OK;
            }
            else if (0 == strcmp(state, "Active"))
            {
                rc(path, "start");
                blob_buf_free(&b);
                return UBUS_STATUS_OK;
            }
        }
        blob_buf_free(&b);
        return UBUS_STATUS_NOT_FOUND;
    }
    else
    {
        blob_buf_free(&b);
        return UBUS_STATUS_OK;
    }
}
#endif
static struct ubus_method main_object_methods[] = {
	UBUS_METHOD("set", service_handle_set, service_set_attrs),
	UBUS_METHOD("add", service_handle_set, service_set_attrs),
	UBUS_METHOD("list", service_handle_list, service_list_attrs),
	UBUS_METHOD("delete", service_handle_delete, service_del_attrs),
	UBUS_METHOD("update_start", service_handle_update, service_attrs),
	UBUS_METHOD("update_complete", service_handle_update, service_attrs),
	UBUS_METHOD("event", service_handle_event, event_policy),
	UBUS_METHOD("validate", service_handle_validate, validate_policy),
	UBUS_METHOD("get_data", service_get_data, get_data_policy),
#ifdef __SC_BUILD__
	UBUS_METHOD("get", service_handle_get, get_policy),
	UBUS_METHOD("state", service_handle_state, state_policy),
	UBUS_METHOD("delete_instance", service_handle_delete_instance, service_del_attrs),

#endif
};

static struct ubus_object_type main_object_type =
#ifdef __SC_BUILD__
	UBUS_OBJECT_TYPE(name, main_object_methods);
#else
	UBUS_OBJECT_TYPE("service", main_object_methods);
#endif

static struct ubus_object main_object = {
#ifdef __SC_BUILD__
    .name = name,
#else
	.name = "service",
#endif
	.type = &main_object_type,
	.methods = main_object_methods,
	.n_methods = ARRAY_SIZE(main_object_methods),
};

int
service_start_early(char *name, char *cmdline)
{
	void *instances, *instance, *command, *respawn;
	char *t;

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "name", name);
	instances = blobmsg_open_table(&b, "instances");
	instance = blobmsg_open_table(&b, "instance1");
	command = blobmsg_open_array(&b, "command");
	t = strtok(cmdline, " ");
	while (t) {
		blobmsg_add_string(&b, NULL, t);
		t = strtok(NULL, " ");
	}
	blobmsg_close_array(&b, command);
	respawn = blobmsg_open_array(&b, "respawn");
	blobmsg_add_string(&b, NULL, "3600");
	blobmsg_add_string(&b, NULL, "1");
	blobmsg_add_string(&b, NULL, "0");
	blobmsg_close_array(&b, respawn);
	blobmsg_close_table(&b, instance);
	blobmsg_close_table(&b, instances);

	return service_handle_set(NULL, NULL, NULL, "add", b.head);
}

void service_event(const char *type, const char *service, const char *instance)
{
	if (!ctx)
		return;

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "service", service);
	if (instance)
		blobmsg_add_string(&b, "instance", instance);
	ubus_notify(ctx, &main_object, type, b.head, -1);
}
#ifdef __SC_BUILD__
static void receive_event(struct ubus_context *ctx, struct ubus_event_handler *ev,
			  const char *type, struct blob_attr *msg)
{
    char *t;
    if (NULL == type || NULL== msg)
        return;
    t = malloc(strlen(type)+sizeof("event.")+1);
    if (NULL == t)
        return;
    snprintf(t, strlen(type)+sizeof("event.")+1, "event.%s", type);
    trigger_event(t, msg);
    if (t)
        free(t);
    return;
}
#endif
void ubus_init_service(struct ubus_context *_ctx)
{
#ifdef __SC_BUILD__
    memset(&ev, 0, sizeof(ev));
	ev.cb = receive_event;
    if (host)
        snprintf(name, sizeof(name), SERVICE_OBJECT_PREFIX"%s", host);
    else
        snprintf(name, sizeof(name), SERVICE_OBJECT_PREFIX"1");
#endif
	ctx = _ctx;
	ubus_add_object(ctx, &main_object);
#ifdef __SC_BUILD__
    ubus_register_event_handler(ctx, &ev, "*");
#endif
}

void
service_init(void)
{
	avl_init(&services, avl_strcmp, false, NULL);
	service_validate_init();
#ifdef __SC_BUILD__
	avl_init(&xservices, avl_strcmp, false, NULL);
#endif
}
#ifdef __SC_BUILD__
#define AUTOSTART_PATH "InternetGatewayDevice.SoftwareModules.ExecutionUnit.%d.AutoStart"
#define RUNLEVEL_PATH "InternetGatewayDevice.SoftwareModules.ExecutionUnit.%d.RunLevel"
#define EE_ENABLE_PATH "InternetGatewayDevice.SoftwareModules.ExecEnv.%d.Enable"
static const struct blobmsg_policy object_policy[1] = {
	[0] = { .name = "object", .type = BLOBMSG_TYPE_TABLE }
};
static void __get_res(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct blob_attr *tb[1], *t;
    char path[1024];
    char enable[1024];
    struct xservice *s = (struct xservice *)(req->priv);
    if (NULL == msg)
        return;
	blobmsg_parse(object_policy, 1, tb, blob_data(msg), blob_len(msg));
    if (tb[0])
    {
        t = blobmsg_data(tb[0]);
        if (t)
        {
            snprintf(path, sizeof(path), AUTOSTART_PATH, s->index);
            if (0 == strcmp(path, blobmsg_name(t)))
            {
                if (0 != strcmp(blobmsg_get_string(t), "1"))
                    s->autostart = false;
                else
                    s->autostart = true;
            }
            snprintf(path, sizeof(path), RUNLEVEL_PATH, s->index);
            if (0 == strcmp(path, blobmsg_name(t)))
            {
                s->runlevel = atoi(blobmsg_get_string(t));
            }
            snprintf(enable, sizeof(path), EE_ENABLE_PATH, s->index);
            if ( 0 == strcmp(enable, blobmsg_name(t)) )
            {
                if (0 != strcmp(blobmsg_get_string(t), "1"))
                    s->autostart = false;
                else
                    s->autostart = true;
            }
        }
    }
    return;
}
static void __get_autostart(struct xservice *s)
{
    void *tbl;
    unsigned int id;
    char path[128];
    if (ubus_lookup_id(ctx, "v1.DataModel", &id)<0)
        return;
	blob_buf_init(&b, 0);
    tbl = blobmsg_open_array(&b, "object");
    snprintf(path, sizeof(path), AUTOSTART_PATH, s->index);
    blobmsg_add_string(&b, NULL, path);
    blobmsg_close_table(&b, tbl);
    if (0 != ubus_invoke(ctx, id, "get", b.head, __get_res, s, 1000))
        s->autostart = true;
    blob_buf_free(&b);
}
static void __get_runlevel(struct xservice *s)
{
    void *tbl;
    unsigned int id;
    char path[128];
    if (ubus_lookup_id(ctx, "v1.DataModel", &id)<0)
        return;
	blob_buf_init(&b, 0);
    tbl = blobmsg_open_array(&b, "object");
    snprintf(path, sizeof(path), RUNLEVEL_PATH, s->index);
    blobmsg_add_string(&b, NULL, path);
    blobmsg_close_table(&b, tbl);
    if (0 != ubus_invoke(ctx, id, "get", b.head, __get_res, s, 1000))
        s->runlevel = -1;
    blob_buf_free(&b);

}
static void __get_ee_enable(struct xservice *s)
{
    void *tbl;
    unsigned int id;
    char path[128];
    if (ubus_lookup_id(ctx, "v1.DataModel", &id)<0)
        return;
	blob_buf_init(&b, 0);
    tbl = blobmsg_open_array(&b, "object");
    snprintf(path, sizeof(path), EE_ENABLE_PATH, s->index);
    blobmsg_add_string(&b, NULL, path);
    blobmsg_close_table(&b, tbl);
    if (0 != ubus_invoke(ctx, id, "get", b.head, __get_res, s, 1000))
        s->autostart = false;
    blob_buf_free(&b);

}
void load_services(struct ubus_context *_ctx)
{
	struct xservice *s;
    struct dirent *ent;
    DIR *d;
    struct blob_attr *tb[9];
    char path[128];
	
    ctx = _ctx;
    if (host)
        snprintf(servs, sizeof(servs), SERVICES_OBJECT, host);
    else
        snprintf(servs, sizeof(servs), SERVICES_OBJECT, "1");
	ubus_add_object(ctx, &servs_object);
    memset(&t, 0, sizeof(struct uloop_timeout));
    t.cb = __remove_ubus_obj;
    memset(&monitor, 0, sizeof(struct uloop_timeout));
    monitor.cb = __monitor_procs_timer_handler;
    d = opendir(EUINFOS);
    if (d)
    {
        while ((ent=readdir(d)))
        {
            if (ent->d_name[0] == '.')
                continue;
            snprintf(path, sizeof(path), EUINFOS"%s", ent->d_name);
            blob_buf_init(&b, 0);
            blobmsg_add_json_from_file(&b, path);
            blobmsg_parse(eu_policy, sizeof(eu_policy)/sizeof(struct blobmsg_policy), tb, blob_data(b.head), blob_len(b.head));
            if (tb[0])
            {
                s = calloc(1, sizeof(struct xservice));
                if (NULL == s)
                {
                    blob_buf_free(&b);
                    continue;
                }
                s->name = strdup(blobmsg_get_string(tb[0]));
                if (NULL == s->name)
                {
                    free(s);
                    blob_buf_free(&b);
                    continue;
                }
                s->avl.key = s->name;
                s->index = atoi(ent->d_name);
                if (tb[1])
                    snprintf(s->euid, sizeof(s->euid), "%s", blobmsg_get_string(tb[1]));
                if (tb[2])
                    snprintf(s->execenvlabel, sizeof(s->execenvlabel), "%s", blobmsg_get_string(tb[2]));
                if (tb[3])
                    snprintf(s->vendor, sizeof(s->vendor), "%s", blobmsg_get_string(tb[3]));
                if (tb[4])
                    snprintf(s->version, sizeof(s->version), "%s", blobmsg_get_string(tb[4]));
                if (tb[5])
                    snprintf(s->description, sizeof(s->description), "%s", blobmsg_get_string(tb[5]));
                if (tb[6])
                    snprintf(s->executionenvref, sizeof(s->executionenvref), "%s", blobmsg_get_string(tb[6]));
                if (tb[7])
                    snprintf(s->package, sizeof(s->package), "%s", blobmsg_get_string(tb[7]));
                else
                    s->package[0] = 0;
                if (tb[8])
                    s->list = strdup(blobmsg_get_string(tb[8]));
                __get_autostart(s);
                __get_runlevel(s);
                if (0 > avl_insert(&xservices, &s->avl))
                {
                    free((char *)s->name);
                    if (s->list)
                        free((char *)s->list);
                    free(s);
                    blob_buf_free(&b);
                    continue;
                }
                INIT_LIST_HEAD(&s->procs);
            }
        }
        closedir(d);
    }
    d = opendir(EEINFOS);
    if (d)
    {
        while ((ent=readdir(d)))
        {
            if (ent->d_name[0] == '.')
                continue;
            snprintf(path, sizeof(path), EEINFOS"%s", ent->d_name);
            blob_buf_init(&b, 0);
            blobmsg_add_json_from_file(&b, path);
            blobmsg_parse(ee_policy, sizeof(ee_policy)/sizeof(struct blobmsg_policy), tb, blob_data(b.head), blob_len(b.head));
            s = calloc(1, sizeof(struct xservice));
            if (NULL == s)
            {
                blob_buf_free(&b);
                continue;
            }
            s->name = strdup(blobmsg_get_string(tb[0]));
            if (NULL == s->name)
            {
                blob_buf_free(&b);
                free(s);
                continue;
            }
            if (tb[1])
                snprintf(s->euid, sizeof(s->euid), "%s", blobmsg_get_string(tb[1]));
            s->avl.key = s->name;
            s->index = atoi(ent->d_name);
            __get_ee_enable(s);
            s->runlevel = -1;
            if (0 > avl_insert(&xservices, &s->avl))
            {
                free((char *)s->name);
                if (s->list)
                    free((char *)s->list);
                free(s);
                blob_buf_free(&b);
                continue;
            }
            INIT_LIST_HEAD(&s->procs);
            blob_buf_free(&b);
        }
        closedir(d);
    }
}

int service_autostart(char *service)
{
    struct xservice *s;
    char buf[1024]={0};
    int run = 0;
    if (-1 == CurrentRunLevel)
        return 1;
    if ((0 == strcmp(service, "/etc/rc.d/Slcm_du"))
        || (0 == strcmp(service, "/etc/rc.d/S4uevent"))
        || (0 == strcmp(service, "/etc/rc.d/S1init"))
        || (0 == strcmp(service, "/etc/rc.d/S2SmartThings-EE"))
        || (0 == strcmp(service, "/etc/rc.d/S2Security-EE"))
        || (0 == strcmp(service, "/etc/rc.d/S6ftp"))
        || (0 == strcmp(service, "/etc/rc.d/S8samba"))
        || (0 == strcmp(service, "/etc/rc.d/S4smartthing_detector"))
        || (0 == strcmp(service, "/etc/rc.d/S7dlna")))
        return 1;
	avl_for_each_element(&xservices, s, avl) {
        if (0 == strncmp(service, "/etc/rc.d/S", strlen("/etc/rc.d/S")))
        {
            readlink(service, buf, sizeof(buf));
            if (0 == strcmp(&buf[strlen("../init.d/")], s->name))
            {
                if (false == s->autostart || CurrentRunLevel < s->runlevel)
                {
                    break;
                }
                else
                {
                    run = 1;
                    break;
                }
            }
        }
    }
    return run;
}

void service_update_status(char *service, int status)
{
	struct xservice *s;
    char buf[1024]={0};
    int find = 0;	
    avl_for_each_element(&xservices, s, avl) {
        if (0 == strncmp(service, "/etc/rc.d/S", strlen("/etc/rc.d/S")))
        {
            readlink(service, buf, sizeof(buf));
            if (0 == strcmp(&buf[strlen("../init.d/")], s->name))
            {
                find = 1;
                break;
            }
        }
        else if (0 == strncmp(service, "/etc/init.d/", strlen("/etc/init.d/")))
        {
            if (0 == strcmp(service+strlen("/etc/init.d/"), s->name))
            {
                find = 1;
                break;
            }
        }
        else
            ;
    }
    if (1 == find)
    {
        s->status = status;
    }
}
#endif

