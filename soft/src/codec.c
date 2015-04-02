#include "ch.h"
#include "hal.h"
#include "codec.h"

/* Macro used for going in data,command or reset mode */
#define RESET_MODE palSetPad(GPIOE,GPIOE_SPI4_XDCS);palSetPad(GPIOE,GPIOE_SPI4_XCS)
#define COMMAND_MODE palSetPad(GPIOE,GPIOE_SPI4_XDCS);palClearPad(GPIOE,GPIOE_SPI4_XCS)
#define DATA_MODE    palSetPad(GPIOE,GPIOE_SPI4_XCS);palClearPad(GPIOE,GPIOE_SPI4_XDCS)


/* SPI configuration (21MHz, CPHA=0, CPOL=0, MSb first) */
static const SPIConfig hs_spicfg = {
  NULL,
  GPIOB,
  12,
  0
};

/* Buffer used for construcion of read and write command instructions */
static uint8_t instruction[4];
static uint8_t registerContent[4];

static void writeRegister(uint8_t adress, uint16_t command){
  COMMAND_MODE; 
 
  /* Construction of instruction (Write opcode, adress, command) */ 
  instruction[0] = 0x02;
  instruction[1] = adress;
  instruction[2] = (command >> 8);
  instruction[3] = command;
  spiSend(&SPID4,sizeof(instruction),instruction);
  
  RESET_MODE;
}

static uint16_t readRegister(uint8_t adress){
  COMMAND_MODE;
  
  /* Construction of instruction (Read opcode, adress) */ 
  instruction[0] = 0x02;
  instruction[1] = adress;
  spiExchange(&SPID4,sizeof(instruction),instruction,registerContent);
  
  RESET_MODE;
  
  /* Return only the 2 last bytes (data from the register) */
  return ((registerContent[2]<<8) + registerContent[3]);
}

void codecInit(){
  /* Change the mode of the pins used for the codec */
  palSetPadMode(GPIOE,GPIOE_SPI4_XDCS,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOE,GPIOE_SPI4_XCS,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOE,GPIOE_CODEC_DREQ,PAL_MODE_INPUT_PULLUP);
  palSetPadMode(GPIOE,GPIOE_SPI4_SCK,PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOE,GPIOE_SPI4_MISO,PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOE,GPIOE_SPI4_MOSI,PAL_MODE_ALTERNATE(5));

  RESET_MODE;
  
  /* Start of SPI bus */
  spiAcquireBus(&SPID4);
  spiStart(&SPID4, &hs_spicfg);
  spiSelect(&SPID4);
   
  /* Set Volume */
  writeRegister(0xB,0xF0);
  
  readRegister(0xB);
  
}
