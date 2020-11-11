#ifndef EVENT_H_
#define EVENT_H_

#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <sys/epoll.h>

#include "list.h"


enum event_cmd {
	EVENT_EPOLL_ADD,	// Add an epoll handler (auto-removed on close())
	EVENT_EPOLL_DEL,	// Remove an epoll handler
	EVENT_EPOLL_MOD,	// Modify an epoll handler
	EVENT_SIGNAL_ADD,	// Add a signal handler (multiple per signal possible)
	EVENT_SIGNAL_DEL,	// Remove a signal handler
	EVENT_TIMER_ADD,	// Add a timer
	EVENT_TIMER_MOD,	// Modifies a timer
	EVENT_TIMER_DEL,	// Removes a timer
};

enum event_trigger {
	EVENT_NOW,			// Run event handlers immediately
	EVENT_QUEUE,		// Run event handlers on next iteration
};

/* event for EVENT_EPOLL_ADD, EVENT_EPOLL_DEL, EVENT_EPOLL_MOD */
struct event_epoll {
	/* mandatory input */
	int fd;
	uint32_t events;
	void (*handler)(struct event_epoll*, uint32_t revents);
	/* optional input */
	void *context;
};

/* event for EVENT_SIGNAL_ADD, EVENT_SIGNAL_DEL */
struct event_signal {
	/* mandatory input */
	void (*handler)(struct event_signal*, const siginfo_t *siginfo);
	int signal;
	/* optional input */
	void *context;
	/* internal */
	struct list_head _head;
};

/* event for EVENT_TIMER_ADD, EVENT_TIMER_MOD, EVENT_TIMER_DEL */
struct event_timer {
	/* mandatory input */
	unsigned value;		// msec: current timer value (one-shot / first trigger)
	unsigned interval;	// msec: following timer values (interval) (0 = off)
	void (*handler)(struct event_timer*, int64_t now);
	/* optional input */
	void *context;
	/* internal */
	int64_t _next;
	struct list_head _head;
};

// Initialise event system
int event_init();
// Deinitialise event system
void event_deinit();
// Register / unregister struct event_*
int event_ctl(enum event_cmd cmd, void *event);
// Trigger a custom signal / emulate a system signal
int event_trigger(siginfo_t *siginfo, enum event_trigger how);
// Main loop
int event_run();
// Stop the main loop after the current iteration and set return code
void event_stop(int code);
// Set close-on-exec flag on fd and return it
int event_cloexec(int fd);
// Set non-blocking flag on fd and return it
int event_nonblock(int fd);
// Get the current time of the monotonic (not realtime) clock in milliseconds
int64_t event_time();


#endif /* EVENT_H_ */
