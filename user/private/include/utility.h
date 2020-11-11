#ifndef _UTILITY_H_
#define _UTILITY_H_
#include <netinet/in.h>
#include "statistics.h"

#define IPPING_DIAG_CMD "/usr/sbin/ipping_diag"
typedef enum {
    A_ERROR = -1,               /* Generic error return */
    A_OK = 0,                   /* success */
    A_EINVAL,                   /* Invalid parameter */     
    A_NO_MEMORY,  
	A_ENOENT, 
	A_EXIST    
} A_STATUS;
#define UPGRADE_RESULT         "/tmp/upg_result"
#ifndef A_CHAR_DEFINED
typedef char                    A_CHAR;
#define A_CHAR_DEFINED
#endif

#ifndef A_INT8_DEFINED
typedef A_CHAR                  A_INT8;
#define A_INT8_DEFINED
#endif

#ifndef A_INT16_DEFINED
typedef short                   A_INT16;
#define A_INT16_DEFINED
#endif

#ifndef A_INT32_DEFINED
typedef int                     A_INT32;
#define A_INT32_DEFINED
#endif

#ifndef A_UCHAR_DEFINED
typedef unsigned char           A_UCHAR;
#define A_UCHAR_DEFINED
#endif

#ifndef A_UINT8_DEFINED
typedef A_UCHAR                 A_UINT8;
#define A_UINT8_DEFINED
#endif

#ifndef A_UINT16_DEFINED
typedef unsigned short          A_UINT16;
#define A_UINT16_DEFINED
#endif

#ifndef A_UINT32_DEFINED
typedef unsigned int            A_UINT32;
#define A_UINT32_DEFINED
#endif

#ifndef A_UINT_DEFINED
typedef unsigned int            A_UINT;
#define A_UINT_DEFINED
#endif

#ifndef A_BOOL_DEFINED
typedef int                     A_BOOL;
#define A_BOOL_DEFINED
#endif

#ifndef A_INT64_DEFINED
typedef long long               A_INT64;
#define A_INT64_DEFINED
#endif

#ifndef A_UINT64_DEFINED
typedef unsigned long long      A_UINT64;
#define A_UINT64_DEFINED
#endif

#ifndef A_LONGSTATS_DEFINED
typedef A_UINT64                A_LONGSTATS;
#define A_LONGSTATS_DEFINED
#endif

#ifndef UINT32_DEFINED
typedef A_UINT32                UINT32;
#define UINT32_DEFINED
#endif

#ifndef INT16_DEFINED
typedef A_INT16                 INT16;
#define INT16_DEFINED
#endif

#ifndef  INT32_DEFINED
typedef A_INT32			        INT32;
#define INT32_DEFINED
#endif

#ifndef CHAR_DEFINED
typedef char			        CHAR;
#define CHAR_DEFINED
#endif

#ifndef BYTE_DEFINED
typedef unsigned char		    BYTE;
#define BYTE_DEFINED
#endif

#ifndef WORD_DEFINED
typedef unsigned short 		    WORD;
#define WORD_DEFINED
#endif

#ifndef DWORD_DEFINED
typedef unsigned int		    DWORD;
#define DWORD_DEFINED
#endif

#ifndef VOID_DEFINED
typedef void 			        VOID;
#define VOID_DEFINED
#endif


#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#define LOCAL   static

#define WLAN_MAC_ADDR_SIZE      6
union wlanMACAddr {
    A_UINT8  octets[WLAN_MAC_ADDR_SIZE];
    A_UINT16 words[WLAN_MAC_ADDR_SIZE/2];
};
typedef union wlanMACAddr WLAN_MACADDR;

#define A_MACADDR_COPY(from, to)              \
    do {                                      \
        (to)->words[0] = (from)->words[0];    \
        (to)->words[1] = (from)->words[1];    \
        (to)->words[2] = (from)->words[2];    \
    } while (0)

#define A_MACADDR_COMP(m1, m2)                \
    ((((m1)->words[2] == (m2)->words[2]) &&   \
      ((m1)->words[1] == (m2)->words[1]) &&   \
      ((m1)->words[0] == (m2)->words[0])) == 0)

#define FT_TOOL_PATH "/mnt/1/"

#define util_get_num_from_string_by_comma(word, wordlist, next) \
    for (next = &wordlist[strspn(wordlist, ",")], \
        strncpy(word, next, sizeof(word)), \
        word[strcspn(word, ",")] = '\0', \
        word[sizeof(word) - 1] = '\0', \
        next = strchr(next, ','); \
        strlen(word); \
        next = next ? &next[strspn(next, ",")] : "", \
        strncpy(word, next, sizeof(word)), \
        word[strcspn(word, ",")] = '\0', \
        word[sizeof(word) - 1] = '\0', \
        next = strchr(next, ','))

void echo(const char *file, const char *format, ...);
void util_echo(const char *file, const char *format, ...);
int myPipe(char *command, char **output);
int process_running(char *name);
int get_numeric_by_cmd(char *cmd, char *key);

int SYSTEM(const char *format, ...);
int COMMAND(const char *format, ...);

int scIsIpAddress(A_UINT8 *ipAddr);
#ifdef SUPPORT_MULTIWAN_DOMAIN
char *scUrlIsDomain(A_UINT8 *domain);
#endif
int strNull( char *str);

void scToLows(A_UINT8 *charStr);
void scToUppers(A_UINT8 *charStr); 	
int scValidStr(A_UINT8 *charStr); 	
int scValidString(A_UINT8 *charStr); 	
int scValidUrl(A_UINT8 *charStr);
int scValidIPv6(A_UINT8 *ipStr, A_UINT8 flag);
int scValidGWv6(A_UINT8 *ipStr);
int scValidMSName(A_UINT8 *str, A_UINT8 len);
int scValidDNSName(A_UINT8 *str, A_UINT8 len);
int scValidEmailAddr(A_UINT8 *email, A_UINT8 len);
int scChars2Hexs(unsigned char *charStr, int strLen, char *hexBuf , char *separator);
int macAddrToString(char *macAddress, char *buf , char *separator);
void mac_string_plus(char *mac, char separator, char val); 
int convert_mac_to_lower_case(char *mac);
A_INT16 scHex2Char(A_UINT8 * str, A_UINT8 len, A_INT32 * value);
A_INT16 scHexs2Chars(A_UINT8 * hexs, A_UINT8 * str, A_UINT8 len, A_INT16 interval);
void scMacStr17ToStr12(A_UINT8 *str17, A_UINT8 *str12);
void scMacStr12ToStr17(A_UINT8 *str12, A_UINT8 *str17, char *separator);
A_BOOL  scValidHex(char ch);
A_BOOL  scValidHexs(char *str, int len);
A_BOOL 	scValidNetbiosName(char *pName, A_UINT16 len);
A_BOOL scValidIpAddress(A_UINT32 ipAddress);
A_BOOL scValidIpMask(A_UINT32 ipMask, A_UINT32 *pValidIpMask);
A_BOOL scValidIpGateWay(A_UINT32 gateway);
A_BOOL scValidIpMaskGateWay(A_UINT32 ipaddr, A_UINT32 netmask, A_UINT32 gateway);
A_BOOL asciiToPassphraseKey(A_UINT8 *pstr, A_UINT8 *pPpKey, int encryptedKeyLen);
A_BOOL scValidpassPhrase(char *phrase, int len);
char *shell_string_encode(char code, char *str);
char *shell_string_encode_specchar(const char *str);
char *string_encode(char code, char *str);
char *string_decode(char *str);
void sgml_encode(char *pStrDest, char *pStrSrc, A_BOOL readOnly);
void scSecretHide(char *secret);
A_BOOL scSecretHidden(char *secret);
A_UINT16 scGetWord(A_UINT8 * buf);
void scSetWord(A_UINT8 * buf, A_UINT16 wValue);
A_UINT32 scGetDword(A_UINT8 * buf);
void scSetDword(A_UINT8 * buf, A_UINT32 dwValue);	
void util_xml_decoder(char *pStrDest, char *pStrSrc);
void util_xml_encoder(char *pStrDest, char *pStrSrc);
/* Input of util_xml_encryption must be a string */
char *util_xml_encryption(char *str);
char *util_xml_decryption(char *str);


typedef struct if_info_s{
	char ifname[16];
	char ipaddr[16];
	char mac[18];
	char mac_raw[18];
	char mask[16];
	struct in_addr gw;
	int  mtu;
}if_info_t;

struct net_device_info {
	unsigned long long int rx_packets;	/* total packets received       */
	unsigned long long int tx_packets;	/* total packets transmitted    */
	unsigned long long int rx_bytes;	/* total bytes received         */
	unsigned long long int tx_bytes;	/* total bytes transmitted      */
	unsigned long long int rx_errors;	/* bad packets received         */
	unsigned long long int tx_errors;	/* packet transmit problems     */
	unsigned long long int rx_dropped;	/* no space in linux buffers    */
	unsigned long long int tx_dropped;	/* no space available in linux  */
	unsigned long long int rx_multicast;	/* multicast packets received   */
	unsigned long long int rx_compressed;
	unsigned long long int tx_compressed;
	unsigned long long int collisions;
	
    unsigned long long int rx_fifo_errors;	/* recv'r fifo overrun          */
    unsigned long long int tx_fifo_errors;
    unsigned long long int rx_frame_errors;	/* recv'd frame alignment error */
    unsigned long long int tx_carrier_errors;
    unsigned long long int tx_multicast_packets;
    unsigned long long int rx_multicast_packets;
    unsigned long long int tx_multicast_bytes;
    unsigned long long int rx_multicast_bytes;
    unsigned long long int tx_broadcast_packets;
    unsigned long long int rx_broadcast_packets;
    unsigned long long int tx_unicast_packets;
    unsigned long long int rx_unicast_packets;
    unsigned long long int rx_unknownproto_packets;

};

typedef struct cfg_elem_s
{
    char *value;
    char *name;
}cfg_elem_t;

#define    MAX_APP_CFG_PATH_LEN            100

#define    LOAD_CFG_SUCCESS        0
#define    LOAD_CFG_ERROR          -1


/* add end */

int getCfgInfo(const char *cfgFilePath, cfg_elem_t *cfg);

int get_sockfd(void);
int getMgtBrInfo(if_info_t *if_info);
char *getMgtBrBroadcast(const char *lan_interface);
int getIFIPaddress(char *if_name, char *ipaddr);
int util_setIPFragment(char *if_name, int flag);
int util_resetIFstats(char *if_name);
int getIFInfo(char *if_name, if_info_t *if_info);
int getIFAdvInfo(char *if_name, if_adv_info_t *if_info);
int getFlashMacAddress(char *pMac);
void getManufacturerName(char *buffer);
int sc_getPidByName(const char *name);

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN            46
#endif
#define MAX_PRODUCT_BASE_NAME_LEN   128
#define MAX_PRODUCT_RF_SKU_TAG_LEN  128
#define MAX_PRODUCT_NAME_LEN  (MAX_PRODUCT_BASE_NAME_LEN + MAX_PRODUCT_RF_SKU_TAG_LEN)
#ifdef CONFIG_SUPPORT_IPV6
typedef struct if_infov6_s{
	char ifname[16];
	char ipaddr[INET6_ADDRSTRLEN];
    char gw[INET6_ADDRSTRLEN];
	
}if_infov6_t;

int getMgtBrv6Info(if_infov6_t *if_info);
int getIP6Info(char *ifname, char *address);
int util_get_local_IPv6(char *ifname, char *address );
int util_getIP6_GlobalInfo(char *ifname, char *address );
int util_getIP6_ULAInfo(char *ifname, char *address );
int util_getIP6GwInfo(char *ifname, char *gw );
int util_get_local_IPv6_with_prefixlen(char *ifname, char *address );
int util_getIP6_GlobalInfo_with_prefixlen(char *ifname, char *address );
int util_getIP6_ULAInfo_with_prefixlen(char *ifname, char *address );
int util_check_ipv6_is_same_subnet(char *ip1, char *ip2, int plen);
int util_check_ipv6_is_same_address(char *ip1, char *ip2);
int util_get_ipv6_prefix_len(char *prefix);
char* util_ipv6_plen_to_mask(int plen);
char* util_get_ipv6_sub_str(char *ip, char *prefix);
int util_check_ipv6_is_neigh(const char* ip);
void util_gen_eui64_addr(unsigned char *mac2, unsigned char *mac1);
#endif
int util_get_pid(char *filePidName);
int util_rewrite_file(char *filename, const char *format, ...);
int getEthernetStatus(void);
char * apCfgScPidGet(void);
int getDeviceStartType(void);

void country_list_generate(void);
void set_default_country(void);

A_BOOL  scCompositor(int v[], int left, int right);
A_BOOL scConversionSSID(A_UINT8 *src, A_UINT8 *dest);
A_BOOL scConversionASCII(A_UINT8 *src, A_UINT8 *dest);
A_BOOL scVoIPExist(void);
A_BOOL  scWirelessCardExist(int unit);
unsigned char scWirelessCardType(int unit);
int  sc5GChannelExtensionGet(int channel);
int  scChannelExtensionGet(int channel);
int scChannelVailableCheck(int unit, int channel);

A_STATUS scWirelessRadioInfoStrGet(char* buf, int unit);

int scCountrySupportHT40(int unit, int country);
int scCountrySupport11A(int country);
int scChannelSupportHT40(int channel);
int scChannelSupportHT(int channel);

A_BOOL sc_RadioChannelUpdate(void);
A_BOOL sc_sleep(unsigned int val);
char *scManRateStrGet(int datarate);
int scWlanUpDownRecord(int );
int syslog_savetoall(void);

char* getSubnetStr(int ip, int mask);
int getSubnetPrefixLen(int mask);

int getLanToWanIdBinding(int *);


/*
 * get ethernet port status
 */
int scEthernetPortStatusGet(int);

int scFileCopy(char *File_from, char *File_to);

enum{
	MODE_FULL = 1,
	MODE_HALF = 0,
};
enum{
	SPEED_1000 = 0,
	SPEED_100 = 2,
	SPEED_10 = 4,
	ETHER_ON = 8,
	ETHER_OFF = 16,
};

int scUpgStatusGet(char *);

enum{
	POE_LINK_ERROR = -1,
	POE_LINK_DISCONNECT = 0,
	POE_LINK_CONNECT,
	POE_LINK_WAIT,
	POE_LINK_FAILED,
};
			
enum{
	POE_LCP_ERROR = -1,
	POE_LCP_SUCCESS = 0,
	POE_LCP_INIT,
	POE_LCP_COMEUP,
	POE_LCP_DOWN,
	POE_LCP_REJECT,
};

enum{
	POE_AUTH_ERROR = -1,
	POE_AUTH_SUCCESS = 0,
	POE_AUTH_FAIL,
};
enum{
	INTERNET_ERROR = -1,
	INTERNET_CONNECT = 0,
	INTERNET_FAIL,
	INTERNET_DISCONNECT,
};

enum{
	DIAG_DSL_PHY_LINK = 0,
	DIAG_DSL_LINK_STATBLE,
	DIAG_DSL_CRC_ERROR,
    DIAG_FIBRA_LINK,
    DIAG_INTERNET_DGW,
    DIAG_INTERNET_DNS,
    DIAG_INTERNET_UP_LINK,
    DIAG_INTERNET_DOWN_LINK,
    DIAG_TELEPHONE_ACTIVE,
#ifdef VFIE
    DIAG_TELEPHONE_REGIATRATION_NC1,
    DIAG_TELEPHONE_REGIATRATION_NC2,
    DIAG_TELEPHONE_REGIATRATION_DIS1,
    DIAG_TELEPHONE_REGIATRATION_DIS2,
    DIAG_TELEPHONE_REGIATRATION_RG1,
    DIAG_TELEPHONE_REGIATRATION_RG2,
    DIAG_TELEPHONE_REGIATRATION_AF1,
    DIAG_TELEPHONE_REGIATRATION_AF2,
#else
    DIAG_TELEPHONE_REGIATRATION,
#endif
    DIAG_WIFI_AUTHENTICATION,
    DIAG_WIFI_2G_INTERFRENCE,
    DIAG_WIFI_5G_INTERFRENCE,
    DIAG_LAN1_RECEIVE,
    DIAG_LAN1_TRANSMISSION,
    DIAG_LAN2_RECEIVE,
    DIAG_LAN2_TRANSMISSION,
    DIAG_LAN3_RECEIVE,
    DIAG_LAN3_TRANSMISSION,
    DIAG_LAN4_RECEIVE,
    DIAG_LAN4_TRANSMISSION,
    DIAG_ACS_SERVER,
    DIAG_MEMORY_UTILIZATION,
    DIAG_CPU_UTILIZATION,
    DIAG_UMTS_LINK,
#ifdef CONFIG_SUPPORT_WEBAPI
    DIAG_NO_ERROR,
#endif
    DIAG_LAST_KEY,
};
char *scVoIPToneTypeNameMappingGet(int type);
char *scVoIPCoderTypeMappingGet(int type);
#define SC_CRYPT_SALT           "$1$SERCOMM$"

long get_uptime(void);
int scIsAllnumber(A_UINT8 *str);
void getTimeofDay(char *buffer);
char * Get_ISO8601_Time();
char * util_change_time_to_day(time_t *t);
void getUpTime(char *buffer);
char *scGetStdTime(int sec);
#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff"

int divide_str_to_array(char *s, char divide_symbol, char **array, int array_num);
void free_array_str(char **array, int array_num);

void get_iptables_mac_with_mask(const char *mac, char *iptablesMac, char *mac_mask);
void get_ebtables_mac_with_token(const char *mac, char *ebtablesMac, char *token);
int sc_is_ipv4_prefix(char *str);
int sc_is_validate_value_range(char *str, char token, int begin_c, int end_c);
int sc_is_validate_dscp_value(char *str, int begin_c, int end_c);
int sc_is_validate_interface(char *str);
int sc_is_validate_mac(char *str);
int util_mask_to_prefix(char *mask);
char* util_prefix_to_mask(int mask_len);
int util_convert_string_to_array(int ** array, char* p, char token);
int util_del_sub_str(char *head, char *sub);
int util_check_ip_is_in_same_subnet(char *ip_addr1, char *ip_addr2, char *submask);
int util_Valid_port(A_UINT8 *port);
int util_Valid_time(A_UINT8 *time);
int util_ValidIpv6Mask(char* ipStr, int flag);
#define _UTIL_MBUG_
void _util_mbug(const char *file, const char* func, int line, char *format, ...);
#define util_mbug(format, arg...) _util_mbug(__FILE__, __FUNCTION__, __LINE__, format, ##arg);

struct parsed_url {
	char *scheme;               /* mandatory */
	char *host;                 /* mandatory */
	char *port;                 /* optional */
	char *path;                 /* optional */
	char *query;                /* optional */
	char *fragment;             /* optional */
	char *username;             /* optional */
	char *password;             /* optional */
};

int parse_url(char *url, struct parsed_url **ppurl);
void parsed_url_free(struct parsed_url *purl);
unsigned long util_crc32(char *data, int length);
int util_ramdom_str(char *buf, int len);
int util_ramdom_int(int base, int range);
int util_lock(char *file, int retry_time);
void  util_unlock(int fd);
char* util_convert_capa(char *capa);
int util_check_dns_active(char *hostname, int timeout);
int util_check_if_up(char* if_name);
int util_is_valid_string(char *name);
int util_command_valid_check(char* cmd);
int util_check_is_valid_mask(char* mask_str);
int util_check_is_valid_ip(char* ip_str);
int util_check_is_128_ram(void);
int set_sched_prio(pid_t pid, int sched_algo, int priority);
int get_sched_policy(pid_t pid);
int get_sched_priority(pid_t pid);
int compare_hm(char *ta, char *tb);
int is_in_range(char *now, char *stime, char *etime);
int util_log_to_terminal(char *file_name, char *buf);

#define LOCK_RETRY_COUNT    3
int util_lock_reg(int fd, short type, int retry_time);

#define util_read_lock(fd, retry_time) \
    util_lock_reg((fd), F_RDLCK, (retry_time))

#define util_write_lock(fd, retry_time) \
    util_lock_reg((fd), F_WRLCK, (retry_time))

#define util_un_lock(fd, retry_time)\
    util_lock_reg((fd), F_UNLCK, (retry_time))

int util_is_process_exsit(char *pid_file);
ssize_t util_safe_write(int fd, const void *buf, size_t count);
ssize_t util_full_write(int fd, const void *buf, size_t len);
ssize_t util_safe_read(int fd, void *buf, size_t count);
int util_uri_compare(char *uri1, char *uri2);
int util_check_ip_is_private_ip(char *ipaddr);
int util_log2(int value);
char * util_url_encode(const char *s);
char *util_url_decode(const char * s);
char *util_ascii_string_separated(char *oldstr, int len);
int util_is_contain_ascii(char * str);
void util_delete_file(const char *path);


typedef struct urn_policy_table
{
	char *urn;
	int app_id;
	int flow_id; 
}urn_policy_table;
extern urn_policy_table policy_table[];
#endif



