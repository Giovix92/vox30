/* dhcpc.h */
#ifndef _DHCPC_H
#define _DHCPC_H


#define INIT_SELECTING	0
#define REQUESTING	1
#define BOUND		2
#define RENEWING	3
#define REBINDING	4
#define INIT_REBOOT	5
#define RENEW_REQUESTED 6
#define RELEASED	7


struct client_config_t {
	char foreground;		/* Do not fork */
	char quit_after_lease;		/* Quit after obtaining lease */
	char abort_if_no_lease;		/* Abort if no lease */
	char background_if_no_lease;	/* Fork to background if no lease */
	char *interface;		/* The name of the interface to use */
	char *pidfile;			/* Optionally store the process ID */
	char *script;			/* User script to run at dhcp events */
	unsigned char *clientid;	/* Optional client id to use */
#ifdef __SC_BUILD__
	unsigned char *vendorid;	/* Optional vendor id to use */
#ifdef CONFIG_SUPPORT_TR111
	unsigned char *vendorinfo;
#endif
#endif
	unsigned char *hostname;	/* Optional hostname to use */
#ifdef __SC_BUILD__
	unsigned char *server;      /*for fix l2tp bug*/
#endif
	int ifindex;			/* Index number of the interface to use */
	unsigned char arp[6];		/* Our arp address */
#ifdef __SC_BUILD__
	char *user;
        char option2_source_ntp;
        char option42_source_ntp;
	int wanid;
	unsigned char *opt57_maxlen;
    int support_auth;
    unsigned char *auth_key; //? support option 90
    unsigned char key_id[5];
    u_int64_t prevrd;
    unsigned int mark;
    unsigned int sc_mark;
#endif
};

extern struct client_config_t client_config;


#endif
