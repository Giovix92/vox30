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
static int timeout = 30;
static char *str;
static char *data1;
extern ngx_module_t ngx_http_ubus_module;

static char* ngx_http_ubus_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void ngx_ubus_receive(struct ubus_request *req, int type, struct blob_attr *msg);

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

    NULL,
    NULL
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

static bool blobmsg_add_array(struct blob_buf *b, struct array_list *a)
{
    int i, len;
    for (i = 0, len = array_list_length(a); i < len; i++) {
        if (!blobmsg_add_json_element(b, NULL, array_list_get_idx(a, i)))
            return false;
    }
    return true;
}

static void
ngx_ubus_receive(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct blob_buf *b = (struct blob_buf *)(req->priv);
    if (!msg)
        return;
    blobmsg_add_field(b, BLOBMSG_TYPE_TABLE, "", blob_data(msg), blob_len(msg));
    str = blobmsg_format_json(blob_data(b->head), true);
}

static void
ngx_body_handler(ngx_http_request_t *r)
{
    int n, n1;
    n = r->request_body->bufs->buf->last - r->request_body->bufs->buf->pos + 1;
    if(r->request_body->bufs->next == NULL){
        data1 = ngx_pcalloc(r->pool, ((n + 1)*sizeof(char)));
        if(data1 == NULL){
            return;
        }
        ngx_cpystrn(data1, r->request_body->bufs->buf->pos, n);
        data1[ngx_strlen(data1)] = '\0';
    }
    else
    {
        n1 = r->request_body->bufs->next->buf->last - r->request_body->bufs->next->buf->pos + 1;
        data1 = ngx_pcalloc(r->pool, ((n + n1 + 1)*sizeof(char)));
        if(data1 == NULL){
            return;
        }
        ngx_cpystrn(data1, r->request_body->bufs->buf->pos, n);
        ngx_cpystrn(data1 + ngx_strlen(data1), r->request_body->bufs->next->buf->pos, n1);
        data1[ngx_strlen(data1)] = '\0';
        r->request_body->bufs = r->request_body->bufs->next;
    }
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
ngx_http_ubus_sendout(ngx_http_request_t *r, char *str1)
{
    ngx_int_t rc;
    ngx_str_t response;
    ngx_chain_t out;
    ngx_buf_t *bf;
    response.data = (u_char *)str1;
    response.len = ngx_strlen(str1);
    r->headers_out.content_type.len = sizeof("application/json") - 1;
    r->headers_out.content_type.data = (u_char *)"application/json";
    r->headers_out.charset.len = ngx_strlen("UTF-8");
    r->headers_out.charset.data = (u_char *)"UTF-8";
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = response.len;
    bf = ngx_create_temp_buf(r->pool, 128);
    if(bf == NULL){
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    out.buf = bf;
    out.next = NULL;
    bf->pos = response.data;
    bf->last = response.data + (response.len);
    bf->memory = 1;
    rc = ngx_http_send_header(r);
    if(rc != NGX_OK)
        return rc;
    bf->last_in_chain = 1;
    bf->last_buf = 1;
    return ngx_http_output_filter(r, &out);
}

static ngx_int_t
ngx_http_ubus_handler(ngx_http_request_t *r)
{
    struct ubus_context *ctx;
    ctx = ubus_connect(NULL);
    if(!ctx){
        return;
    }
    struct json_object *jsobj;
    struct json_tokener *jstok;
    struct blob_buf buf = {0};
    struct blob_buf b = {0};
    ngx_int_t rc1;
    int o_type, back;
    uint32_t id;
    int ret, ret1;
    char *num1, *data;
    char num[1024], data2[100];

    ngx_memzero(num, sizeof(num));
    ngx_cpystrn(num, r->uri.data, ngx_strlen(r->uri.data));
    ngx_transfer(num);
    if((ngx_strstr(num, "api")) != NULL){
        data = ngx_strstr(num, "api");
        data = strchr(data, '.');
        data++;
        num1 = strchr(data, '?');
        if(num1 != NULL)
            *num1 = '\0';
        else{
            num1 = strchr(data, ' ');
            if(num1 != NULL)
                *num1 = '\0';
        }
    }
    ngx_memzero(data2, sizeof(data2));
    ngx_cpystrn(data2, r->method_name.data, ngx_strlen(r->method_name.data));
    num1 = strchr(data2, ' ');
    if(num1 != NULL)
        *num1 = '\0';
    blob_buf_init(&buf, 0);
    blob_buf_init(&b, 0);
    if(ubus_lookup_id(ctx, data, &id))
    {
        ubus_free(ctx);
        blob_buf_free(&buf);
        blob_buf_free(&b);
        return NGX_ERROR;
    }
    if(ngx_strcmp(data2, "GET") == 0)
    {
        ret = ubus_invoke(ctx, id, "get", buf.head, ngx_ubus_receive, &b, timeout*500);
        if(ret == 3){
            ret1 = ubus_invoke(ctx, id, "Get", buf.head, ngx_ubus_receive, &b, timeout*500);
            if(ret1 == 3)
                ubus_invoke(ctx, id, "List", buf.head, ngx_ubus_receive, &b, timeout*500);
        }
        
        blob_buf_free(&buf);
        blob_buf_free(&b);
        ubus_free(ctx);
        back = ngx_http_ubus_sendout(r, str);
        free(str);
        return back;
    }else if(ngx_strcmp(data2, "PUT") == 0 || ngx_strcmp(data2, "POST") == 0)
    {
        
        rc1 = ngx_http_read_client_request_body(r, ngx_body_handler);
        if(rc1 >= NGX_HTTP_SPECIAL_RESPONSE){
            blob_buf_free(&buf);
            blob_buf_free(&b);
            ubus_free(ctx);
            return rc1;
        }
        jstok = json_tokener_new();
        jsobj = json_tokener_parse_ex(jstok, data1, ngx_strlen(data1));
        if(jsobj)
        {
            o_type = json_object_get_type(jsobj);
            if(json_type_object == o_type){
                blobmsg_add_object(&buf, jsobj);
            }else if(json_type_array == o_type){
                blobmsg_add_array(&buf, json_object_get_array(jsobj));
            }else{
                blob_buf_free(&buf);
                blob_buf_free(&b);
                ubus_free(ctx);
                return NGX_ERROR;
            }
        }
        if(ngx_strcmp(data2, "PUT") == 0){
            ret = ubus_invoke(ctx, id, "set", buf.head, ngx_ubus_receive, &b, timeout*500);
            if(ret == 3)
                ubus_invoke(ctx, id, "Set", buf.head, ngx_ubus_receive, &b, timeout*500);
        }
        else{
            ret = ubus_invoke(ctx, id, "add", buf.head, ngx_ubus_receive, &b, timeout*500);
            if(ret == 3)
                ubus_invoke(ctx, id, "Add", buf.head, ngx_ubus_receive, &b, timeout*500);
        }

        blob_buf_free(&buf);
        blob_buf_free(&b);
        ubus_free(ctx);
        json_tokener_free(jstok);
        back = ngx_http_ubus_sendout(r, str);
        free(str);
        return back;
    }else if(ngx_strcmp(data2, "DELETE") == 0)
    {
        rc1 = ngx_http_read_client_request_body(r, ngx_body_handler);
        if(rc1 >= NGX_HTTP_SPECIAL_RESPONSE){
            ret = ubus_invoke(ctx, id, "delete", buf.head, ngx_ubus_receive, &b, timeout*500);
            if(ret == 3)
                ubus_invoke(ctx, id, "Delete", buf.head, ngx_ubus_receive, &b, timeout*500);

            blob_buf_free(&buf);
            blob_buf_free(&b);
            ubus_free(ctx);       
            back = ngx_http_ubus_sendout(r, str);
            free(str);
            return back;
        }
        else{
            jstok = json_tokener_new();
            jsobj = json_tokener_parse_ex(jstok, data1, ngx_strlen(data1));
            if(jsobj)
            {
                o_type = json_object_get_type(jsobj);
                if(json_type_object == o_type){
                    blobmsg_add_object(&buf, jsobj);
                }else if(json_type_array == o_type){
                    blobmsg_add_array(&buf, json_object_get_array(jsobj));
                }else{
                    blob_buf_free(&buf);
                    blob_buf_free(&b);
                    ubus_free(ctx);
                    return NGX_ERROR;
                }
            }
            ret = ubus_invoke(ctx, id, "delete", buf.head, ngx_ubus_receive, &b, timeout*500);
            if(ret == 3)
                ubus_invoke(ctx, id, "Delete", buf.head, ngx_ubus_receive, &b, timeout*500);
            blob_buf_free(&buf);
            blob_buf_free(&b);
            ubus_free(ctx);
            json_tokener_free(jstok);
            back = ngx_http_ubus_sendout(r, str);
            free(str);
            return back;
        }
    }
    else
    {
        blob_buf_free(&buf);
        blob_buf_free(&b);
        ubus_free(ctx);
        return NGX_ERROR;
    }
}

static char *
ngx_http_ubus_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_ubus_handler;
    return NGX_CONF_OK;
}
