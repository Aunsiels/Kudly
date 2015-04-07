#include "ch.h"
#include "hal.h"
#include "codecDefinitions.h"
#include "codec.h"
#include "usb_serial.h"

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

static uint8_t sineTest[] = {0x53,0xef,0x6e,126,0,0,0,0};
static uint8_t sineTestEnd[] = {0x45,0x78,0x69,0x74,0,0,0,0};

static void loadPatch(void) {
  int i;
  for (i=0;i<CODE_SIZE;i++) {
    writeRegister(atab[i], dtab[i]);
  }
}

void codecReset(void){

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

  /* Start of SPI bus */
  spiAcquireBus(&SPID4);
  spiStart(&SPID4, &hs_spicfg);
  spiSelect(&SPID4);

  codecReset();

  /* Go to test mode */
  writeRegister(SCI_MODE,readRegister(SCI_MODE)|0x20);

  /* Loop to test the output */
  while(1){
    sendData(sineTest,8);
    palTogglePad(GPIOA,0);
    chThdSleepMilliseconds(500);
    sendData(sineTestEnd,8);
    chThdSleepMilliseconds(500);
  }
}

void codecLowPower(){
  /* Set clock settings : x1.0 to disable the PLL and save power */
  writeRegister(SCI_CLOCKF,0x0000);
  /* Reduce the samplerate, the VSDSP core will just wait for an interrupt, thus saving power */
  writeRegister(SCI_AUDATA,0x0010);
  /* Set the attenuation to his maximum */
  writeRegister(SCI_VOL,0xffff);

}


