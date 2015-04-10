#include "ch.h"
#include "hal.h"
#include "shell_cfg.h"
#include "chprintf.h"
#include "usb_serial.h"
#include "shell.h"
#include "led.h"
#include <stdlib.h>
#include "sd_perso.h"
#include "sccb.h"
#include "wifi.h"
#include "hug_sensors.h"
#include "hand_sensors.h"
#include "codec.h"
#include "camera.h"

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define SHELL_MAX_ARGUMENTS 5

/* Thread of the shell */
static Thread *shelltp = NULL;

/* Command to test the shell */
static void cmdTest(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;

    if (argc > 0) {
        chprintf(chp, "Usage : test\r\n");
        return;
    }
    chprintf(chp, "The shell seems to work :)\r\n");
}

/* List of commands */
static const ShellCommand commands[] = {
    {"camera"      , cmdCamera     },
    {"sccbwrite"   , cmdWrite      },
    {"sccbread"    , cmdRead       },
    {"testSD"      , testSd        },
    {"mv"          , cmdMv         },
    {"rm"          , cmdRm         },
    {"touch"       , cmdTouch      },
    {"mkdir"       , cmdMkdir      },
    {"cat"         , cmdCat        },
    {"pwd"         , cmdPwd        },
    {"cd"          , cmdCd         },
    {"ls"          , cmdLs         },
    {"test"        , cmdTest       },
    {"tree"        , cmdTree       },
    {"wifi"        , cmdWifi       },
    {"wifiTest"    , cmdWifiTest   },
    {"led"         , cmdLed        },
    {"ledtest"     , cmdLedtest    },
    {"hugsensors"  , cmdHugSensors },
    {"handsensors" , cmdHandSensors},
    {"play"        , cmdPlay       },
    {"encode"      , cmdEncode     },
    {NULL          , NULL          }
};

/* Config of the shell */
static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};

/* Initialization */
void shellPersoInit(){
    /* Initialization of the thread */
    shellInit();
    /* Start the shell */
    while (!(!shelltp && (SDU1.config->usbp->state == USB_ACTIVE)))
        chThdSleepMilliseconds(1000);
    if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE)){
        shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    }
}
