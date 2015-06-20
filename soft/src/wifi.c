#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "string.h"
#include "chprintf.h"
#include "usb_serial.h"
#include "led.h"
#include <stdio.h>
#include <stdlib.h>
#include "wifi_manager.h"
#include "wifi_parsing.h"

/* Is streaming activated ? */
volatile int streaming = 0;

/* Event listener for the end of reading by usart */
static EventListener lstEndToReadUsart;

/* Mailbox for received data */
static msg_t mb_buf[10000];
MAILBOX_DECL(mbReceiveWifi, mb_buf, sizeof(mb_buf)/sizeof(msg_t));

/* Special strings to print */
static char crlf[]  = "\r\n";
static char space[] = " ";
static char cmdMessage[120];

/* Char and buffer used by wifi receive */
static char wifi_buffer;

/* Some string used by initialization to configure network */
static char ssid[]    = "set wlan.ssid \"Bob\"\r\n";
static char passkey[] = "set wlan.passkey \"Archibald\"\r\n";
static char save[]    = "save\r\n";

/* http request on Kudly website */
static char cfg_echoOff[]     = "set system.cmd.echo off\r\n";
static char cfg_printLevel0[] = "set system.print_level 0\r\n";
static char cfg_promptOff[]   = "set system.cmd.prompt_enabled 0\r\n";
static char cfg_headersOn[]   = "set system.cmd.header_enabled 1\r\n";

/* String for lowpower mode */
static char wakeUp[] = "set system.wakeup.events gpio22\r\n";
static char sleep[]  = "sleep\r\n";

static char dollar[] = "$$$\r\n";

/* Streaming command */
static char busMode[]        = "set bus.mode command\r\n";
static char autoJoin[]       = "set wlan.auto_join.enabled 1\r\n";
static char remoteHost[]     = "set tcp.client.remote_host 192.168.1.105\r\n";
static char remotePort[]     = "set tcp.client.remote_port 9000\r\n";
static char autoInterface[]  = "set tcp.client.auto_interface wlan\r\n";
static char autoRetries[]    = "set tcp.client.auto_retries 255\r\n";
static char autoStart[]      = "set tcp.client.auto_start 0\r\n";
static char keepAlive[]      = "set tcp.keepalive.enabled 1\r\n";
static char initialTimeout[] = "set tcp.keepalive.initial_timeout 10\r\n";
static char ratio[]          = "set network.buffer.rxtx_ratio 20\r\n";
static char bufferSize[]     = "set network.buffer.size 40000\r\n";
static char retryTimeout[]   = "set tcp.keepalive.retry_timeout 1\r\n";
static char list[]           = "list\r\n";

/* Mutex for wifi access */
MUTEX_DECL(wifiAccessMtx);
MUTEX_DECL(writeMtx);

/* Serial driver that uses usart3 */
static SerialConfig uartCfg =
{
    230400,
    0,
    0,
    USART_CR3_CTSE | USART_CR3_RTSE
};

/* Thread that reads wifi data and puts it on Mailbox */
static msg_t usartReadInMB_thd(void * args) {
    (void)args;

    EventListener el;
    chEvtRegister(&(SD3.event), &el, EVENT_MASK(1));
    chRegSetThreadName("read");

    while(1) {
        if( sdReadTimeout(&SD3,(uint8_t *) &wifi_buffer, 1, TIME_IMMEDIATE) ==
            0){
            palClearPad(GPIOD,GPIOD_WIFI_UART_RTS);
            sdRead(&SD3,(uint8_t *) &wifi_buffer, 1);
        }
        if(chEvtWaitOneTimeout(EVENT_MASK(1), TIME_IMMEDIATE)) port_halt();
        if(chMBPost(&mbReceiveWifi,(msg_t)wifi_buffer, TIME_IMMEDIATE) !=
            RDY_OK){
            palSetPad(GPIOD,GPIOD_WIFI_UART_RTS);
            chMBPost(&mbReceiveWifi,(msg_t)wifi_buffer, TIME_INFINITE);
        }
    }
    return 0;
}

/* Sends data by wifi */
void wifiWriteByUsart(char * message, int length){
    chMtxLock(&writeMtx);
    chEvtRegisterMask(&srcEndToReadUsart, &lstEndToReadUsart,EVENT_MASK(1));
    sdWrite(&SD3, (uint8_t*)message, length);
    chEvtWaitOne(EVENT_MASK(1));
    chEvtUnregister(&srcEndToReadUsart, &lstEndToReadUsart);
    chMtxUnlock();
}

/* Same as above but don't want to wait for the response */
void wifiWriteNoWait(char * message, int length){
    chMtxLock(&writeMtx);
    sdWrite(&SD3, (uint8_t*)message, length);
    chMtxUnlock();
}

/*  Launches the wifi reading */
static void wifiReadByUsart(void) {
    static WORKING_AREA(usartReadInMB_wa, 2048);

    chThdCreateStatic(
	usartReadInMB_wa, sizeof(usartReadInMB_wa),
	NORMALPRIO + 0, usartReadInMB_thd, NULL);
}

/* Command shell to speak with wifi module in command mode */
void cmdWifi(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)chp;
    int i;
    for(i = 0; i < argc; i++){
	strcat(cmdMessage ,argv[i]);
	strcat(cmdMessage ,space);
    }
    strcat(cmdMessage ,crlf);
    wifiWriteByUsart(cmdMessage, strlen(cmdMessage));
    cmdMessage[0]='\0';
}

/* Initialization of wifi network */
void wifiInitByUsart(void) {
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_TX, PAL_MODE_ALTERNATE(7) |
        PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP |
        PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_RX, PAL_MODE_ALTERNATE(7) |
        PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP |
        PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_CTS, PAL_MODE_ALTERNATE(7) |
        PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP |
        PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_RTS, PAL_STM32_MODE_OUTPUT |
        PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP |
        PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode (GPIOD,GPIOD_WIFI_WAKEUP, PAL_MODE_OUTPUT_PUSHPULL);
    palClearPad (GPIOD,GPIOD_WIFI_WAKEUP);

    /* Start usart 3 */
    sdStart(&SD3, &uartCfg);
    /* Fill mailbox with usart data */
    wifiReadByUsart();

    /* Read wifi by usart */
    usartRead();

    sdStart(&SD3, &uartCfg);
    wifiWriteNoWait(dollar, sizeof(dollar)-1);
    chThdSleepMilliseconds(1000);

    writeSerial("Wifi configuration...\r\n");

    wifiWriteByUsart(autoJoin,       sizeof(autoJoin)       - 1);
    wifiWriteByUsart(remoteHost,     sizeof(remoteHost)     - 1);
    wifiWriteByUsart(remotePort,     sizeof(remotePort)     - 1);
    wifiWriteByUsart(autoInterface,  sizeof(autoInterface)  - 1);
    wifiWriteByUsart(autoRetries,    sizeof(autoRetries)    - 1);
    wifiWriteByUsart(autoStart,      sizeof(autoStart)      - 1);
    wifiWriteByUsart(keepAlive,      sizeof(keepAlive)      - 1);
    wifiWriteByUsart(initialTimeout, sizeof(initialTimeout) - 1);
    wifiWriteByUsart(ratio,          sizeof(ratio)          - 1);
    wifiWriteByUsart(bufferSize,     sizeof(bufferSize)     - 1);
    wifiWriteByUsart(retryTimeout,   sizeof(retryTimeout)   - 1);
    wifiWriteByUsart(busMode,        sizeof(busMode)        - 1);

    wifiWriteByUsart(cfg_echoOff,     sizeof(cfg_echoOff)     - 1);
    wifiWriteByUsart(cfg_printLevel0, sizeof(cfg_printLevel0) - 1);
    wifiWriteByUsart(cfg_headersOn,   sizeof(cfg_headersOn)   - 1);
    wifiWriteByUsart(cfg_promptOff,   sizeof(cfg_promptOff)   - 1);
    wifiWriteByUsart(wakeUp,          sizeof(wakeUp)          - 1);
    wifiWriteByUsart(ssid,            sizeof(ssid)            - 1);
    wifiWriteByUsart(passkey,         sizeof(passkey)         - 1);
    wifiWriteByUsart(save,            sizeof(save)            - 1);

    /* Loop to test the network connection */
    bool_t state;
    while(TRUE){
	state = wifiNup();
	if (state == TRUE){
	    ledSetColorRGB(0, 0, 255, 0);
	    break;
	}
	else
	    ledSetColorRGB(0, 255, 0, 0);

	chThdSleepMilliseconds(500);
	ledSetColorRGB(0, 0, 0, 0);
	chThdSleepMilliseconds(500);
    }
    chThdSleepMilliseconds(500);
    ledSetColorRGB(0, 0, 0, 0);
    wifiWriteByUsart(list , sizeof(list)-1);
    writeSerial("Wifi ready to use\r\n");

    /*
     * Configuring wifi module in machine friendly command mode
     * cf : http://wiconnect.ack.me/2.1/serial_interface#configuration
     */
}

static msg_t wifiSleep(void){
    writeSerial("Wifi is sleeping\r\n");
    wifiWriteByUsart(sleep, sizeof(sleep)-1);
    return 0;
}

static void wifiWakeUp(void){
    palSetPad (GPIOD,GPIOD_WIFI_WAKEUP);
    chThdSleepMilliseconds(100);
    palClearPad (GPIOD,GPIOD_WIFI_WAKEUP);
    writeSerial("Wifi is woke up\r\n");
}

/* Command to sleep the wifi module */
void cmdWifiSleep(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)chp;
    (void)argc;
    (void)argv;
    wifiSleep();
}

/* Command to wake up the wifi module */
void cmdWifiWakeUp(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)chp;
    (void)argc;
    (void)argv;
    wifiWakeUp();
}
