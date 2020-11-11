#ifndef __SC_IPERF3_C__
#define __SC_IPERF3_C__
int __kernel_iperf_tx_stream_thread(void *arg);
int __iperf_rx_hook(struct sk_buff *skb);
typedef unsigned long long kernel_iperf_size_t;
struct kernel_iperf_stream{
    kernel_iperf_size_t packet_count;
    double    jitter;
    kernel_iperf_size_t outoforder_packets;
    kernel_iperf_size_t cnt_error;
    int local_port;
    int remote_port;
    int id;
    int index;
    SLIST_ENTRY(kernel_iperf_stream) streams;
};
typedef struct iperf_stream_setting
{
    int parallel;
    int buffer_len;
    kernel_iperf_size_t rate;
    int reverse;
    int remote_ip;
    int remote_port;
    int local_ip;
    int local_port;
    int packet_check;
    int burst;
    int duration;
    unsigned char src_mac[32];
    unsigned char dest_mac[32];
    char ifName[32];
    int connection_mode;/*1 DHCP,2 PPPoE*/
    int vlan_enable;
    int session;
}IPERF_STREAM_INFO;
struct report{
    kernel_iperf_size_t bytes_received;
    kernel_iperf_size_t bytes_received_this_interval;
    kernel_iperf_size_t packets_received_total;
    kernel_iperf_size_t packets_received_this_interval;
    kernel_iperf_size_t bytes_sent;
    kernel_iperf_size_t bytes_sent_this_interval;
    kernel_iperf_size_t packets_sent_total;
    kernel_iperf_size_t packets_sent_this_interval;

    kernel_iperf_size_t outoforder_packets;
    kernel_iperf_size_t cnt_error;
};
typedef struct kernel_iperf_test{
    int stream_state;
    int enable_hook;
    int enable_tx;
    int do_upload;
    int run;
    kernel_iperf_size_t bytes_sent;
    kernel_iperf_size_t bytes_received;
    int blocks_sent;
    struct iperf_stream_setting streams_setting;
    struct task_struct *iperf_tx_thread;
    SLIST_HEAD(_slisthead, kernel_iperf_stream) streams;
    struct report report_test[128];
}KERNEL_IPERF_TEST;
#endif
