/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

#include "curl_setup.h"

#include <curl/curl.h>

#include "urldata.h"
#include "getinfo.h"

#include "vtls/vtls.h"
#include "connect.h" /* Curl_getconnectinfo() */
#include "progress.h"

/* The last #include files should be: */
#include "curl_memory.h"
#include "memdebug.h"
#ifdef __SC_BUILD__
extern ic_info_t icInfo[512];
extern if_inf_t if_info_bom_fl;
extern if_inf_t if_info_eom_fl;
extern if_inf_t if_info_for_total_start;
extern if_inf_t if_info_for_total_end;
extern struct timeval bomTime_fl;
extern struct timeval eomTime_fl;
extern unsigned long int testBytesReceived_fl;
extern unsigned long int testBytesSent_fl;
#endif

/*
 * This is supposed to be called in the beginning of a perform() session
 * and should reset all session-info variables
 */
CURLcode Curl_initinfo(struct SessionHandle *data)
{
  struct Progress *pro = &data->progress;
  struct PureInfo *info = &data->info;

  pro->t_nslookup = 0;
  pro->t_connect = 0;
#ifdef __SC_BUILD__
  pro->t_rom.tv_sec = 0;
  pro->t_rom.tv_usec = 0;
  pro->t_bom.tv_sec = 0;
  pro->t_bom.tv_usec = 0;
  pro->t_eom.tv_sec = 0;
  pro->t_eom.tv_usec = 0;
  pro->t_request.tv_sec = 0;
  pro->t_request.tv_usec = 0;
  pro->t_response.tv_sec = 0;
  pro->t_response.tv_usec = 0;
  pro->total_br = 0;
  pro->total_bs = 0;
  pro->timeUp = 0;
#endif
  pro->t_appconnect = 0;
  pro->t_pretransfer = 0;
  pro->t_starttransfer = 0;
  pro->timespent = 0;
  pro->t_redirect = 0;

  info->httpcode = 0;
  info->httpproxycode = 0;
  info->httpversion = 0;
  info->filetime = -1; /* -1 is an illegal time and thus means unknown */
  info->timecond = FALSE;

  free(info->contenttype);
  info->contenttype = NULL;

  info->header_size = 0;
  info->request_size = 0;
  info->numconnects = 0;

  info->conn_primary_ip[0] = '\0';
  info->conn_local_ip[0] = '\0';
  info->conn_primary_port = 0;
  info->conn_local_port = 0;

  return CURLE_OK;
}

static CURLcode getinfo_char(struct SessionHandle *data, CURLINFO info,
                             char **param_charp)
{
  switch(info) {
  case CURLINFO_EFFECTIVE_URL:
    *param_charp = data->change.url?data->change.url:(char *)"";
    break;
  case CURLINFO_CONTENT_TYPE:
    *param_charp = data->info.contenttype;
    break;
  case CURLINFO_PRIVATE:
    *param_charp = (char *) data->set.private_data;
    break;
  case CURLINFO_FTP_ENTRY_PATH:
    /* Return the entrypath string from the most recent connection.
       This pointer was copied from the connectdata structure by FTP.
       The actual string may be free()ed by subsequent libcurl calls so
       it must be copied to a safer area before the next libcurl call.
       Callers must never free it themselves. */
    *param_charp = data->state.most_recent_ftp_entrypath;
    break;
  case CURLINFO_REDIRECT_URL:
    /* Return the URL this request would have been redirected to if that
       option had been enabled! */
    *param_charp = data->info.wouldredirect;
    break;
  case CURLINFO_PRIMARY_IP:
    /* Return the ip address of the most recent (primary) connection */
    *param_charp = data->info.conn_primary_ip;
    break;
  case CURLINFO_LOCAL_IP:
    /* Return the source/local ip address of the most recent (primary)
       connection */
    *param_charp = data->info.conn_local_ip;
    break;
  case CURLINFO_RTSP_SESSION_ID:
    *param_charp = data->set.str[STRING_RTSP_SESSION_ID];
    break;

  default:
    return CURLE_UNKNOWN_OPTION;
  }

  return CURLE_OK;
}

static CURLcode getinfo_long(struct SessionHandle *data, CURLINFO info,
                             long *param_longp)
{
  curl_socket_t sockfd;

  union {
    unsigned long *to_ulong;
    long          *to_long;
  } lptr;

  switch(info) {
  case CURLINFO_RESPONSE_CODE:
    *param_longp = data->info.httpcode;
    break;
  case CURLINFO_HTTP_CONNECTCODE:
    *param_longp = data->info.httpproxycode;
    break;
  case CURLINFO_FILETIME:
    *param_longp = data->info.filetime;
    break;
  case CURLINFO_HEADER_SIZE:
    *param_longp = data->info.header_size;
    break;
  case CURLINFO_REQUEST_SIZE:
    *param_longp = data->info.request_size;
    break;
  case CURLINFO_SSL_VERIFYRESULT:
    *param_longp = data->set.ssl.certverifyresult;
    break;
  case CURLINFO_REDIRECT_COUNT:
    *param_longp = data->set.followlocation;
    break;
  case CURLINFO_HTTPAUTH_AVAIL:
    lptr.to_long = param_longp;
    *lptr.to_ulong = data->info.httpauthavail;
    break;
  case CURLINFO_PROXYAUTH_AVAIL:
    lptr.to_long = param_longp;
    *lptr.to_ulong = data->info.proxyauthavail;
    break;
  case CURLINFO_OS_ERRNO:
    *param_longp = data->state.os_errno;
    break;
  case CURLINFO_NUM_CONNECTS:
    *param_longp = data->info.numconnects;
    break;
  case CURLINFO_LASTSOCKET:
    sockfd = Curl_getconnectinfo(data, NULL);

    /* note: this is not a good conversion for systems with 64 bit sockets and
       32 bit longs */
    if(sockfd != CURL_SOCKET_BAD)
      *param_longp = (long)sockfd;
    else
      /* this interface is documented to return -1 in case of badness, which
         may not be the same as the CURL_SOCKET_BAD value */
      *param_longp = -1;
    break;
  case CURLINFO_PRIMARY_PORT:
    /* Return the (remote) port of the most recent (primary) connection */
    *param_longp = data->info.conn_primary_port;
    break;
  case CURLINFO_LOCAL_PORT:
    /* Return the local port of the most recent (primary) connection */
    *param_longp = data->info.conn_local_port;
    break;
#ifdef __SC_BUILD__
  case CURLINFO_TIME_UP:
    *param_longp = data->progress.timeUp;
    break;
#endif
  case CURLINFO_CONDITION_UNMET:
    /* return if the condition prevented the document to get transferred */
    *param_longp = data->info.timecond ? 1L : 0L;
    break;
  case CURLINFO_RTSP_CLIENT_CSEQ:
    *param_longp = data->state.rtsp_next_client_CSeq;
    break;
  case CURLINFO_RTSP_SERVER_CSEQ:
    *param_longp = data->state.rtsp_next_server_CSeq;
    break;
  case CURLINFO_RTSP_CSEQ_RECV:
    *param_longp = data->state.rtsp_CSeq_recv;
    break;

  default:
    return CURLE_UNKNOWN_OPTION;
  }

  return CURLE_OK;
}

static CURLcode getinfo_double(struct SessionHandle *data, CURLINFO info,
                               double *param_doublep)
{
  switch(info) {
  case CURLINFO_TOTAL_TIME:
    *param_doublep = data->progress.timespent;
    break;
  case CURLINFO_NAMELOOKUP_TIME:
    *param_doublep = data->progress.t_nslookup;
    break;
  case CURLINFO_CONNECT_TIME:
    *param_doublep = data->progress.t_connect;
    break;
#ifdef __SC_BUILD__
  case CURLINFO_TESTBR_FL:
    *param_doublep = testBytesReceived_fl;
    break;
  case CURLINFO_TESTBS_FL:
    *param_doublep = testBytesSent_fl;
    break;
  case CURLINFO_TOTALBR_FL:
    *param_doublep = if_info_eom_fl.rx_bytes - if_info_bom_fl.rx_bytes;
    break;
  case CURLINFO_TOTALBS_FL:
    *param_doublep = if_info_eom_fl.tx_bytes - if_info_bom_fl.tx_bytes;
    break;
  case CURLINFO_PERIOD_FL:
    *param_doublep = (eomTime_fl.tv_sec - bomTime_fl.tv_sec)*1000000 + eomTime_fl.tv_usec - bomTime_fl.tv_usec;
    break;
  case CURLINFO_TOTALBS:
    *param_doublep = data->progress.total_bs;
    break;
  case CURLINFO_TOTALBR:
    *param_doublep = data->progress.total_br;
    break;
  case CURLINFO_INTERFACE_RX_START:
    *param_doublep = if_info_for_total_start.rx_bytes;
    break;
  case CURLINFO_INTERFACE_TX_START:
    *param_doublep = if_info_for_total_start.tx_bytes;
    break;
  case CURLINFO_INTERFACE_RX_END:
    *param_doublep = if_info_for_total_end.rx_bytes;
    break;
  case CURLINFO_INTERFACE_TX_END:
    *param_doublep = if_info_for_total_end.tx_bytes;
    break;
#endif
  case CURLINFO_APPCONNECT_TIME:
    *param_doublep = data->progress.t_appconnect;
    break;
  case CURLINFO_PRETRANSFER_TIME:
    *param_doublep =  data->progress.t_pretransfer;
    break;
  case CURLINFO_STARTTRANSFER_TIME:
    *param_doublep = data->progress.t_starttransfer;
    break;
  case CURLINFO_SIZE_UPLOAD:
    *param_doublep =  (double)data->progress.uploaded;
    break;
  case CURLINFO_SIZE_DOWNLOAD:
    *param_doublep = (double)data->progress.downloaded;
    break;
  case CURLINFO_SPEED_DOWNLOAD:
    *param_doublep =  (double)data->progress.dlspeed;
    break;
  case CURLINFO_SPEED_UPLOAD:
    *param_doublep = (double)data->progress.ulspeed;
    break;
  case CURLINFO_CONTENT_LENGTH_DOWNLOAD:
    *param_doublep = (data->progress.flags & PGRS_DL_SIZE_KNOWN)?
      (double)data->progress.size_dl:-1;
    break;
  case CURLINFO_CONTENT_LENGTH_UPLOAD:
    *param_doublep = (data->progress.flags & PGRS_UL_SIZE_KNOWN)?
      (double)data->progress.size_ul:-1;
    break;
  case CURLINFO_REDIRECT_TIME:
    *param_doublep =  data->progress.t_redirect;
    break;

  default:
    return CURLE_UNKNOWN_OPTION;
  }

  return CURLE_OK;
}

static CURLcode getinfo_slist(struct SessionHandle *data, CURLINFO info,
                              struct curl_slist **param_slistp)
{
  union {
    struct curl_certinfo *to_certinfo;
    struct curl_slist    *to_slist;
  } ptr;

  switch(info) {
  case CURLINFO_SSL_ENGINES:
    *param_slistp = Curl_ssl_engines_list(data);
    break;
  case CURLINFO_COOKIELIST:
    *param_slistp = Curl_cookie_list(data);
    break;
  case CURLINFO_CERTINFO:
    /* Return the a pointer to the certinfo struct. Not really an slist
       pointer but we can pretend it is here */
    ptr.to_certinfo = &data->info.certs;
    *param_slistp = ptr.to_slist;
    break;
  case CURLINFO_TLS_SESSION:
    {
      struct curl_tlssessioninfo **tsip = (struct curl_tlssessioninfo **)
                                          param_slistp;
      struct curl_tlssessioninfo *tsi = &data->tsi;
      struct connectdata *conn = data->easy_conn;
      unsigned int sockindex = 0;
      void *internals = NULL;

      *tsip = tsi;
      tsi->backend = Curl_ssl_backend();
      tsi->internals = NULL;

      if(!conn)
        break;

      /* Find the active ("in use") SSL connection, if any */
      while((sockindex < sizeof(conn->ssl) / sizeof(conn->ssl[0])) &&
            (!conn->ssl[sockindex].use))
        sockindex++;

      if(sockindex == sizeof(conn->ssl) / sizeof(conn->ssl[0]))
        break; /* no SSL session found */

      /* Return the TLS session information from the relevant backend */
#ifdef USE_OPENSSL
      internals = conn->ssl[sockindex].ctx;
#endif
#ifdef USE_GNUTLS
      internals = conn->ssl[sockindex].session;
#endif
#ifdef USE_NSS
      internals = conn->ssl[sockindex].handle;
#endif
#ifdef USE_GSKIT
      internals = conn->ssl[sockindex].handle;
#endif
      if(internals) {
        tsi->internals = internals;
      }
      /* NOTE: For other SSL backends, it is not immediately clear what data
         to return from 'struct ssl_connect_data'; thus we keep 'internals' to
         NULL which should be interpreted as "not supported" */
    }
    break;
  default:
    return CURLE_UNKNOWN_OPTION;
  }

  return CURLE_OK;
}
#ifdef __SC_BUILD__
static CURLcode getinfo_timeval(struct SessionHandle *data, CURLINFO info,
                               struct timeval *param_timeval)
{
  switch(info) {
  case CURLINFO_REQUEST_TIME:
    *param_timeval = data->progress.t_request;
    break;
  case CURLINFO_RESPONSE_TIME:
    *param_timeval = data->progress.t_response;
    break;
  case CURLINFO_ROM_TIME:
    *param_timeval = data->progress.t_rom;
    break;
  case CURLINFO_BOM_TIME:
    *param_timeval = data->progress.t_bom;
    break;
  case CURLINFO_EOM_TIME:
    *param_timeval = data->progress.t_eom;
    break;
  default:
    return CURLE_UNKNOWN_OPTION;
  }

  return CURLE_OK;
}
static CURLcode getinfo_icinfo(struct SessionHandle *data, CURLINFO info,
                                ic_info_t **param_icinfo)
{
    switch(info) {
        case CURLINFO_GET_ICINFO:
            *param_icinfo = &icInfo[1];
            break;
        default:
            return CURLE_UNKNOWN_OPTION;
    }

    return CURLE_OK;
}
#endif
static CURLcode getinfo_socket(struct SessionHandle *data, CURLINFO info,
                               curl_socket_t *param_socketp)
{
  switch(info) {
  case CURLINFO_ACTIVESOCKET:
    *param_socketp = Curl_getconnectinfo(data, NULL);
    break;
  default:
    return CURLE_UNKNOWN_OPTION;
  }

  return CURLE_OK;
}

CURLcode Curl_getinfo(struct SessionHandle *data, CURLINFO info, ...)
{
  va_list arg;
  long *param_longp = NULL;
  double *param_doublep = NULL;
  char **param_charp = NULL;
  struct curl_slist **param_slistp = NULL;
  curl_socket_t *param_socketp = NULL;
#ifdef __SC_BUILD__
  struct timeval* param_timeval = NULL;
  ic_info_t **param_icinfo = NULL;
#endif
  int type;
  CURLcode result = CURLE_UNKNOWN_OPTION;

  if(!data)
    return result;

  va_start(arg, info);

  type = CURLINFO_TYPEMASK & (int)info;
  switch(type) {
  case CURLINFO_STRING:
    param_charp = va_arg(arg, char **);
    if(param_charp)
      result = getinfo_char(data, info, param_charp);
    break;
  case CURLINFO_LONG:
    param_longp = va_arg(arg, long *);
    if(param_longp)
      result = getinfo_long(data, info, param_longp);
    break;
  case CURLINFO_DOUBLE:
    param_doublep = va_arg(arg, double *);
    if(param_doublep)
      result = getinfo_double(data, info, param_doublep);
    break;
  case CURLINFO_SLIST:
    param_slistp = va_arg(arg, struct curl_slist **);
    if(param_slistp)
      result = getinfo_slist(data, info, param_slistp);
    break;
  case CURLINFO_SOCKET:
    param_socketp = va_arg(arg, curl_socket_t *);
    if(param_socketp)
      result = getinfo_socket(data, info, param_socketp);
    break;
#ifdef __SC_BUILD__
  case CURLINFO_TIMEVAL:
    param_timeval = va_arg(arg, struct timeval *);
    if(param_timeval)
      result = getinfo_timeval(data, info, param_timeval);
    break;
  case CURLINFO_ICINFO:
    param_icinfo = va_arg(arg, ic_info_t **);
    if(param_icinfo)
        result = getinfo_icinfo(data, info, param_icinfo);
    break;
#endif
  default:
    break;
  }

  va_end(arg);

  return result;
}
