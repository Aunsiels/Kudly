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
#include "camera.h"
#include "codec.h"
#include "hug_sensors.h"
#include "hand_sensors.h"
#include "codec.h"
#include "wifi.h"
#include "wifi_manager.h"
#include "imu.h"
#include "temperature.h"
#include "pir.h"

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define SHELL_MAX_ARGUMENTS 5

/* Thread of the shell */
static Thread *shelltp = NULL;

/* Working area of the shell controler */
static WORKING_AREA(waShellController, 256);

/* Command to test the shell */
static void cmdTest(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;
    (void)chp;

    if (argc > 0) {
        writeSerial( "Usage : test\r\n");
        return;
    }
    writeSerial( "The shell seems to work :)\r\n");
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
    {"led"         , cmdLed        },
    {"ledtest"     , cmdLedtest    },
    {"hugsensors"  , cmdHugSensors },
    {"handsensors" , cmdHandSensors},
    {"play"        , cmdPlay       },
    {"encode"      , cmdEncode     },
    {"stop"        , cmdStop       },
    {"volume"      , cmdVolume     },
    {"testVolume"  , cmdTestVolume },
    {"c"           , cmdControl    },
    {"getwifi"     , cmdWifiGet    },
    {"postwifi"    , cmdWifiPost   },
    {"uploadwifi"  , cmdWifiUpload },
    {"parsewifi"   , cmdWifiXml    },
    {"sleepwifi"   , cmdWifiSleep  },
    {"wakeupwifi"  , cmdWifiWakeUp },
    {"temperature" , cmdTemperature},
    {"imu"         , cmdImu        },
    {"pir"         , testPir       },
    {NULL          , NULL          }
};

/* Config of the shell */
static const ShellConfig shell_cfg1 = {
    (BaseSequentialStream *)&SDU1,
    commands
};

/*
 * Initializes and controls the shell
 */
static msg_t shellController (void *arg) {
    (void) arg;

    /* Initialization of the thread */
    shellInit();
    /* Start the shell */
    while (TRUE) {
        if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE)) {
            shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
        } else if (chThdTerminated(shelltp)) {
            chThdRelease(shelltp);
            shelltp = NULL;
        }
        chThdSleepMilliseconds(1000);
    }
    return 0;
}

/* Initialization */
void shellPersoInit() {
    /* Begins the shell controller thread */
    chThdCreateStatic(waShellController, sizeof(waShellController), NORMALPRIO,
                      shellController, NULL);
}
