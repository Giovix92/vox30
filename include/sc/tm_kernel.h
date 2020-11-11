
#ifndef TM_KERNEL_H
#define TM_KERNEL_H

extern int (*sc_tm_inc_rx_hook)(struct net_device *dev, struct sk_buff *skb);
extern int (*sc_tm_inc_tx_hook)(struct net_device *dev, struct sk_buff *skb);
extern int (*sc_tm_check_limited_hook)(struct net_device *dev);

#endif
