#include "hal.h"
#include "ch.h"
#include "pir.h"

EVENTSOURCE_DECL(pirEvent);

/*
 * Callback function for the pir
 */
static void extPir(EXTDriver * extp, expchannel_t channel){
    (void) extp;
    (void) channel;
    chSysLockFromIsr();
    chEvtBroadcastI(&pirEvent);
    chSysUnlockFromIsr();
}

/*
 * Pir interrupt configuration
 */
static EXTChannelConfig config[] = {EXT_CH_MODE_BOTH_EDGE 
                                  | EXT_CH_MODE_AUTOSTART
                                  | EXT_MODE_GPIOD, 
                                  extPir};

void pirInit(){
    palSetPadMode(GPIOD,14,PAL_MODE_INPUT_PULLDOWN);
    chSysLock();
    extSetChannelModeI(&EXTD1,14, config);
    chSysUnlock();
}
