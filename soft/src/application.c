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
#include "websocket.h"

/* Working area hands */
static WORKING_AREA(waHands, 1024);
/* Working area hug */
static WORKING_AREA(waHug, 1024);
/* Working area temperature */
static WORKING_AREA(waTemp, 1024);
/* Working area pir */
static WORKING_AREA(waPir, 1024);
/* Working area cry */
static WORKING_AREA(waCry, 1024);
/* Working area educ color */
static WORKING_AREA(waEducColor, 1024);
/* Working area educ letters */
static WORKING_AREA(waEducLetters, 1024);
/* Working area educ letters */
static WORKING_AREA(waXmlPolling, 1024);
/* Working area streaming */
static WORKING_AREA(waStreaming, 1024);
/* Working area Story */
static WORKING_AREA(waStory, 1024);

static MUTEX_DECL(appliMtx);

static char * pirSound = "pir.ogg";

static char value[] = "value=";
static char temperatureSend[12];

static msg_t pirThread(void * args) {
    (void) args;

    /* Event listener for change */
    EventListener el;

    while(1) {
	chMtxLock(&appliMtx);
	writeSerial("Pir ...\r\n");
        int res = palReadPad(GPIOD,GPIOD_PIR);
        if (res) {
            cmdPlay((BaseSequentialStream *) &SDU1, 1, &pirSound);
            postAndRead("192.168.1.105:9000/presence","value=1");
	    writeSerial("End ...\r\n");
	    chMtxUnlock();
            chThdSleepSeconds(60);
        } else {
            postAndRead("192.168.1.105:9000/presence","value=0");
	    writeSerial("End ...\r\n");
	    chMtxUnlock();
            chThdSleepSeconds(60);
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
	chMtxLock(&appliMtx);
	writeSerial("Temperature ...\r\n");
        int temperature = (int) getTemperatureNotHandled();
        itoa(temperature, temperatureSend+6,10);
        postAndRead("192.168.1.105:9000/temp", temperatureSend);
	chMtxUnlock();
        chThdSleepSeconds(600);
	writeSerial("End ...\r\n");
    }
    return 0;
}

static char * kuddle = "hum.ogg";

/*
 * Thread for hug
 */
static msg_t hugThread(void * args) {
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
            writeSerial("Hug ...\r\n");
            cmdPlay((BaseSequentialStream *) &SDU1, 1, &kuddle);
            cmdLedtest((BaseSequentialStream *) &SDU1, 0, NULL);
            writeSerial("end hug \r\n");

	    chMtxUnlock();
	    writeSerial("End ...\r\n");
        }


    }
    return 0;
}

int getSize(stkalign_t * buf, int size) {
    uint32_t * b = (uint32_t *) buf;
    for(int i = 0; i < size; i++)
        if (b[i] == 0x55555555) return i*4;
    return -1;
}

static char * photoSound = "photo.ogg";

/*
 * Thread for hands sensors
 */
static msg_t handsThread(void * args) {
    (void) args;
    chRegSetThreadName("Photo");
    static EventListener eventPhotoLst;
    chEvtRegisterMask(&eventPhotoSrc , &eventPhotoLst ,EVENT_MASK(1));

    while (1) {
        chEvtWaitOne(EVENT_MASK(1));
        chMtxLock(&appliMtx);

        cmdPlay((BaseSequentialStream *) &SDU1, 1, &photoSound);
        chThdSleepSeconds(4);
        cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);

        /* Turn on led */
        ledSetColorRGB(0, 255, 255, 255);
        writeSerial("Begin photo\r\n");
        chThdSleepMilliseconds(100);
        photo("photo.jpg");
        writeSerial("End photo\r\n");
        chThdSleepMilliseconds(100);

        ledSetColorRGB(0, 0, 0, 0);

        uploadFile("192.168.1.105:9000/sendimage", "photo.jpg",
                   "photo.jpg");
        f_unlink("photo.jpg");
        chMtxUnlock();
        ledSetColorRGB(0, 0, 255, 0);
        chThdSleepMilliseconds(500);
        ledSetColorRGB(0, 0, 0, 0);
    }

    return 0;
}

/*
 * Thread for the cry detection
 */
static msg_t cryThread(void * args) {
    (void) args;

    static char itoaBuff[10];
    static char * encodeCry[2] = {"cry.ogg","10"};

    while(1){
	chMtxLock(&appliMtx);
        writeSerial("Sound ...\r\n");
	/* Test the volume */
	static char * durationTest[1] = {"1"};
	cmdTestVolume((BaseSequentialStream *) &SDU1, 1, durationTest);
	chThdSleepMilliseconds(200);
	itoa(audioLevel,itoaBuff,10);
	char levelSend[30] = "value=";
	strcat(levelSend,itoaBuff);
	writeSerial("Audio : %s\r\n\r\n",levelSend);
	/* We send the audio level to the server every 10 seconds */
	postAndRead("192.168.1.105:9000/cry",levelSend);
	/* If we overtake a threeshold, activity is set and we encode sound */

	// TODO set a right threeshold with a good micro

	if(0){
	    writeSerial("\r\n\r\nNOOOOOOOW\r\n\r\n");
	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);
	    postAndRead("192.168.1.105/activity","value=1");
	    cmdEncode((BaseSequentialStream *) &SDU1, 2, encodeCry);
	    chThdSleepSeconds(10);
	    cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);
	    writeSerial("Begin upload\r\n");
	    chThdSleepMilliseconds(100);
	    uploadFile("192.168.1.105/sendimage", "cry.ogg",
		       "cry.ogg");
	    f_unlink("cry.ogg");
	}
	else{
	    postAndRead("192.168.1.105:9000/activity","value=0");
	}
	chMtxUnlock();
	writeSerial("End ...\r\n");
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

/*
 * The game to teach colors
 */
static msg_t educColorThread(void * args) {
    (void) args;

    uint32_t readValues;
    uint16_t * low = (uint16_t *) &readValues;
    uint16_t * high = low + 1;

    static EventListener eventGameLst;
    chEvtRegisterMask(&eventGameSrc , &eventGameLst ,EVENT_MASK(1));

    while (1) {
	chEvtWaitOne(EVENT_MASK(1));
	chMtxLock(&appliMtx);
	writeSerial("Color game ...\r\n");

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
	writeSerial("End ...\r\n");
    }

    return 0;
}

static char * lettersSound  = "alphabetKudly.ogg";
static char * encodeAlphabet[2] = {"alphabetChild.ogg","10"};

/*
 * Teach letters
 */
static msg_t educLettersThread(void * args) {
    (void) args;
    chRegSetThreadName("Letter");
    static EventListener eventSoundLst;
    chEvtRegisterMask(&eventSoundSrc , &eventSoundLst ,EVENT_MASK(1));

    while (1) {
	chEvtWaitOne(EVENT_MASK(1));
	chMtxLock(&appliMtx);
	writeSerial("eduLetter ...\r\n");

	cmdPlay((BaseSequentialStream *) &SDU1, 1, &lettersSound);
	chThdSleepSeconds(13);

	cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);
	chThdSleepSeconds(1);
	cmdEncode((BaseSequentialStream *) &SDU1, 2, encodeAlphabet);
	chThdSleepMilliseconds(10100);
	cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);

	writeSerial("reacording finished\r\n");
	uploadFile("192.168.1.105:9000/sendimage", "alphabetChild.ogg",
		   "alphabetChild.ogg");
	f_unlink("alphabetChild.ogg");
	ledSetColorRGB(0, 0, 255, 0);
	chThdSleepMilliseconds(500);
	ledSetColorRGB(0, 0, 0, 0);
	chMtxUnlock();
	writeSerial("End ...\r\n");
    }
    return 0;
}

static msg_t xmlPollingThread(void * args) {
    (void) args;
    chRegSetThreadName("xmlPolling");
    while (1) {
	chMtxLock(&appliMtx);
	writeSerial("Polling ...\r\n");
        parsePage("192.168.1.105:9000/config");
	chMtxUnlock();
	writeSerial("End ...\r\n");
	chThdSleepSeconds(5);
    }
    return 0;
}

static char * storySound  = "story.ogg";

static msg_t storyThread(void * args) {
    (void) args;
    chRegSetThreadName("Strory");

    static EventListener eventStoryLst;
    chEvtRegisterMask(&eventStorySrc , &eventStoryLst ,EVENT_MASK(1));

    while (1) {
        chEvtWaitOne(EVENT_MASK(1));
        writeSerial("Story ...\r\n");
        chMtxLock(&appliMtx);
        cmdPlay((BaseSequentialStream *) &SDU1, 1, &storySound);

        for(int i = 0; i<100; i++) {
            ledSetColorHSV(0, 240, 100-i, 100);
            chThdSleepMilliseconds(750);
        }
        cmdStop((BaseSequentialStream *) &SDU1, 0, NULL);
        ledSetColorHSV(0, 0, 0, 0);
        chMtxUnlock();
    }
    return 0;
}

static msg_t streamingThread(void * args){
    (void)args;
    static EventListener lst;
    chEvtRegisterMask(&eventStreamSrc, &lst , EVENT_MASK(1));

    while(1){
	chEvtWaitOne(EVENT_MASK(1));
	chMtxLock(&appliMtx);
	writeSerial("Streaming ...\r\n");
	cmdFullDuplex(NULL,0,NULL);
	chThdSleepSeconds(30);
	cmdStopStream(NULL,0,NULL);
	chMtxUnlock();
	writeSerial("End ...\r\n");
    }
    return 0;
}

/*
 * Initializes the threads
 */
void applicationInit() {
    chThdCreateStatic(waXmlPolling, sizeof(waXmlPolling), NORMALPRIO,
		      xmlPollingThread, NULL);
    chThdCreateStatic(waHands, sizeof(waHands), NORMALPRIO, handsThread, NULL);
    chThdCreateStatic(waStory, sizeof(waStory), NORMALPRIO, storyThread, NULL);
    chThdCreateStatic(waHug, sizeof(waHug), NORMALPRIO, hugThread, NULL);
    chThdCreateStatic(waTemp, sizeof(waTemp), NORMALPRIO, tempThread, NULL);
    chThdCreateStatic(waPir, sizeof(waPir), NORMALPRIO, pirThread, NULL);
    chThdCreateStatic(waCry, sizeof(waCry), NORMALPRIO, cryThread, NULL);
    chThdCreateStatic(waEducColor, sizeof(waEducColor), NORMALPRIO,
		      educColorThread, NULL);
    chThdCreateStatic(waEducLetters, sizeof(waEducLetters), NORMALPRIO,
		      educLettersThread, NULL);
    chThdCreateStatic(waStreaming, sizeof(waStreaming), NORMALPRIO,
		      streamingThread, NULL);
}
