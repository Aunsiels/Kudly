#include "ch.h"
#include "hal.h"
#include "shell_cfg.h"
#include "chprintf.h"
#include "usb_serial.h"
#include "shell.h"
#include <stdlib.h>
#include "sd_perso.h"
#include "ff.h"

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)

//Thread of the shell
Thread *shelltp = NULL;

//List of commands
static const ShellCommand commands[] = {
  {"testSD" , testSd     },
  {"mv"     , cmdMv      },
  {"rm"     , cmdRm      },
  {"touch"  , cmdTouch   },
  {"mkdir"  , cmdMkdir   },
  {"cat"    , cmdCat     },
  {"pwd"    , cmdPwd     },
  {"cd"     , cmdCd      },
  {"ls"     , cmdLs      },
  {"tree"   , cmdTree    },
  {NULL     , NULL       }
};

//Config of the shell
static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU2,
  commands
};

//Initialization
void shell_init(){
    //Initialization of the thread
    shellInit();
    //Start the shell
    while (!(!shelltp && (SDU2.config->usbp->state == USB_ACTIVE)))
        chThdSleepMilliseconds(1000);
    if (!shelltp && (SDU2.config->usbp->state == USB_ACTIVE)){
        shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    }
}
