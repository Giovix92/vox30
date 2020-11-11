#include "config.h"
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <libubox/blob.h>
#include <libubox/blobmsg.h>
#include <libubus.h>
#include "opkg_conf.h"
#include "opkg_cmd.h"
#include "pkg_vec.h"
#include "pkg.h"
#include "pkg_parse.h"
#include "file_util.h"
#include "opkg_message.h"
#include "../libbb/libbb.h"

static struct ubus_context *ctx;
static struct blob_buf b;

enum {
    PACKAGE_ID,
    PACKAGE_MAX
}PACKAGE_POLICY;
static const struct blobmsg_policy list_policy[] = 
{
    [PACKAGE_ID] = {.name = "package", .type = BLOBMSG_TYPE_STRING}
};
enum {
    UPDATE_REPOSITORY,
    UPDATE_MAX
}UPDATE_POLICY;
static const struct blobmsg_policy update_policy[] = 
{
    [UPDATE_REPOSITORY] = {.name = "repository", .type = BLOBMSG_TYPE_STRING}
};

static int __list(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    int i;
    pkg_vec_t *available;
    pkg_t *pkg;
    char *pkg_name = NULL, *version;
    struct blob_attr *tb[PACKAGE_MAX];
    void *tbl;
    char buf[512]; //the length of package name ??
    if (msg){
        blobmsg_parse(list_policy, ARRAY_SIZE(list_policy), tb, blob_data(msg), blob_len(msg));
        if (tb[PACKAGE_ID])
        {
            pkg_name = blobmsg_data(tb[PACKAGE_ID]);
        }
    }
    conf->pfm = PFM_SOURCE;
    blob_buf_init(&b, 0);
    
    if (opkg_conf_init())
		goto err0;
    if (pkg_hash_load_feeds())
        goto err1;

    if (pkg_hash_load_status_files())
        goto err1;

    available = pkg_vec_alloc();
    pkg_hash_fetch_available(available);
    pkg_vec_sort(available, pkg_compare_names);

    tbl = blobmsg_open_array(&b, "Packages");
    for (i=0; i < available->len; i++) {
        pkg = available->pkgs[i];
	    if (pkg_name && fnmatch(pkg_name, pkg->name, 0)) 
	        continue;
        version = pkg_version_str_alloc(pkg);
        snprintf(buf, sizeof(buf), "%s - %s", pkg->name, version?:"");
        blobmsg_add_string(&b, NULL, buf);
        if (version)
        {
            free(version);
            version = NULL;
        }
    }
    blobmsg_close_table(&b, tbl);
    pkg_vec_free(available);
#ifdef HAVE_CURL
	opkg_curl_cleanup();
#endif
err1:
	opkg_conf_deinit();
err0:
    ubus_send_reply(ctx, req, b.head);
    blob_buf_free(&b);
	free_error_list();
    return UBUS_STATUS_OK;
}
static int __install(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    int err= -1;
    char *pkg_name;
    struct blob_attr *tb[PACKAGE_MAX];
	opkg_cmd_t *cmd;

    if (msg){
        blobmsg_parse(list_policy, ARRAY_SIZE(list_policy), tb, blob_data(msg), blob_len(msg));
        if (tb[PACKAGE_ID])
        {
            pkg_name = blobmsg_data(tb[PACKAGE_ID]);
        }
        else
            return UBUS_STATUS_INVALID_ARGUMENT; 
    }
    else
        return UBUS_STATUS_INVALID_ARGUMENT; 
    blob_buf_init(&b, 0);
	cmd = opkg_cmd_find("install");
    if (NULL == cmd)
        return UBUS_STATUS_INVALID_ARGUMENT;
    conf->pfm = cmd->pfm;
    if (opkg_conf_init())
		goto err0;
    if (pkg_hash_load_feeds())
        goto err1;

    if (pkg_hash_load_status_files())
        goto err1;
	err = opkg_cmd_exec(cmd, 1, (const char **) (&pkg_name));

#ifdef HAVE_CURL
	opkg_curl_cleanup();
#endif
err1:
	opkg_conf_deinit();
err0:
    if (err)
    {
        blobmsg_add_u32(&b, "result", -1);
    }
    else
        blobmsg_add_u32(&b, "result", 0);
    ubus_send_reply(ctx, req, b.head);
    blob_buf_free(&b);
	free_error_list();
    return UBUS_STATUS_OK;
}
static int __remove(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    int err= -1;
    char *pkg_name;
    struct blob_attr *tb[PACKAGE_MAX];
	opkg_cmd_t *cmd;

    if (msg){
        blobmsg_parse(list_policy, ARRAY_SIZE(list_policy), tb, blob_data(msg), blob_len(msg));
        if (tb[PACKAGE_ID])
        {
            pkg_name = blobmsg_data(tb[PACKAGE_ID]);
        }
        else
            return UBUS_STATUS_INVALID_ARGUMENT; 
    }
    else
        return UBUS_STATUS_INVALID_ARGUMENT; 
    cmd = opkg_cmd_find("remove");
    if (NULL == cmd)
        return UBUS_STATUS_INVALID_ARGUMENT;
    conf->pfm = cmd->pfm;
  
    blob_buf_init(&b, 0);
    if (opkg_conf_init())
		goto err0;

    if (pkg_hash_load_status_files())
        goto err1;
    
	err = opkg_cmd_exec(cmd, 1, (const char **) (&pkg_name));
#ifdef HAVE_CURL
	opkg_curl_cleanup();
#endif
err1:
	opkg_conf_deinit();
err0:
    if (err)
    {
        blobmsg_add_u32(&b, "result", -1);
    }
    else
        blobmsg_add_u32(&b, "result", 0);
    ubus_send_reply(ctx, req, b.head);
    blob_buf_free(&b);
	free_error_list();
    return UBUS_STATUS_OK;
}
static int __upgrade(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    int err= -1;
    char *pkg_name;
    struct blob_attr *tb[PACKAGE_MAX];
	opkg_cmd_t *cmd;

    if (msg){
        blobmsg_parse(list_policy, ARRAY_SIZE(list_policy), tb, blob_data(msg), blob_len(msg));
        if (tb[PACKAGE_ID])
        {
            pkg_name = blobmsg_data(tb[PACKAGE_ID]);
        }
        else
            return UBUS_STATUS_INVALID_ARGUMENT; 
    }
    else
        return UBUS_STATUS_INVALID_ARGUMENT; 
    blob_buf_init(&b, 0);
	cmd = opkg_cmd_find("upgrade");
    if (NULL == cmd)
        return UBUS_STATUS_INVALID_ARGUMENT;
    conf->pfm = cmd->pfm;
    if (opkg_conf_init())
		goto err0;
    if (pkg_hash_load_feeds())
        goto err1;

    if (pkg_hash_load_status_files())
        goto err1;
	err = opkg_cmd_exec(cmd, 1, (const char **) (&pkg_name));

#ifdef HAVE_CURL
	opkg_curl_cleanup();
#endif
err1:
	opkg_conf_deinit();
err0:
    if (err)
    {
        blobmsg_add_u32(&b, "result", -1);
    }
    else
        blobmsg_add_u32(&b, "result", 0);
    ubus_send_reply(ctx, req, b.head);
    blob_buf_free(&b);
	free_error_list();
    return UBUS_STATUS_OK;
}
static int __update(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    struct blob_attr *tb[UPDATE_MAX];
    FILE *fp ;
	opkg_cmd_t *cmd;
    int err;
    
    if (NULL == msg)
        return UBUS_STATUS_INVALID_ARGUMENT;
    fp = fopen("/etc/opkg/repository.conf", "w+");
    if (NULL == fp)
        return UBUS_STATUS_UNKNOWN_ERROR; 
    blobmsg_parse(update_policy, ARRAY_SIZE(update_policy), tb, blob_data(msg), blob_len(msg));
    if (tb[UPDATE_REPOSITORY])
    {
        fprintf(fp, "src repository %s\n", blobmsg_get_string(tb[UPDATE_REPOSITORY]));
    }
    fclose(fp);

    cmd = opkg_cmd_find("update");
    if (NULL == cmd)
        return UBUS_STATUS_INVALID_ARGUMENT;
    conf->pfm = cmd->pfm;
    if (opkg_conf_init())
		goto err0;
    if (pkg_hash_load_feeds())
        goto err1;

    if (pkg_hash_load_status_files())
        goto err1;
	err = opkg_cmd_exec(cmd, 0, NULL);

#ifdef HAVE_CURL
	opkg_curl_cleanup();
#endif
err1:
	opkg_conf_deinit();
err0:
    blob_buf_init(&b, 0);
    if (err)
    {
        blobmsg_add_u32(&b, "result", -1);
    }
    else
        blobmsg_add_u32(&b, "result", 0);
    ubus_send_reply(ctx, req, b.head);
    blob_buf_free(&b);
	free_error_list();
    return UBUS_STATUS_OK;
}
static void __opkg_update()
{
	opkg_cmd_t *cmd;
    cmd = opkg_cmd_find("update");
    conf->pfm = cmd->pfm;
    if (opkg_conf_init())
		goto err0;
    if (pkg_hash_load_feeds())
        goto err1;

    if (pkg_hash_load_status_files())
        goto err1;
	opkg_cmd_exec(cmd, 0, NULL);

#ifdef HAVE_CURL
	opkg_curl_cleanup();
#endif
err1:
	opkg_conf_deinit();
err0:
	free_error_list();
    return;
}
int main(int argc, char ** argv)
{
    int ret = -1;
    struct ubus_method opkg_methods[] = {
        UBUS_METHOD("list", __list, list_policy),
        UBUS_METHOD("install", __install, list_policy),
        UBUS_METHOD("remove", __remove, list_policy),
        UBUS_METHOD("upgrade", __upgrade, list_policy),
        UBUS_METHOD("update", __update, update_policy)
    };
    struct ubus_object_type opkg_object_type = UBUS_OBJECT_TYPE("opkg", opkg_methods);
    struct ubus_object obj={
        .name = "opkg",
        .type = &opkg_object_type,
        .methods = opkg_methods,
        .n_methods = ARRAY_SIZE(opkg_methods),
    };
    __opkg_update();
    ctx = ubus_connect(NULL);
    if (NULL == ctx)
        return 0;
    ret = ubus_add_object(ctx, &obj);
    if (ret)
    {
        ubus_free(ctx);
        return 0;
    }
    ubus_add_uloop(ctx);
    //daemon(0, 1);

    uloop_run();
    ubus_free(ctx);
    uloop_done();
    return 0;
}
