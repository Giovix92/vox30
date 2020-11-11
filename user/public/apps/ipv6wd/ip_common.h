#define RC          "/usr/sbin/rc"
#define WAN_MAX_NUM		8
int handle_addrinfo(const struct sockaddr_nl *who,
			  struct nlmsghdr *n,
			  void *arg);
int handle_route(const struct sockaddr_nl *who,
        struct nlmsghdr *n, void *arg);

int start_ipv6wd(unsigned groups, int scan_interval);
extern struct rtnl_handle rth;

#ifndef	INFINITY_LIFE_TIME
#define     INFINITY_LIFE_TIME      0xFFFFFFFFU
#endif
