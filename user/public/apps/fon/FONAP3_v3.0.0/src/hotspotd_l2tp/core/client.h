#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdbool.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "lib/list.h"


enum client_status {
	CLSTAT_NOALLOC = 0,	// Internal state
	CLSTAT_NOAUTH,		// unauthorized
	CLSTAT_LOGIN,		// login in process
	CLSTAT_AUTHED,		// authorized
	CLSTAT_LOGOUT,		// logging out
};

enum client_login {
	LOGIN_SUCCESS			= 0,
	LOGIN_PENDING			= LOGIN_SUCCESS,

// Logout status
	LOGOUT_USER 			= 1,
	LOGOUT_DISCONNECT 		= 2,
	LOGOUT_LOST_SERVICE		= 3,
	LOGOUT_IDLE 			= 4,
	LOGOUT_SESSION_LIMIT		= 5,
	LOGOUT_ADMIN_RESET		= 6,
	LOGOUT_ADMIN_REBOOT		= 7,
	LOGOUT_NAS_ERROR		= 9,
	LOGOUT_SERVICE_UNAVAILABLE	= 15,
	LOGOUT_ABORT			= 40,

// Login status
	LOGIN_ALREADY			= 50, // Chillispot madness compatibility...
	LOGIN_DENIED			= 51,
	LOGIN_ERROR			= 52,
#ifdef __SC_BUILD__
	LOGIN_LIMIT			= 53,
#endif
	LOGIN_NOTYET			= 55,
	LOGIN_TIMEOUT			= 58,
	LOGIN_INVALID_REQUEST		= 61,
	LOGIN_CHALLENGE			= 62,
	LOGIN_BUSY			= 63,
	LOGIN_INSUFFICIENT_PAYMENT,
	LOGIN_INSUFFICIENT_SESSIONS,
	LOGIN_ISPFAIL_AUTHSERVER,
	LOGIN_ISPFAIL_OVERLOAD,
	_LOGIN_FAIL_MIN			= LOGIN_ALREADY,
};

enum client_method {
	CLIENT_GRANT = 1,	// grant access without logging in
	CLIENT_PAP,		// use PAP to authenticate
	CLIENT_CHAP,		// use CHAP to authenticate
	CLIENT_EAP		// use EAP to authenticate
};

enum client_req {
	CLIENT_LOGIN = 0,
	CLIENT_ACCOUNTING_START = 1,
	CLIENT_ACCOUNTING_STOP = 2,
	CLIENT_ACCOUNTING_INTERIM = 3,
	CLIENT_LOGOUT = 4,
	CLIENT_UPDATE_ADDRESSES = 5
};

struct client;
struct client_backend;
typedef void (client_cb)(struct client* client, enum client_login status, void *ctx);
typedef void (client_update_cb)(struct client *client,
		enum client_login status, const struct client_backend *next);
typedef void (client_auth_cb)(struct client* client, enum client_req request,
		client_update_cb *cb, void **ctx);
typedef int (client_stats_cb)(uint16_t client_id, uint64_t *byte_in, uint64_t *byte_out, 
			      uint32_t *pkt_in, uint32_t *pkt_out);
typedef uint64_t (client_get_servicemap_cb)(const char *type);
typedef void (client_set_services_cb)(const uint32_t id, uint64_t servicemap, bool open);
typedef char *(client_get_cap_cb)(bool all, uint64_t servicemap);

struct client {
	uint16_t id;
	enum client_status status;
	enum client_login login_status;
	int ifindex;
	uint8_t mac[6];
	uint8_t challenge[16];
	int64_t time_last_touched;
	int64_t limit_time_idle;
	uint64_t servicemap;
	bool serviced;
	struct client_session {
		enum client_method method;
		char *username;
		uint8_t *key;
		size_t keylen;
		uint8_t *state;
		size_t statelen;
		uint8_t *eap;
		size_t eaplen;
		uint32_t term_action;
		char *userurl;
		char *reply;
		char *logon_unknown_params;
		bool hasgcap;
		struct {
			uint8_t *send_key;
			uint8_t *recv_key;
			uint16_t send_keylen;
			uint16_t recv_keylen;
		} mppe;
		void *class;
		void *cui;
		uint32_t id;
		uint32_t cui_len;
		uint32_t pkts;
		uint32_t interim_interval;
		uint32_t limit_time_conn;
		uint64_t limit_bytes_out;
		uint64_t limit_bytes_in;
		uint64_t limit_bytes_total;
		int64_t time_start;
		int64_t time_last_interim;
		client_cb *login_cb;
		void *login_ctx;
		client_auth_cb *backend;
		void *backend_ctx;
		client_stats_cb *stats_cb;
		char *auth_conf;
		char *acct_conf;
		bool noacct;
		bool authed;
		bool addrs_need_update;
		bool addr_update_in_progress;
		union {
			struct sockaddr sa;
			struct sockaddr_in in;
			struct sockaddr_in6 in6;
		} uamaddr;
		struct in6_addr *req_ip6addr;
		size_t req_ip6addr_cnt;
		struct in_addr *req_ipaddr;
		size_t req_ipaddr_cnt;
	} sess;
};

/**
 * Return the client ID of a client.
 *
 * returns client ID
 */
int client_id(const struct client *client);

/**
 * Return the number of currently logged in clients.
 */
int client_served();

/**
 * Start the login process for a client.
 *
 * cl:		client handle
 * method:	login method
 * user:	username to login
 * key:		key or token to login with (e.g. password)
 * len:		length of the key or token
 * login_cb:	callback to use when login procedure is completed
 * login_ctx:	context parameter for the callback
 */
void client_login(struct client *cl, enum client_method method,
		const char *user, const uint8_t *key, size_t len,
		client_cb *login_cb, void *login_ctx);

/**
 * End the session of the given user or abort his login attempt with the
 * given login status / logout reason.
 */
void client_logout(struct client *cl, enum client_login reason);



/**
 * Renew client addresses with latest information available
 */
void client_renew_addresses(struct client *cl);

/**
 * Inform that a specific client has changed its IP-addresses
 */
void client_inform_address_update(int ifindex, const uint8_t mac[6]);

/**
 * Set servicemap for a specific client given its type
 */
void client_get_services(struct client *cl, const char *type);

/**
 * Open service ports for a specific client
 */
void client_set_services(struct client *cl, bool open);

/**
 * Get "cap" value from service controller for a specific client or all
 */
char *client_get_cap(const struct client *cl);

/**
 * Find the client handle or create a new client handle for a user with
 * the address addr of given address family af.
 *
 * returns client handle or NULL on error
 */
struct client* client_register(int ifindex, int af, const void *addr);
#ifdef __SC_BUILD__
unsigned client_get_current_authercated_users(struct client *cl);
unsigned client_is_reach_max_login_user(struct client *cl);
#endif

/**
 * Find the client backend with the given name.
 */
const struct client_backend* client_get_backend(const char *name);


struct client_backend {
	struct list_head _head;		// (internal)
	client_auth_cb *auth;		// authentication handler
	char name[16];			// name
};

struct client_serv {
	client_get_servicemap_cb *get_servicemap;
	client_set_services_cb *set_services;
	client_get_cap_cb *get_cap;
};

/*
 * Register a new client backend.
 * (internally called by BACKEND_REGISTER)
 */
void client_backend(struct client_backend *ba);

/*
 * Register the services controller
 * (internally called by SERVICES_REGISTER)
 */
void client_services(struct client_serv *serv);

/**
 * Register a new client backend.
 */
#define BACKEND_REGISTER(ba) \
static void __attribute__((constructor)) _clinit(void) { client_backend(&ba); }

/**
 * Register the services controller
 */
#define SERVICES_REGISTER(serv) \
static void __attribute__((constructor)) _clinit(void) { client_services(&serv); }

#endif /* CLIENT_H_ */
