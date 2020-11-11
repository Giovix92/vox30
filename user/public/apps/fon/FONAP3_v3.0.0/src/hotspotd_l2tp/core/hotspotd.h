#ifndef HOTSPOTD_H_
#define HOTSPOTD_H_

#include "lib/list.h"
#include <signal.h>

#define HOTSPOT_CODENAME	"hotspotd"
#define HOTSPOT_VERSIONCODE	HOTSPOT_CODENAME "/1.3.5"

// Check each module before modifying
#define HOTSPOT_LIMIT_USER	4000
#define HOTSPOT_LIMIT_RES	(HOTSPOT_LIMIT_USER + 96)

char devname[10];
char redirect_url_default[256];
extern bool status_online;

#ifdef __SC_BUILD__
extern volatile int server_status;
#endif
enum hotspot_control {
	HOTSPOT_CTRL_INIT,		// Start hotspot modules
	HOTSPOT_CTRL_DEINIT,		// Stop hotspot modules
	HOTSPOT_CTRL_APPLY,		// Apply configuration (soft restart)
// internal:
	HOTSPOT_RESIDENT_INIT,		// Start resident modules
	HOTSPOT_RESIDENT_DEINIT,	// Stop all modules including residents
};

/**
 * Changes the operation mode of the hotspotd modules.
 *
 * returns status
 */
int hotspot_control(enum hotspot_control cmd);

/*
 * Register a module
 * (internally called by MODULE_REGISTER and RESIDENT_REGISTER)
 */
struct hotspot_module;
void hotspot_module(struct hotspot_module *mod);


/*
 * Register a multicall callback
 * (internally called by MULTICALL_REGISTER
 */
struct hotspot_multicall;
void hotspot_multicall(struct hotspot_multicall *mcall);

/*
 * Check whether configuration value was found. If not, stop daemon cleanly.
 */
bool hotspot_assertconf_string(const char *name, const char *value);
bool hotspot_assertconf_int(const char *name, const int *value);
bool hotspot_assertconf_u8(const char *name, const uint8_t *value);
bool hotspot_assertconf_u16(const char *name, const uint16_t *value);
bool hotspot_assertconf_u32(const char *name, const uint32_t *value);

#define HOTSPOT_ASSERTCONF(NAME, VAL) \
	bool ret = true;	\
	if(!VAL || !(*VAL)) {	\
		syslog(LOG_ERR, "%s not found", NAME);	\
		event_stop(HOTSPOT_CONFIG_ERROR);	\
		errno = EINVAL;	\
		ret = false;	\
	}	\
	return ret;

/**
 * Register a new module with priority PRIO (smaller means higher priority)
 * and its functions NAME_init, NAME_apply and NAME_deinit.
 */
#define MODULE_REGISTER(NAME, PRIO) \
static struct hotspot_module _moddata = { .prio = PRIO, .init = NAME ## _init, .apply = NAME ## _apply, .deinit = NAME ## _deinit, .status = 0, .name = #NAME }; \
static void __attribute__((constructor)) _modinit(void) { hotspot_module(&_moddata); }

/**
 * Register a new resident module.
 */
#define RESIDENT_REGISTER(NAME, PRIO) \
static struct hotspot_module _resdata = { .prio = PRIO, .init = NAME ## _init, .apply = NAME ## _apply, .deinit = NAME ## _deinit, .status = 0, .name = #NAME, .resident = 1 }; \
static void __attribute__((constructor)) _resinit(void) { hotspot_module(&_resdata); }

/**
 * Register a new multicall
 */
#define MULTICALL_REGISTER(NAME, CALLBACK) \
static struct hotspot_multicall _mcall_ ## NAME = { .call = CALLBACK, .name = #NAME }; \
static void __attribute__((constructor)) _mcinit_ ## NAME(void) { hotspot_multicall(&_mcall_ ## NAME); }

/*
 * Hotspotd return codes
 */
enum hotspot_return {
#ifdef __SC_BUILD__
    HOTSPOT_LOWER_BW = -6,
    HOTSPOT_TUNNEL_FAIL = -5,
    HOTSPOT_INIT = -4,
    HOTSPOT_STOP = -3,
    HOTSPOT_START = -2,
    HOTSPOT_CONFIGRELOAD = -1,
#endif
	HOTSPOT_OK = 0,
	HOTSPOT_USAGE_ERROR = 1,
	HOTSPOT_CONFIG_ERROR = 2,
	HOTSPOT_SYSTEM_ERROR = 3,
	HOTSPOT_RUNTIME_ERROR = 4,
	HOTSPOT_SIGNALLED = 5,
};

/*
 * Internal hotspotd signals
 */
enum hotspotd_event {
	SIGNAL_ROUTING_IFSTATE	= 0x101,
	SIGNAL_ROUTING_IFADDR	= 0x102,
};

struct hotspot_module {
	struct list_head _head;
	int prio;
	int resident;
	int status;
	int (*init)();
	int (*apply)();
	void (*deinit)();
	char name[32];
};

struct hotspot_multicall {
	struct list_head _head;
	int (*call)(int argc, char *const *argv);
	char name[32];
};
#ifdef __SC_BUILD__
#ifdef SC_DEBUG
#define DPRINTF  printf
#else
#define DPRINTF(...)
#endif
#endif
#endif /* HOTSPOTD_H_ */
