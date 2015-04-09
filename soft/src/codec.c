#include "ch.h"
#include "hal.h"
#include "codecDefinitions.h"
#include "codec.h"
#include "usb_serial.h"
#include "ff.h"
#include "chprintf.h"

#define FILE_BUFFER_SIZE 32
#define SDI_MAX_TRANSFER_SIZE 32
#define REC_BUFFER_SIZE 32
#define min(a,b) (((a)<b))?(a):(b)

/* SPI configuration (21MHz, CPHA=0, CPOL=0, MSb first) */
static const SPIConfig hs_spicfg = {
    NULL,
    GPIOE,
    11,
    (1 << 4) | (1 << 3)
};

/* Buffer used for construcion of read and write command instructions */
static uint8_t instruction[4];
static uint8_t registerContent[4];

/* Write in a register of the codec */
static void writeRegister(uint8_t adress, uint16_t command){
    COMMAND_MODE;

    /* Construction of instruction (Write opcode, adress, command) */
    instruction[0] = 0x02;
    instruction[1] = adress;
    instruction[2] = (command >> 8);
    instruction[3] = command;
    spiSend(&SPID4,sizeof(instruction),instruction);

    RESET_MODE;

    /* Wait until the writing operation is done */
    while(palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0);
}

/* Write a 16 bit data in the ram of the codec */
void writeRam(uint16_t adress, uint16_t data){

    writeRegister(SCI_WRAMADDR,adress);
    writeRegister(SCI_WRAM,data);

}

/* Write a 32 bit data in the ram of the codec */
void writeRam32(uint16_t adress, uint32_t data){

    writeRegister(SCI_WRAMADDR,adress);
    writeRegister(SCI_WRAM,(uint16_t) data);
    writeRegister(SCI_WRAM,(uint16_t) (data >> 16));

}

static uint16_t readRegister(uint8_t adress){

    /* Wait until it's possible to read from SCI */
    while(palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0);

    COMMAND_MODE;

    /* Construction of instruction (Read opcode, adress) */
    instruction[0] = 0x03;
    instruction[1] = adress;
    spiExchange(&SPID4,sizeof(instruction),instruction,registerContent);

    RESET_MODE;

    /* Return only the 2 last bytes (data from the register) */
    return ((registerContent[2]<<8) + registerContent[3]);
}

/* Read a 16 bit data from the ram of the codec */
uint16_t readRam(uint16_t adress){
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
void sendData(const uint8_t * data, int size){
    int i;

    /* Wait until it's possible to send data */
    while(palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0);

    DATA_MODE;

    for(i = 0 ; i < size ; i++)
	spiSend(&SPID4,1,data++);

    RESET_MODE;
}

static void loadPatch(void) {
    int i;
    for (i=0;i<CODE_SIZE;i++) {
	writeRegister(atab[i], dtab[i]);
    }
}

void codecReset(void){

    /* Start of SPI bus */
    spiAcquireBus(&SPID4);
    spiStart(&SPID4, &hs_spicfg);
    spiSelect(&SPID4);

    RESET_MODE;

    /* Software reset of the codec */
    writeRegister(SCI_MODE,0x4);
    /* Wait until reset is complete */
    while(palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0);

    /* Load the patch of the codec */
    loadPatch();

    /* Use native SPI modes and use both XDCS ans XCS for chip select */
    writeRegister(SCI_MODE,0x800);
    /* Set Clock settings : x4.5 multiplier (+ x1 when needed, to encode in Ogg Vorbis)  */
    writeRegister(SCI_CLOCKF,0xC800);
    /* Set encoding samplerate to 16000Hz, in mono mode */
    writeRegister(SCI_AUDATA,0x3E80);
    /* Both left and right volumes are 0x24 * -0.5 = -18.0 dB */
    writeRegister(SCI_VOL,0x2424);

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
}

void codecLowPower(){
    /* Set clock settings : x1.0 to disable the PLL and save power */
    writeRegister(SCI_CLOCKF,0x0000);
    /* Reduce the samplerate, the VSDSP core will just wait for an interrupt, thus saving power */
    writeRegister(SCI_AUDATA,0x0010);
    /* Set the attenuation to his maximum */
    writeRegister(SCI_VOL,0xffff);
    /* Stop the SPI bus */
    spiStop(&SPID4);
}


static uint8_t playBuf[FILE_BUFFER_SIZE];
static FIL readFp;

void codecPlayMusic(char *name){
    UINT bytesNumber;

    uint8_t endFillByte=0;
    int cptEndFill=0;
    int cptReset=0;
    int cptTrame=0;
    int t;
    /* Open a file in reading mode */
    f_open(&readFp,name,FA_OPEN_EXISTING | FA_READ);
    /* Get the file contain and keep it in a buffer */
    while(!(f_read(&readFp,playBuf,FILE_BUFFER_SIZE,&bytesNumber))){
	/* Send the whole file to VS1063 */
	t = min(SDI_MAX_TRANSFER_SIZE, bytesNumber);
	sendData(playBuf,t);
	cptTrame++;
    }
    if((readRegister(SCI_HDAT1)&readRegister(SCI_HDAT0))!=0){
	palSetPad(GPIOA, 0);
	chThdSleepMilliseconds(300);
	palClearPad(GPIOA, 0);
	chThdSleepMilliseconds(300);
    }
    f_close(&readFp);
    /* Read the extra parameters in order to obtain the endFillByte */
    endFillByte=(uint8_t)readRam(PAR_END_FILL_BYTE);
    /* Send the 2052 bytes of endFillByte at the end of a whole file transmission */
    for(cptEndFill=0; cptEndFill<2052; cptEndFill++){
	sendData(&endFillByte,1);
	palTogglePad(GPIOA, 2);
    }
    /* Set SCI_MODE bit SM_CANCEL */
    writeRegister(SCI_MODE, readRegister(SCI_MODE | SM_CANCEL));
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
}

static WORKING_AREA(waEncode, 128);
static FIL encodeFp;
static uint8_t recBuf[REC_BUFFER_SIZE];
int stopRecord = 0;
int playerState = 1;
int duration = 0;

static msg_t waitRecording(void *arg){
    (void) arg;

    /* Collect the data in HDAT0/1 */
    chThdSleepMilliseconds(duration);

    /* Stop the acquisition */
    stopRecord = 1;  
    //writeRegister(SCI_MODE,readRegister(SCI_MODE) | SM_CANCEL);
    
    return 0;
}

void codecEncodeSound(char * name, int dur){

    /* Set the samplerate at 16kHz */
    writeRegister(SCI_AICTRL0,16000);
    /* G&in = 1 (best quality) */
    writeRegister(SCI_AICTRL1,1024);
    /* Maximum gain amplification at x4 */
    writeRegister(SCI_AICTRL2,4096);
    /* Set in mono mode, and in format OGG Vorbis */
    writeRegister(SCI_AICTRL3, RM_63_FORMAT_OGG_VORBIS | RM_63_ADC_MODE_MONO);
    /* Set quality mode to 5 */
    writeRegister(SCI_WRAMADDR, RQ_MODE_QUALITY | 5);

    /* Start encoding procedure */
    writeRegister(SCI_MODE,readRegister(SCI_MODE) | SM_LINE1 | SM_ENCODE);
    writeRegister(SCI_AIADDR,0x50);

    /* Set the duration of the recording */
    duration = dur;
  
    f_open(&encodeFp,name,FA_WRITE | FA_OPEN_ALWAYS);

    chThdCreateStatic(waEncode, sizeof(waEncode),NORMALPRIO, waitRecording,NULL);

    uint16_t data;
    UINT bw;
    uint16_t endFillByte;
  
    /* Wait for the beginning of the ogg vorbis encodeFpe*/
    //while((readRegister(SCI_HDAT0) != 'O') | (readRegister(SCI_HDAT0) != 'g'));

    while(playerState){
	int n;
	
	/* See if there is some data available */
	if((n = readRegister(SCI_RECWORDS)) > 0) {
	    int i;
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
	    if(stopRecord && !readRegister(SCI_RECWORDS) /*!(readRegister(SCI_MODE) & SM_CANCEL)*/){
		playerState = 0;
	    }
	}
    }
    
    endFillByte = readRam(PAR_END_FILL_BYTE);
    
    /* If it's odd lenght, endFillByte should be added */
    if(endFillByte & (1 << 15))
	f_write(&encodeFp,(uint8_t *)&endFillByte,1,&bw);

    f_close(&encodeFp);
    writeRam(PAR_END_FILL_BYTE,0);

    palTogglePad(GPIOA,0);

    /* Wait until the codec exit the encoding mode */
    while((readRegister(SCI_MODE) & SM_ENCODE) == 1);

    codecReset();

    playerState = 1;
    stopRecord = 0;
}

void cmdPlay(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;

    if (argc == 0) {
	chprintf(chp, "Enter the file name after the command Play\r\n");
	return;
    }
    codecPlayMusic(argv[0]);
}

void cmdEncode(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;

    if (argc != 2) {
	chprintf(chp, "Enter the file, and the duration of recording name after the command Encode\r\n");
	return;
    }
    codecEncodeSound(argv[0],(int)argv[1]);
}
