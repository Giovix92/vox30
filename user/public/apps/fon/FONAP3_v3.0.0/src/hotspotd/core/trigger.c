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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
#include <sys/wait.h>
#include <sys/types.h>

#include <lib/config.h>
#include <lib/list.h>
#include <lib/event.h>

#define TRIGGER_ARGC 128

#include "hotspotd.h"
#ifndef __SC_BUILD__
#include "routing.h"
#endif
#include "trigger.h"

extern char **environ;

static struct list_head handles = LIST_HEAD_INIT(handles);
static void trigger_handle(struct trigger_handle *hndl, short code, short status);
static void trigger_sigchld(struct event_signal *ev, const siginfo_t *sinfo);

struct event_signal event_sigchld = {
	.signal = SIGCHLD,
	.handler = trigger_sigchld,
};

struct trigger_handle {
	struct list_head _head;
	pid_t pid;
	trigger_cb *cb;
	void *ctx;
};


static int trigger_init() {
	event_ctl(EVENT_SIGNAL_ADD, &event_sigchld);
	return 0;
}

static int trigger_apply() {
	return 0;
}

static void trigger_deinit() {
	event_ctl(EVENT_SIGNAL_DEL, &event_sigchld);
	while (!list_empty(&handles)) {
		struct trigger_handle *hnd;
		hnd = list_first_entry(&handles, struct trigger_handle, _head);
		pid_t pid = hnd->pid;
		kill(pid, SIGTERM);
		trigger_handle(hnd, CLD_KILLED, SIGTERM);
		waitpid(pid, NULL, 0);
	}
}


// Convert a trigger string to an argument vector
static size_t trigger_argv(char *cmd, char **argv, size_t len) {
	char *saveptr;
	for (size_t i = 0; i < len-1; ++i) {
		argv[i] = strtok_r(cmd, " \t", &saveptr);
		if (!argv[i])
			return i;
		cmd = NULL;
	}
	return len;
}

// Copy an environment vector into a new one
static size_t trigger_envv(char *const *from, char **to, size_t len) {
	if (len == 0)
		return 0;

	size_t fromln;
	for (fromln = 0; from && from[fromln] && fromln < len - 1; fromln++);

	memcpy(to, from, fromln * sizeof(char*));
	to[fromln] = NULL;
	return fromln;
}

// Create and execute a new trigger and register an event to handle its completion
struct trigger_handle* trigger_run(const char *cmd, char *const *argv,
		char *const *env, trigger_cb *cb, void *ctx) {
	char *exec_argv[TRIGGER_ARGC], *exec_env[TRIGGER_ARGC];
	char cmdbuf[1024];

	// Read trigger command string
	if (!cmd || strlen(cmd) >= sizeof(cmdbuf))
		return NULL;
	strcpy(cmdbuf, cmd);

	// Create argument vector
	size_t i = trigger_argv(cmdbuf, exec_argv, TRIGGER_ARGC - 1);
	if (argv)
		while (i < TRIGGER_ARGC - 1 && *argv)
			exec_argv[i++] = *argv++;
	exec_argv[i] = NULL;

	// Create environment vector
	char env_iface[32];
#ifdef __SC_BUILD__
	const char *iface = routing_cfg.iface_eap;
#else
	const char *iface = config_get_string("main", "iface", NULL);
#endif
	hotspot_assertconf_string("main.iface", iface);
	snprintf(env_iface, sizeof(env_iface), "HOTSPOT_IFACE=%s",
			iface?iface:"");

	i = 0;
	exec_env[i++] = "HOTSPOT_MANAGER=" HOTSPOT_VERSIONCODE;
	exec_env[i++] = env_iface;

	i += trigger_envv(env, &exec_env[i], TRIGGER_ARGC - i - 1);
	trigger_envv(environ, &exec_env[i], TRIGGER_ARGC - i - 1);

	// Verify executable
	if (access(exec_argv[0], X_OK)) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "trigger: unable to execute %s: %s\n",
					exec_argv[0], strerror(errno));
#else
		syslog(LOG_WARNING, "trigger: unable to execute %s: %s",
					exec_argv[0], strerror(errno));
#endif
		return NULL;
	}

	// Create trigger handle
	struct trigger_handle *hndl = malloc(sizeof(*hndl));
	if (!hndl)
		return NULL;

	// Create child process and execute command
	pid_t pid = fork();
	if (pid < 0) {
		free(hndl);
		return NULL;
	} else if (pid == 0) {
		execve(exec_argv[0], exec_argv, exec_env);
		_exit(128);
	}
#ifdef __SC_BUILD__
    log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "trigger: launched %s (PID %i)", cmd, (int)pid);
#else
	syslog(LOG_INFO, "trigger: launched %s (PID %i)", cmd, (int)pid);
#endif
	// Register event handler for completion
	hndl->pid = pid;
	hndl->cb = cb;
	hndl->ctx = ctx;
	list_add(&hndl->_head, &handles);

	return hndl;
}

void trigger_cancel(struct trigger_handle *handle) {
	kill(handle->pid, SIGTERM);
	handle->cb = NULL;
}

void trigger_wait(struct trigger_handle *hndl) {
	int s;
	if (waitpid(hndl->pid, &s, 0) == hndl->pid) {
		trigger_handle(hndl,
			WIFEXITED(s) ? CLD_EXITED : CLD_KILLED,
			WIFEXITED(s) ? WEXITSTATUS(s) : WTERMSIG(s));
	}
}

static void trigger_sigchld(struct event_signal *ev, const siginfo_t *sinfo) {
	int s;
	pid_t pid;

	while ((pid = waitpid(-1, &s, WNOHANG)) > 0) {
		struct trigger_handle *hndl;
		list_for_each_entry(hndl, &handles, _head) {
			if (hndl->pid != pid)
				continue;

			trigger_handle(hndl,
				WIFEXITED(s) ? CLD_EXITED : CLD_KILLED,
				WIFEXITED(s) ? WEXITSTATUS(s) : WTERMSIG(s));
			break;
		}
	}
}

static void trigger_handle(struct trigger_handle *hndl, short code, short status) {
#ifdef __SC_BUILD__
    log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "trigger: %i exited (status: %i, code: %i)",
				(int)hndl->pid, (int)code, (int)status);
#else
	syslog(LOG_INFO, "trigger: %i exited (status: %i, code: %i)",
				(int)hndl->pid, (int)code, (int)status);
#endif
	if (hndl->cb)
		hndl->cb(hndl->ctx, code, status);

	waitpid(hndl->pid, NULL, WNOHANG);
	list_del(&hndl->_head);
	free(hndl);
}

RESIDENT_REGISTER(trigger, 180)
