#ifndef __SAL_DSL_H__
#define __SAL_DSL_H__

typedef struct {
    int snrmin;
}dsl_s;

enum{
    DSL_ACTION_SNR
};

enum
{
    DSL_ITEM_SNR_UP_MIN,
    DSL_ITEM_SNR_DOWN_MIN
};

struct dsl_packet{
    int item;
    int id;
    int action;
    dsl_s data;
};

#define UBUS_DSL_SNR_OBJECT      "statsd.dsl.snr"

int sal_wan_get_min_snr_up(int wan_id, dsl_s *dsl_t, int time_m);
int sal_wan_get_min_snr_down(int wan_id, dsl_s *dsl_t, int time_m);
#endif
