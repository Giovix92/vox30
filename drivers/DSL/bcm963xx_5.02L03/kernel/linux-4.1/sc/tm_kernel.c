
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODEVERSIONS
#endif
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>

int (*sc_tm_inc_rx_hook)(struct net_device *dev, struct sk_buff *skb) = NULL;
int (*sc_tm_inc_tx_hook)(struct net_device *dev, struct sk_buff *skb) = NULL;
int (*sc_tm_check_limited_hook)(struct net_device *dev) = NULL;
EXPORT_SYMBOL(sc_tm_inc_rx_hook);
EXPORT_SYMBOL(sc_tm_inc_tx_hook);
EXPORT_SYMBOL(sc_tm_check_limited_hook);
