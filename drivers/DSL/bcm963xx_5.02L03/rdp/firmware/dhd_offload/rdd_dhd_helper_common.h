/*
   Copyright (c) 2014 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2014:DUAL/GPL:standard
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
:>
*/

#ifndef _RDD_DHD_HELPER_COMMON_H
#define _RDD_DHD_HELPER_COMMON_H


#include "dhd_defs.h"
#include "rdpa_types.h"
#include "rdpa_dhd_helper_basic.h"
#include "bdmf_shell.h"



typedef struct {  
    uint16_t wr_idx;          /* locally maintained wr_idx */
    uint16_t rd_idx;          /* locally maintained rd_idx */
    uint8_t  *ring_base;
    uint16_t *wr_idx_addr;    /* address for shared wr_idx (between dongle and runner) */
    uint16_t *rd_idx_addr;    /* address for shared rd_idx (between dongle and runner) */
    uint32_t *mb_int_mapped;  /* IO MAP address for mailbox to dongle */
} rdd_dhd_rx_post_ring_t;



/* Init Flow Rings array, including feeding the RX_post array with FPMs */
int rdd_dhd_hlp_cfg(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, int enable);
bdmf_boolean rdd_dhd_helper_flow_ring_is_enabled(uint32_t flow_ring_idx);
void rdd_dhd_helper_shell_cmds_init(bdmfmon_handle_t rdd_dir);
int rdd_dhd_helper_aggregation_timeout_set(uint32_t radio_idx, int access_category, uint8_t aggregation_timeout);
int rdd_dhd_helper_aggregation_timeout_get(uint32_t radio_idx, int access_category, uint8_t *aggregation_timeout);
int rdd_dhd_helper_aggregation_size_set(uint32_t radio_idx, int access_category, uint8_t aggregation_size);
int rdd_dhd_helper_aggregation_size_get(uint32_t radio_idx, int access_category, uint8_t *aggregation_size);
int rdd_dhd_helper_aggregation_bypass_cpu_tx_set(uint32_t radio_idx, bdmf_boolean enable);
int rdd_dhd_helper_aggregation_bypass_cpu_tx_get(uint32_t radio_idx, bdmf_boolean *enable);
int rdd_dhd_helper_aggregation_bypass_non_udp_tcp_set(uint32_t radio_idx, bdmf_boolean enable);
int rdd_dhd_helper_aggregation_bypass_non_udp_tcp_get(uint32_t radio_idx, bdmf_boolean *enable);
int rdd_dhd_helper_aggregation_bypass_tcp_pktlen_set(uint32_t radio_idx, uint8_t pkt_len);
int rdd_dhd_helper_aggregation_bypass_tcp_pktlen_get(uint32_t radio_idx, uint8_t *pkt_len);


/* Definitions taken from DHD driver (cannot include it's header) */
typedef union {
    struct {
        uint32_t low;
        uint32_t high;
    };
    struct {
        uint32_t low_addr;
        uint32_t high_addr;
    };
    uint64_t u64;
} addr64_t;


typedef struct 
{
    /* message type */
    uint8_t msg_type;
    /* interface index this is valid for */
    uint8_t if_id;
    /* flags */
    uint8_t flags;
    /* alignment */
    uint8_t reserved;
    /* packet Identifier for the associated host buffer */
    uint32_t request_id;
} cmn_msg_hdr_t;


typedef struct 
{
    /* common message header */
    cmn_msg_hdr_t cmn_hdr;
    /* provided meta data buffer len */
    uint16_t metadata_buf_len;
    /* provided data buffer len to receive data */
    uint16_t data_buf_len;
    /* alignment to make the host buffers start on 8 byte boundary */
    uint32_t rsvd;
    /* provided meta data buffer */
    addr64_t metadata_buf_addr;
    /* provided data buffer to receive data */
    addr64_t data_buf_addr;
} host_rxbuf_post_t;

#endif /* _RDD_DHD_HELPER_COMMON_H */

