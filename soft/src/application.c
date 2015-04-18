#include "ch.h"
#include "hal.h"
#include "application.h"
#include "hug_sensors.h"
#include "hand_sensors.h"
#include "camera.h"
#include "led.h"
#include "ff.h"
#include "wifi_manager.h"

/* Working area hands */
static WORKING_AREA(waHands, 512);

/*
 * Thread for hands sensors
 */

static msg_t handsThread(void * args) {
    (void) args;
    uint32_t readValues;
    uint16_t * low = (uint16_t *) &readValues;
    uint16_t * high = low + 1;

    while (1) {
        int timeBegin = chTimeNow();
        readValues = getHandValues();

        /* While pressed, wait */
        while (*low > 300 && *high > 300){
            readValues = getHandValues();
            chThdSleepMilliseconds(100);
        }
        
        /* If pressed lon enought, photo */
        if(chTimeNow() - timeBegin > 5000){
            /* Turn on led */
            ledSetColorRGB(0, 255, 255, 255);
            photo("photo.jpg");
            uploadFile("kudly.herokuapp.com/sendimage", "photo.jpg",
                "photo.jpg");
            f_unlink("photo.jpg");
            ledSetColorRGB(0, 0, 0, 0);
        }
        chThdSleepMilliseconds(1000);
    }
    return 0;
}

/*
 * Initializes the threads
 */
void applicationInit() {
    chThdCreateStatic(waHands, sizeof(waHands), NORMALPRIO, handsThread, NULL); 
}
