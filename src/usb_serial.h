#ifndef _USB_SERIAL_H_
#define _USB_SERIAL_H_

#include <stdint.h>

extern void init_usb_serial(void); 
extern void usb_send_string(char *data);
extern void usb_send_data(char *data, uint16_t size);
extern void usb_printf(char *fmt, ...);

#endif
