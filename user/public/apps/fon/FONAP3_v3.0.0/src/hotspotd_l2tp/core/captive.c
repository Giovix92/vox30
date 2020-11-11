/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log.h>
#endif
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef HOTSPOTD_OPENSSL
#include <openssl/sha.h>
#else 
#include <ctaocrypt/sha.h>
#endif

#include "lib/event.h"
#include "lib/list.h"
#include "lib/base64.h"
#include "lib/usock.h"
#include "lib/md5.h"
#include "lib/config.h"
#include "lib/hexlify.h"
#include "client.h"
#include "firewall.h"
#include "routing.h"
#include "hotspotd.h"

#define URLBUFSIZE 4096

#ifndef HOTSPOTD_OPENSSL
#define SHA_DIGEST_LENGTH 20
#endif

enum captive_type {
	CAPTIVE_REDIRECT,
	CAPTIVE_PORTAL
};

static struct captive_config {
	int redirect_maxconn;	/* Maximum no. of connections */
	const char *redirect_url;/* Redirect target URL */
	int portal_maxconn;		/* Maximum no. of connections */
	int portal_port;		/* Portal port*/
#ifdef __SC_BUILD__
    union {
		struct in_addr ipv4;
		struct in6_addr ipv6;
	} redirect_addr;
#endif
	int redirect_port;		/* Redirect port */
	const char *nasid;		/* NAS ID */
	const char *secret;		/* Captive secret */
} cfg = { .redirect_url = NULL, .nasid = NULL };

enum logon_params{
	RESPONSE,
	USERNAME,
	PASSWORD,
	WISPREAPMSG,
	TYPE,
	TOKEN,
	__LOGON_PARAMS_MAX
};

static const char *logon_params[__LOGON_PARAMS_MAX] = {
	[RESPONSE] = "response",
	[USERNAME] = "username",
	[PASSWORD] = "password",
	[WISPREAPMSG] = "wispreapmsg",
	[TYPE] = "type",
	[TOKEN] = "token"
};

static struct list_head list_thread = LIST_HEAD_INIT(list_thread);
static int threads[2] = {0, 0};

struct captive_thread {
	struct list_head _head;
	struct event_epoll data;
	union {
		struct in_addr ipv4;
		struct in6_addr ipv6;
	} addr;
	char *buffer;
	FILE *fp;
	int32_t timeout;
	uint16_t af;
	int8_t type;
};

static void captive_deinit();
static void captive_new(struct event_epoll *event, uint32_t revents);
static void captive_portal(struct event_epoll *event, uint32_t revents);
static void captive_destroy(struct captive_thread *thread);
static void captive_cleanup(struct event_timer *timer, int64_t now);
static int captive_param
(const char *query, const char *prefix, char *cout, size_t len);
static char *captive_params_extra(const char *url_params, const char
		**known_params);
static void captive_param_encrypt(char **param_en, size_t *len, const char
		*param_de, const struct client *cl);
static void captive_param_decrypt(uint8_t *key, size_t *keylen, const char
*param, const struct client *cl);
static void captive_redir(struct captive_thread *thread, enum client_login res,
		struct client *cl);

static struct event_epoll event_redirect = {
	.fd = -1,
	.events = EPOLLIN | EPOLLET,
	.handler = captive_new,
};

static struct event_epoll event_portal = {
	.fd = -1,
	.events = EPOLLIN | EPOLLET,
	.handler = captive_new,
};

static struct event_timer event_timeout = {
	.interval = 0,
	.handler = captive_cleanup,
};

static int captive_apply() {
	int ret = 0;
	// Refresh because pointers might be invalidated
	cfg.redirect_url = config_get_string("redirect", "url", NULL);
	if(!cfg.redirect_url) {
			cfg.redirect_url = redirect_url_default;
	}
	cfg.nasid = config_get_string("main", "nasid", NULL);
	if(!hotspot_assertconf_string("main.nasid", cfg.nasid)) {
		ret = -1;
	}

	cfg.secret = config_get_string("portal", "secret", NULL);
	if(!hotspot_assertconf_string("portal.secret", cfg.secret)) {
		ret = -1;
	}

	if (firewall_set_service(IPPROTO_TCP, cfg.portal_port, true)) {
		ret = -1;
	}
	return ret;
}


static int captive_init() {
	INIT_LIST_HEAD(&list_thread);
	threads[CAPTIVE_REDIRECT] = threads[CAPTIVE_PORTAL] = 0;

	// Read config
	cfg.redirect_maxconn = config_get_int("redirect", "maxconn", 32);

	cfg.portal_maxconn = config_get_int("portal", "maxconn", 32);
	const char *portal_port = config_get_string("portal", "port", "3990");
	cfg.portal_port = atoi(portal_port);

	const char *iface = config_get_string("main", "iface", "");
	const char *redirect_port = config_get_string("redirect", "port", "3989");
	cfg.redirect_port = atoi(redirect_port);
#ifdef __SC_BUILD__
	const char *r_addr = config_get_string("redirect", "addr", "192.168.2.1");
    inet_pton(AF_INET, r_addr, &(cfg.redirect_addr.ipv4));
#endif
	int sopts = USOCK_TCP | USOCK_SERVER | USOCK_TRANSPARENT
		| USOCK_BINDTODEV | USOCK_NONBLOCK;

	// The captive socket
	event_redirect.fd = usock(sopts, iface, redirect_port);
	if (event_redirect.fd < 0)
		goto err;
	event_ctl(EVENT_EPOLL_ADD, &event_redirect);

	// The portal socket
	if (cfg.portal_port) {
		event_portal.fd = usock(sopts, iface, portal_port);
		if (event_portal.fd < 0)
			goto err;
		event_ctl(EVENT_EPOLL_ADD, &event_portal);
	}
	syslog(LOG_INFO, "Configuring captive firewall");
	if (firewall_set_redirect(IPPROTO_TCP, 80, cfg.redirect_port, true))
		goto err;

	event_timeout.interval = 5000;
	event_ctl(EVENT_TIMER_ADD, &event_timeout);

	if (captive_apply())
		goto err;

	return 0;

	int errno_preserve;
err:
	errno_preserve = errno;
	captive_deinit();
	errno = errno_preserve;
	return -1;
}

static void captive_deinit() {
	while (!list_empty(&list_thread)) {
		struct captive_thread *thread =
		list_first_entry(&list_thread, struct captive_thread, _head);
		captive_destroy(thread);
	}

	close(event_redirect.fd);
	close(event_portal.fd);
	event_redirect.fd = event_portal.fd = -1;

	firewall_set_service(IPPROTO_TCP, cfg.portal_port, false);
	firewall_set_redirect(IPPROTO_TCP, 80, cfg.redirect_port, false);

	if (event_timeout.interval) {
		event_ctl(EVENT_TIMER_DEL, &event_timeout);
		event_timeout.interval = 0;
	}

	cfg.redirect_url = NULL;
	cfg.nasid = NULL;
}

static void captive_new(struct event_epoll *event, uint32_t revents) {
	int sock;
	union {
		struct sockaddr_in6 in6;
		struct sockaddr_in in;
	} addr;
	socklen_t addrlen = sizeof(addr);

	// TODO: Change timeout to something other than 2-3s
	int32_t timeout = (event_time() + 3000) / 1000;

	while ((sock = accept(event->fd, (void*)&addr, &addrlen)) >= 0) {
		event_cloexec(sock);
		event_nonblock(sock);

		/* Map mapped-IPv4 back to IPv4 */
		if (addr.in6.sin6_family == AF_INET6
		&& IN6_IS_ADDR_V4MAPPED(&addr.in6.sin6_addr)) {
			addr.in.sin_addr.s_addr =
					addr.in6.sin6_addr.s6_addr32[3];
			addr.in.sin_family = AF_INET;
		}

		int type = -1;
		if (event == &event_redirect
		&& threads[CAPTIVE_REDIRECT] < cfg.redirect_maxconn) {
			type = CAPTIVE_REDIRECT;
		} else if (event == &event_portal
		&& threads[CAPTIVE_PORTAL] < cfg.portal_maxconn) {
			type = CAPTIVE_PORTAL;
		}

		struct captive_thread *thread = NULL;
		if (type == -1 || !(thread = malloc(sizeof(*thread)))
		|| !(thread->fp = fdopen(sock, "r+"))) {
			free(thread);
			close(sock);
			continue;
		}
		setvbuf(thread->fp, NULL, _IOFBF, 2048);

		threads[type]++;
		list_add(&thread->_head, &list_thread);
		thread->data.fd = sock;
		thread->data.events = EPOLLIN | EPOLLET;
		thread->data.handler = captive_portal;
		thread->data.context = thread;
		event_ctl(EVENT_EPOLL_ADD, &thread->data);

		thread->timeout = timeout;
		thread->buffer = NULL;
		thread->type = type;

		if (addr.in.sin_family == AF_INET) {
			thread->addr.ipv4 = addr.in.sin_addr;
		} else {
			thread->addr.ipv6 = addr.in6.sin6_addr;
		}
		thread->af = addr.in.sin_family;
	}
}

// This is a callback invoked when the login process is complete.
static void captive_login(struct client *cl,
				enum client_login status, void *ctx) {
	captive_redir((struct captive_thread*)ctx, status, cl);
}
// Captive thread handler
static void captive_portal(struct event_epoll *event, uint32_t revents) {
	struct captive_thread *thread = event->context;

	char buf[2048], *saveptr, *c;
	if (!thread->buffer) { /* Haven't read request yet */
		if (!fgets_unlocked(buf, sizeof(buf), thread->fp)) {
			if (errno == EAGAIN)
				return;
			else
				goto destroy;
		}

		c = strtok_r(buf, " \t", &saveptr);
		if (!c || strcmp(c, "GET")
				|| !(c = strtok_r(NULL, " \t", &saveptr))
				|| !(thread->buffer = strdup(c)))
			goto destroy;
	}

	errno = 0;
	while (fgets_unlocked(buf, sizeof(buf), thread->fp)
			&& buf[0] != '\r' && buf[0] != '\n') {
		if (thread->type == CAPTIVE_REDIRECT &&
				!strncasecmp(buf, "host:", 5)) {
			c = strtok_r(buf, " \t", &saveptr);
			if (!c)
				goto destroy;

			char *host = strtok_r(NULL, " \t\r\n", &saveptr);
#ifdef __SC_BUILD__
			if (!host)
				goto destroy;
#endif
			size_t len = strlen(host) + strlen(thread->buffer) + 8;
			if (!host || !(c = malloc(len)))
				goto destroy;

			strcpy(c, "http://");
			strcat(c, host);
			strcat(c, thread->buffer);

			free(thread->buffer);
			thread->buffer = c;
		}
	}

	if (errno == EAGAIN)
		return;
	else if (errno != 0)
		goto destroy;

	fflush_unlocked(thread->fp);
	/* Got the full request */

	event_ctl(EVENT_EPOLL_DEL, &thread->data);
	//TODO: fix if client is EAP
	struct client *cl = client_register(routing_cfg.iface_index,
			thread->af, &thread->addr);
	if (!cl)
		goto destroy;
#ifdef __SC_BUILD__
    if(client_is_reach_max_login_user(cl))
    {
        if (thread->type == CAPTIVE_REDIRECT)
        {
            if(cl->sess.userurl)
            {
                free(cl->sess.userurl);
                cl->sess.userurl = NULL;
            }
            cl->sess.userurl = thread->buffer;
            thread->buffer = NULL;
        }
        captive_redir(thread, LOGIN_LIMIT, cl);
        return;
    }
#endif
	if (thread->type == CAPTIVE_REDIRECT) {
        if(cl->sess.userurl)
        {
		    free(cl->sess.userurl);
            cl->sess.userurl = NULL;
        }
		cl->sess.userurl = thread->buffer;
		thread->buffer = NULL;
		captive_redir(thread, LOGIN_NOTYET, cl);
		return;
	}
#ifdef __SC_BUILD__
#else
	syslog(LOG_DEBUG, "Captive request %s", thread->buffer);
#endif
	if (!strncmp(thread->buffer, "/logon", 6)) {
		enum client_method method = -1;
		char p1[256], p2[1792] = {0}, param[32];
		uint8_t key[1265], type[256];
		size_t keylen = 0;

		// Extract query string parameters
		sprintf(param, "%s=", logon_params[RESPONSE]);
		if (!captive_param(thread->buffer, param,
					p2, sizeof(p2))) {
			method = CLIENT_CHAP;
			unhexlify(key, p2, 32);
			keylen = 16;
		} else {
			sprintf(param, "%s=", logon_params[PASSWORD]);
			if (!captive_param(thread->buffer, param,
					p2, sizeof(p2))) {
				method = CLIENT_PAP;
				captive_param_decrypt(key, &keylen, p2, cl);
			} else {
				sprintf(param, "%s=", logon_params[WISPREAPMSG]);
				if (!captive_param(thread->buffer, param,
						p2, sizeof(p2))) {
					int buflen = sizeof(key);
					if (!base64_decode(key, &buflen, (uint8_t*)p2,
										strlen(p2))) {
						method = CLIENT_EAP;
						keylen = buflen;
					}
				}
			}
		}

		sprintf(param, "%s=", logon_params[USERNAME]);
		if (method <= 0 || captive_param(thread->buffer,
					param, p1, sizeof(p1))) {
			fputs_unlocked("HTTP/1.1 403 Forbidden\r\n\r\n",
								thread->fp);
			goto destroy;
		}


		client_set_services(cl, false);	// Clear previous service rules
		cl->servicemap = 0;
		cl->sess.hasgcap = false;
		
		sprintf(param, "%s=", logon_params[TYPE]);	
		if (!captive_param(thread->buffer, param, p2, sizeof(p2))) {
			strncpy((char *)type, p2, sizeof(type));

			sprintf(param, "%s=", logon_params[TOKEN]);
			if (!captive_param(thread->buffer, param, p2, sizeof(p2))) {
				unsigned char *enctype = NULL;
				size_t size_type = 0;
				unsigned char hash[SHA_DIGEST_LENGTH];
				unsigned char unhex_token[SHA_DIGEST_LENGTH]; // SHA_DIGEST_LENGTH = 20

				captive_param_encrypt((char **)&enctype, &size_type, (char *)type, cl);

#ifdef HOTSPOTD_OPENSSL			       
				SHA1(enctype, strlen((char *)enctype), hash);
#else 
				Sha sha;
				
				InitSha(&sha);
				ShaUpdate(&sha, (byte*)enctype, strlen((char *)enctype));
				ShaFinal(&sha, (byte *)hash);
#endif 
				memset(unhex_token, 0, sizeof(unhex_token));
				unhexlify(unhex_token, p2, strlen(p2));
				
				if (!memcmp(hash, unhex_token, sizeof(hash))) {
					// Token matchs
#ifdef __SC_BUILD__
#else
					syslog(LOG_INFO, "client type = %s", type);
#endif
					client_get_services(cl, (char *)type);	
					cl->sess.hasgcap = true;
				} else {
#ifdef __SC_BUILD__
#else
					syslog(LOG_DEBUG, "Value of token %s, length %d", p2, strlen(p2));
					syslog(LOG_DEBUG, "Client type %s, value after encrytion %s, length %d", type, enctype, strlen((char *)enctype));
#endif
					char hash_str[41];
					char *ptr;
					memset(hash_str, 0, sizeof(hash_str));
						
					ptr = hash_str;
					for(int i = 0; i < SHA_DIGEST_LENGTH; i++) {
						sprintf(ptr, "%02x", hash[i]);
						ptr += 2;
					}
#ifdef __SC_BUILD__
                    log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "client type doesn't match. Calculated sha1 %s for size_type %d\n", hash_str, size_type);
#else
					syslog(LOG_DEBUG, "client type doesn't match. Calculated sha1 %s for size_type %d", hash_str, size_type);
#endif
					cl->servicemap = 0;
					cl->sess.hasgcap = false;
				}
				
				free(enctype);
			} 
		}

		cl->sess.logon_unknown_params = captive_params_extra(&thread->buffer[7],
				logon_params); 

		// Disable timeout as we will get a reply from auth handler
		thread->timeout = INT32_MAX;

		// Use callback
		client_login(cl, method, p1, key, keylen, captive_login, thread);
		return; // Return as we don't want the thread to be destroyed yet
	} else if (!strncmp(thread->buffer, "/logoff", 7)
	|| !strncmp(thread->buffer, "/abort", 6)) {
		if (!strncmp(thread->buffer, "/abort", 6) && cl->status == CLSTAT_AUTHED) {
			captive_redir(thread, LOGIN_ALREADY, cl);
		} else {
			client_logout(cl, LOGOUT_USER);
			captive_redir(thread, LOGOUT_ABORT, cl);
		}
		return;
	} else {
		fputs_unlocked("HTTP/1.1 404 Not Found\r\n\r\n", thread->fp);
	}


destroy:
	captive_destroy(thread);
}


// Destroy a thread and free its resources
static void captive_destroy(struct captive_thread *thread) {
	fclose(thread->fp);
	list_del(&thread->_head);
	threads[thread->type]--;
    if(thread->buffer)
    {
        free(thread->buffer);
    }
	free(thread);
}

// Captive thread garbage collector
static void captive_cleanup(struct event_timer *timer, int64_t now) {
	int32_t tnow = now / 1000;
	struct captive_thread *c, *n;
	list_for_each_entry_safe(c, n, &list_thread, _head)
		if (c->timeout <= tnow)
			captive_destroy(c);
}

// Searches the HTTP query-string "query" for a parameter "prefix"
// and puts its urldecoded value in "cout"
static int captive_param
(const char *query, const char *prefix, char *cout, size_t len) {
	char dec[] = "00";
	const char *cin = strcasestr(query, prefix);
	if (!cin || !len)
		return -1;
	cin += strlen(prefix) - 1;

	// URL-decoding (%-decoding)
	while (--len > 0 && *++cin && *cin != '&') {
		if (*cin == '%') {
			if (!isxdigit(cin[1]) || !isxdigit(cin[2]))
				return -1;
			dec[0] = *++cin;
			dec[1] = *++cin;
			unsigned char *co = (unsigned char*)(cout++);
			*co = strtol(dec, NULL, 16);
		} else {
			*cout++ = (*cin == '+') ? ' ' : *cin;
		}
	}

	*cout = 0;
	return 0;
}

// Extract parameters from url, which are not included in known_params
static char *captive_params_extra(const char *url_params, const char
		**known_params) 
{
	int ii = 0, resoff = 0, numparams = __LOGON_PARAMS_MAX;
	char res[URLBUFSIZE] = {0}, buffer[URLBUFSIZE] = {0};
	strcpy(buffer, url_params);

	if(strlen(buffer)) {
		char *tok;
		tok = strtok(buffer, "&");
		while(tok) {
			for(ii = 0; ii < numparams; ii++) {
				if(!strncmp(tok, known_params[ii], strchr(tok, '=')-tok)) {
					break;
				}
			}
			if(ii == numparams) {
				resoff += snprintf(res + resoff, sizeof(res) - resoff, "&%s", tok);

			}
			tok = strtok(NULL, "&");
		}
	}
	return strndup(res, strlen(res));
}

// URL-encodes a string
static char* captive_urlencode(const char *in, int encode_plus) {
	static const char urlencode_tbl[] = "0123456789ABCDEF";
	char *out = malloc(strlen(in) * 3 + 1), *cout = out;
	const char *cin = in - 1;
	if (!out) {
		return NULL;
	}

	while (*++cin) {
		if (!isalnum(*cin) && *cin != '-' && *cin != '_'
				&& *cin != '.' && *cin != '~') {
			unsigned char *d = (unsigned char*)cin;
			*cout++ = '%';
			*cout++ = urlencode_tbl[*d >> 4];
			*cout++ = urlencode_tbl[*d & 0x0f];
		} else {
			*cout++ = (encode_plus && *cin == ' ') ? '+' : *cin;
		}
	}

	*cout = 0;
	return out;
}

// Encrypt param (password algorithm)
static void captive_param_encrypt(char **param_en, size_t *len, const char
		*param_de, const struct client *cl)
{
	unsigned int _len = strlen(param_de);

	if(cfg.secret) {
		uint8_t md5buf[16], *encbin = NULL, *param_de_fill = NULL, *md5fill = NULL;
		unsigned int chunks = 0, fill = 0, enclen = 0;
		const uint8_t chunksize = 16;

		// XOR-Phrase = md5(Challenge + Secret)
		md5_state_t md5;
		md5_init(&md5);
		md5_append(&md5, cl->challenge, sizeof(cl->challenge));
		md5_append(&md5, (void*)cfg.secret, strlen(cfg.secret));
		md5_finish(&md5, md5buf);

		chunks = (_len)/chunksize;
		if((_len) % chunksize) {
			chunks = chunks + 1;
		}
		enclen = chunksize * chunks;
		fill = enclen - _len; 
		if(!(param_de_fill = (uint8_t *)malloc(enclen))) {
			syslog(LOG_ERR, "error in malloc");
			return;
		}
		memcpy(param_de_fill, param_de, _len);
		memset(param_de_fill + enclen - fill, 0, fill);
		if(!(md5fill = (uint8_t *)malloc(enclen))) {
			syslog(LOG_ERR, "error in malloc");
			free(param_de_fill);
			return;
		}
		for(int ii = 0; ii < chunks; ii++) {
			memcpy(md5fill + ii * chunksize, md5buf, chunksize);	
		}
		_len = enclen;
	
		// XOR-Param to get encrypted text
		if(!(encbin = (uint8_t *)malloc(enclen))) {
			syslog(LOG_ERR, "error in malloc");
			free(param_de_fill);
			free(md5fill);
			return;
		}
		for(size_t ii = 0; ii < _len; ii ++) {
			encbin[ii] = param_de_fill[ii] ^ md5fill[ii];
		}	
		if(!(*param_en) && !(*param_en = (char *)malloc((_len)*2))) {
				syslog(LOG_ERR, "error in malloc");
				free(encbin);
				free(param_de_fill);
				free(md5fill);
				return;
		}
		hexlify(*param_en, encbin, _len, 0);
		free(param_de_fill);
		free(md5fill);
		free(encbin);
	} else if (_len <= sizeof(*param_en)) {
		// Don't let params go out clear if no uamsecret is configured
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "No uamsecret, parameter can't be encrypted, set to"
				" zeros instead\n");
#else
		syslog(LOG_DEBUG, "No uamsecret, parameter can't be encrypted, set to"
				" zeros instead"); 
#endif
		if(!(*param_en) && !(*param_en = (char *)malloc((_len)))) {
			syslog(LOG_ERR, "error in malloc");
			return;
		}
		memset(*param_en, 0, _len);
	}
	*len = _len;
}

// Decrypt param (password algorithm)
static void captive_param_decrypt(uint8_t *param_de, size_t *len, const char
		*param_en, const struct client *cl)
{
	*len = strlen(param_en);

	if (cfg.secret) {
		unhexlify(param_de, param_en, *len);
		*len /= 2;

		uint8_t md5buf[16];

		// XOR-Phrase = md5(Challenge + Secret)
		md5_state_t md5;
		md5_init(&md5);
		md5_append(&md5, cl->challenge,
				sizeof(cl->challenge));
		md5_append(&md5, (void*)cfg.secret,
					strlen(cfg.secret));
		md5_finish(&md5, md5buf);

		// XOR-Param to get cleartext
		for (size_t i = 0; i < *len; ++i)
			param_de[i] ^= md5buf[i % 16];
	} else if (*len <= sizeof(param_de)) {
		memcpy(param_de, param_en, *len);
	}
}

// Return cap value given by service controller.
// Returned value needs to be freed by the caller unless cl==NULL
static char *captive_get_cap(const struct client *cl)
{
	return client_get_cap(cl);
}

// The generic HTTP redirect reply generator
static void captive_redir(struct captive_thread *thread, enum client_login res,
		struct client *cl) {
	char *resstr = NULL, uamip[INET6_ADDRSTRLEN + 2] = {0};

	if (cl->sess.uamaddr.sa.sa_family != thread->af) {
		if (thread->af == AF_INET) {
#ifdef __SC_BUILD__
        cl->sess.uamaddr.in.sin_addr = cfg.redirect_addr.ipv4;
#else
		routing_addresses(cl->ifindex, AF_INET,
					&cl->sess.uamaddr.in.sin_addr, 1);
#endif
			cl->sess.uamaddr.in.sin_port = htons(cfg.redirect_port);
		} else if (thread->af == AF_INET6) {
			routing_addresses(cl->ifindex, AF_INET6,
					&cl->sess.uamaddr.in6.sin6_addr, 1);
			cl->sess.uamaddr.in6.sin6_port = htons(cfg.redirect_port);
		}
		cl->sess.uamaddr.sa.sa_family = thread->af;
	}

	if (cl->sess.uamaddr.sa.sa_family == AF_INET6) {
		uamip[0] = '[';
		inet_ntop(AF_INET6, &cl->sess.uamaddr.in6.sin6_addr,
					uamip + 1, sizeof(uamip) - 1);
		strcat(uamip, "]");
		cl->sess.uamaddr.in6.sin6_port = htons(cfg.portal_port);
	} else {
		inet_ntop(AF_INET, &cl->sess.uamaddr.in.sin_addr,
					uamip, sizeof(uamip));
		cl->sess.uamaddr.in.sin_port = htons(cfg.portal_port);
	}
	syslog(LOG_DEBUG, "%s: URL redirect with reason %d",
		__FUNCTION__, res);
	switch (res) {
		case LOGIN_ALREADY:
			resstr = "already";
			break;

		case LOGIN_SUCCESS:
			resstr = "success";
			break;

		case LOGOUT_ABORT:
		case LOGOUT_USER:
			resstr = "logoff";
			break;

		case LOGIN_NOTYET:
			resstr = "notyet";
			break;

		case LOGIN_CHALLENGE:
			resstr = "challenge";
			break;

		case LOGIN_BUSY:
			resstr = "busy";
			break;

		default:
			resstr = "failed";
			break;
	}

	char urlbuf[URLBUFSIZE] = {0};
	char urlbuf_wisp[URLBUFSIZE] = {0}; /* A copy of notyet url with res=login instead */
	int urloff_wisp = 0;

	char sep = strchr(cfg.redirect_url, '?') ? '&' : '?';
	int urloff = snprintf(urlbuf, sizeof(urlbuf),
			"%s%cres=%s&uamip=%s&uamport=%i&nasid=%s",
			cfg.redirect_url, sep, resstr, uamip,
			cfg.portal_port, cfg.nasid);

	if (res == LOGIN_NOTYET) {
		urloff_wisp = snprintf(urlbuf_wisp, sizeof(urlbuf_wisp),
				"%s%cres=login&uamip=%s&uamport=%i&nasid=%s",
				cfg.redirect_url, sep, uamip,
				cfg.portal_port, cfg.nasid);
	}


	if (cl) {
		if (res != LOGIN_ALREADY) {
			char buf[33];
			hexlify(buf, cl->challenge,
					sizeof(cl->challenge), 0);

			urloff += snprintf(urlbuf + urloff,
					sizeof(urlbuf) - urloff,
					"&challenge=%s", buf);
			if (res == LOGIN_NOTYET) {
				urloff_wisp += snprintf(urlbuf_wisp + urloff_wisp,
							sizeof(urlbuf_wisp) - urloff_wisp,
							"&challenge=%s", buf);
			}

		} else if (res == LOGIN_SUCCESS) {
			urloff += snprintf(urlbuf + urloff,
					sizeof(urlbuf) - urloff,
					"&uid=%s", cl->sess.username);
		}

		char clientip[INET6_ADDRSTRLEN];
		inet_ntop(thread->af, &thread->addr, clientip, sizeof(clientip));

		urloff += snprintf(urlbuf + urloff, sizeof(urlbuf) - urloff,
				"&mac=%02X-%02X-%02X-%02X-%02X-%02X&ip=%s",
				cl->mac[0], cl->mac[1], cl->mac[2],
				cl->mac[3], cl->mac[4], cl->mac[5], clientip);

		if (res == LOGIN_NOTYET) {
			urloff_wisp += snprintf(urlbuf_wisp + urloff_wisp, sizeof(urlbuf_wisp) - urloff_wisp,
						"&mac=%02X-%02X-%02X-%02X-%02X-%02X&ip=%s",
						cl->mac[0], cl->mac[1], cl->mac[2],
						cl->mac[3], cl->mac[4], cl->mac[5], clientip);
		}


		if (cl->sess.method == CLIENT_EAP && cl->sess.key) {
			char buf[1720];
			int buflen = sizeof(buf);
			if (!base64_encode((uint8_t*)buf, &buflen,
						cl->sess.eap, cl->sess.eaplen)) {
				urloff += snprintf(urlbuf + urloff,
						sizeof(urlbuf) - urloff,
						"&eapmsg=%s", buf);
				if (res == LOGIN_NOTYET) {
					urloff_wisp += snprintf(urlbuf_wisp + urloff_wisp,
								sizeof(urlbuf_wisp) - urloff_wisp,
								"&eapmsg=%s", buf);
				}
			}
		}

		if (cl->sess.userurl) {
			char *encoded = captive_urlencode(cl->sess.userurl, 1);
			if (encoded) {
				urloff += snprintf(urlbuf + urloff,
						sizeof(urlbuf) - urloff,
						"&userurl=%s", encoded);
				if (res == LOGIN_NOTYET) {
					urloff_wisp += snprintf(urlbuf_wisp + urloff_wisp,
								sizeof(urlbuf_wisp) - urloff_wisp,
								"&userurl=%s", encoded);	
				}

				free(encoded);
			}
		}

		const char *reply = cl->sess.reply;
		if (!reply) {
			if (res == LOGIN_TIMEOUT)
				reply = "SERVER_TIMEOUT";
			else if (res == LOGIN_ISPFAIL_OVERLOAD)
				reply = "TOO_MANY_USERS";
			else if (res == LOGIN_DENIED)
				reply = "BAD_PASSWORD";
#ifdef __SC_BUILD__
			else if (res == LOGIN_LIMIT)
				reply = "202:Hotspot_Limit_Exceeded";
#endif
			else
				reply = "UNKNOWN_ERROR";
		}

		if (!strcmp(resstr, "failed")) 
			urloff += snprintf(urlbuf + urloff,
					sizeof(urlbuf) - urloff,
					"&reply=%s", reply);

		if ((res == LOGIN_SUCCESS || !strcmp(resstr, "failed")) &&
				cl->sess.logon_unknown_params) {
			urloff += snprintf(urlbuf + urloff, sizeof(urlbuf) -urloff, "%s",
				cl->sess.logon_unknown_params);
		}

		if(res == LOGIN_NOTYET || res == LOGOUT_ABORT || res == LOGOUT_USER) {
			const char *cap = captive_get_cap(NULL);
			char *enccap = NULL;
			size_t size_enccap = 0;
			if(cap) {
				captive_param_encrypt(&enccap, &size_enccap, cap, cl);
			}
			urloff += snprintf(urlbuf + urloff, sizeof(urlbuf) -urloff,
					"&cap=%s", enccap?enccap:"");
			if (res == LOGIN_NOTYET) {
				urloff_wisp += snprintf(urlbuf_wisp + urloff_wisp, 
							sizeof(urlbuf_wisp) -urloff_wisp,
							"&cap=%s", enccap?enccap:"");
			}
			free(enccap);
		} else if(res == LOGIN_SUCCESS) {
			if(cl->sess.hasgcap) {
				char *gcap = captive_get_cap(cl), *encgcap = NULL;
				size_t size_encgcap = 0;
				if(gcap) {
					captive_param_encrypt(&encgcap, &size_encgcap, gcap, cl);
				}
				urloff += snprintf(urlbuf + urloff, sizeof(urlbuf) -urloff,
						"&gcap=%s", encgcap?encgcap:"");
				free(gcap);
				free(encgcap);
			}
		}
	}

	if (res == LOGIN_NOTYET) {
		const char *location = config_get_string("main", "location_name", "");

		fprintf(thread->fp, "HTTP/1.1 302 Found\r\n"
			"Server: " HOTSPOT_CODENAME "\r\n"
			"Connection: close\r\n"
			"Location: %s\r\n"
			"Content-Type: text/html\r\n\r\n"
			"<HTML><!--\n"
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<WISPAccessGatewayParam "
			"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
			"xsi:noNamespaceSchemaLocation="
			"\"http://www.acmewisp.com/WISPAccessGatewayParam.xsd\">\n"
			"<Redirect>\n"
			"<AccessProcedure>1.0</AccessProcedure>\n"
			"<AccessLocation>12</AccessLocation>\n"
			"<LocationName>%s</LocationName>\n"
			"<LoginURL>%s</LoginURL>\n"
			"<AbortLoginURL>http://%s:%d/logoff</AbortLoginURL>\n"
			"<MessageType>110</MessageType>\n"
			"<ResponseCode>0</ResponseCode>\n"
			"</Redirect>\n"
			"</WISPAccessGatewayParam>\n--></HTML>",
			urlbuf, location, urlbuf_wisp, uamip, cfg.portal_port);
	} else {
		fprintf(thread->fp, "HTTP/1.1 302 Found\r\n"
			"Server: " HOTSPOT_CODENAME "\r\n"
			"Connection: close\r\n"
			"Location: %s\r\n\r\n", urlbuf);
	}

	captive_destroy(thread);
}

MODULE_REGISTER(captive, 360)
