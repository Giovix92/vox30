#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/in.h>
#include <linux/init.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#include <asm/system.h>
#endif
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <net/sock.h>
#include <net/checksum.h>
#include <linux/if_ether.h>	/* For the statistics structure. */
#include <linux/if_arp.h>	/* For ARPHRD_ETHER */
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/percpu.h>
#include <net/net_namespace.h>
MODULE_LICENSE("GPL");
struct net_device *nulldev;
/*
 * The higher levels take care of making this non-reentrant (it's
 * called with bh's disabled).
 */
static int nulldev_xmit(struct sk_buff *skb, struct net_device *dev)
{
    dev->stats.tx_dropped++;
    dev_kfree_skb(skb);
    return 0;
}

static void nulldev_dev_free(struct net_device *dev)
{
    free_netdev(dev);
}

static const struct net_device_ops nulldev_ops = {
    .ndo_start_xmit= nulldev_xmit,
};

/*
 * The nulldev device is special. There is only one instance
 * per network namespace.
 */
static void nulldev_setup(struct net_device *dev)
{
    dev->mtu		= (16 * 1024) + 20 + 20 + 12;
    dev->hard_header_len	= ETH_HLEN;	/* 14	*/
    dev->addr_len		= ETH_ALEN;	/* 6	*/
    dev->tx_queue_len	= 0;
    dev->type		= ARPHRD_ETHER;
    dev->flags		|= IFF_NOARP;
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,3,0)
    dev->features 	|= NETIF_F_HW_CSUM
#else
    dev->features 	|= NETIF_F_NO_CSUM
#endif
        | NETIF_F_HIGHDMA
        | NETIF_F_NETNS_LOCAL;
    dev->netdev_ops		= &nulldev_ops;
    dev->destructor		= nulldev_dev_free;
    random_ether_addr(dev->dev_addr);
}

/* Setup and register the nulldev device. */
static int  __init nulldev_net_init(void)
{
    int err;

    err = -ENOMEM;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
    nulldev = alloc_netdev(0, "null", nulldev_setup);
#else
    nulldev = alloc_netdev(sizeof(struct net_device),  "null", 0,  nulldev_setup);
#endif
    if (!nulldev)
        goto out;

    err = register_netdev(nulldev);
    if (err)
        goto out_free_netdev;

    return 0;


out_free_netdev:
    free_netdev(nulldev);
out:
    return err;
}

static void nulldev_net_exit(void)
{
    if(nulldev)
        unregister_netdev(nulldev);
}

module_init(nulldev_net_init);
module_exit(nulldev_net_exit);
