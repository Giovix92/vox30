#ifndef __EVENT_DEFINITIONS_H__
#define __EVENT_DEFINITIONS_H__

#define EVENT_WAN_OBJECT_PREFIX "event.wan."
#define EVENT_WAN_ENABLE_REMOTEACCESS "event.wan.remoteaccess.enable"
#define EVENT_WAN_ACTIVE_REMOTEACCESS "event.wan.remoteaccess.active"
#define EVENT_WAN_DEACTIVE_REMOTEACCESS "event.wan.remoteaccess.deactive"
#define EVENT_WAN_ENABLE_CGN "event.wan.cgn.enable"
#define EVENT_WAN_DISABLE_CGN "event.wan.cgn.disable"
#define EVENT_WAN_ACTIVE_CGN "event.wan.cgn.active"
#define EVENT_WAN_DEACTIVE_CGN "event.wan.cgn.deactive"
#define EVENT_WAN_DETECT_START "event.wan.detect.start"
#define EVENT_WAN_UPDATE_CGN_NOTIFICATIONDELAY "event.wan.update.cgnnotificationdelay"
#define EVENT_OBJECT "event"
#define EVENT_TYPE_NEWCONNECTED "NewConnected"
#define EVENT_TYPE_NEWDHCPOPT "NEWDHCPOPT"
#define EVENT_TYPE_DEVICECONNECTED "DeviceConnected"
#define EVENT_TYPE_MISSEDCALL "MissedCall"
#define EVENT_TYPE_INCOMINGCALL "IncomingCall"
#define EVENT_TYPE_NEWMESSAGE "NewMessage"
#define EVENT_TYPE_PUSHNOTIFY_ENABLE "NotifyEnable"
#define EVENT_TYPE_GUESTEXPIRE "GuestExpire"
#define EVENT_TYPE_WPSPAIRING "WPSPairing"
#define EVENT_TYPE_NTPSTATUS "NTPStatus"
#define EVENT_TYPE_WANSTATUS "WANStatus"
#define EVENT_NOTIFICATION_OBJECT "v1.Notification"
#define EVENT_LIST_OBJECT "v1.Notification.Event"
#define EVENT_WAN_CHANGE "wan.change"
#ifdef CONFIG_SUPPORT_PRPL_HL_API
#define EVENT_HOST_NEEDUPDATE "host.needupdate"
#define EVENT_BUTTON_ACTION "button.action"
#endif
#endif

