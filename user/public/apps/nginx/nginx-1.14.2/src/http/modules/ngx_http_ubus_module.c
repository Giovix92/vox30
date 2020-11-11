#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <json_object.h>
static struct ubus_context *ctx;
#define ubus_URI "/data/about.json"
#define NGX_HTTP_MSG_GET "GET"
#define NGX_HTTP_MSG_POST "POST"


typedef struct{
    ngx_http_upstream_conf_t upstream;
}ngx_http_ubus_conf_t;

typedef struct{
    ngx_http_status_t status;
    ngx_str_t backendServer;
}ngx_http_ubus_ctx_t;

extern ngx_module_t ngx_http_ubus_module;
static ngx_str_t ngx_http_proxy_hide_headers[] = {
    ngx_string("Date"),
    ngx_string("Server"),
    ngx_string("X-Pad"),
    ngx_string("X-Accel-Expires"),
    ngx_string("X-Accel-Redirect"),
    ngx_string("X-Accel-Limit-Rate"),
    ngx_string("X-Accel-Buffering"),
    ngx_string("X-Accel-Charset"),
    ngx_null_string
};

static char* ngx_http_ubus_merge_loc_conf(ngx_conf_t *cf, void *parent, void *conf);
//static ngx_int_t ubus_upstream_create_request(ngx_http_request_t *r);
//static ngx_int_t ngx_http_ubus_reinit_request(ngx_http_request_t *r);
//static ngx_int_t ubus_process_status_line(ngx_http_request_t *r);
//static void ubus_upstream_finalize_request(ngx_http_request_t *r, ngx_int_t rc);
static char* ngx_http_ubus_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void* ngx_http_ubus_create_loc_conf(ngx_conf_t *cf);

static ngx_command_t ngx_http_ubus_commands[] = {
    {
        ngx_string("ubus_pass"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
        ngx_http_ubus_pass,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    ngx_null_command
};

static ngx_http_module_t ngx_http_ubus_module_ctx = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ngx_http_ubus_create_loc_conf,
    ngx_http_ubus_merge_loc_conf
};

ngx_module_t ngx_http_ubus_module = {
    NGX_MODULE_V1,
    &ngx_http_ubus_module_ctx,
    ngx_http_ubus_commands,
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

static void
ngx_ubus_allow(struct ubus_request *req, int type, struct blob_attr *msg)
{
    void *tb1;

    tb1 = blobmsg_open_table(&req, "object");
    blobmsg_add_string(&req, "data", blob_data(msg));
    blobmsg_close_table(&req, tb1);
    blob_buf_free(&req);
    return NGX_OK;
}

static ngx_int_t
ngx_http_ubus_handler(ngx_http_request_t *r)
{
    printf("1.ubus_handler\n");
    ngx_http_ubus_ctx_t *myctx;
    ngx_http_ubus_conf_t *mycf;
    //ngx_http_upstream_t *u;
    myctx = ngx_http_get_module_ctx(r, ngx_http_ubus_module);
    if(myctx == NULL){
        myctx = ngx_palloc(r->pool, sizeof(ngx_http_ubus_ctx_t));
        if(myctx == NULL){
            return NGX_ERROR;
        }
        ngx_http_set_ctx(r, myctx, ngx_http_ubus_module);
    }
    if(ngx_http_upstream_create(r) != NGX_OK){
        ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "ngx_http_upstream_create failed");
        return NGX_ERROR;
    }
    mycf = (ngx_http_ubus_conf_t *)ngx_http_get_module_loc_conf(r, ngx_http_ubus_module);
    //u = r->upstream;
    //u->conf = &mycf->upstream;
    //u->buffering = mycf->upstream.buffering;
    

    ctx = ubus_connect(NULL);
    if(!ctx)
        return;
    char data = 'A';
    struct client *cl;
    struct json_object *jsobj;
    struct json_tokener *jstok;
    struct blob_buf buf = {0};
    unsigned int id;
    bool allow = false;
    void *tb1;
    //du->post_len += strlen(data);
    jstok = json_tokener_new();
    jsobj = json_tokener_parse_ex(jstok, data, strlen(data));
    blob_buf_init(&buf, 0);
    if(ubus_lookup_id(ctx, ubus_URI, &id))
    {
        ubus_free(ctx);
        return NGX_ERROR;
    }
    if(NGX_HTTP_MSG_GET == cl->request.method)
    {
        tb1 = blobmsg_open_table(&buf, "object");
        blobmsg_add_string(&buf, "data", data);
        blobmsg_close_table(&buf, tb1);
        ubus_invoke(ctx, id, "get", buf.head, ngx_ubus_allow, &buf, 2500);
        blob_buf_free(&buf);
        ubus_free(ctx);
    }else if(NGX_HTTP_MSG_POST == cl->request.method)
    {
        tb1 = blobmsg_open_table(&buf, "object");
        blobmsg_add_string(&buf, "data", data);
        blobmsg_close_table(&buf, tb1);
        ubus_invoke(ctx, id, "add", buf.head, ngx_ubus_allow, &buf, 2500);
        blob_buf_free(&buf);
        ubus_free(ctx);
    }
    else
    {
        return NGX_ERROR;
    }
    //u->resolved = (ngx_http_upstream_resolved_t *)ngx_palloc(r->pool, sizeof(ngx_http_upstream_resolved_t));
    //if(u->resolved == NULL){
    //    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "ngx_pcalloc resolved error:%s", strerror(errno));
    //    return NGX_ERROR;
    //}
    //}
    //r->main->count++;
    //ngx_http_upstream_init(r);
    return NGX_DONE;
}

/*
static ngx_int_t
ubus_upstream_create_request(ngx_http_request_t *r)
{
    printf("2.create_request\n");
    int sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un backendSockAddr;
    backendSockAddr.sun_family = PF_UNIX;
    strcpy(backendSockAddr.sun_path, "/tmp/server.socket");
    int result = connect(sockfd, (struct sockaddr *)&backendSockAddr, sizeof(backendSockAddr));
    int result = connect(sockfd, r->upstream->resolved->sockaddr, r->upstream->resolved->socklen);
    if(result == -1){
        printf("connect faile:%s\n", strerror(errno));
        exit(1);
    }
    char ch = 'A';
    write(sockfd, &ch, 1);
    return NGX_OK;
}

static ngx_int_t
ngx_http_ubus_reinit_request(ngx_http_request_t *r)
{
    printf("3.reinit_request\n");
    return NGX_OK;
}

static ngx_int_t
ubus_process_status_line(ngx_http_request_t *r)
{
    printf("4.status_line\n");
    int sockfd;;
    char ch;
    read(sockfd, &ch, 1);
    printf("get char from server:%c\n", ch);
    close(sockfd);
    return NGX_OK;
}

static void
ubus_upstream_finalize_request(ngx_http_request_t *r, ngx_int_t rc)
{
    printf("5.finalize_request\n");
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "ubus_upstream_finalize_request");
}
*/
static void *
ngx_http_ubus_create_loc_conf(ngx_conf_t *cf)
{
    printf("6.create_loc_conf\n");
    ngx_http_ubus_conf_t *mycf;
    mycf = (ngx_http_ubus_conf_t *)ngx_pcalloc(cf->pool, sizeof(ngx_http_ubus_conf_t));
    if(mycf == NULL){
        return NGX_CONF_ERROR;
    }
    mycf->upstream.connect_timeout = 60000;
    mycf->upstream.send_timeout = 60000;
    mycf->upstream.read_timeout = 60000;

    mycf->upstream.store_access = 0600;

    mycf->upstream.buffering = 0;
    mycf->upstream.bufs.num = 8;

    mycf->upstream.bufs.size = ngx_pagesize;
    mycf->upstream.buffer_size = ngx_pagesize;

    mycf->upstream.busy_buffers_size = 2 * ngx_pagesize;
    mycf->upstream.temp_file_write_size = 2 * ngx_pagesize;

    mycf->upstream.max_temp_file_size = 1024 * 1024 * 1024;

    mycf->upstream.hide_headers = NGX_CONF_UNSET_PTR;
    mycf->upstream.pass_headers = NGX_CONF_UNSET_PTR;

    return mycf;
}

static char *
ngx_http_ubus_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    printf("7.merge_loc_conf\n");
    ngx_http_ubus_conf_t *prev = (ngx_http_ubus_conf_t *)parent;
    ngx_http_ubus_conf_t *conf = (ngx_http_ubus_conf_t *)child;

    ngx_hash_init_t hash;
    hash.max_size = 100;
    hash.bucket_size = 1024;
    hash.name = "proxy_headers_hash";

    if(ngx_http_upstream_hide_headers_hash(cf, &conf->upstream, &prev->upstream, ngx_http_proxy_hide_headers, &hash) != NGX_OK){
        return NGX_CONF_ERROR;
    }
    return NGX_CONF_OK;
}

static char *
ngx_http_ubus_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    printf("8.ubus_pass\n");
    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_ubus_handler;
    return NGX_CONF_OK;
}
