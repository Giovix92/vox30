#ifndef _STATISTICS_H_
#define _STATISTICS_H_

/* add by archer for statistical */
typedef struct if_adv_info_s{
    char ifname[16];
    unsigned long long tx_bytes;
    unsigned long long rx_bytes;
    unsigned long long tx_packets;
    unsigned long long rx_packets;
    unsigned long long tx_drops;
    unsigned long long rx_drops;
    unsigned long long tx_errors;
    unsigned long long rx_errors;
    unsigned long long rx_crc_errors;
    unsigned long long tx_crc_errors;
    unsigned long long tx_multicast_packets;
    unsigned long long rx_multicast_packets;
    unsigned long long tx_multicast_bytes;
    unsigned long long rx_multicast_bytes;
    unsigned long long tx_broadcast_packets;
    unsigned long long rx_broadcast_packets;
    unsigned long long tx_unicast_packets;
    unsigned long long rx_unicast_packets;
    unsigned long long rx_unknownproto_packets;
}if_adv_info_t;

#endif


