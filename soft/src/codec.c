#include "ch.h"
#include "hal.h"
#include "codecDefinitions.h"
#include "codec.h"
#include "usb_serial.h"
#include "led.h"
#include "ff.h"
#include "chprintf.h"
#include "websocket.h"
#include <stdlib.h>

#define FILE_BUFFER_SIZE 32
#define SDI_MAX_TRANSFER_SIZE 32
#define REC_BUFFER_SIZE 32
#define min(a,b) (((a)<b))?(a):(b)

/* Event and working area for playback and encoding threads */
EVENTSOURCE_DECL(eventSourcePlay);
EVENTSOURCE_DECL(eventSourceEncode);
EVENTSOURCE_DECL(eventSourceVolume);
EVENTSOURCE_DECL(eventSourceWaitEncoding);
EVENTSOURCE_DECL(eventSourceFullDuplex);
EVENTSOURCE_DECL(eventSourceSendData);

static WORKING_AREA(waEncode, 2048);
static WORKING_AREA(waPlayback, 2048);
static WORKING_AREA(waVolume, 2048);
static WORKING_AREA(waWaitEncoding, 128);
static WORKING_AREA(waFullDuplex, 2048);
static WORKING_AREA(waSendData, 1024);

volatile int volLevel = 5;

/* Functions used to access registers and ram of the codec */
static void writeRegister(uint8_t adress, uint16_t command);
static void writeRam(uint16_t adress, uint16_t data);
void writeRam32(uint16_t adress, uint32_t data);
static uint16_t readRegister(uint8_t adress);
static uint16_t readRam(uint16_t adress);
uint32_t readRam32(uint16_t adress);
/* Function used to send data to the codec (no more than 32 bytes) */
static void sendData(const uint8_t * data, int size);
/* Function used to load a patch in the codec (called at each software register) */
static void loadPatch(void);

/* SPI configuration */
static const SPIConfig hs_spicfg = {
    NULL,
    GPIOE,
    11,
    (1 << 4) | (1 << 3)
};

static uint8_t playBuf[FILE_BUFFER_SIZE];
static FIL readFp;
static char * namePlayback;
volatile uint8_t control;

static msg_t threadPlayback(void *arg){
    (void) arg;
    
    static EventListener eventListener;
    chEvtRegisterMask(&eventSourcePlay,&eventListener,1);
  
    UINT bytesNumber;
    uint8_t endFillByte=0;
    int cptEndFill=0;
    int cptReset=0;
    int t;

    while (TRUE) {
        chEvtWaitOne(1);
        /* Can't playback if SPI is not ready (typicaly when encoding) */
        if(SPID4.state != 2){
            writeSerial("SPI not ready\r\n");
            continue;
        }
        /* Reset the control command */
        control = 0;
        /* Open a file in reading mode */
        f_open(&readFp,namePlayback,FA_OPEN_EXISTING | FA_READ);
        /* Get the file contain and keep it in a buffer */
        while(!(f_read(&readFp,playBuf,FILE_BUFFER_SIZE,&bytesNumber))){
            /* Send the whole file to VS1063 */
            t = min(SDI_MAX_TRANSFER_SIZE, bytesNumber);
            sendData(playBuf,t);
            if(t != SDI_MAX_TRANSFER_SIZE)
                break;
            switch(control){
            case 'q' :
                writeRegister(SCI_MODE,readRegister(SCI_MODE) | SM_CANCEL);
                break;
            case '+' :
                if(volLevel == 100)
                    break;
                else{
                    volLevel+=5;
                    codecVolume(volLevel);
                    break;
                }
            case '-' :
                if(volLevel == 0)
                    break;
                else{
                    volLevel-=5;
                    codecVolume(volLevel);
                    break;
                }
		
            default:
                break;
            }
            if(control == 'q'){
                control = 0;
                break;
            }
            control = 0;
        }

        f_close(&readFp);

        /* Read the extra parameters in order to obtain the endFillByte */
        endFillByte=(uint8_t)readRam(PAR_END_FILL_BYTE);
        /* Send the 2052 bytes of endFillByte at the end of a whole file transmission */
        for(cptEndFill=0; cptEndFill<2052; cptEndFill++){
            sendData(&endFillByte,1);
        }
        /* Set SCI_MODE bit SM_CANCEL */
        writeRegister(SCI_MODE, readRegister(SCI_MODE) | SM_CANCEL);
        while(readRegister(SCI_MODE)&SM_CANCEL){
            for(cptEndFill=0; cptEndFill<32; cptEndFill++)
                sendData(&endFillByte,1);
            cptReset++;
            /* Test if SM_CANCEL hasn't cleared after sending 2048 bytes */
            if(cptReset==63) {
                cptReset=0;
                codecReset();
                break;
            }
        } 
      
        if((readRegister(SCI_HDAT1)&readRegister(SCI_HDAT0))!=0){
            writeSerial("Error transmiiting audio file\r\n");
            return 0;
        }
    }
    return 0;
}


static FIL encodeFp;
static uint8_t recBuf[REC_BUFFER_SIZE];
volatile int stopRecord = 0;
volatile int playerState = 1;
volatile int duration = 0;
static char * nameEncode;

static msg_t waitRecording(void *arg){
    (void) arg;
    
    static EventListener eventListener;
    chEvtRegisterMask(&eventSourceWaitEncoding,&eventListener,1);

    while(1){
        chEvtWaitOne(1);
        if(duration != 0){
            /* Collect the data in HDAT0/1 */
            chThdSleepMilliseconds(duration*1000);
	    
            /* Stop the acquisition */
            writeRegister(SCI_MODE,readRegister(SCI_MODE) | SM_CANCEL); 
            stopRecord = 1;
        }
    }
    return 0;
}

static msg_t threadEncode(void *arg){
    (void) arg;

    static EventListener eventListener;
    chEvtRegisterMask(&eventSourceEncode,&eventListener,1);

    uint16_t data;
    UINT bw;
    uint16_t endFillByte;
    
    while(1){
        /* Wait for the thread to be called */
        chEvtWaitOne(1);
        /* Can't encode if SPI is not ready (typicaly when playback) */
        if(SPID4.state != 2){
            writeSerial("SPI not ready\r\n");
            continue;
        }
        /* Set volume at maximum (for now micro is not pre-amplified) */
        codecVolume(100);
        /* Set the samplerate at 16kHz */
        writeRegister(SCI_AICTRL0,16000);
        /* Automatic gain control */
        writeRegister(SCI_AICTRL1,0);
        /* Maximum gain amplification at x40 */
        writeRegister(SCI_AICTRL2,40000);
        /* Set in mono mode, and in format OGG Vorbis */
        writeRegister(SCI_AICTRL3, RM_63_FORMAT_OGG_VORBIS | RM_63_ADC_MODE_MONO);
        /* Set quality mode to 5 */
        writeRegister(SCI_WRAMADDR, RQ_MODE_QUALITY | 5);
	
        /* Start encoding procedure */
        writeRegister(SCI_MODE,readRegister(SCI_MODE) | SM_ENCODE);
        writeRegister(SCI_AIADDR,0x50);

        f_open(&encodeFp,nameEncode,FA_WRITE | FA_OPEN_ALWAYS);

        chSysLock();
        chEvtBroadcastI(&eventSourceWaitEncoding);
        chSysUnlock();   
      
        while(playerState){
            int n,i;
            /* See if there is some data available */
            if((n = readRegister(SCI_RECWORDS)) > 0) {
                uint8_t *rbp = recBuf;
                n = min(n, REC_BUFFER_SIZE/2);
                for (i=0; i<n; i++) {
                    data = readRegister(SCI_RECDATA);
                    *rbp++ = (uint8_t)(data >> 8);
                    *rbp++ = (uint8_t)(data & 0xFF);
                }
                f_write(&encodeFp, recBuf, 2*n, &bw);
            }   	    
            else{
                if(stopRecord && !readRegister(SCI_RECWORDS)){
                    playerState = 0;
                    ledSetColorRGB(2,0,0,0);        
                }
            }
        }
        
        endFillByte = readRam(PAR_END_FILL_BYTE);
        
        /* If it's odd lenght, endFillByte should be added */
        if(endFillByte & (1 << 15))
            f_write(&encodeFp,(uint8_t *)&endFillByte,1,&bw);
    
        f_close(&encodeFp);
        writeRam(PAR_END_FILL_BYTE,0);
    
        /* Wait until the codec exit the encoding mode */
        while((readRegister(SCI_MODE) & SM_ENCODE) == 1);

        codecReset();

        playerState = 1;
        stopRecord = 0;
    }
    return 0;
}

static msg_t threadTestVolume(void *arg){
    (void) arg;

    static EventListener eventListener;
    chEvtRegisterMask(&eventSourceVolume,&eventListener,1);

    while(1){
        ledSetColorRGB(2,0,0,0);
        /* Wait for the thread to be called */
        chEvtWaitOne(1);
        /* Can't encode if SPI is not ready (typicaly when playback) */
        if(SPID4.state != 2){
            writeSerial("SPI not ready\r\n");
            continue;
        }
        /* Set volume at maximum (for now micro is not pre-amplified) */
        codecVolume(100);
        /* Set the samplerate at 16kHz */
        writeRegister(SCI_AICTRL0,16000);
        /* Automatic gain control */
        writeRegister(SCI_AICTRL1,0);
        /* Maximum gain amplification at x40 */
        writeRegister(SCI_AICTRL2,40000);
        /* Set in mono mode, and in format OGG Vorbis */
        writeRegister(SCI_AICTRL3, RM_63_FORMAT_OGG_VORBIS | RM_63_ADC_MODE_MONO);
        /* Set quality mode to 5 */
        writeRegister(SCI_WRAMADDR, RQ_MODE_QUALITY | 5);
	
        /* Start encoding procedure */
        writeRegister(SCI_MODE,readRegister(SCI_MODE) | SM_ENCODE);
        writeRegister(SCI_AIADDR,0x50);

        chSysLock();
        chEvtBroadcastI(&eventSourceWaitEncoding);
        chSysUnlock();   
      
        while(playerState){
            uint16_t level=0;
            /* See if there is some data available */
            if((readRegister(SCI_RECWORDS)) > 0) {
                readRegister(SCI_RECDATA);
                    while ((level = readRam(PAR_ENC_CHANNEL_MAX)) == 0);
                    writeRam(PAR_ENC_CHANNEL_MAX,0);
                writeSerial("Level : %d\r\n", level);
                ledSetColorRGB(2,level,0,0);
                chThdSleepMilliseconds(100);
            }   	    
            else{
                if(stopRecord && !readRegister(SCI_RECWORDS)){
                    playerState = 0;        
                }
            }
        }
    
        writeRam(PAR_END_FILL_BYTE,0);
    
        /* Wait until the codec exit the encoding mode */
        while((readRegister(SCI_MODE) & SM_ENCODE) == 1);

        codecReset();

        playerState = 1;
        stopRecord = 0;
    }
    return 0;
}

static msg_t threadFullDuplex(void *arg){
    (void) arg;

    static EventListener eventListener;
    chEvtRegisterMask(&eventSourceFullDuplex,&eventListener,1);
    
    while(1){
        /* Wait for the thread to be called */
        chEvtWaitOne(1);
        /* Can't encode if SPI is not ready (typicaly when playback or encoding) */
        if(SPID4.state != 2){
            writeSerial("SPI not ready\r\n");
            continue;
        }
        /* Set volume at maximum (for now micro is not pre-amplified) */
        codecVolume(75);
        /* Set the samplerate at 8kHz */ 
        writeRegister(SCI_AICTRL0,8000);
        /* Automatic gain control */
        writeRegister(SCI_AICTRL1,0);
        /* Maximum gain amplification at x40 */
        writeRegister(SCI_AICTRL2,40000);
        /* Set in mono mode, in format PCM (non-compressed), full duplex mode, no header generated */
        writeRegister(SCI_AICTRL3, RM_63_FORMAT_PCM | RM_63_ADC_MODE_MONO | RM_63_CODEC | RM_63_NO_RIFF);
        cmdDlWave(NULL, 0, NULL);

        /*Start the sending of datas to SDI (for playback during streaming */
        chSysLock();
        chEvtBroadcastI(&eventSourceSendData);
        chSysUnlock();   

        /* Start encoding procedure */
        writeRegister(SCI_MODE,readRegister(SCI_MODE) | SM_ENCODE);
        writeRegister(SCI_AIADDR,0x50);
    
        while(playerState){
            /* See if there is some data available */
            if(readRegister(SCI_RECWORDS) > 0){
                chMBPost(&mbCodecOut,readRegister(SCI_RECDATA),TIME_INFINITE);
            }
	    
            else if(stopRecord){
                playerState = 0;
            }
        }
        writeRam(PAR_END_FILL_BYTE,0);
    
        /* Wait until the codec exit the encoding mode */
        while((readRegister(SCI_MODE) & SM_ENCODE) == 1);

        codecReset();

        playerState = 1;
        stopRecord = 0;
    }
    return 0;
}

uint8_t streamBuf[32];
static FIL testFp;
static char testName[] = "winner.wav";

static msg_t threadSendData(void *arg){
    (void) arg;

    UINT bw;
    (void)bw;

    static msg_t dataRecv;

    static EventListener eventListener;
    chEvtRegisterMask(&eventSourceSendData,&eventListener,1);

    while(TRUE){
        chEvtWaitOne(1);

        f_open(&testFp, testName, FA_WRITE | FA_OPEN_ALWAYS);

        //for(int j = 0 ; j < 2000 ; j++) {
        while(playerState) {
            int i;
            /* Complete the buffer from the mail box*/
            for(i = 0 ; i < 16 ; i++) {	
                chMBFetch(&mbCodecIn, &dataRecv, TIME_INFINITE);
                streamBuf[2 * i]     = (uint8_t)(dataRecv);
                streamBuf[2 * i + 1] = (uint8_t)(dataRecv >> 8);
            }
            /* Send the buffer to the codec */
            sendData(streamBuf,32);
            //f_write(&testFp, streamBuf, 32, &bw); 
        }

        if(!f_close(&testFp)) {
            writeSerial("Ouais! #fete");
        } else {
            writeSerial("Nan");
        }
    }

    return(0);
}

void codecVolume(int volume) {
    uint8_t inverseVolume;
    inverseVolume = 255 - (uint8_t)(volume*2.5);
    writeRegister(SCI_VOL,(inverseVolume << 8)| inverseVolume);
    volLevel = volume;
}

void cmdPlay(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;

    if (argc != 1) {
        chprintf(chp, "Enter the file name after the command Play\r\n");
        return;
    }

    namePlayback = argv[0];
    chSysLock();
    chEvtBroadcastI(&eventSourcePlay);
    chSysUnlock();   
}

void cmdEncode(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;

    if (argc != 2) {
        chprintf(chp, "Enter the file, and the duration of recording name after the command Encode\r\n");
        return;
    }

    nameEncode = argv[0];
    duration  = strtol(argv[1],NULL,10);
    chSysLock();
    chEvtBroadcastI(&eventSourceEncode);
    chSysUnlock();
}

void cmdTestVolume(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;

    if (argc != 1) {
        chprintf(chp, "Enter the duration of recording for the volume test\r\n");
        return;
    }
    duration  = strtol(argv[0],NULL,10);
    chSysLock();
    chEvtBroadcastI(&eventSourceVolume);
    chSysUnlock();
}


void cmdFullDuplex(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;
    (void) argc;
    (void) chp;
    
    chSysLock();
    chEvtBroadcastI(&eventSourceFullDuplex);
    chSysUnlock();
}

void cmdVolume(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;

    if (argc != 1) {
        chprintf(chp, "Enter the volume level after the command Volume\r\n");
        return;
    }

    codecVolume(strtol(argv[0],NULL,10));   
}

void cmdStop(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;
    (void) argc;
    (void) chp;
    
    /* Stop the encoding (when duration is set to 0) */
    writeRegister(SCI_MODE,readRegister(SCI_MODE) | SM_CANCEL); 
    stopRecord = 1;  
}

void cmdControl(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;
    
    if (argc != 1) {
        chprintf(chp, "Enter the command (q = quit / + = vol up / - = vol down)\r\n");
        return;
    }

    control = (uint8_t)argv[0][0];
        
    return;
}


void codecInit(){
    /* Change the mode of the pins used for the codec and his SPI bus */
    palSetPadMode(GPIOE,GPIOE_SPI4_XDCS,PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(GPIOE,GPIOE_SPI4_XCS,PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(GPIOE,GPIOE_CODEC_DREQ,PAL_MODE_INPUT_PULLUP | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(GPIOE,GPIOE_SPI4_SCK,PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(GPIOE,GPIOE_SPI4_MISO,PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(GPIOE,GPIOE_SPI4_MOSI,PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
    
    
    codecReset();
    
    /* Create the threads to perform playback and recording (they are waiting on en eventlistener) */
    chThdCreateStatic(waPlayback, sizeof(waPlayback),NORMALPRIO, threadPlayback,NULL);
    chThdCreateStatic(waEncode, sizeof(waEncode),NORMALPRIO, threadEncode,NULL);
    chThdCreateStatic(waVolume, sizeof(waVolume),NORMALPRIO, threadTestVolume,NULL);
    chThdCreateStatic(waFullDuplex, sizeof(waFullDuplex),NORMALPRIO, threadFullDuplex,NULL);
    chThdCreateStatic(waSendData, sizeof(waSendData),NORMALPRIO, threadSendData,NULL);

    /* Thread to count the duration of recording */
    chThdCreateStatic(waWaitEncoding, sizeof(waWaitEncoding),NORMALPRIO, waitRecording,NULL);
}

void codecReset(void){
    /* Start of SPI bus */ 
    spiAcquireBus(&SPID4);
    spiStart(&SPID4, &hs_spicfg);
    spiSelect(&SPID4);
    spiReleaseBus(&SPID4);

    RESET_MODE;

    /* Software reset of the codec */
    writeRegister(SCI_MODE,SM_RESET);
    /* Wait until reset is complete */
    while(palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0);

    /* Load the patch of the codec */
    loadPatch();

    /* Use native SPI modes and use both XDCS ans XCS for chip select */
    writeRegister(SCI_MODE,SM_SDINEW);
    /* Set Clock settings : x4.5 multiplier (+ x1 when needed, to encode in Ogg Vorbis)  */
    writeRegister(SCI_CLOCKF,SC_MULT_53_45X|SC_ADD_53_10X);
    /* Set encoding samplerate to 16000Hz, in mono mode */
    writeRegister(SCI_AUDATA,0x3E80);
    /* Both left and right volumes are at middle (50 over 100) */
    codecVolume(50);
}

void codecLowPower(void){
    /* Set clock settings : x1.0 to disable the PLL and save power */
    writeRegister(SCI_CLOCKF,0x0000);
    /* Reduce the samplerate, the VSDSP core will just wait for an interrupt, thus saving power */
    writeRegister(SCI_AUDATA,0x0010);
    /* Set the attenuation to his maximum */
    codecVolume(0);
    /* Stop the SPI bus */
    spiStop(&SPID4);
}


/* Buffer used for construcion of read and write command instructions */
static uint8_t instruction[4];
static uint8_t registerContent[4];

/* Write in a register of the codec */
static void writeRegister(uint8_t adress, uint16_t command){
    /* Wait until it's possible to write in registers */
    while((palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0));
    
    COMMAND_MODE;
    
    /* Construction of instruction (Write opcode, adress, command) */
    instruction[0] = 0x02;
    instruction[1] = adress;
    instruction[2] = (command >> 8);
    instruction[3] = command;
    spiAcquireBus(&SPID4);
    spiSend(&SPID4,sizeof(instruction),instruction);
    spiReleaseBus(&SPID4);

    RESET_MODE;
}

/* Write a 16 bit data in the ram of the codec */
static void writeRam(uint16_t adress, uint16_t data){
    writeRegister(SCI_WRAMADDR,adress);
    writeRegister(SCI_WRAM,data);
}

/* Write a 32 bit data in the ram of the codec */
void writeRam32(uint16_t adress, uint32_t data){
    writeRegister(SCI_WRAMADDR,adress);
    writeRegister(SCI_WRAM,(uint16_t) data);
    writeRegister(SCI_WRAM,(uint16_t) (data >> 16));
}

/* Read in  a register of a codec */
static uint16_t readRegister(uint8_t adress){
    /* Wait until it's possible to read from SCI */
    while((palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0));

    COMMAND_MODE;

    /* Construction of instruction (Read opcode, adress) */
    instruction[0] = 0x03;
    instruction[1] = adress;
    spiAcquireBus(&SPID4);
    spiExchange(&SPID4,sizeof(instruction),instruction,registerContent);
    spiReleaseBus(&SPID4);

    RESET_MODE;
    
    /* Return only the 2 last bytes (data from the register) */
    return ((registerContent[2]<<8) + registerContent[3]);
}

/* Read a 16 bit data from the ram of the codec */
static uint16_t readRam(uint16_t adress){
    writeRegister(SCI_WRAMADDR,adress);
    return readRegister(SCI_WRAM);
}

/* Read a 32 bit data from the ram of the codec */
uint32_t readRam32(uint16_t adress){
    uint16_t lsb,msb;
    writeRegister(SCI_WRAMADDR,adress+1);
    msb = readRegister(SCI_WRAM);
    writeRegister(SCI_WRAMADDR,adress);
    lsb = readRegister(SCI_WRAM);
    return (lsb |((uint32_t)msb << 16));
}

/* Function to send data (SDI), maximum of 32 bytes */
static void sendData(const uint8_t * data, int size){
    int i;

    /* Wait until it's possible to send data */
    while((palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0));

    DATA_MODE;

    for(i = 0 ; i < size ; i++){
        spiAcquireBus(&SPID4);
        spiSend(&SPID4,1,data++);
        spiReleaseBus(&SPID4);
    }
    
    RESET_MODE;
}

/* Function to load the patch in the codec */
static void loadPatch(void){
    int i;
    for (i=0;i<CODE_SIZE;i++) {
        writeRegister(atab[i], dtab[i]);
    }
}


