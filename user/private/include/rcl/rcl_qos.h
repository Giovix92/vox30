#ifndef _RCL_QOS_H_
#define _RCL_QOS_H_

#define CLS_POLICY_LENGTH_MAX 1024
#define QOS_CLASSIFY_EBT 		"/tmp/qos_classify_ebt"
#define QOS_CLASSIFY_IPT 		"/tmp/qos_classify_ipt"
#define QOS_CLASSIFY_IP6T 		"/tmp/qos_classify_ip6t"
#define QOS_CLASSIFY_IPT_WAN 	"/tmp/qos_classify_ipt_wan"
#define QOS_CLASSIFY_IP6T_WAN 	"/tmp/qos_classify_ip6t_wan"

#define EBT_PROC_NAME "/proc/net/ebt_opt_list_"
#define IPT_PROC_NAME "/proc/net/ipt_opt_list_"

#define QOSCLSPOLICY 		"QOS_CLS_POLICY"
#define QOSCLSPOLICYPRER 	"PRE_QOS_CLS_POLICY"
#define QOSCLSPOLICYPRERWAN     "PRE_QOS_CLS_POLICY_WAN"
#define QOSCLSPOLICYPREBOOST     "PRE_QOS_CLS_POLICY_BOOST"
#ifdef CONFIG_SUPPORT_FON
#define QOSCLSPOLICYPRERFON     "PRE_QOS_CLS_POLICY_FON"
#endif
#define QOSCLSPOLICYPRERWLAN     "PRE_QOS_CLS_POLICY_WLAN"
#define QOSCLSPOLICYPRERWLAN_MAIN     "PRE_QOS_CLS_WLAN_MAIN"
#define QOSCLSPOLICYPRERWLAN_GUEST     "PRE_QOS_CLS_WLAN_GUEST"
#define QOSCLSPOLICYPOST 	"POST_QOS_CLS_POLICY"
#define QOSCLSPOLICYPOSTWAN     "POST_QOS_CLS_POLICY_WAN"
#define QOSCLSPOLICYOUT 	"OUTPUT_QOS_CLS_POLICY"

#define QOS_CLS_EBT_APPEND_PREFIX 		    "/usr/sbin/ebtables -t broute -A "QOSCLSPOLICY" "
#define QOS_CLS_EBT_APPEND_POSTFIX          "/usr/sbin/ebtables -t nat -A "QOSCLSPOLICY" "

#define QOS_CLS_IPT_APPEND_PRER   		    "/usr/sbin/iptables -t mangle -A "QOSCLSPOLICYPRER" "
#define QOS_CLS_IPT_APPEND_PRER_WAN   	    "/usr/sbin/iptables -t mangle -A "QOSCLSPOLICYPRERWAN" "
#define QOS_CLS_IPT_APPEND_PRER_WLAN_MAIN   	"/usr/sbin/iptables -t mangle -A "QOSCLSPOLICYPRERWLAN_MAIN" "
#define QOS_CLS_IPT_APPEND_PRER_WLAN_GUEST   	"/usr/sbin/iptables -t mangle -A "QOSCLSPOLICYPRERWLAN_GUEST" "
#define QOS_CLS_IPT_APPEND_POST   		    "/usr/sbin/iptables -t mangle -A "QOSCLSPOLICYPOST" "
#define QOS_CLS_IPT_APPEND_POST_WAN   	    "/usr/sbin/iptables -t mangle -A "QOSCLSPOLICYPOSTWAN" "
#define QOS_CLS_IPT_APPEND_OUT   		    "/usr/sbin/iptables -t mangle -A "QOSCLSPOLICYOUT" "
#ifdef CONFIG_SUPPORT_IPV6
#define QOS_CLS_IP6T_APPEND_PRER   		    "/usr/sbin/ip6tables -t mangle -A "QOSCLSPOLICYPRER" "
#define QOS_CLS_IP6T_APPEND_PRER_WAN   	    "/usr/sbin/ip6tables -t mangle -A "QOSCLSPOLICYPRERWAN" "
#define QOS_CLS_IP6T_APPEND_PRER_WLAN_MAIN   	"/usr/sbin/ip6tables -t mangle -A "QOSCLSPOLICYPRERWLAN_MAIN" "
#define QOS_CLS_IP6T_APPEND_PRER_WLAN_GUEST   	"/usr/sbin/ip6tables -t mangle -A "QOSCLSPOLICYPRERWLAN_GUEST" "
#define QOS_CLS_IP6T_APPEND_POST   		    "/usr/sbin/ip6tables -t mangle -A "QOSCLSPOLICYPOST" "
#define QOS_CLS_IP6T_APPEND_POST_WAN   	    "/usr/sbin/ip6tables -t mangle -A "QOSCLSPOLICYPOSTWAN" "
#define QOS_CLS_IP6T_APPEND_OUT   		    "/usr/sbin/ip6tables -t mangle -A "QOSCLSPOLICYOUT" "
#endif

#define QOS_UPDATE_WAN_CHANGE   "wan_change"

/* GPON OUTPUT QUEUE */
#define MAX_USER_DEFINED_QUEUE 4
#define QOS_WANID_TO_RCL_WANID(id) (id)
#define RCL_WANID_TO_QOS_WANID(id) (id)

#endif

