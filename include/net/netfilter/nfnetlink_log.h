#ifndef _KER_NFNETLINK_LOG_H
#define _KER_NFNETLINK_LOG_H
#ifdef __SC_BUILD__
void
nfulnl_log_packet(struct net *net,
	          u_int8_t pf,
		  unsigned int hooknum,
		  const struct sk_buff *skb,
		  const struct net_device *in,
		  const struct net_device *out,
		  const struct nf_loginfo *li_user,
		  const char *prefix,
          const char *suffix);
#else
void
nfulnl_log_packet(struct net *net,
		  u_int8_t pf,
		  unsigned int hooknum,
		  const struct sk_buff *skb,
		  const struct net_device *in,
		  const struct net_device *out,
		  const struct nf_loginfo *li_user,
		  const char *prefix);
#endif
#define NFULNL_COPY_DISABLED    0xff

#endif /* _KER_NFNETLINK_LOG_H */

