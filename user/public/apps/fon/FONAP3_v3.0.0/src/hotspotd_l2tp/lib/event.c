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
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <features.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <limits.h>

#include "list.h"
#include "event.h"
#ifdef __SC_BUILD__
#include "core/hotspotd.h"
#include "log.h"
#include "lib/config.h"
#endif
// Woooooh, this get's ugly (uclibc ftw).
// Basically race conditions here should not be harmful
// as signals get written to an fd attached to epoll.
// Be careful though.
#ifdef __UCLIBC__
#define epoll_pwait epoll_xwait
int epoll_xwait(int epfd, struct epoll_event *events,
int maxevents, int timeout, const sigset_t *sigmask) {
	sigset_t origmask;
	sigprocmask(SIG_SETMASK, sigmask, &origmask);
	int ret = epoll_wait(epfd, events, maxevents, timeout);
	sigprocmask(SIG_SETMASK, &origmask, NULL);
	return ret;
}
#endif

static int fd_epoll = -1;				/* event multiplexer */
static int fd_sigpipe_out = -1;			/* signal synchronisation pipe */
static volatile int event_exit = -1, event_exitcode = 0;

static struct list_head list_signal = LIST_HEAD_INIT(list_signal);
static struct list_head list_timer = LIST_HEAD_INIT(list_timer);

static sigset_t sig_default, sig_registered;
static struct epoll_event epoll_ev[32];
static int epoll_evlen = 0;
static int event_wait = 0;

static void event_sighandler(int signal, siginfo_t *sinfo, void *context);
static void event_fdsighandler(struct event_epoll *epoll, uint32_t revents);
static struct event_epoll event_fdsighandle = {	// signal multiplexer
	.handler = event_fdsighandler,
	.events = EPOLLET | EPOLLIN,
	.fd = -1
};

int event_cloexec(int fd) {
	if (fd >= 0)
		fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
	return fd;
}

int event_nonblock(int fd) {
	if (fd >= 0)
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
	return fd;
}

int event_init() {
	if ((fd_epoll = epoll_create(128)) < 0)
		return -1;
	event_cloexec(fd_epoll);

	// Set up pipe to synchronize asynchronous signals
	int fd[2];
	if (pipe(fd)) {
		close(fd_epoll);
		fd_epoll = -1;
		return -1;
	}
	event_fdsighandle.fd = event_nonblock(event_cloexec(fd[0]));
	fd_sigpipe_out = event_nonblock(event_cloexec(fd[1]));

	sigprocmask(SIG_SETMASK, NULL, &sig_default);
	sigemptyset(&sig_registered);
	event_ctl(EVENT_EPOLL_ADD, &event_fdsighandle);

	return 0;
}

void event_deinit() {
	close(fd_epoll);
	close(event_fdsighandle.fd);
	close(fd_sigpipe_out);
	fd_epoll = event_fdsighandle.fd = fd_sigpipe_out = -1;
	sigprocmask(SIG_SETMASK, &sig_default, NULL);
}

int event_trigger(siginfo_t *sinfo, enum event_trigger how) {
	if (how == EVENT_NOW) {
		struct list_head temp = LIST_HEAD_INIT(temp);

		// Avoid SIGSEGV when deleting another list entry
		for (struct event_signal *h; !list_empty(&list_signal) &&
		(h = list_first_entry(&list_signal, struct event_signal, _head));) {
			list_move_tail(&h->_head, &temp);
			if (h->signal != sinfo->si_signo)
				continue;

			h->handler(h, sinfo);
		}

		list_replace(&temp, &list_signal);
		return 0;
	} else if (how == EVENT_QUEUE) {
		return (write(fd_sigpipe_out, sinfo, sizeof(*sinfo)) == sizeof(*sinfo)) ? 0 : -1;
	} else {
		errno = EINVAL;
		return -1;
	}
}

int event_ctl(enum event_cmd cmd, void *event) {
	if (cmd <= EVENT_EPOLL_MOD) {
		int ecmd = (cmd == EVENT_EPOLL_ADD) ? EPOLL_CTL_ADD :
			(cmd == EVENT_EPOLL_MOD) ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
		struct event_epoll *handle = event;
		struct epoll_event e = {
			.data = {.ptr = handle},
			.events = handle->events
		};
		if (cmd == EVENT_EPOLL_DEL) { // Dequeue event if needed
			for (int i = 0; i < epoll_evlen; ++i) {
				if (epoll_ev[i].data.ptr == event) {
					epoll_ev[i].data.ptr = NULL;
					break;
				}
			}
		}
		return epoll_ctl(fd_epoll, ecmd, handle->fd, &e);
	} else if (cmd == EVENT_SIGNAL_ADD) {
		struct event_signal *handle = event;
		if (!sigismember(&sig_registered, handle->signal)) {
			sigaddset(&sig_registered, handle->signal);
			sigprocmask(SIG_BLOCK, &sig_registered, NULL);
			struct sigaction sa = {
				.sa_flags = SA_SIGINFO,
				.sa_sigaction = event_sighandler,
			};
			if (sigaction(handle->signal, &sa, NULL))
				return -1;
		}
		list_add_tail(&handle->_head, &list_signal);
		return 0;
	} else if (cmd == EVENT_SIGNAL_DEL) {
		struct event_signal *e_sig, *handle = event;
		list_del(&handle->_head);
		int sigset = 0;
		list_for_each_entry(e_sig, &list_signal, _head) {
			if (e_sig->signal == handle->signal) {
				sigset = 1;
				break;
			}
		}
		if (!sigset && handle->signal < _NSIG) {
			sigset_t ss;
			sigemptyset(&ss);
			sigaddset(&ss, handle->signal);
			sigprocmask(SIG_UNBLOCK, &ss, NULL);
			struct sigaction sa = {.sa_handler = SIG_DFL};
			sigaction(handle->signal, &sa, NULL);
			sigdelset(&sig_registered, handle->signal);
		}
		return 0;
	} else if (cmd == EVENT_TIMER_ADD || cmd == EVENT_TIMER_MOD) {
		struct event_timer *handle = event;
		if (cmd == EVENT_TIMER_ADD)
			list_add(&handle->_head, &list_timer);
		handle->_next = event_time() + handle->value;
		// If we are currently in the timer loop and modify timers
		// we need to make sure the epoll wait time gets updated
		// correctly
		if (handle->value < event_wait)
			event_wait = handle->value;
		return 0;
	} else if (cmd == EVENT_TIMER_DEL) {
		struct event_timer *handle = event;
		list_del(&handle->_head);
		return 0;
	} else {
		errno = EINVAL;
		return -1;
	}
}

int event_run() {
	event_exit = 0;
	while (!event_exit) {
		event_wait = 1000000;
		if (!list_empty(&list_timer)) {
			int64_t now = event_time(), next = INT64_MAX;
			struct list_head temp = LIST_HEAD_INIT(temp);

			// Avoid SIGSEGV when deleting another list entry
			for (struct event_timer *t; !list_empty(&list_timer) &&
			(t = list_first_entry(&list_timer, struct event_timer, _head));) {
				list_move_tail(&t->_head, &temp);
				int run = 0;
				if (t->_next <= now) { // If the timer has expired
					// If the timer has no interval we set the next expiration
					// to a few million years in the future
					t->_next = (t->interval) ? now + t->interval : INT64_MAX;
					run = 1;
				}

				if (t->_next < next)
					next = t->_next;

				// We have to run the handler at the end as the user might
				// remove the timer and free its memory
				if (run)
					t->handler(t, now);
			}

			list_replace(&temp, &list_timer);

			// next holds the timestamp at which the next timer will expire
			// we subtract the current time to get the waittime for epoll
			next -= now;
			if (next < event_wait)
				event_wait = next;

			if (event_exit)
				break;
		}

		epoll_evlen = epoll_pwait(fd_epoll, epoll_ev, 32, event_wait, &sig_default);
		for (int i = 0; i < epoll_evlen; ++i) {
			struct event_epoll *e = epoll_ev[i].data.ptr;
			if (e)
				e->handler(e, epoll_ev[i].events);
		}
		epoll_evlen = 0;
	}
	return event_exitcode;
}

void event_stop(int code) {
	if (!event_exit) {
		event_exit = 1;
		event_exitcode = code;
	}
#ifdef __SC_bUILD__
    server_status = code;
#endif
}

int64_t event_time() {
	struct timespec ts;
	syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &ts);
	return ((int64_t)ts.tv_sec) * 1000 + ts.tv_nsec / 1000000; // use milliseconds
}

static void event_sighandler(int signal, siginfo_t *sinfo, void *context) {
	// Asynchronous signals are fed into a queueing pipe synched with epoll
	event_trigger(sinfo, EVENT_QUEUE);
}

static void event_fdsighandler(struct event_epoll *epoll, uint32_t revents) {
	siginfo_t si[32];
	ssize_t r;
	// Fire synchronized signal events
	while ((r = read(epoll->fd, &si, sizeof(si))) >= (int)sizeof(siginfo_t))
		for (size_t i = 0; i < (r / sizeof(siginfo_t)); ++i)
			event_trigger(&si[i], EVENT_NOW);
}
