#ifndef IPT_H_
#define IPT_H_

enum ipt_tbl {
	IPT_FILTER,
	IP6T_FILTER,
	IPT_NAT,
	IP6T_NAT,
	IPT_MANGLE,
	IP6T_MANGLE,
};

enum ipt_cmd {
	IPT_APPEND,
	IPT_PREPEND,
	IPT_DELETE,
	IPT_FLUSHCHAIN,
	IPT_NEWCHAIN,
	IPT_DELETECHAIN,
	IPT_POLICY,
	IPT_TEST,
};

void ipt_path(const char *iptables, const char *ip6tables);
int ipt(enum ipt_tbl tbl, enum ipt_cmd cmd, const char *chain, ...);
#define ipt_filter(cmd, ...) ipt(IPT_FILTER, cmd, __VA_ARGS__, NULL)
#define ip6t_filter(cmd, ...) ipt(IP6T_FILTER, cmd, __VA_ARGS__, NULL)
#define ipt_nat(cmd, ...) ipt(IPT_NAT, cmd, __VA_ARGS__, NULL)
#define ip6t_nat(cmd, ...) ipt(IP6T_NAT, cmd, __VA_ARGS__, NULL)
#define ipt_mangle(cmd, ...) ipt(IPT_MANGLE, cmd, __VA_ARGS__, NULL)
#define ip6t_mangle(cmd, ...) ipt(IP6T_MANGLE, cmd, __VA_ARGS__, NULL)

//#define IPT_DEBUG

#endif /* IPT_H_ */
