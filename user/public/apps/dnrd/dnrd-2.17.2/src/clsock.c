#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <log/slog.h>
#include "domnode.h"
#include "common.h"
#include "lib.h"
#include "dns.h"
#include "clsock.h"
#include "query.h"
#include "master.h"
#include "cache.h"
/* Only for debug */
#define printf mbug
#include <stdarg.h>

void mbug(char *format,...){
    va_list args;
    FILE *fp;
    
    if(opt_debug == 0)
        return 0;
    
    fp = fopen("/dev/console","a");
    if(!fp){
        return;
    }
    va_start(args,format);
    vfprintf(fp,format,args);
    va_end(args);
    fprintf(fp,"\n");
    fflush(fp);
    fclose(fp);
}



#define MAX_MSG_LEN         (512)
#define MAX_DOMAIN_LEN      (200) 

typedef struct
{
	uint32_t action;
	uint32_t act_type;
	uint32_t length;
	char param[0];
}c_data_t;

enum {
    ACT_ADD_DOMAIN,
    ACT_DEL_DOMAIN,
    ACT_MOD_DOMAIN,
	ACT_DIS_DOMAIN,
#if (defined(__SC_BUILD__) && (defined(CONFIG_SUPPORT_WEB_PRIVOXY)))
#ifdef CONFIG_SUPPORT_FON
    ACT_ADD_FILTER_IP,
    ACT_DEL_FILTER_IP,
    ACT_RES_FILTER_IP,
#endif
    ACT_ADD_PROVI,
    ACT_ADD_SUBNET,
#endif
#ifdef __SC_BUILD__
    ACT_SET_OPTION15,
#endif
#ifdef CONFIG_SUPPORT_FON
    ACT_ADD_WHITELIST,
    ACT_DEL_WHITELIST,
    ACT_ENABLE_FON_REDIRECT,
#endif
	ACT_DIS_QLIST,
    ACT_ADD_NAMEIP,
    ACT_DEL_NAMEIP,
    ACT_ADD_NAMEIP_X,
    ACT_DEL_NAMEIP_X,
    ACT_ADD_NAMEIP_HOST,
    ACT_DEL_NAMEIP_HOST,
#ifdef CONFIG_SUPPORT_WEB_URL_FILTER
    ACT_ADD_FILTER_DOMAIN,
    ACT_DEL_FILTER_DOMAIN,
#endif
    ACT_ADD_NAMESRV,
    ACT_DEL_NAMESRV
#ifdef CONFIG_SUPPORT_REDIRECT
        ,
    ACT_ENABLE_REDIRECT,
    ACT_DISABLE_REDIRECT
#endif
};

enum {
	ACT_TYPE_NULL = 0,
	ACT_TYPE_DEL_ALL, /* Delete all domain in domain list(Empty) */
	ACT_TYPE_DEL_DOMAIN, /* Delete specific domain */
	ACT_TYPE_DEL_DOMAIN_WANID, /* Delete specific domain+wanid*/
	ACT_TYPE_DEL_WANID, /*Delete spcific wanid*/
    ACT_TYPE_DEL_DOMAIN_TYPE
};

static uint32_t switch_dstr_to_dtype(const char *dstr)
{
    if(strcasecmp(dstr, "data") == 0) {
        return DOMAIN_TYPE_DATA;
    } else if(strcasecmp(dstr, "tr069") == 0) {
        return DOMAIN_TYPE_TR069;
    } else if(strcasecmp(dstr, "ntp") == 0) {
        return DOMAIN_TYPE_NTP;
    } else if(strcasecmp(dstr, "iptv") == 0) {
        return DOMAIN_TYPE_IPTV;
    } else if(strcasecmp(dstr, "voip") == 0) {
        return DOMAIN_TYPE_VOIP;
    } else if(strcasecmp(dstr, "cli") == 0) {
        return DOMAIN_TYPE_CLI;
    } else if(strcasecmp(dstr, "ipphone") == 0) {
        return DOMAIN_TYPE_IPPHONE;
    } else {
        return DOMAIN_TYPE_UNKNOWN;
    }
}

static char *switch_dtype_to_str(uint32_t dtype)
{
    static char dstr[16];
    
    memset(dstr, 0, sizeof(dstr));

    switch(dtype)
    {
    case DOMAIN_TYPE_DATA: 
        strcpy(dstr, "data");
        break;
    case DOMAIN_TYPE_TR069:
        strcpy(dstr, "tr069");
        break;
    case DOMAIN_TYPE_NTP:  
        strcpy(dstr, "ntp");
        break;
    case DOMAIN_TYPE_IPTV: 
        strcpy(dstr, "iptv");
        break;
    case DOMAIN_TYPE_VOIP: 
        strcpy(dstr, "voip");
        break;
    case DOMAIN_TYPE_CLI: 
        strcpy(dstr, "cli");
        break;
    case DOMAIN_TYPE_IPPHONE: 
        strcpy(dstr, "ipphone");
        break;
    default:
        strcpy(dstr, "unknown");
    }

    return dstr;
}

static int del_srvnode_by_wan_id(srvnode_t *srv_head, int32_t wan_id)
{
	srvnode_t *cur_srv = NULL, *pre_srv = NULL;
	
	if(srv_head == NULL)
	{
		printf("srv_head == NULL.\n");
		return 0;
	}

	pre_srv = srv_head;
	cur_srv = srv_head->next;
	while(cur_srv != srv_head) {
		if(cur_srv->wan_id == wan_id) {
			destroy_srvnode(del_srvnode_after(pre_srv));
			cur_srv = pre_srv->next;
			continue;
		}
		pre_srv = pre_srv->next;
		cur_srv = cur_srv->next;
	}
	return 1;
}

static int add_domain_to_dlist(char *add_info, int info_len)
{
    c_data_t *pdata = NULL;
    char *domain = NULL,*type = NULL, *s = NULL, *sep = NULL;
    char *domain_temp = NULL;
    domnode_t *p = NULL;
    srvnode_t *srv = NULL;

	pdata = (c_data_t *)add_info;

	if(pdata->length == 0)
	{
		log_msg(LOG_ERR, "%s:%d - Nothing to be add.", 
				__FUNCTION__, __LINE__);
		return 0;
	}

    if(pdata->length + sizeof(c_data_t) != info_len)
    {
        log_msg(LOG_ERR, "%s:%d - Parse length error. info_len = %d, param_len = %d", 
                __FUNCTION__, __LINE__, info_len, pdata->length);
        return 0;
    }

    if((domain = (char *)malloc(pdata->length + 1)) == NULL)
    {
        log_msg(LOG_ERR, "%s:%d - malloc fail.", __FUNCTION__, __LINE__);
        return 0;
    }
    domain_temp = domain;

	memcpy(domain, pdata->param, pdata->length);
	domain[pdata->length] = '\0';
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
	sep = strchr(domain, (int)'*');
#else
	sep = strchr(domain, (int)':');
#endif
	if (sep) {
		char *domain = NULL;
		
		*sep = '\0';
		domain = sep + 1;
        if((sep = strchr(domain, (int)'#')) != NULL)
        {
            *sep = '\0';
            type = domain;
            domain = sep + 1;
        }
		
        /* add domain to list */
        s = make_cname((char *)strnlwr(domain, MAX_DOMAIN_LEN), MAX_DOMAIN_LEN);
        /* need to check return value, add by miracle, 2011-05-11 */
        if (NULL == s) {
            domain = domain_temp;
            free(domain);
            return 0;
        }
        if(type)
        {   
            if((p = search_domnode_by_dnametype(domain_list, s, atoi(type))) == NULL)
            {
                p = add_domain(domain_list, load_balance, s, MAX_DOMAIN_LEN);
                p->domain_type = atoi(type);
            }
        }    
        else
        {
            if ((p = search_domnode(domain_list, s)) == NULL) {
			    p = add_domain(domain_list, load_balance, s, MAX_DOMAIN_LEN);
                p->domain_type = DOMAIN_TYPE_UNKNOWN;
		    }
        }
	} else
		p = domain_list;
    
	if ((srv = add_srv_by_pri(p->srvlist, domain)) == NULL) {
			log_debug(1, "Add server to domain failed");
	} else {
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
		srv->addr.sin6_family = AF_INET6;
		srv->addr.sin6_port = htons(53);
#else
		srv->addr.sin_family = AF_INET;
		srv->addr.sin_port = htons(53);
#endif
	}	
    p->current = p->srvlist->next; //pointer to highest priority server
  
#if 0
    printf("\tdns = %s(%d)\n", inet_ntoa(p->current->addr.sin_addr), srv->srv_pri);
    printf("\tinf_addr = %s\n", inet_ntoa(p->current->inf_addr.sin_addr));
    printf("\tinf_name = %s(%d)\n", p->current->inf, p->current->inf_pri);
    printf("\twan_id = %d\n", p->current->wan_id);
    printf("\ttime_out = %d\n", p->current->time_out);
#endif
    domain = domain_temp;
    free(domain);

    return 1;
}


static int del_domain_from_dlist(char *del_info, int info_len)
{
	c_data_t *pdata = NULL;
	char *domain = NULL, *cname = NULL, *pwanid = NULL, *domain_type = NULL;
	int32_t wan_id = -1;
    uint32_t dtype = 0;

	pdata = (c_data_t *)del_info;

	if(pdata->length + sizeof(c_data_t) != info_len)
	{
		log_msg(LOG_ERR, "%s:%d - Parse length error. info_len = %d, param_len = %d", 
				__FUNCTION__, __LINE__, info_len, pdata->length);
		return 0;
	}

	if(pdata->length == 0)
	{
		domnode_t *p = NULL;
        p = empty_domlist(domain_list);
		clear_srvlist(p->srvlist);
	}
	else
	{
		switch(pdata->act_type)
		{
			case ACT_TYPE_DEL_DOMAIN:
				if((domain = (char *)malloc(pdata->length + 1)) == NULL)
				{
					log_msg(LOG_ERR, "%s:%d - malloc fail.", __FUNCTION__, __LINE__);
					return 0;
				}
				memcpy(domain, pdata->param, pdata->length);
				domain[pdata->length] = '\0';
				
                if((cname = make_cname(strnlwr(domain, MAX_DOMAIN_LEN), MAX_DOMAIN_LEN)) == NULL)
				{
					log_msg(LOG_ERR, "%s:%d - make_cname fail.", __FUNCTION__, __LINE__);
					free(domain);
					return 0;
				}

				del_domain(domain_list, cname, MAX_DOMAIN_LEN);
                cache_expire_by_domain(domain);
				free(domain);
				free(cname);
				break;
			case ACT_TYPE_DEL_DOMAIN_WANID:
				if((domain = (char *)malloc(pdata->length + 1)) == NULL)
				{
					log_msg(LOG_ERR, "%s:%d - malloc fail.", __FUNCTION__, __LINE__);
					return 0;
				}
				memcpy(domain, pdata->param, pdata->length);
				domain[pdata->length] = '\0';

				if((pwanid = strchr(domain, (int)'#')))
				{
					*pwanid = '\0';
					pwanid++;
					wan_id = atoi(pwanid);
				}
				
				if((cname = make_cname(strnlwr(domain, MAX_DOMAIN_LEN), MAX_DOMAIN_LEN)) == NULL)
				{
					log_msg(LOG_ERR, "%s:%d - make_cname fail.", __FUNCTION__, __LINE__);
					free(domain);
					return 0;
				}
				/* Base on domain name delete matched wanid server */
				{
					domnode_t *p = domain_list;
					srvnode_t *srv_head = NULL;

					while(p->next != domain_list) {
						if(strncmp(p->next->domain, cname, MAX_DOMAIN_LEN) == 0) {
							srv_head = p->next->srvlist;
							break;
						}
						p = p->next;
					}
                    query_delete_by_domain_wan_id(p->next, wan_id);
					del_srvnode_by_wan_id(srv_head, wan_id);
				}
				free(cname);
				free(domain);
				break;
			case ACT_TYPE_DEL_WANID:
				if((pwanid = (char *)malloc(pdata->length + 1)) == NULL)
				{
					log_msg(LOG_ERR, "%s:%d - malloc fail.", __FUNCTION__, __LINE__);
					return 0;
				}
				memcpy(pwanid, pdata->param, pdata->length);
				pwanid[pdata->length] = '\0';
				wan_id = atoi(pwanid);
#ifdef __SC_BUILD__
                free(pwanid);
#endif
				/* Base on wanid delete all the matched item */
				{
                    query_delete_by_wan_id(wan_id);
					domnode_t *p = domain_list->next;	
					while(p != domain_list) {
						del_srvnode_by_wan_id(p->srvlist, wan_id);
						p = p->next;
					}
					del_srvnode_by_wan_id(p->srvlist, wan_id);
					
					p = domain_list;
					while(p->next != domain_list) {
						if(p->next->srvlist == p->next->srvlist->next) {
							domnode_t *del_node = p->next;
							p->next = (p->next)->next;
							destroy_domnode(del_node);
							continue;
						}
						p = p->next;
					}
				}
                break;
            case ACT_TYPE_DEL_DOMAIN_TYPE:
            {
                domnode_t *p = domain_list;
                if((domain_type = (char *)malloc(pdata->length + 1)) == NULL)
				{
					log_msg(LOG_ERR, "%s:%d - malloc fail.", __FUNCTION__, __LINE__);
					return 0;
				}
				memcpy(domain_type, pdata->param, pdata->length);
				domain_type[pdata->length] = '\0';
                dtype = switch_dstr_to_dtype(domain_type);
                free(domain_type);
                 
                    
                while(p->next != domain_list)
                {
                    if(p->next->domain_type == dtype)
                    {
						domnode_t *del_node = p->next;
                        p->next = (p->next)->next;
                        destroy_domnode(del_node);
                        continue;
                    }
                    p = p->next;
                }
                if(p->next == domain_list && p->next->domain_type == dtype)
                {
                    query_delete_by_domain(domain_list);
                    clear_srvlist(domain_list->srvlist);
                }              
                break;
            }
            default:
				printf("Mis-Matched Delete Action.\n");
		}
	}

	return 1;
}


static int dis_domain_in_dlist(char *dis_info, int info_len)
{
	c_data_t *pdata = NULL;
	domnode_t *p = domain_list;	
	int i = 0;
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    char buf[64];// used for ipv6
#endif
	pdata = (c_data_t *)dis_info;
	if(pdata->length + sizeof(c_data_t) != info_len)
	{
		printf("%s:%d - Parse length error. info_len = %d, param_len = %d", 
				__FUNCTION__, __LINE__, info_len, pdata->length);
		return 0;
	}
	
    do{
        if(p->domain != NULL) {
			printf("%s\n", cname2asc(p->domain));
            printf("type = %s\n", switch_dtype_to_str(p->domain_type));
		} else {
			printf("domain(%d) == NULL\n", i);
            printf("type = %s\n", switch_dtype_to_str(p->domain_type));
		}
		
		if(p->srvlist != NULL) {	
			srvnode_t *srv = p->srvlist;
			srv = srv->next;
			while(srv != p->srvlist) {
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
				printf("\tdns = %s(%d)\n", inet_ntop(AF_INET6,&srv->addr.sin6_addr,buf,sizeof(buf)), srv->srv_pri);
				printf("\tinf_addr = %s\n", inet_ntop(AF_INET6,&srv->inf_addr.sin6_addr,buf,sizeof(buf)));
#else
				printf("\tdns = %s(%d)\n", inet_ntoa(srv->addr.sin_addr), srv->srv_pri);
				printf("\tinf_addr = %s\n", inet_ntoa(srv->inf_addr.sin_addr));
#endif
				printf("\tinf_name = %s(%d)\n", srv->inf, srv->inf_pri);
				printf("\twan_id = %d\n", srv->wan_id);
				printf("\ttime_out = %d\n", srv->time_out);
				srv = srv->next;
			}
		}
		p = p->next;
		i++;
	}while(p != domain_list);

	return 1;
}

int dis_query_list(char *dis_info, int info_len, int flag)
{
	c_data_t *pdata = NULL;
	query_t *q = NULL;	
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    char buf[64];
#endif
    if(flag == 1) 
    {
        pdata = (c_data_t *)dis_info;
        if(pdata->length + sizeof(c_data_t) != info_len)
        {
            printf("%s:%d - Parse length error. info_len = %d, param_len = %d", 
                   __FUNCTION__, __LINE__, info_len, pdata->length);
            return 0;
        }
    }
    
    for(q = &qlist; q->next != &qlist; q = q->next)
    {
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
        printf("Server = %s\n", inet_ntop(AF_INET6,&q->next->srv->addr.sin6_addr,buf,sizeof(buf)));
        printf("client = %s\n", inet_ntop(AF_INET6,&q->next->client.sin6_addr,buf,sizeof(buf)));
#else
        printf("Server = %s\n", inet_ntoa(q->next->srv->addr.sin_addr));
        printf("client = %s\n", inet_ntoa(q->next->client.sin_addr));
#endif
        printf("client_qid = %x\n", q->next->client_qid);
        printf("my_qid = %x\n", q->next->my_qid);
    }    

	return 1;
}

int add_nameip_to_dlist(char * add_info,int info_len)
{
	c_data_t *pdata = NULL;
    char * domain;
    char * tmp;
    char * sep = NULL;
    char * name = NULL;
    char * ip = NULL;
    pdata = (c_data_t *)add_info;
    if(pdata->length + sizeof(c_data_t) != info_len)
    {
        printf("%s:%d - Parse length error. info_len = %d, param_len = %d", 
                __FUNCTION__, __LINE__, info_len, pdata->length);
        return 0;
    }

	if((domain = (char *)malloc(pdata->length + 1)) == NULL)
	{
		log_msg(LOG_ERR, "%s:%d - malloc fail.", __FUNCTION__, __LINE__);
		return 0;
	}
    tmp = domain;
	memcpy(domain, pdata->param, pdata->length);
	domain[pdata->length] = '\0';
	
	sep = strrchr(domain, (int)'-');
	if (sep) 
    {
		*sep = '\0';
        name =domain;
		domain = sep + 1;
    }
    if((sep = strchr(domain, (int)'#')) != NULL)
    {
        *sep = '\0';
        ip = domain;
        domain = sep + 1;
    }
    add_nameip(name,strlen(name)+1,ip);
    free(tmp);
    return 1;
}

#if (defined(__SC_BUILD__) && (defined(CONFIG_SUPPORT_FON)))
int add_whitelist(char *add_info, int info_len)
{
	c_data_t *pdata = NULL;
    char * domain;
    char * tmp;
    char * name = NULL;
    pdata = (c_data_t *)add_info;
    if(pdata->length + sizeof(c_data_t) != info_len)
    {
        printf("%s:%d - Parse length error. info_len = %d, param_len = %d", 
                __FUNCTION__, __LINE__, info_len, pdata->length);
        return 0;
    }

	if((domain = (char *)malloc(pdata->length + 1)) == NULL)
	{
		log_msg(LOG_ERR, "%s:%d - malloc fail.", __FUNCTION__, __LINE__);
		return 0;
	}
    tmp = domain;
	memcpy(domain, pdata->param, pdata->length);
	domain[pdata->length] = '\0';
	
    add_white_list(domain);
    free(tmp);
    return 1;
}
#endif
#if (defined(__SC_BUILD__) && (defined(CONFIG_SUPPORT_WEB_PRIVOXY)))
int add_provision(char *value, int length)
{
    memset(provisioncode, 0, sizeof(provisioncode));
    memcpy(provisioncode, value, length);
    provisioncode[length] = '\0';
    return 1;
}

#ifdef CONFIG_SUPPORT_FON
int add_filter_ip_to_dlist(char * add_info,int info_len)
{
	c_data_t *pdata = NULL;
    pdata = (c_data_t *)add_info;
    
    if(pdata->length + sizeof(c_data_t) != info_len)
    {
        printf("%s:%d - Parse length error. info_len = %d, param_len = %d", 
                __FUNCTION__, __LINE__, info_len, pdata->length);
        return 0;
    }
	
    add_filter_ip(pdata->param);
    return 1;
}

int del_filter_ip_from_dlist(char * add_info,int info_len)
{
	c_data_t *pdata = NULL;
    pdata = (c_data_t *)add_info;
    
    if(pdata->length + sizeof(c_data_t) != info_len)
    {
        printf("%s:%d - Parse length error. info_len = %d, param_len = %d", 
                __FUNCTION__, __LINE__, info_len, pdata->length);
        return 0;
    }
	
    del_filter_ip(pdata->param);
    return 1;
}
#endif
int add_subnet(char * add_info,int info_len)
{
    c_data_t *pdata = NULL;
    char * sep = NULL;
    char * msubnet = NULL;
    char * mask = NULL;
#ifdef CONFIG_SUPPORT_FON
    char * fsubnet = NULL;
    char * fmask = NULL;
    char * ofsubnet = NULL;
    char * ofmask = NULL;
#endif
    char * subnet = NULL;
    
    pdata = (c_data_t *)add_info;
    if(pdata->length + sizeof(c_data_t) != info_len)
    {
        return 0;
    }
    
    memset(mainsubnet, 0, sizeof(mainsubnet));
    memset(mainmask, 0, sizeof(mainmask));
#ifdef CONFIG_SUPPORT_FON
    memset(fonsubnet, 0, sizeof(fonsubnet));
    memset(fonmask, 0, sizeof(fonmask));
    memset(ofonsubnet, 0, sizeof(ofonsubnet));
    memset(ofonmask, 0, sizeof(ofonmask));
#endif
    
    subnet = pdata->param;  
    sep = strchr(subnet, (int)'-');
    if (sep) 
    {
        *sep = '\0';
        msubnet = subnet;
        subnet = sep + 1;
    }
    if((sep = strchr(subnet, (int)'-')) != NULL)
    {
        *sep = '\0';
        mask = subnet;
        subnet = sep + 1;
    }
#ifdef CONFIG_SUPPORT_FON
    if((sep = strchr(subnet, (int)'-')) != NULL)
    {
        *sep = '\0';
        fsubnet = subnet;
        subnet = sep + 1;
    }
    if((sep = strchr(subnet, (int)'-')) != NULL)
    {
        *sep = '\0';
        fmask = subnet;
        subnet = sep + 1;
    }
    if((sep = strchr(subnet, (int)'-')) != NULL)
    {
        *sep = '\0';
        ofsubnet = subnet;
        subnet = sep + 1;
    }
    if((sep = strchr(subnet, (int)'-')) != NULL)
    {
        *sep = '\0';
        ofmask = subnet;
        subnet = sep + 1;
    }
#endif
    
    snprintf(mainsubnet,sizeof(mainsubnet),"%s",msubnet);
    snprintf(mainmask,sizeof(mainmask),"%s",mask);
#ifdef CONFIG_SUPPORT_FON
    snprintf(fonsubnet,sizeof(fonsubnet),"%s",fsubnet);
    snprintf(fonmask,sizeof(fonmask),"%s",fmask);
    snprintf(ofonsubnet,sizeof(ofonsubnet),"%s",ofsubnet);
    snprintf(ofonmask,sizeof(ofonmask),"%s",ofmask);
#endif
    return 1;
}
#endif
#ifdef CONFIG_SUPPORT_WEB_URL_FILTER
int del_filter_domain_from_dlist(char * add_info,int info_len)
{
	c_data_t *pdata = NULL;
    pdata = (c_data_t *)add_info;
    
    if(pdata->length + sizeof(c_data_t) != info_len)
    {
        printf("%s:%d - Parse length error. info_len = %d, param_len = %d", 
                __FUNCTION__, __LINE__, info_len, pdata->length);
        return 0;
    }
	
    del_domain_mac_list();
    return 1;
}

int add_filter_domain_to_dlist(char * add_info,int info_len)
{
	c_data_t *pdata = NULL;
    char * info;
    char * tmp;
    char * sep = NULL;
    char * name = NULL;
    char * mac = NULL;
    pdata = (c_data_t *)add_info;
    if(pdata->length + sizeof(c_data_t) != info_len)
    {
        printf("%s:%d - Parse length error. info_len = %d, param_len = %d", 
                __FUNCTION__, __LINE__, info_len, pdata->length);
        return 0;
    }

	if((info = (char *)malloc(pdata->length + 1)) == NULL)
	{
		log_msg(LOG_ERR, "%s:%d - malloc fail.", __FUNCTION__, __LINE__);
		return 0;
	}
    tmp = info;
	memcpy(info, pdata->param, pdata->length);
	info[pdata->length] = '\0';
	
	sep = strrchr(info, (int)'-');
	if (sep) 
    {
		*sep = '\0';
        name =info;
		info = sep + 1;
    }
    if((sep = strchr(info, (int)'#')) != NULL)
    {
        *sep = '\0';
        mac = info;
        info = sep + 1;
    }
    add_domain_mac_list(name,mac);
    free(tmp);
    return 1;
}
#endif
#ifdef __SC_BUILD__
int set_option15(char *op_str, int info_len)
{
	c_data_t *pdata = NULL;
    char buf[256] = "";
    pdata = (c_data_t *)op_str;
    if(pdata->length + sizeof(c_data_t) != info_len)
        return 0;

    snprintf(buf, info_len, "%s", pdata->param);
    update_option15(buf);
    return 1;
}
#endif
int add_nameip_host_to_dlist(char * add_info,int info_len)
{
	c_data_t *pdata = NULL;
    char * domain;
    char * tmp;
    char * sep = NULL;
    char * name = NULL;
    char * ip = NULL;
    pdata = (c_data_t *)add_info;
    if(pdata->length + sizeof(c_data_t) != info_len)
    {
        printf("%s:%d - Parse length error. info_len = %d, param_len = %d", 
                __FUNCTION__, __LINE__, info_len, pdata->length);
        return 0;
    }

	if((domain = (char *)malloc(pdata->length + 1)) == NULL)
	{
		log_msg(LOG_ERR, "%s:%d - malloc fail.", __FUNCTION__, __LINE__);
		return 0;
	}
    tmp = domain;
	memcpy(domain, pdata->param, pdata->length);
	domain[pdata->length] = '\0';
	
	sep = strrchr(domain, (int)'-');
	if (sep) 
    {
		*sep = '\0';
        name =domain;
		domain = sep + 1;
    }
    if((sep = strchr(domain, (int)'#')) != NULL)
    {
        *sep = '\0';
        ip = domain;
        domain = sep + 1;
    }
    add_nameip_host(name,strlen(name)+1,ip);
    free(tmp);
    return 1;
}
int add_nameip_x_to_dlist(char * add_info,int info_len)
{
	c_data_t *pdata = NULL;
    char * domain;
    char * tmp;
    char * sep = NULL;
    char * name = NULL;
    char * ip = NULL;
    pdata = (c_data_t *)add_info;
    if(pdata->length + sizeof(c_data_t) != info_len)
    {
        printf("%s:%d - Parse length error. info_len = %d, param_len = %d", 
                __FUNCTION__, __LINE__, info_len, pdata->length);
        return 0;
    }

	if((domain = (char *)malloc(pdata->length + 1)) == NULL)
	{
		log_msg(LOG_ERR, "%s:%d - malloc fail.", __FUNCTION__, __LINE__);
		return 0;
	}
    tmp = domain;
	memcpy(domain, pdata->param, pdata->length);
	domain[pdata->length] = '\0';
	
	sep = strrchr(domain, (int)'-');
	if (sep) 
    {
		*sep = '\0';
        name =domain;
		domain = sep + 1;
    }
    if((sep = strchr(domain, (int)'#')) != NULL)
    {
        *sep = '\0';
        ip = domain;
        domain = sep + 1;
    }
    add_nameip_x(name,strlen(name)+1,ip);
    free(tmp);
    return 1;
}

int add_namesrv_to_dlist(char * add_info,int info_len)
{
	c_data_t *pdata = NULL;
    char * domain;
    char * tmp;
    char * sep = NULL;
    char * name = NULL;
    char * ip = NULL;
    pdata = (c_data_t *)add_info;
    if(pdata->length + sizeof(c_data_t) != info_len)
    {
        printf("%s:%d - Parse length error. info_len = %d, param_len = %d", 
                __FUNCTION__, __LINE__, info_len, pdata->length);
        return 0;
    }

	if((domain = (char *)malloc(pdata->length + 1)) == NULL)
	{
		log_msg(LOG_ERR, "%s:%d - malloc fail.", __FUNCTION__, __LINE__);
		return 0;
	}
    tmp = domain;
	memcpy(domain, pdata->param, pdata->length);
	domain[pdata->length] = '\0';
	
	sep = strchr(domain, (int)'-');
	if (sep) 
    {
		*sep = '\0';
        name =domain;
		domain = sep + 1;
    }
    if((sep = strchr(domain, (int)'#')) != NULL)
    {
        *sep = '\0';
        ip = domain;
        domain = sep + 1;
    }
    add_namesrv(name,strlen(name)+1,ip, strlen(ip), atoi(domain?:"0"));
    free(tmp);
    return 1;
}


int cls_handle(void)
{
	char msg[MAX_MSG_LEN];
	int recvlen, fromlen;
	struct sockaddr_un from;
	c_data_t *pdata = NULL;

        memset(msg, 0, MAX_MSG_LEN);	
	fromlen = sizeof(struct sockaddr_un);
#if 0
	recvlen = recvfrom(csock, msg, MAX_MSG_LEN, MSG_DONTWAIT, 
			(struct sockaddr *)&from, (socklen_t *)&fromlen);
#else
	recvlen = recvfrom(csock, msg, MAX_MSG_LEN, 0, 
			(struct sockaddr *)&from, (socklen_t *)&fromlen);
#endif
	if(recvlen <= 0) {
                log_dns(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "DNS relay cann't recieve local command.\n");
		return 0;
	}

	pdata = (c_data_t *)msg;

	switch(pdata->action)
	{
		case ACT_ADD_DOMAIN:
			add_domain_to_dlist(msg, recvlen);
			break;
		case ACT_DEL_DOMAIN:
			del_domain_from_dlist(msg, recvlen);
			break;
		case ACT_DIS_DOMAIN:
			dis_domain_in_dlist(msg, recvlen);
			break;
#if (defined(__SC_BUILD__) && (defined(CONFIG_SUPPORT_WEB_PRIVOXY)))
#ifdef CONFIG_SUPPORT_FON
        case ACT_ADD_FILTER_IP:
            add_filter_ip_to_dlist(msg,recvlen);
            break;
        case ACT_DEL_FILTER_IP:
            del_filter_ip_from_dlist(msg,recvlen);
            break;
        case ACT_RES_FILTER_IP:
            reset_filter_ip();
            break;
#endif
        case ACT_ADD_PROVI:
            add_provision(pdata->param,pdata->length);
            break;
        case ACT_ADD_SUBNET:
            add_subnet(msg,recvlen);
            break;
#endif
#ifdef CONFIG_SUPPORT_FON
        case ACT_ADD_WHITELIST:
            add_whitelist(msg,recvlen);
            break;
        case ACT_DEL_WHITELIST:
            reset_white_list();
            break;
        case ACT_ENABLE_FON_REDIRECT:
            enable_redirect_fon(pdata->param);
            break;
#endif
	    case ACT_DIS_QLIST:
            dis_query_list(msg, recvlen, 1);
            break;
        case ACT_ADD_NAMEIP:
            add_nameip_to_dlist(msg,recvlen);
            break;
        case ACT_ADD_NAMEIP_X:
            add_nameip_x_to_dlist(msg,recvlen);
            break;
        case ACT_ADD_NAMEIP_HOST:
            add_nameip_host_to_dlist(msg,recvlen);
            break;
#ifdef CONFIG_SUPPORT_WEB_URL_FILTER
        case ACT_ADD_FILTER_DOMAIN:
            add_filter_domain_to_dlist(msg,recvlen);
            break;
        case ACT_DEL_FILTER_DOMAIN:
            del_filter_domain_from_dlist(msg,recvlen);
            break;
#endif
        case ACT_ADD_NAMESRV:
            add_namesrv_to_dlist(msg,recvlen);
            break;
        case ACT_DEL_NAMEIP:
            reset_nameip();
            add_nameip("localhost", sizeof("localhost"), "127.0.0.1");
            /*add local back*/
            break;
        case ACT_DEL_NAMEIP_X:
            reset_nameip_x();
            break;
        case ACT_DEL_NAMESRV:
            reset_namesrv();
            break;
#ifdef CONFIG_SUPPORT_REDIRECT
        case ACT_ENABLE_REDIRECT:
            enable_redirect(pdata->param);
            break;
        case ACT_DISABLE_REDIRECT:
            disable_redirect();
            break;
#endif
#ifdef __SC_BUILD__
        case ACT_SET_OPTION15:
            set_option15(msg,recvlen);
            break;
#endif
        default:
            log_msg(LOG_ERR, "recvfrom lsock error");
            return 0;
	}

	return 1;
}

#define DTYPE_SEP_STR       ("ww2w")
#define DNS_QUERY_OFFSET    (12)
/*
 * Description:
 *      Find the special char '#' from domain name, and parse 
 *      string after '#' as domain type, and need delete '#xxxx' 
 *      from dns packet
 * Parameter:
 *      dns_pkt: dns packet received from socket
 *      len:     dns packet length
 * Retrun:
 *      domain type
 * */
int sc_dtype_parse(char *dns_pkt, int *len)
{
    char *p = NULL, *q = NULL, *r = NULL;
    int dtype = 0;
    int len_after_r = 0;
    
    if((p = strstr(&dns_pkt[DNS_QUERY_OFFSET], DTYPE_SEP_STR)) == NULL)
    {
        return -1; 
    }
    
    dtype = *(p + strlen(DTYPE_SEP_STR)) - '0';
    
    switch(dtype){
        case DOMAIN_TYPE_UNKNOWN:
        case DOMAIN_TYPE_DATA:
        case DOMAIN_TYPE_TR069:
        case DOMAIN_TYPE_NTP:
        case DOMAIN_TYPE_IPTV:
        case DOMAIN_TYPE_VOIP:
        case DOMAIN_TYPE_CLI:
        case DOMAIN_TYPE_IPPHONE:
            break;
        default:
            printf("Dtype mis-matched.\n");
        return -1;
    }

    q = p - 1;
    r = p + strlen(p);
    len_after_r = *len - (r - dns_pkt);
    memcpy(q, r, len_after_r);
    *len = (q - dns_pkt) + len_after_r;
    
    return dtype;
}

void sc_dns_answer_parse(int dtype, char *msg, int *len)
{
    int i = 0;
    char buffer[MAX_MSG_LEN];
    int query_offset = DNS_QUERY_OFFSET;
    int answers_offset = 0;
    int query_name_end_offset = query_offset + strlen(&msg[query_offset]);
    int buff_len = *len - query_name_end_offset;
    unsigned short temp_len = query_name_end_offset;
    dnsheader_t *dheader = (dnsheader_t *)msg;
    int dns_ancount = ntohs(dheader->ancount);
    int dns_nscount = ntohs(dheader->nscount);
    int dns_arcount = ntohs(dheader->arcount);
    unsigned short data_len = 0;
    unsigned short name_offset = 0;
    int insert_len = strlen(DTYPE_SEP_STR) + 1;

    if(dtype == -1)
        return;
    
   if(temp_len > (query_offset + MAX_DOMAIN_LEN))
      return; 

    memcpy(buffer, msg + temp_len, buff_len);
    msg[temp_len++] = insert_len;
    memcpy(&msg[temp_len], DTYPE_SEP_STR, strlen(DTYPE_SEP_STR)); 
    temp_len += strlen(DTYPE_SEP_STR);
    msg[temp_len++] = dtype + 0x30;
    memcpy(&msg[temp_len], buffer, buff_len);
    insert_len++;
    *len += insert_len;

    answers_offset = temp_len + 5; //5 means \0 + TYPE(2)+ CLASS(2) 
    for(i = 0; i < dns_ancount; i++)
    {
        if((unsigned char)msg[answers_offset] == 0xC0)//Compressed
        {
            msg[answers_offset] &= 0x0F;
            name_offset = ntohs(*(unsigned short *)&msg[answers_offset]); 
            if(name_offset > temp_len)
            {
                name_offset += insert_len;
                name_offset = htons(name_offset);
                memcpy(&msg[answers_offset], &name_offset, 2);
            }
            msg[answers_offset] |= 0xC0;
            answers_offset += 2;

            answers_offset += 2; //Type
            answers_offset += 2; //CLASS
            answers_offset += 4; //TTL
            data_len = ntohs(*(unsigned short *)&msg[answers_offset]);
            answers_offset += 2; //Data Length
            answers_offset += data_len;
        }
    }


    for(i = 0; i < dns_nscount; i++)
    {
        if((unsigned char)msg[answers_offset] == 0xC0)//Compressed
        {
            msg[answers_offset] &= 0x0F;
            name_offset = ntohs(*(unsigned short *)&msg[answers_offset]); 
            if(name_offset > temp_len)
            {
                name_offset += insert_len;
                name_offset = htons(name_offset);
                memcpy(&msg[answers_offset], &name_offset, 2);
            }
            msg[answers_offset] |= 0xC0;
            answers_offset += 2;

            answers_offset += 2; //Type
            answers_offset += 2; //CLASS
            answers_offset += 4; //TTL
            data_len = ntohs(*(unsigned short *)&msg[answers_offset]);
            answers_offset += 2; //Data Length
            answers_offset += data_len;
        }
    }
    for(i = 0; i < dns_arcount; i++)
    {
        if((unsigned char)msg[answers_offset] == 0xC0)//Compressed
        {
            msg[answers_offset] &= 0x0F;
            name_offset = ntohs(*(unsigned short *)&msg[answers_offset]); 
            if(name_offset > temp_len)
            {
                name_offset += insert_len;
                name_offset = htons(name_offset);
                memcpy(&msg[answers_offset], &name_offset, 2);
            }
            msg[answers_offset] |= 0xC0;
            answers_offset += 2;

            answers_offset += 2; //Type
            answers_offset += 2; //CLASS
            answers_offset += 4; //TTL
            data_len = ntohs(*(unsigned short *)&msg[answers_offset]);
            answers_offset += 2; //Data Length
            answers_offset += data_len;
        }
    }
    return;
}


