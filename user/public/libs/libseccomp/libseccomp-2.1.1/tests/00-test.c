
/*
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of version 2.1 of the GNU Lesser General Public License as
 * published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses>.
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <seccomp.h>

#include "util.h"

int main(void)
{
	scmp_filter_ctx ctx;
	int status;

	ctx = seccomp_init(SCMP_ACT_KILL);
	if (ctx == NULL)
		return 1;

	status = seccomp_rule_add_exact(ctx, SCMP_ACT_ALLOW, SCMP_SYS(prctl), 0);
	if (status < 0)
		return 1;

	status = seccomp_rule_add_exact(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
	if (status < 0)
		return 1;

#if 1
	status = seccomp_load(ctx);
	if (status < 0)
		return 1;
#endif

	status = seccomp_reset(ctx, SCMP_ACT_ALLOW);
	if (status < 0)
		return 1;

#if 0
	status = seccomp_load(ctx);
	if (status < 0)
		return 1;
#endif

	write(2, "OK\n", 3);

	return 0;
}