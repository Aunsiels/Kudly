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
#include "pir.h"

/* Working area hands */
static WORKING_AREA(waHands, 1024);
/* Working area hug */
static WORKING_AREA(waHug, 128);
/* Working area temperature */
static WORKING_AREA(waTemp, 128);
/* Working area pir */
static WORKING_AREA(waPir, 128);
/* Working area cry */
static WORKING_AREA(waCry, 1024);
/* Working area educ color */
static WORKING_AREA(waEducColor, 128);
/* Working area educ letters */
static WORKING_AREA(waEducLetters, 1024);

static MUTEX_DECL(appliMtx);

static char * pirSound = "pir.ogg";

static char value[] = "value=";
static char temperatureSend[12];


static msg_t pirThread(void * args) {
    (void) args;

    /* Event listener for change */
    EventListener el;

    while(1){
        int res = palReadPad(GPIOD,GPIOD_PIR);
        if (res){
            cmdPlay((BaseSequentialStream *) &SDU1, 1, &pirSound);
            postAndRead("kudly.herokuapp.com/presence","value=1");
            chThdSleepSeconds(60);
        } else {
            postAndRead("kudly.herokuapp.com/presence","value=0");
        }
        chEvtRegisterMask(&pirEvent, &el,EVENT_MASK(1));
        chEvtWaitOne(EVENT_MASK(1));
        chEvtUnregister(&pirEvent, &el);
    }
    return 0;
}

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
        postAndRead("kudly.herokuapp.com/temp", temperatureSend);
        chThdSleepSeconds(600);
    }
    return 0;
}

static char * kuddle = "hum.ogg";

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

	chThdSleepMilliseconds(1000);

        readValues = getHugValues();

        if (*low > *lowF + 100 || *low < *lowF - 100 ||
	    *high > *highF + 100 || *high < *highF - 100){
	    if(!chMtxTryLock(&appliMtx))
		continue;
            cmdPlay((BaseSequentialStream *) &SDU1, 1, &kuddle);
            cmdLedtest((BaseSequentialStream *) &SDU1, 0, NULL);

	    chMtxUnlock();
        }
    }
    return 0;
}

int getSize(stkalign_t * buf, int size){
    uint32_t * b = (uint32_t *) buf;
    for(int i = 0; i < size; i++)
        if (b[i] == 0x55555555) return i*4;
    return -1;
}


static char * photo = "photo.ogg";
/*
 * Thread for hands sensors
 */
static msg_t handsThread(void * args) {
    (void) args;
    uint32_t readValues;
    uint16_t * low = (uint16_t *) &readValues;
    uint16_t * high = low + 1;

    while (1) {

	chThdSleepMilliseconds(1000);

        int timeBegin = chTimeNow();
        readValues = getHandValues();

        /* While pressed, wait */
        while (*low > 300 && *high > 300){
            readValues = getHandValues();
            chThdSleepMilliseconds(100);
        }

        /* If pressed long enought, photo */
        if(chTimeNow() - timeBegin > 5000){
	    if(!chMtxTryLock(&appliMtx))
		continue;
            /* Turn on led */
        ledSetColorRGB(0, 255, 255, 255);
        cmdPlay((BaseSequentialStream *) &SDU1, 1, &photo);
        chThdSleepMilliseconds(3000);
        writeSerial("Begin photo\r\n");
        chThdSleepMilliseconds(100);
        photo("photo.jpg");
        writeSerial("End photo\r\n");
        chThdSleepMilliseconds(1000);
        cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);

	    ledSetColorRGB(0, 0, 0, 0);
	    chMtxUnlock();

        uploadFile("kudly.herokuapp.com/sendimage", "photo.jpg",
                   "photo.jpg");
        f_unlink("photo.jpg");
        }
    }
    return 0;
}

static msg_t cryThread(void * args) {
    (void) args;

    static char itoaBuff[10];
    static char * encodeCry[2] = {"cry.ogg","10"};

    while(1){
	/* Test the volume */
	static char * durationTest[1] = {"1"};
	cmdTestVolume((BaseSequentialStream *) &SDU1, 1, durationTest);
	chThdSleepMilliseconds(200);
	itoa(audioLevel,itoaBuff,10);
	char levelSend[30] = "value=";
	strcat(levelSend,itoaBuff);
	writeSerial("Audio : %s\r\n\r\n",levelSend);
	/* We send the audio level to the server every 10 seconds */
	postAndRead("kudly.herokuapp.com/cry",levelSend);
	/* If we overtake a threeshold, activity is set and we encode sound */

	// TODO set a right threeshold with a good micro

	if(0){
	    writeSerial("\r\n\r\nNOOOOOOOW\r\n\r\n");
	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);
	    postAndRead("kudly.herokuapp.com/activity","value=1");
	    cmdEncode((BaseSequentialStream *) &SDU1, 2, encodeCry);
	    chThdSleepSeconds(10);
	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);
	    writeSerial("Begin upload\r\n");
	    chThdSleepMilliseconds(100);
	    uploadFile("kudly.herokuapp.com/sendimage", "cry.ogg",
		       "cry.ogg");
	    f_unlink("cry.ogg");
	}
	else{
	    postAndRead("kudly.herokuapp.com/activity","value=0");
	}
	chThdSleepSeconds(300);
    }
    return 0;
}

static char * colorRedSound   = "earsRed.ogg";
static char * colorGreenSound = "earsGreen.ogg";
static char * colorBlueSound  = "earsBlue.ogg";
static char * handRedSound    = "handButtonRed.ogg";
static char * handBlueSound   = "handButtonBlue.ogg";
static char * successSound    = "rightAnswer.ogg";
static char * wrongSound      = "badAnswer.ogg";

static msg_t educColorThread(void * args) {
    (void) args;

    uint32_t readValues;
    uint16_t * low = (uint16_t *) &readValues;
    uint16_t * high = low + 1;

    while(1){
	chThdSleepMilliseconds(500);

	int timeBegin = chTimeNow();
	readValues = getHandValues();

        /* While pressed, wait */
        while (*low > 300){
            readValues = getHandValues();
            chThdSleepMilliseconds(100);
        }

        /* If pressed lon enought, photo */
        if(chTimeNow() - timeBegin > 1000 && chTimeNow() - timeBegin < 3000){

	    if(!chMtxTryLock(&appliMtx))
		continue;

	    cmdPlay((BaseSequentialStream *) &SDU1, 1, &colorRedSound);
	    ledSetColorRGB(0, 255, 0, 0);
	    /* Wait the sound to finish "my ears are in red" */
	    chThdSleepSeconds(3);
	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);

	    cmdPlay((BaseSequentialStream *) &SDU1, 1, &colorGreenSound);
	    ledSetColorRGB(0, 0, 255, 0);
	    /* Wait the sound to finish "my ears are in green" */
	    chThdSleepSeconds(3);
	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);

	    cmdPlay((BaseSequentialStream *) &SDU1, 1, &colorBlueSound);
	    ledSetColorRGB(0, 0, 0, 255);
	    /* Wait the sound to finish "my ears are in blue" */
	    chThdSleepSeconds(3);
	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);

	    /* Start color test */
	    cmdPlay((BaseSequentialStream *) &SDU1, 1, &handRedSound);
	    ledSetColorRGB(1, 255, 0, 0);
	    ledSetColorRGB(2, 0, 255, 0);
	    readValues = getHandValues();
	    /* While no pressed, wait */
	    while (*low < 300 && *high < 300){
		chThdSleepMilliseconds(100);
		readValues = getHandValues();
	    }

	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);

	    if(*high > *low){
		cmdPlay((BaseSequentialStream *) &SDU1, 1, &successSound);
		chThdSleepSeconds(3);
	    }
	    else{
		cmdPlay((BaseSequentialStream *) &SDU1, 1, &wrongSound);
		chThdSleepSeconds(3);
	    }

	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);

	    chThdSleepMilliseconds(500);

	    /* Another color test */
	    cmdPlay((BaseSequentialStream *) &SDU1, 1, &handBlueSound);
	    ledSetColorRGB(1, 0, 255, 0);
	    ledSetColorRGB(2, 0, 0, 255);
	    readValues = getHandValues();
	    /* While no pressed, wait */
	    while (*low < 300 && *high < 300){
		readValues = getHandValues();
		chThdSleepMilliseconds(100);
	    }

	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);

	    if(*high > *low){
		cmdPlay((BaseSequentialStream *) &SDU1, 1, &successSound);
		chThdSleepSeconds(3);
	    }
	    else{
		cmdPlay((BaseSequentialStream *) &SDU1, 1, &wrongSound);
		chThdSleepSeconds(3);
	    }

	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);
	    ledSetColorRGB(0, 0, 0, 0);

	    chMtxUnlock();
	}
    }

    return 0;
}

static char * lettersSound  = "alphabetKudly.ogg";
static char * encodeAlphabet[2] = {"alphabetChild.ogg","10"};

static msg_t educLettersThread(void * args) {
    (void) args;

    uint32_t readValues;
    uint16_t * low = (uint16_t *) &readValues;
    uint16_t * high = low + 1;

    while(1){
	chThdSleepMilliseconds(500);

	int timeBegin = chTimeNow();
	readValues = getHandValues();

	/* While pressed, wait */
        while (*high > 300){
            readValues = getHandValues();
            chThdSleepMilliseconds(100);
        }

        /* If pressed lon enought, photo */
        if(chTimeNow() - timeBegin > 1000 && chTimeNow() - timeBegin < 3000){

	    if(!chMtxTryLock(&appliMtx))
		continue;

	    cmdPlay((BaseSequentialStream *) &SDU1, 1, &lettersSound);
	    chThdSleepSeconds(13);

	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);
	    cmdEncode((BaseSequentialStream *) &SDU1, 2, encodeAlphabet);
	    chThdSleepMilliseconds(10100);
	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);

	    chMtxUnlock();

	    uploadFile("kudly.herokuapp.com/sendimage", "alphabetChild.ogg",
		       "alphabetChild.ogg");
	    f_unlink("alphabetChild.ogg");
	}
    }
    return 0;
}

/*
 * Initializes the threads
 */
void applicationInit() {
    chMtxInit(&appliMtx);
    chThdCreateStatic(waHands, sizeof(waHands), NORMALPRIO, handsThread, NULL);
    chThdCreateStatic(waHug, sizeof(waHug), NORMALPRIO, hugThread, NULL);
    chThdCreateStatic(waTemp, sizeof(waTemp), NORMALPRIO, tempThread, NULL);
    chThdCreateStatic(waPir, sizeof(waPir), NORMALPRIO, pirThread, NULL);
    chThdCreateStatic(waCry, sizeof(waCry), NORMALPRIO, cryThread, NULL);
    chThdCreateStatic(waEducColor, sizeof(waEducColor), NORMALPRIO, educColorThread, NULL);
    chThdCreateStatic(waEducLetters, sizeof(waEducLetters), NORMALPRIO, educLettersThread, NULL);
}
