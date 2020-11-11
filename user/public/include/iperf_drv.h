#ifndef	_SC_IPERF_DRV_H_
#define	_SC_IPERF_DRV_H_


#define SC_DRV_STR  "scDrv"
#define SC_DRV_FD_NAME_STR  "/dev/"SC_DRV_STR

typedef enum
{
    SC_DRV_IOCTL_SET_IPERF_CREATE_STREAM = 1000,
    SC_DRV_IOCTL_GET_IPERF_CREATE_STREAM_STATE,
    SC_DRV_IOCTL_GET_IPERF_UPDATE_RESULT,
    SC_DRV_IOCTL_SET_IPERF_TEST_DONE,
    SC_DRV_IOCTL_SET_IPERF_TEST_RUN,
} SC_DRV_IOCTL_CMD;

#ifndef IPERF_SPEED_TEST
#define _SLIST_HEAD_(name, type)                      \
struct name {                               \
    struct type *slh_first; /* first element */         \
}
#define _SLIST_ENTRY_(type)                       \
struct {                                \
    struct type *sle_next;  /* next element */          \
}
#define _SLIST_FIRST_(head)   ((head)->slh_first)
#define _SLIST_NEXT_(elm, field)  ((elm)->field.sle_next)
#define _SLIST_FOREACH_(var, head, field)                                \
    for ((var) = _SLIST_FIRST_((head));                                  \
    (var);                                                             \
    (var) = _SLIST_NEXT_((var), field))
#define _TAILQ_HEAD_(name, type)                      \
struct name {                               \
     struct type *tqh_first; /* first element */         \
     struct type **tqh_last; /* addr of last next element */     \
}
#define _TAILQ_ENTRY_(type)                       \
struct {                                \
     struct type *tqe_next;  /* next element */          \
     struct type **tqe_prev; /* address of previous next element */  \
}
typedef unsigned long long kernel_iperf_size_t;
struct kernel_iperf_interval_results{
    kernel_iperf_size_t bytes_transferred;
    int       interval_packet_count;
    int       interval_outoforder_packets;
    int       interval_cnt_error;
    int       packet_count;
    double    jitter;
    int       outoforder_packets;
    int       cnt_error;
    _TAILQ_ENTRY_(kernel_iperf_interval_results) irlistentries;
};
struct kernel_iperf_stream_result{
    kernel_iperf_size_t bytes_received;
    kernel_iperf_size_t bytes_sent;
    kernel_iperf_size_t bytes_received_this_interval;
    kernel_iperf_size_t bytes_sent_this_interval;
    _TAILQ_HEAD_(_irlisthead, kernel_iperf_interval_results) interval_results;
};
struct kernel_iperf_stream{
    struct kernel_iperf_stream_result *result;
    int       packet_count;
    double    jitter;
    int       outoforder_packets;
    int       cnt_error;
    int       local_port;
    int       remote_port;
    int id;
    int       (*rcv) (struct kernel_iperf_stream * stream);
    int       (*snd) (struct kernel_iperf_stream * stream);
    _SLIST_ENTRY_(kernel_iperf_stream) streams;
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
    unsigned short int vlan_id;
}IPERF_STREAM_INFO;
struct report{
    int bytes_received;
    int bytes_received_this_interval;
    int packets_received_total;
    int packets_received_this_interval;

    int outoforder_packets;
    int cnt_error;
};
typedef struct kernel_iperf_test{
    int stream_state;
    int enable_hook;
    int enable_tx;
    int update_state_and_print;
    int bytes_sent;
    int bytes_received;
    int blocks_sent;
    struct iperf_stream_setting streams_setting;
    struct task_struct *iperf_tx_thread;
    _SLIST_HEAD_(_slisthead, kernel_iperf_stream) streams;
    //int (*stats_callback)(struct kernel_iperf_test *);
    //int (*report_callback)(struct kernel_iperf_test *);
    //int (*result_report)(struct kernel_iperf_test *);
    struct report report_test;
}KERNEL_IPERF_TEST;
#endif

int util_scDrv_set_iperf_create_stream_info(IPERF_STREAM_INFO *information);
int util_scDrv_get_iperf_create_stream_state(IPERF_STREAM_INFO *information);
int util_scDrv_get_iperf_stream_interval(struct report *report_test);
int util_scDrv_set_iperf_stream_end(void);
int util_scDrv_set_iperf_stream_run(void);

#endif /* _SC_DRV_H_ */

