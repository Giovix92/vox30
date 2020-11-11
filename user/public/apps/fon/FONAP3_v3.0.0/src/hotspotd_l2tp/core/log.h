#ifndef __LOG_H__
#define __LOG_H__
#include <log/slog.h>

#ifdef syslog
#undef syslog
#endif
#define syslog(pri, format, arg...) log_fon(LOG_DEBUG, NORM_LOG, 0, 0, format, ##arg);
#endif
