
#ifndef __PCPDC_H__
#define __PCPDC_H__

int pcp_refresh_port_mapping(unsigned short eport, int proto, unsigned int lifetime);
int pcp_add_port_mapping(char *source, unsigned short eport, unsigned short iport,
        const char *iaddr, int proto, unsigned int life, 
        const char *desc);
int pcp_delete_port_mapping(unsigned short eport, int proto);
int pcp_del_all_port_mapping(char *source);
int pcp_update_wan_status(int status, char *wan_ip, char *interface);
int pcp_reconfig(char *lan_ip);


#endif
