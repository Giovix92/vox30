/*
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/version.h>


static int dsl_status_old = -1;
extern int dsl_status;
extern wait_queue_head_t dsl_status_queue;

MODULE_DESCRIPTION("dsl status forward module");
MODULE_LICENSE("GPL");

static int event_pooling(void)
{
    int len = 0;
    char message[256];
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
    daemonize("eforwarderd");
#endif
    while(1)
    {
        wait_event_interruptible(dsl_status_queue, dsl_status != dsl_status_old);
        if(dsl_status != dsl_status_old)
        {
            len = snprintf(message, sizeof(message), "dslStateInfo-%d", dsl_status);
            len++;
            (void)kobject_send_uevent(message, len);
            dsl_status_old = dsl_status;
        }
    }
    return 0;
}
static int __init event_forwarder_init(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
    kernel_thread(event_pooling, NULL, CLONE_KERNEL);
#else
    kthread_run(event_pooling, NULL, "kthread_event_forwarder");
#endif
    return 0;
}

static void __exit event_forwarder_exit(void)
{

}

module_init(event_forwarder_init);
module_exit(event_forwarder_exit);

