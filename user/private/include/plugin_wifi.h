#ifndef _VAL_WIFI_H_
#define _VAL_WIFI_H_

/* STA HT cap fields */
#define STA_CAP_LDPC_CODING		0x0001	/* Support for rx of LDPC coded pkts */
#define STA_CAP_40MHZ		    0x0002  /* FALSE:20Mhz, TRUE:20/40MHZ supported */
#define STA_CAP_MIMO_PS_MASK	0x000C  /* Mimo PS mask */
#define STA_CAP_MIMO_PS_SHIFT	0x0002	/* Mimo PS shift */
#define STA_CAP_MIMO_PS_OFF		0x0003	/* Mimo PS, no restriction */
#define STA_CAP_MIMO_PS_RTS		0x0001	/* Mimo PS, send RTS/CTS around MIMO frames */
#define STA_CAP_MIMO_PS_ON		0x0000	/* Mimo PS, MIMO disallowed */
#define STA_CAP_GF			    0x0010	/* Greenfield preamble support */
#define STA_CAP_SHORT_GI_20		0x0020	/* 20MHZ short guard interval support */
#define STA_CAP_SHORT_GI_40		0x0040	/* 40Mhz short guard interval support */
#define STA_CAP_TX_STBC		    0x0080	/* Tx STBC support */
#define STA_CAP_RX_STBC_MASK	0x0300	/* Rx STBC mask */
#define STA_CAP_DELAYED_BA		0x0400	/* delayed BA support */
#define STA_CAP_MAX_AMSDU		0x0800	/* Max AMSDU size in bytes , 0=3839, 1=7935 */
#define STA_CAP_DSSS_CCK		0x1000	/* DSSS/CCK supported by the BSS */
#define STA_CAP_PSMP			0x2000	/* Power Save Multi Poll support */
#define STA_CAP_40MHZ_INTO      0x4000	/* 40MHz Intolerant */
#define STA_CAP_LSIG_TXOP		0x8000	/* L-SIG TXOP protection support */

/* scb vht flags */
#define STA_VHT_LDPCCAP	    0x0001
#define STA_SGI80		    0x0002
#define STA_SGI160		    0x0004
#define STA_VHT_TX_STBCCAP	0x0008
#define STA_VHT_RX_STBCCAP	0x0010
#define STA_SU_BEAMFORMER	0x0020
#define STA_SU_BEAMFORMEE	0x0040
#define STA_MU_BEAMFORMER	0x0080
#define STA_MU_BEAMFORMEE	0x0100
#define STA_VHT_TXOP_PS	    0x0200
#define STA_HTC_VHT_CAP	    0x0400

#define CCASTATS_MAX 9
typedef unsigned int uint32;
typedef unsigned char uint8;
typedef signed char int8;
typedef struct wifi_info_s{
    uint32 tx_packets;
    uint32 tx_bytes;
    uint32 rx_packets;
    uint32 rx_bytes;
    uint32 tx_error;
    uint32 rx_error;
    uint32 tx_upackets;//unicast packets
    uint32 rx_upackets;
    uint32 tx_dpackets;//discard packets
    uint32 rx_dpackets;
    uint32 tx_mpackets;//multicast packets
    uint32 rx_mpackets;//
    uint32 tx_bpackets;//broadcast packets
    uint32 rx_bpackets;
    uint32 unpro_packets;//unknow protocal packets
    uint32  channel;
    uint8 ccastats[CCASTATS_MAX];
    uint32 glitch;
    uint32 badplcp;
    int8 knoise;
    uint8 chan_idle;
    uint32 timestamp;
}wifi_info_t;

typedef struct client_info_s{
    char status[128];
    unsigned long long tx_tot_bytes;
    unsigned long long rx_tot_bytes;
    unsigned int  tx_rate;
    unsigned int  rx_rate;
    int  retrans;
    int  signal_strength;
    int  session_duration;
    unsigned short ht_capabilities;
    int vht_flags;
}client_info_t;

typedef struct wds_station_info_s{
    char mac[18];
    char mode[16];
}wds_station_info_t;

typedef struct neighbor_info_s
{
	char ssid[30];
	char bssid[18];
    char mode[10];
	char band[8];
	int channel;
	char bandWidth[8];
	int rssi;
	int noise;
    int beacon_period;
    int dtim_period;
    int middlechan;
    char chanstr[16];
    uint32 chanspec;
    char protection[30];
    char rateset[50];
    char capability[100];
    char cap_chanspec[50];
    unsigned int vht_cap;
    char ht_capability[32];
    char ht_mcs[64];
    char vht_mcs[8][32];
    char flags[40];
    int SNR;
    int vht_mcsmap;
    char rsn_mcast[2][32];
    char rsn_ucast[2][32];
    char rsn_akm[2][32];
    char rsn_capability[2][80];
    int unicast_count[2];
    int akm_count[2];
    unsigned int rsn_cap[2];
}neighbor_info_t;

struct channel_info_s
{
    int channel;
    char chan_info[100];
};
typedef enum{
    AUTHE_STA_LIST,
    AUTHO_STA_LIST,
    ASSOCLIST
}sta_mac_type;
#define WL_CHANSPEC_BW_MASK         0x0C00
#define WL_CHANSPEC_BW_40           0x0C00
#define WL_CHANSPEC_CTL_SB_MASK     0x0300
#define WL_CHANSPEC_CTL_SB_LOWER    0x0100
int plugin_val_get_wifi_if_status(int id,int *value);
int plugin_val_get_wifi_if_mac(int id,char *value);
int plugin_val_get_wifi_if_cnu(int id,char *value);
int plugin_val_get_wifi_if_sec(int id,char *value);
int plugin_val_get_wifi_if_enc(int id,char *value);
int plugin_val_get_client_stats_info(char *mac, client_info_t *client_info);
int plugin_val_get_wifi_statistics(int id, wifi_info_t* info);
int plugin_val_get_wifi_stats_info(int id, int stats_member);
int plugin_val_wifi_get_info(int wlan_no, int wifi_info_type, char **outbuf);
int plugin_val_wifi_client_check_by_mac(char *mac);
int plugin_val_wifi_check_client_mac(int id, char *mac);
int plugin_val_wifi_get_auth_mac(int type, char **outbuf);
int plugin_val_wifi_get_ver(char **ver);
int plugin_val_wifi_clear_counter(void);
int plugin_val_get_wds_station_info(char *mac, wds_station_info_t station_info[]);
int plugin_val_get_wds_mac_list(char mac[], int len);
int plugin_val_get_channel_imstate(int id);
int plugin_val_get_wifi_channel(void);
void plugin_val_get_wifi_max_speed(double *value);
void plugin_val_get_wifi_5g_max_speed(double *value);
int plugin_get_wifi_init_status(void);
int plugin_val_init_wifi_env(unsigned char* mac);
int plugin_val_get_wifi_radio_enable(int id);
void plugin_val_get_wifi_maxbitrate(int if_index,double* value);
char * plugin_val_wifi_get_channel(int dev_index);
int plugin_val_get_neighbor_info(int *count, neighbor_info_t neighbor_info[], int flag);
int plugin_val_util_get_wifi_ht_capabilities(int id, unsigned int *ht_capabilities);
int plugin_val_util_get_wifi_vht_capabilities(int id, int *vht_capabilities);
#endif
