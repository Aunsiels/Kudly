#ifndef USB_SERIAL_H
#define USB_SERIAL_H

//The serial driver of the usb
extern SerialUSBDriver SDU2;

//Initialize the serial communication
void init_usb_serial (void);

//Thread that receives the datas
extern Thread * USBThread;

//Working area of the usb
extern WORKING_AREA(wa_usb, 128);

//USB Thread function
msg_t usb_thread (void * p);

#endif
