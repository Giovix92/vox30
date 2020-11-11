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
	IPT_APPEND,     //-A
	IPT_PREPEND,    //-I
	IPT_DELETE,     //-D
	IPT_FLUSHCHAIN, //-F
	IPT_NEWCHAIN,   //-N
	IPT_DELETECHAIN,//-X
	IPT_POLICY,     //-P
	IPT_TEST,       //-v
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
