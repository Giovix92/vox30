#ifndef _CAL_VIDEO_H_
#define _CAL_VIDEO_H_


char *cal_video_get_igmp_max_groups(void);
int   cal_video_set_igmp_max_groups(char *value);
char *cal_video_get_igmp_max_group_members(void);
int   cal_video_set_igmp_max_group_members(char *value);
char *cal_video_get_igmp_robustness(void);
int   cal_video_set_igmp_robustness(char *value);
char *cal_video_get_igmp_querier_version(void);
int   cal_video_set_igmp_querier_version(char *value);
char *cal_video_get_igmp_query_interval(void);
int   cal_video_set_igmp_query_interval(char *value);
char *cal_video_get_igmp_query_response_interval(void);
int   cal_video_set_igmp_query_response_interval(char *value);
char *cal_video_get_igmp_unsolicited_report_interval(void);
int   cal_video_set_igmp_unsolicited_report_interval(char *value);
char *cal_video_get_igmp_fast_leave_enable(void);
int   cal_video_set_igmp_fast_leave_enable(char *value);
char *cal_video_get_igmp_last_member_query_interval(void);
int   cal_video_set_igmp_last_member_query_interval(char *value);
char *cal_video_get_igmp_last_member_query_count(void);
int   cal_video_set_igmp_last_member_query_count(char *value);
char *cal_video_get_igmp_proxy_enable(void);
int   cal_video_set_igmp_proxy_enable(char *value);
int cal_video_set_igmp_snooping_enable(char *value);
char *cal_video_get_igmp_proxy_log_level(void);
int   cal_video_set_igmp_proxy_log_level(char *value);
char *cal_video_get_igmp_proxy_blcok_groups(void);
int   cal_video_set_igmp_proxy_blcok_groups(char *value);
char *cal_video_get_igmp_proxy_bootcast_group(void);
int   cal_video_set_igmp_proxy_bootcast_group(char *value);
char *cal_video_get_igmp_proxy_network_interface(void);
int   cal_video_set_igmp_proxy_network_interface(char *value);
int cal_video_get_igmp_proxy_wan_id(void);
char *cal_video_get_gui_enable(void);
char *cal_video_get_igmp_upstream_version(void);
int   cal_video_set_igmp_upstream_version(char *value);
char *cal_video_get_igmp_inform_enable(void);
int   cal_video_set_igmp_inform_enable(char *value);
char *cal_video_get_igmp_inform_interval(void);
int   cal_video_set_igmp_inform_interval(char *value);
char *cal_video_get_igmp_inform_version(void);
int   cal_video_set_igmp_inform_version(char *value);




#endif /* _CAL_VIDEO_H_ */
