#include "hal.h"
#include "ch.h"
#include "pir.h"
#include "chprintf.h"

/* Event for the pir */
static EVENTSOURCE_DECL(pirEvent);

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
static EXTChannelConfig config[] = 
    {{EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART  | EXT_MODE_GPIOD, extPir}};

/*
 * Initialization function
 */
void pirInit(){
    palSetPadMode(GPIOD,14,PAL_MODE_INPUT_PULLDOWN);
    extSetChannelMode(&EXTD1,14, config);
}

/*
 * Tests if the pir works, to use in a terminal. Does 10 times a change
 */
void testPir(BaseSequentialStream *chp, int argc, char *argv[]){
    (void) argv;
    if (argc > 0){
        chprintf(chp, "Usage : pir\r\n");
        return;
    }
    /* Event listener for change */
    EventListener el;
    eventmask_t check;
    /* Register event */
    chEvtRegisterMask(&pirEvent, &el,0);
    int i;
    for(i = 0; i <10 ; ++i){
        /* Wait for a change for 10 seconds */
        int res = palReadPad(GPIOD,14);
        if (res){
            chprintf(chp, "Current : mouvement detected\r\n");
        } else {
            chprintf(chp, "Current : mouvement stopped\r\n");
        }
        chprintf(chp, "Please change state");
        check = chEvtWaitOneTimeout(0,MS2ST(10000));
        if (check == 0) {
            chprintf(chp, "The operation timed out\r\n");
            break;
        } else {
            res = palReadPad(GPIOD,14);
            if (res){
                chprintf(chp, "Mouvement detected\r\n");
            } else {
                chprintf(chp, "Mouvement stopped\r\n");
            }
        }
    }
    if (i == 10){
        chprintf(chp, "The 10 changes have been detected\r\n");
    } else {
        chprintf(chp, "Some changes were not detected\r\n");
    }
}
