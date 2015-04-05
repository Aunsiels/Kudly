#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "string.h"
#include "chprintf.h"
#include "usb_serial.h"

// Mailbox for received data
static msg_t mb_buf[32];
MAILBOX_DECL(mb, mb_buf, 32);

char wifi_buffer[1];
char c;

static SerialConfig uartCfg =
{
    115200,
    0,
    0,
    0// bit rate
};

static msg_t usartRead_thd(void * args) {
    (void)args;

    while(1) {
        if(chMBFetch(&mb, (msg_t *)&c, TIME_INFINITE) == RDY_OK) {
            writeSerial("%c", c);

            /*
             * Then send byte to the codec, the SD card...
             */
        }
    }

    return 0;
}

static msg_t usartReadInMB_thd(void * args) {
    (void)args;

    while(1) {
        sdRead(&SD3,(uint8_t *) wifi_buffer, 1);
        chMBPost(&mb, wifi_buffer[0], TIME_INFINITE);
    }

    return 0;
}


void wifiInitByUsart(void){  
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_TX, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_RX, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_CTS, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_RTS, PAL_MODE_ALTERNATE(7));
    sdStart(&SD3, &uartCfg);
}

void wifiWriteByUsart(char * message, int length){
    sdWrite(&SD3, (uint8_t*)message, length); 
}

void wifiStopByUsart(void){
    sdStop(&SD3);
}

void wifiReadByUsartTimeout(int timeout){
    (void)timeout;
    sdReadTimeout(&SD3,(uint8_t *) wifi_buffer, 1, timeout);
}

void wifiReadByUsart(void){
    sdRead(&SD3,(uint8_t *) wifi_buffer, 256);
}

void wifiUsartRead(void) {
    static WORKING_AREA(usartRead_wa, 128);
    static WORKING_AREA(usartReadInMB_wa, 128);

    chThdCreateStatic(
            usartReadInMB_wa, sizeof(usartReadInMB_wa),
            NORMALPRIO, usartReadInMB_thd, NULL);

    chThdCreateStatic(
            usartRead_wa, sizeof(usartRead_wa),
            NORMALPRIO, usartRead_thd, NULL);
}

/*
 * wifi command for shell
 */
void cmdWifi(BaseSequentialStream *chp, int argc, char *argv[]) {

    //static int startCmd, endCmd;
    (void)argv;

    static int i;
    static char space[1] = " ";
    static char end[2] = "\n\r";

    // Wrong command
    if(argc == 0) {
        chprintf(chp, "Usage : wifi command\r\n");
        return;
    }

    /* 
     * Parsing command & sending to wifi
     */
        for(i = 0 ; i < argc ; i++) {
            wifiWriteByUsart(argv[i], strlen(argv[i]));
            wifiWriteByUsart(space, 1);
        }

        wifiWriteByUsart(end, 2);

    return;
}
