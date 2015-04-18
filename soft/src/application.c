#include "ch.h"
#include "hal.h"
#include "application.h"
#include "hug_sensors.h"
#include "hand_sensors.h"
#include "camera.h"
#include "led.h"
#include "ff.h"
#include "wifi_manager.h"
#include "usb_serial.h"
#include "codec.h"
#include "wifi_parsing.h"
#include <string.h>
#include "temperature.h"
#include <stdlib.h>

/* Working area hands */
static WORKING_AREA(waHands, 1024);
/* Working area hug */
static WORKING_AREA(waHug, 128);
/* Working area hug */
static WORKING_AREA(waPhoto, 1024);
/* Working area temperature */
static WORKING_AREA(waTemp, 128);

static char * kuddle = "kuddle.ogg";

static char value[] = "value=";
static char temperatureSend[12];

/*
 * Thread for temperature
 */
static msg_t tempThread(void * args) {
    (void) args;
    temperatureSend[0] = 0;
    strcat(temperatureSend, value);
    while (1){
        int temperature = (int) getTemperatureNotHandled();
        itoa(temperature, temperatureSend+6,10);
        postAndRead("/temp", temperatureSend);
        chThdSleepSeconds(600);
    }
    return 0;
}

/*
 * Thread for hug
 */
static msg_t hugThread(void * args){
    (void) args;
    uint32_t formerValue;
    uint16_t * lowF = (uint16_t *) &formerValue;
    uint16_t * highF = lowF + 1;
    uint32_t readValues;
    uint16_t * low = (uint16_t *) &readValues;
    uint16_t * high = low + 1;

    formerValue = getHugValues();
    while(1) {
        readValues = getHugValues();
        if (*low > *lowF + 100 || *low < *lowF - 100 ||
                *high > *highF + 100 || *high < *highF - 100){
            cmdPlay((BaseSequentialStream *) &SDU1, 1, &kuddle); 
            cmdLedtest((BaseSequentialStream *) &SDU1, 0, NULL); 
        }
        chThdSleepMilliseconds(1000);   
    }
    return 0;
}

int getSize(stkalign_t * buf, int size){
    uint32_t * b = (uint32_t *) buf;
    for(int i = 0; i < size; i++)
        if (b[i] == 0x55555555) return i*4;
    return -1;
}

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
            writeSerial("Begin photo\r\n");
            chThdSleepMilliseconds(100);
            photo("photo.jpg");
            writeSerial("End photo\r\n");
            chThdSleepMilliseconds(100);
            uploadFile("kudly.herokuapp.com/sendimage", "photo.jpg",
                "photo.jpg");
            f_unlink("photo.jpg");
            ledSetColorRGB(0, 0, 0, 0);
        }
        chThdSleepMilliseconds(1000);
    }
    return 0;
}

static EventListener eventPhotoLst;
static char * photoSong ="photo.ogg";

static msg_t photoThread(void * args) {
    (void) args;

    chEvtRegisterMask(&eventPhotoSrc, &eventPhotoLst, EVENT_MASK(1));

    while (1) {
	chEvtWaitOne(EVENT_MASK(1));
	cmdPlay((BaseSequentialStream *) &SDU1, 1, &photoSong);
	chThdSleepMilliseconds(3000);

        /* Turn on led */
	ledSetColorRGB(0, 255, 255, 255);
	writeSerial("Begin photo\r\n");
	chThdSleepMilliseconds(100);
	/* Take photo */
	photo("photo.jpg");
	writeSerial("End photo\r\n");
	chThdSleepMilliseconds(100);
	uploadFile("kudly.herokuapp.com/sendimage", "photo.jpg",
		   "photo.jpg");
	f_unlink("photo.jpg");
	/* Turn off led */
	ledSetColorRGB(0, 0, 0, 0);
    }
    return 0;
}

/*
 * Initializes the threads
 */
void applicationInit() {
    chThdCreateStatic(waHands, sizeof(waHands), NORMALPRIO, handsThread, NULL); 
    chThdCreateStatic(waHug, sizeof(waHug), NORMALPRIO, hugThread, NULL);
    chThdCreateStatic(waPhoto, sizeof(waPhoto), NORMALPRIO, photoThread, NULL); 
    chThdCreateStatic(waTemp, sizeof(waTemp), NORMALPRIO, tempThread, NULL); 
}
