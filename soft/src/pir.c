#include "hal.h"
#include "ch.h"
#include "pir.h"


/*
 * Pir interrupt configuration
 */
static EXTChannelConfig config[] = {EXT_CH_MODE_BOTH_EDGE 
                                  | EXT_CH_MODE_AUTOSTART
                                  | EXT_MODE_GPIOD, 
                                  extPir},

/*
 * Configuration of the EXT
 * TODO : when others ext sources, init in an other file
 */
static EXTConfig extcfg = {
  {
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL}
  }
};

void pirInit(){
    palSetPadMode(GPIOD,14,PAL_MODE_INPUT_PULLDOWN);
    chSysLock();
    extSetChannelModeI(&EXTD1,14, config);
    chSysUnlock();
}
