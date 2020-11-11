#ifndef __UEVENT_UTIL_H__
#define __UEVENT_UTIL_H__
#if defined(SUPPORT_USB_STORAGE) || defined(SUPPORT_USB_PRINTER) || defined(CONFIG_SUPPORT_3G)  
int uevent_register_usb_state(const char * buf);
int uevent_update_usb_state(int bus, char *port);
int uevent_update_usblp_state(int bus, char *port, char *name);
#endif
#endif
