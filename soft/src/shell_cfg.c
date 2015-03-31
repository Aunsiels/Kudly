#include "ch.h"
#include "hal.h"
#include "shell_cfg.h"
#include "chprintf.h"
#include "usb_serial.h"
#include "shell.h"
#include <stdlib.h>

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)

/* Thread of the shell */
static Thread *shelltp = NULL;

static uint8_t receivedDatas[1];

/* Command to test the shell */
static void cmd_toggle(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;

    if (argc > 0) {
        chprintf(chp, "Usage : test\r\n");
        return;
    }
    chprintf(chp, "The shell seems to work :)");
}

/* List of commands */
static const ShellCommand commands[] = {
  {NULL     , NULL       }
};

/* Config of the shell */
static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU2,
  commands
};

/* Initialization */
void shell_init(){
    /* Initialization of the thread */
    shellInit();
    /* Start the shell */
    while (!(!shelltp && (SDU2.config->usbp->state == USB_ACTIVE)))
        chThdSleepMilliseconds(1000);
    if (!shelltp && (SDU2.config->usbp->state == USB_ACTIVE)){
        shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    }
}
