#ifndef _SAL_DHCP_H_
#define _SAL_DHCP_H_

#define MAX_HOSTNAME_LEN 40

typedef struct  
{
	unsigned char chaddr[16];       /* mac */
	unsigned int yiaddr;	        /* network order */
	unsigned int expires;	        /* host order */
	unsigned char hostname[MAX_HOSTNAME_LEN];
}dhcp_lease;

int sal_dhcp_get_lease_info(dhcp_lease **lease_info);
int sal_dhcp_get_lease_info_gui(dhcp_lease **lease_info);


#endif
