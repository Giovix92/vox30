#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <json_object.h>
#include <libubox/blob.h>
#include <libubus.h>
#include <json.h>
#define TIMEOUT 5
extern ngx_module_t ngx_http_ubus_access_module;
static ngx_int_t ngx_http_ubus_access_init(ngx_conf_t *cf);
static ngx_int_t ngx_http_ubus_access_handler(ngx_http_request_t *r);
static char *ngx_http_ubus_access(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_command_t ngx_http_ubus_access_commands[] = {
    {
        ngx_string("ubus_access"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
        ngx_http_ubus_access,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    ngx_null_command
};

static ngx_http_module_t ngx_http_ubus_access_module_ctx = {
    NULL,
    ngx_http_ubus_access_init,

    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL
};

ngx_module_t ngx_http_ubus_access_module = {
    NGX_MODULE_V1,
    &ngx_http_ubus_access_module_ctx,
    ngx_http_ubus_access_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};

struct logindata{
    char session[64];
    char expires[64];
};
static char ses[64] = {0};
static bool *result;
enum{
    SES_ACCESS,
    __SES_MAX,
};
static const struct blobmsg_policy ses_policy[__SES_MAX] = {
    [SES_ACCESS] = { .name = "access", .type = BLOBMSG_TYPE_BOOL },
};

static void
ubus_login_allowed_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct blobmsg_policy ses1_policy[] = {
        [0] = { .name = "ubus_rpc_session", .type = BLOBMSG_TYPE_STRING },
        [1] = { .name = "expires", .type = BLOBMSG_TYPE_INT32 }
    };
    struct blob_attr *t[2] = {0};
    struct logindata *d = (struct logindata*)req->priv;
    if(!msg)
        return;
    blobmsg_parse(ses1_policy, sizeof(ses1_policy)/sizeof(struct blobmsg_policy), t, blobmsg_data(msg), blobmsg_len(msg));
    if(t[0])
        snprintf(d->session, sizeof(d->session), "%s", blobmsg_get_string(t[0]));
    if(t[1])
        snprintf(d->expires, sizeof(d->expires), "%s", blobmsg_get_u32(t[1]));
    ngx_cpystrn(ses, d->session, ngx_strlen(d->session) + 1);
}

static void
ubus_access_allowed_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct blob_attr *t[__SES_MAX];
    bool *allow = (bool *)req->priv;
    if(!msg)
        return;
    blobmsg_parse(ses_policy, __SES_MAX, t, blob_data(msg), blob_len(msg));
    if(t[SES_ACCESS])
        *allow = blobmsg_get_bool(t[SES_ACCESS]);
    result = allow;
}

static void
ngx_transfer(char *num)
{
    while(*(num) != '\0')
    {
        if(*(num) == '/')
            *(num) = '.';
        num++;
    }
}

static ngx_int_t
ngx_http_ubus_access_handler(ngx_http_request_t *r)
{
    struct ubus_context *ctx;
    ctx = ubus_connect(NULL);
    if(!ctx)
        return;
    uint32_t id;
    bool allow1;
    struct logindata du;
    struct blob_buf buf = {0};
    struct blob_buf b = {0};
    void *tb, *tb1, *tb2;
    char *num1, *data;
    char num[1024], data2[100], num2[16];
    ngx_int_t rc;
    ngx_int_t rc1;

    ngx_memzero(num, sizeof(num));
    ngx_cpystrn(num, r->uri.data, ngx_strlen(r->uri.data));
    ngx_transfer(num);
    if(ngx_strstr(num, "v1") != NULL){
        data = ngx_strstr(num, "v1");
        if(strchr(data, ' ') != NULL){
            num1 = strchr(data, ' ');
            *num1 = '\0';
        }
    }else{
        data = ngx_strstr(num, "api");
        data = strchr(data, '.');
        data++;
        if(strchr(data, ' ') != NULL){
            num1 = strchr(data, ' ');
            *num1 = '\0';
        }
    }
    ngx_memzero(data2, sizeof(data2));
    ngx_cpystrn(data2, r->method_name.data, ngx_strlen(r->method_name.data));
    if(strchr(data2, ' ') != NULL)
        *(strchr(data2, ' ')) = '\0';

    ngx_memzero(&du, sizeof(struct logindata));
    blob_buf_init(&buf, 0);
    blob_buf_init(&b, 0);
    rc1 = ngx_http_auth_basic_user(r);
    if(rc1 == NGX_DECLINED){
        ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
                "no user/password was provided for basic authentication");
    }
    if(rc1 == NGX_ERROR)
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    ngx_memzero(num2, sizeof(num2));
    ngx_cpystrn(num2, r->headers_in.user.data, ngx_strlen(r->headers_in.user.data));
    if(strchr(num2, ':') != NULL){
        *(strchr(num2, ':')) = '\0';
    }
    if(r->headers_in.authorization == NULL){
        return NGX_DECLINED;
    }
    else{
        if(ubus_lookup_id(ctx, "auth", &id)){
            blob_buf_free(&buf);
            blob_buf_free(&b);
            return NGX_DECLINED;
        }
        blobmsg_add_string(&buf, "from", "webapi");
        tb1 = blobmsg_open_table(&buf, "data");
        blobmsg_add_string(&buf, "grant_type", "password");
        tb2 = blobmsg_open_table(&buf, "data");
        blobmsg_add_string(&buf, "username", num2);
        blobmsg_add_string(&buf, "password", r->headers_in.passwd.data);
        blobmsg_add_u32(&buf, "timeout", 600);
        blobmsg_close_table(&buf, tb2);
        blobmsg_close_table(&buf, tb1);
        ubus_invoke(ctx, id, "login", buf.head, ubus_login_allowed_cb, &du, TIMEOUT * 500);
        blob_buf_free(&buf);

        if(ubus_lookup_id(ctx, "auth", &id)){
            blob_buf_free(&buf);
            blob_buf_free(&b);
            return NGX_DECLINED;
        }
        blobmsg_add_string(&b, "from", "webapi");
        tb = blobmsg_open_table(&b, "data");
        blobmsg_add_string(&b, "session", ses);
        blobmsg_add_string(&b, "scope", "ubus");
        blobmsg_add_string(&b, "object", data);
        blobmsg_add_string(&b, "function", data2);
        blobmsg_add_string(&b, "data", "dddd");
        blobmsg_close_table(&b, tb1);
        ubus_invoke(ctx, id, "access", b.head, ubus_access_allowed_cb, &allow1, TIMEOUT * 500);
        blob_buf_free(&b);
        ubus_free(ctx);

        if(!(*result))
        {
            return NGX_HTTP_FORBIDDEN;
        }
        else{
            return NGX_DECLINED;
        }
    }
}

static char *
ngx_http_ubus_access(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
   return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_ubus_access_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
    if(h == NULL){
        return NGX_ERROR;
    }

    *h = ngx_http_ubus_access_handler;

    return NGX_OK;
}
