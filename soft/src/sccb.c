#include "hal.h"
#include "ch.h"
#include "sccb.h"
#include "chprintf.h"
#include <stdlib.h>
#include "usb_serial.h"

#define DELAY             4
#define SCCB_UNINIT       0
#define SCCB_READY        1
#define SIO_C             9
#define SIO_D             8
#define READ_ADDRESS      97
#define WRITE_ADDRESS     96

static int state = 0;

static BSEMAPHORE_DECL(sccb_clk, 1);

static void callback_gpt4(GPTDriver *gptp){   
    (void) gptp;

    chSysLockFromIsr();
    chBSemSignalI(&sccb_clk);
    chSysUnlockFromIsr();
}

/* General Purpose Timer config */
static GPTConfig gpt4cfg = {
    1000000, /* timer clock 1MHz */
    callback_gpt4, /* Timer callback */
    0
};

static void sccbWait(int time){
    gptStartOneShot(&GPTD4, time);
    chBSemWait(&sccb_clk);
}
                    

/*
 * Initialize the sccb
 */
void sccbInit(){
    /* Open drain mode */
    palSetPadMode(GPIOC, SIO_D, PAL_MODE_OUTPUT_OPENDRAIN);
    palSetPadMode(GPIOC, SIO_C, PAL_MODE_OUTPUT_OPENDRAIN);
    /* Initializes the value of the pins */
    palSetPad(GPIOC, SIO_C);
    palSetPad(GPIOC, SIO_D);

    /* Init GPT */
    do {
	gptStart(&GPTD4, &gpt4cfg);
    }while(GPTD4.state != GPT_READY);
    /* Ready to be used */
    state = SCCB_READY;
}

/*
 * Sends the begin of transimission sequence
 */
static void sccbStartTransmission(void){
    /* Not initialized yet */
    if(state != SCCB_READY) return;

    /* A transmission begins with SIO_C and SIO_D both high and then both low */
    palSetPad(GPIOC, SIO_D);
    /* start condition setup time, min 600ns */
    sccbWait(DELAY);

    palSetPad(GPIOC, SIO_C);
    /* t high, min 600ns */
    sccbWait(DELAY);

    palClearPad(GPIOC, SIO_D);
    /* start condition hold time, min 600ns */
    sccbWait(DELAY);

    palClearPad(GPIOC, SIO_C);
    /* t low, min 1.3us */
    sccbWait(DELAY/2);
}

/*
 * Sends the end of transmission sequence
 */
static void sccbStopTransmission(void){
    /* Not initialized yet */
    if(state != SCCB_READY) return;

    /* An end is SIO_D low and then both SIO_C and SIO_D high */
    palClearPad(GPIOC, SIO_D);
    sccbWait(DELAY/2);

    palSetPad(GPIOC, SIO_C);
    /* Stop condition setup time */
    sccbWait(DELAY);

    palSetPad(GPIOC,SIO_D);
    /* Bus free time before new start */
    sccbWait(DELAY);
}

/*
 * Send Acknowledgement after reading
 */
static void sccbNA(void){
    /* Not initialized yet */
    if(state != SCCB_READY) return;

    /* Shows that data have been received */
    palSetPad(GPIOC, SIO_D);
    sccbWait(DELAY/2);

    /* TIC */
    palSetPad(GPIOC, SIO_C);
    sccbWait(DELAY);

    /* TAC */
    palClearPad(GPIOC, SIO_C);
    sccbWait(DELAY);

    /* 
     * When there is an Acknoledgement, it will be the end of the transmission
     * because we acknowledge only the there is a read answer 
     */
    palClearPad(GPIOC, SIO_D);
    sccbWait(DELAY);
}

/*
 * Send a byte to the camera
 */
static int sccbSendByte(uint8_t data){
    /* Not initialized yet */
    if(state != SCCB_READY) return 0;
    
    int i, success;
    /* We send all the bits, beginning the MSB*/
    for(i = 0; i < 8; ++i){
        if((data << i) & 0x80)
            palSetPad(GPIOC, SIO_D);
        else
            palClearPad(GPIOC, SIO_D);
        sccbWait(DELAY/2);
        
        /* TIC */
        palSetPad(GPIOC, SIO_C);
        sccbWait(DELAY);

        /* TAC */
        palClearPad(GPIOC, SIO_C);
        sccbWait(DELAY/2);
    }

    palClearPad(GPIOC, SIO_D);
    /* Now we check if the camera received something */
    /* We get ready to ready what is on SIO_D */
    palSetPadMode(GPIOC, SIO_D, PAL_MODE_INPUT);
    sccbWait(DELAY/2);

    /* TIC */
    palSetPad(GPIOC, SIO_C);
    sccbWait(DELAY);

    /* We read the acknowledgement */
    success = 1 - palReadPad(GPIOC, SIO_D);    

    /* TAC */
    palClearPad(GPIOC, SIO_C);
    sccbWait(DELAY);

    /* We put the output mode again */
    palSetPadMode(GPIOC, SIO_D, PAL_MODE_OUTPUT_OPENDRAIN);
    sccbWait( DELAY);

    return success;
}

/*
 * Read byte sent by the camera
 */
static uint8_t sccbReadByte(void){
    /* Not initialized yet */
    if(state != SCCB_READY) return 0;
    
    /* Ready to read */
    palSetPadMode(GPIOC, SIO_D, PAL_MODE_INPUT);
    sccbWait(DELAY);

    uint8_t data = 0;

    int i;
    /* We read all the bits, beginning the MSB*/
    for(i = 0; i < 8; ++i){
        sccbWait(DELAY/2);
        /* TIC */
        palSetPad(GPIOC, SIO_C);
        sccbWait(DELAY/2);

        /* Lets read */
        /* We first move the last read data */
        data = data << 1;
        /* Read the value */
        if (palReadPad(GPIOC, SIO_D))
            data++;

        sccbWait(DELAY/2);

        /* TAC */
        palClearPad(GPIOC, SIO_C);
        sccbWait(DELAY/2);
    }

    /* We put the output mode again */
    palSetPadMode(GPIOC, SIO_D, PAL_MODE_OUTPUT_OPENDRAIN);

    return data;
}

/*
 * Write data to a register of the camera
 */
int sccbWrite(uint8_t registerAddress, uint8_t value){
    /* Not initialized yet */
    if(state != SCCB_READY) return 0;

    sccbStartTransmission();  

    /* 
     * The slave address is on 7 bits, the last one (the less significative) is
     * 0 if we are writting.
     */
     if (sccbSendByte(WRITE_ADDRESS)){
         if (sccbSendByte(registerAddress)){
             if (sccbSendByte(value)){
                 sccbStopTransmission();
                 return 1;
             }
         }
     }

     /* Something failed */
     sccbStopTransmission();
     return 0;
}

/*
 * Read the value of a register
 */

int sccbRead(uint8_t registerAddress, uint8_t * value){
    sccbStartTransmission();

    if (sccbSendByte(WRITE_ADDRESS)){
        if(sccbSendByte(registerAddress)){
            /* Now the camera will answer */
            sccbStopTransmission();
            /* The camera needs a new transmission */
            sccbStartTransmission();

            /* We now ask to read on the read address */
            if (sccbSendByte(READ_ADDRESS)){
                /* Read value */
                *value = sccbReadByte();
                /* Acknowledgement */
                sccbNA();
                /* Stop transmission */
                sccbStopTransmission();
                return 1;
            }
        }
    }
    /* Stop transmission */
    sccbStopTransmission();
    /* The transmission failed */
    return 0;
}

void cmdWrite(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)chp;
    if (argc != 2){
        writeSerial( "Usage : sccbWrite register(hex) value(hex)\r\n");
        return;
    }
    char ** useless = NULL;
    uint8_t addr = strtol(argv[0], useless, 16);
    uint8_t val  = strtol(argv[1], useless, 16);

    if(sccbWrite(addr, val)){
        writeSerial( "Transmission succeeded\r\n");
    } else {
        writeSerial( "Transmission failed\r\n");
    }
}

void cmdRead(BaseSequentialStream *chp, int argc, char *argv[]){
    (void) argv;
    (void)chp;
    if (argc != 1){
        writeSerial( "Usage : sccbRead register(hex)\r\n");
        return;
    }
    char ** useless = NULL;
    uint8_t addr = strtol(argv[0], useless, 16);
    uint8_t value = 0;

    if(sccbRead(addr, &value)){
        writeSerial( "Value of the register : %X\r\n", value);
    } else {
        writeSerial( "Transmission failed\r\n");
    }
}


