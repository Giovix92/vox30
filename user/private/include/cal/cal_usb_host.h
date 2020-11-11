#ifdef  _CAL_USB_HOST_H_
#define _CAL_USB_HOST_H_

int cal_usb_host_set_device_product_id(char* value, int host_index, int device_index);
int cal_usb_host_set_device_vendor_id(char* value, int host_index, int device_index);
int cal_usb_host_set_device_manufacturer(char* value, int host_index, int device_index);
int cal_usb_host_set_device_product_class(char* value, int host_index, int device_index);
int cal_usb_host_set_device_serial_number(char* value, int host_index, int device_index);
int cal_usb_host_set_device_usb_version(char* value, int host_index, int device_index);
int cal_usb_host_set_device_device_class(char* value, int host_index, int device_index);
int cal_usb_host_set_device_device_subclass(char* value, int host_index, int device_index);
int cal_usb_host_set_device_device_protocol(char* value, int host_index, int device_index);
int cal_usb_host_set_device_rate(char* value, int host_index, int device_index);
int cal_usb_host_add_device_entry(int host_index, int *device_index);
int cal_usb_host_del_device_entry(int host_index, int device_index);
int cal_usb_host_del_last_device_entry(int host_index);
int cal_usb_host_del_all_device_entry(int host_index);

#endif
