/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#include <utility.h>
#endif
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "lib/event.h"
#include "lib/config.h"
#include "lib/urandom.h"
#include "lib/str.h"
#include "hotspotd.h"

static struct list_head mods = LIST_HEAD_INIT(mods);
static struct list_head mcalls = LIST_HEAD_INIT(mcalls);
static const char *pidfile = NULL;
static char *const *main_argv = NULL;
static int main_argc = 0,daemonized = 0;
bool status_online = false;
#ifdef __SC_BUILD__
volatile int server_status = HOTSPOT_INIT;
#ifdef CONFIG_SUPPORT_5G_QD
char* ethif_mapping_to_qd_if(char* ethif)
{
    static char qd_if[64] = {0};
    if(strcmp(ethif,"eth3.100") == 0)
        snprintf(qd_if,sizeof(qd_if),"%s","wifi1");
    else if (strcmp(ethif,"eth3.200") == 0)
        snprintf(qd_if,sizeof(qd_if),"%s","wifi2");
            
    return qd_if;
}
#endif
#endif
// Register a module
void hotspot_module(struct hotspot_module *m) {
	struct list_head *c;
	for (c = mods.next;
	c != &mods && list_entry(c, struct hotspot_module, _head)->prio < m->prio;
	c = c->next);

	list_add_tail(&m->_head, c);
}

// Register a multicall
void hotspot_multicall(struct hotspot_multicall *m) {
	list_add_tail(&m->_head, &mcalls);
}

// Control API
int hotspot_control(enum hotspot_control cmd) {
	if (cmd == HOTSPOT_CTRL_INIT || cmd == HOTSPOT_RESIDENT_INIT) {
		struct hotspot_module *mod;
		list_for_each_entry(mod, &mods, _head) {
			if (mod->status
			|| (cmd == HOTSPOT_RESIDENT_INIT && !mod->resident))
				continue;
#ifndef __SC_BUILD__
			syslog(LOG_INFO, "Initializing module: %s", mod->name);
#endif
            DPRINTF("mod %s init start\n",mod->name);
			if (mod->init()) {
#ifdef __SC_BUILD__
                log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Failed to initialize %s: %s",
							mod->name, strerror(errno));
#else
				syslog(LOG_ERR, "Failed to initialize: %s",
							strerror(errno));
#endif
				return HOTSPOT_SYSTEM_ERROR;
			} else {
				mod->status = 1;
			}
			if (cmd == HOTSPOT_CTRL_INIT) {
				status_online = true;
			}
		}

		return 0;
	} else if (cmd == HOTSPOT_CTRL_DEINIT
					|| cmd == HOTSPOT_RESIDENT_DEINIT) {
		struct hotspot_module *mod;
		list_for_each_entry_reverse(mod, &mods, _head) {
			if (!mod->status
			|| (cmd != HOTSPOT_RESIDENT_DEINIT && mod->resident))
				continue;
#ifdef __SC_BUILD__
            log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Deinitializing module: %s",
								mod->name);
#else
			syslog(LOG_INFO, "Deinitializing module: %s",
								mod->name);
#endif
            DPRINTF("mod %s deinit start\n",mod->name);
			mod->deinit();
			mod->status = 0;
		}
		if (cmd == HOTSPOT_CTRL_DEINIT) {
			status_online = false;
		}
		return 0;
	} else if (cmd == HOTSPOT_CTRL_APPLY) {
		struct hotspot_module *mod;
		list_for_each_entry(mod, &mods, _head) {
			if (!mod->status)
				continue;
#ifndef __SC_BUILD__
			syslog(LOG_INFO, "Applying module: %s", mod->name);
#endif
			if (mod->apply()) {
				return HOTSPOT_RUNTIME_ERROR;
			} else {
				mod->status = 1;
			}
		}
		return 0;
	} else {
		return HOTSPOT_RUNTIME_ERROR;
	}
}

bool hotspot_assertconf_string(const char *name, const char *value)
{
	HOTSPOT_ASSERTCONF(name, value)
}

bool hotspot_assertconf_int(const char *name, const int *value)
{
	HOTSPOT_ASSERTCONF(name, value)
}

bool hotspot_assertconf_u8(const char *name, const uint8_t *value)
{
	HOTSPOT_ASSERTCONF(name, value)
}

bool hotspot_assertconf_u16(const char *name, const uint16_t *value)
{
	HOTSPOT_ASSERTCONF(name, value)
}

bool hotspot_assertconf_u32(const char *name, const uint32_t *value)
{
	HOTSPOT_ASSERTCONF(name, value)
}

static int configure() {
	const char *configfile = "hotspotd";
	optind = 1;
	int c, verbosity = 0, daemonize = 0, logflags = LOG_CONS;
	while ((c = getopt(main_argc, main_argv, "c:o:p:devh")) != -1) {
		switch (c) {
		case 'c':
			configfile = optarg;
			break;

		case 'v':
			verbosity++;
			break;

		case 'd':
			daemonize = 1;
			break;

		case 'e':
			logflags |= LOG_PERROR;
			break;

		case 'p':
			pidfile = optarg;
			break;

		case 'o':
			break;

		default:
			fprintf(stderr,
				"hotspotd " HOTSPOT_VERSIONCODE " - speedy hotspot solution\n"
				"(c) 2011 Steven Barth, John Crispin\n\n"
				"Usage: %s [options]\n\n"
				"Options:\n"
				"	-c <name>			Use configuration file /etc/config/<name>\n"
				"	-o section.key=val		Override config value from config file\n"
				"	-e				Output log messages on stderr\n"
				"	-v				Be more verbose (might be used multiple times)\n"
				"	-d				Daemonize\n"
				"	-p <pidfile>			Write a pidfile\n"
				"	-h				Show this help\n\n",
			main_argv[0]);
			return HOTSPOT_USAGE_ERROR;
		}
	}
	closelog();
	openlog(HOTSPOT_VERSIONCODE, logflags, LOG_DAEMON);

	struct rlimit lim = {HOTSPOT_LIMIT_RES, HOTSPOT_LIMIT_RES};
	setrlimit(RLIMIT_NOFILE, &lim);

	if (verbosity == 0) {
		setlogmask(LOG_UPTO(LOG_WARNING));
	} else if (verbosity == 1) {
		setlogmask(LOG_UPTO(LOG_INFO));
	}


	// Configure
	if (config_init(configfile)) {
#ifdef __SC_BUILD__
        log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Failed to read configuration, check "
			"/etc/config/%s: %s", configfile, strerror(errno));
#else
		syslog(LOG_ERR, "Failed to read configuration, check "
			"/etc/config/%s: %s", configfile, strerror(errno));
#endif
		return HOTSPOT_CONFIG_ERROR;
	}

	// Override config values with command line parameters
	optind = 1;
	while ((c = getopt(main_argc, main_argv, "c:o:p:devh")) != -1) {
		char *or, *sec, *opt, *saveptr;
		if (c != 'o' || !(or = strdup(optarg)))
			continue;

		if ((sec = strtok_r(or, "+.", &saveptr))
		&& (opt = strtok_r(NULL, "=", &saveptr))) {
			char *val = strtok_r(NULL, "\r\n", &saveptr);
			size_t optlen = strlen(opt);
			if (opt[optlen-1] == '+') {
				opt[optlen-1] = 0;
				config_add_string(sec, opt, val);
			} else {
				config_set_string(sec, opt, val);
			}
		}

		free(or);
	}

	const char *__devname = config_get_string("main", "devname", NULL);
	if(!hotspot_assertconf_string("main.devname", __devname)) {
		return HOTSPOT_CONFIG_ERROR;
	} 
	strncpy(devname, __devname, sizeof(devname));
#ifndef __SC_BUILD__
	const char *redirect_url_template = config_get_string("redirect",
			"templateurl", NULL);
	if(!hotspot_assertconf_string("redirect.templateurl",
			redirect_url_template)) {
		return HOTSPOT_CONFIG_ERROR;
	}
	char *devnamedup = strdup(devname);
	snprintf(redirect_url_default, sizeof(redirect_url_default),
			redirect_url_template, strlwr(devnamedup));
	free(devnamedup);
#endif
	if (daemonize && !daemonized) {
		pid_t pid = fork();
		if (pid < 0)
			return 1;

		if (pid) {
			if (pidfile) {
				FILE *fp = fopen(pidfile, "w");
				if (fp) {
					fprintf(fp, "%i", (int)pid);
					fclose(fp);
				}
			}
			_exit(0);
		} else {
			if (chdir("/")) {
				// Dummy
			}
			int fd = open("/dev/null", O_RDWR);
			if (fd >= 0) {
				dup2(fd, STDIN_FILENO);
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				close(fd);
			}
			setsid();
			daemonized = 1;
		}
	} else if (!daemonized) {
		if (pidfile) {
			FILE *fp = fopen(pidfile, "w");
			if (fp) {
				fprintf(fp, "%i", getpid());
				fclose(fp);
			}
		}
	}

	return HOTSPOT_OK;
}

// Signalhandler for SIGTERM, SIGINT, SIGHUP
static void sighandler(int signal) {
	if (signal == SIGHUP) {
#ifdef __SC_BUILD__
		event_stop(HOTSPOT_CONFIGRELOAD);
#else
		event_stop(HOTSPOT_OK);
#endif
	} else {
		event_stop(HOTSPOT_SIGNALLED);
	}
}
int main(int argc, char *const *argv) {
	struct hotspot_multicall *mc;
	list_for_each_entry(mc, &mcalls, _head)
		if (*argv && strlen(mc->name) <= strlen(*argv)
		&& !strcmp(mc->name, *argv + strlen(*argv) - strlen(mc->name)))
			return mc->call(argc, argv);

	openlog(HOTSPOT_VERSIONCODE, LOG_CONS | LOG_PERROR, LOG_DAEMON);
	if (getuid() != 0) {
#ifdef __SC_BUILD__
        log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Must have superuser rights to run FON!\n");
#else
		syslog(LOG_ERR, "Must have superuser rights to run!");
#endif
		return HOTSPOT_RUNTIME_ERROR;
	}
#ifdef __SC_BUILD__
    log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "FON staring up...\n");
#else
	syslog(LOG_WARNING, "starting up...");
#endif
	struct sigaction sa = {.sa_handler = sighandler};
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);

	main_argc = argc;
	main_argv = argv;

	if (urandom_init() || event_init()) {
#ifdef __SC_BUILD__
        log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "unable to initialize FON core\n");
#else
		syslog(LOG_CRIT, "unable to initialize core");
#endif
		return HOTSPOT_RUNTIME_ERROR;
	}

	// Configure and load modules
#ifdef __SC_BUILD__
    server_status = HOTSPOT_INIT;
#else
	int state = 0;
#endif
#ifndef __SC_BUILD__
	do {
		if ((state = configure()))
			return state;
		// Load residents and enter main loop
		hotspot_control(HOTSPOT_RESIDENT_INIT);
		state = event_run();
		hotspot_control(HOTSPOT_RESIDENT_DEINIT);
	} while (!state);
#else
	do {
        DPRINTF("server status %d\n",server_status);
        if(server_status == HOTSPOT_INIT || server_status == HOTSPOT_CONFIGRELOAD)
        {
            if ((server_status = configure()))
                return server_status;
        }
        if(server_status == HOTSPOT_OK || server_status == HOTSPOT_START)
        {
            hotspot_control(HOTSPOT_RESIDENT_INIT);
            server_status = event_run(1);
        }
        else
        {
            server_status = event_run(0);
        }
        sleep(1);
	} while (server_status <= HOTSPOT_OK);
    hotspot_control(HOTSPOT_RESIDENT_DEINIT);
#endif

	event_deinit();
	urandom_deinit();

	if (pidfile)
		unlink(pidfile);

	closelog();
#ifdef __SC_BUILD__
	return server_status;
#else
	return state;
#endif
}
