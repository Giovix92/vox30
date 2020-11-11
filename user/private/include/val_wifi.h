#ifndef _VAL_WIFI_H_
#define _VAL_WIFI_H_

/* Flags for client_info_t indicating properties of STA */
#define WL_STA_N_CAP        0x00002000  /* STA 802.11n capable */
#define WL_STA_VHT_CAP      0x00100000  /* STA VHT(11ac) capable */

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
#define MCSSET_LEN  16

#define WL_RSPEC_ENCODE_RATE    0x00000000      /* Legacy rate is stored in RSPEC_RATE_MASK */
#define WL_RSPEC_ENCODE_HT      0x01000000      /* HT MCS is stored in RSPEC_RATE_MASK */
#define WL_RSPEC_ENCODE_VHT     0x02000000      /* VHT MCS and Nss is stored in RSPEC_RATE_MASK */
#define WL_RSPEC_ENCODE_HE      0x03000000      /* HE MCS and Nss is stored in RSPEC_RATE_MASK */

#define OLD_NRATE_MCS_INUSE     0x00000080      /* MSC in use,indicates b0-6 holds an mcs */
#define OLD_NRATE_RATE_MASK     0x0000007f      /* rate/mcs value */
#define OLD_NRATE_STF_MASK      0x0000ff00      /* stf mode mask: siso, cdd, stbc, sdm */

#define WL_RSPEC_ENCODING_MASK      0x03000000      /* Encoding of Rate/MCS field */
#define WL_RSPEC_OVERRIDE_RATE      0x40000000      /* bit indicate to override mcs only */

#define WL_RSPEC_RATE_MASK          0x000000FF      /* rate or HT MCS value */
#define WL_RSPEC_VHT_MCS_MASK       0x0000000F      /* VHT MCS value */
#define WL_RSPEC_VHT_NSS_MASK       0x000000F0      /* VHT Nss value */
#define WL_RSPEC_VHT_NSS_SHIFT      4               /* VHT Nss value shift */

#define WIFI_RADAR_2G "/var/lock/wifi_radar_2g.lock"
#define WIFI_RADAR_5G "/var/lock/wifi_radar_5g.lock"
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
typedef struct wifi_assoc_stats_s{
    unsigned long tx_tot_bytes;
    unsigned long rx_tot_bytes;
    unsigned long tx_tot_packets;
    unsigned long rx_tot_packets;
    uint32 tx_error;
    uint32 failed_retrans_count;
}wifi_assoc_stats_t;

typedef struct client_info_s{
    unsigned short ver;
    unsigned int idle;
    char status[128];
    unsigned long long tx_tot_bytes;
    unsigned long long rx_tot_bytes;
    unsigned long tx_tot_packets;
    unsigned long rx_tot_packets;
    unsigned long long tx_ucast_bytes;
    unsigned long long rx_ucast_bytes;
    unsigned int  tx_rate;
    unsigned int  rx_rate;
    int  retrans;
    int  signal_strength;
    int  session_duration;
    unsigned int flags;
    unsigned short ht_capabilities;
    int vht_flags;
    unsigned short mcs[MCSSET_LEN];
    unsigned int tx_failures;
    unsigned int tx_retries;
    unsigned int tx_rspec;
}client_info_t;

typedef struct wds_station_info_s{
    char mac[18];
    char mode[16];
}wds_station_info_t;

typedef struct{                                        
    unsigned  int counter;                                   
#define WIFI_MAC_NUMBER 128                             
#define WIFI_MAC_LEN 18                                 
    char mac_list[WIFI_MAC_NUMBER][WIFI_MAC_LEN];
}client_macinfo;                                       

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
    char encryption[30];
    char rateset[50];
    char basic_rateset[50];
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
    char supported_standard[16];
    char operating_standard[16];
}neighbor_info_t;

struct channel_info_s
{
    int channel;
    char chan_info[100];
};
typedef struct wifi_radar_s
{
	char ssid[30];
	char bssid[18];
    char mode[10];
    char main_chan[4];
    char middle_chan[4];
    char chanstr[16];
	char bandWidth[8];
	char rssi[5];
	char noise[5];
    char beacon_period[16];
    char dtim_period[16];
	char protection[30];
}wifi_radar_t;

typedef struct chanim_stats_s
{
    uint32 glitch;
    uint32 badplcp;
    uint8 ccastats[CCASTATS_MAX];
    int8 bgnoise;
    unsigned short chanspec;
    uint32 timestamp;
    uint32 bphy_glitch;
    uint32 bphy_badplcp;
    uint8 chan_idle;
    unsigned short busy_time;
}chanim_stats_t;
typedef struct wifi_chanim_stats_s
{
    unsigned int buflen;
    unsigned int version;
    unsigned int count;
    chanim_stats_t stats[1];
}wifi_chanim_stats_t;
#ifdef CONFIG_BRCM_SUPPORT
typedef enum{
    AUTHE_STA_LIST,
    AUTHO_STA_LIST,
    ASSOCLIST
}sta_mac_type;
#endif
#define WL_CHANSPEC_BW_MASK         0x0C00
#define WL_CHANSPEC_BW_40           0x0C00
#define WL_CHANSPEC_CTL_SB_MASK     0x0300
#define WL_CHANSPEC_CTL_SB_LOWER    0x0100
int val_get_wifi_ssid(int id,char *value);
int val_get_wifi_if_status(int id,int *value);
int val_get_wifi_if_mac(int id,char *value);
int val_get_wifi_if_cnu(int id,char *value);
int val_get_wifi_if_sec(int id,char *value);
int val_get_wifi_if_enc(int id,char *value);

int val_get_wifi_channel(void);
int val_get_wifi_assoc_stats_info(char* ifname, char* mac, wifi_assoc_stats_t* wifi_assoc_stats);
int val_get_wifi_stats_info(int id, int stats_member);
int val_get_client_stats_info(char *mac, client_info_t *client_info);
void val_get_all_client_mac(client_macinfo* mac_client, int id);
void val_get_all_client_info(int id, char* mac, client_info_t* client);
int val_wifi_client_check_by_mac(char *mac);
int val_wifi_check_client_mac(int id, char *mac);
int val_get_wifi_statistics(int id, wifi_info_t* info);
int val_wifi_clear_counter(void);
#ifdef CONFIG_BRCM_SUPPORT
int val_get_wifi_busytime(int id);
void val_get_wifi_chanim_stats(int id, wifi_info_t *wifi_info);
int val_get_wifi_scan(int id);
int val_get_wifi_scan_results(int id, chanim_stats_t stats[]);
char * val_wifi_get_country(int dev_index);
int val_wifi_get_auth_mac(int type, char **outbuf);
#endif
int val_get_wds_station_info(char *mac, wds_station_info_t station_info[]);
int val_get_wds_mac_list(char mac[], int len);
int val_get_channel_imstate(int id);
double val_get_wifi_max_speed(void);
int val_get_wifi_rssi(int id, int* rssi);
#ifdef CONFIG_SUPPORT_WIFI_5G
double val_get_wifi_5g_max_speed(void);

#ifdef CONFIG_SUPPORT_5G_QD
int val_get_wifi_is_start_prod(void);
int val_wifi_start_prod(unsigned char* mac);
int val_wifi_get_cal_mode(void);
#endif
#endif
int val_get_wifi_radio_enable(int id);
double val_get_wifi_maxbitrate(int if_index);
int val_wifi_get_bandwidth(int dev_index);
char * val_wifi_get_channel(int dev_index);
#ifdef CONFIG_SUPPORT_WEBAPI
int val_get_neighbor_info(int *count, neighbor_info_t neighbor_info[], int flag);
#endif
int val_wifi_get_chan_info(int id, struct channel_info_s chan_info[]);
int val_record_wifi_radar_log(int id);
int val_read_wifi_radar_log(int id, wifi_radar_t wifi_radar[]);
int val_wifi_disconnect_device(int id, char* mac);
#endif
