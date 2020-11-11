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

#include <sys/syscall.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>

int insmod(const char *module, const char *opts)
{
	char *mprobe[] = {"/sbin/modprobe", (char*)module, (char*)opts, NULL};
	char *insmod[] = {"/sbin/insmod", (char*)module, (char*)opts, NULL};

	pid_t pid = vfork();
	if (!pid) {
		execv(mprobe[0], mprobe);
		execv(insmod[0], insmod);
		_exit(128);
	}

	int status;
	if (waitpid(pid, &status, 0) == pid
	&& WIFEXITED(status) && !WEXITSTATUS(status))
		goto out;

	struct utsname uts;
	uname(&uts);

	status = -1;
	if (strlen(module) > 128)
		goto out;

#if 0
	char test[256] = "/sys/module/";
	strcat(test, module);
	if (!access(test, X_OK))
		goto out;
#endif

	char buf[256] = "/lib/modules/";
#if 0
	strcat(buf, uts.release);
	strcat(buf, "/");
#endif
	strcat(buf, module);
	strcat(buf, ".ko");

	int fd = open(buf, O_RDONLY);
	if (fd < 0)
		goto out;

	struct stat s;
	fstat(fd, &s);

	void *data = malloc(s.st_size);
	if (data && read(fd, data, s.st_size) == s.st_size)
		status = syscall(__NR_init_module, data, s.st_size, opts);

	close(fd);
	free(data);
	syslog(LOG_WARNING, "insmod: loading kernel module %s: %s",
						module, strerror(errno));

out:
	return status;
}
