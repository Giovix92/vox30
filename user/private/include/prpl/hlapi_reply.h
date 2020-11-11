#ifndef __HLAPI_REPLY_H__
#define __HLAPI_REPLY_H__
#include <libubox/blobmsg_json.h>

enum{
    HLAPI_OK = 0,
    HLAPI_INVALID_ARGUMENT,
    HLAPI_ARGUMENT_DATA_TYPE_MISMATCH,
    HLAPI_INVALID_COMMAND,
    HLAPI_INVALID_OBJECT,
    HLAPI_MISSING_REQUIRED_ARGUMENT,
    HLAPI_PERMISSION_DENIED,
    HLAPI_TIMEOUT,
    HLAPI_OPERATION_ALREADY_IN_PROGRESS,
    HLAPI_OPERATION_ILLEGAL,
    HLAPI_OPERATION_ERROR,
    HLAPI_ARGUMENT_VALUE_NOT_SUPPORTED,
    HLAPI_RETURNCODE_MAX
};
static char hlapi_return_name[HLAPI_RETURNCODE_MAX][32] = {
    "OK",
    "ARGUMENT_NOT_FOUND",
    "ARGUMENT_DATA_TYPE_MISMATCH",
    "METHOD_NOT_FOUND",
    "OBJECT_NOT_FOUND",
    "ARGUMENT_REQUIRED_MISSING",
    "OPERATION_PERMISSION_DENIED",
    "OPERATION_TIMEOUT",
    "OPERATION_ALREADY_IN_PROGRESS",
    "OPERATION_ILLEGAL",
    "OPERATION_ERROR",
    "ARGUMENT_VALUE_NOT_SUPPORTED",
};
static inline void hlapi_reply(int code, char *description, struct blob_attr *msg, struct blob_buf *b)
{
    void *tbl;
    memset(b, 0, sizeof(struct blob_buf));
    blob_buf_init(b, 0);
    if (code >= HLAPI_RETURNCODE_MAX)
        return;
    tbl = blobmsg_open_table(b, "Header");
    //blobmsg_add_u32(b, "Code", code);
    blobmsg_add_string(b, "Name", hlapi_return_name[code]);
    //if (description)
    //    blobmsg_add_string(b, "Description", description);
    blobmsg_close_table(b, tbl);
    if (msg)
	    blobmsg_add_field(b, BLOBMSG_TYPE_TABLE, "Body", blob_data(msg), blob_len(msg));
    return;
}
#endif
