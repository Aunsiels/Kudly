#include "ch.h"
#include "hal.h"
#include "codec.h"

/* Registers definition */
#define SCI_MODE        0x0
#define SCI_STATUS      0x1
#define SCI_BASS        0x2
#define SCI_CLOCKF      0x3
#define SCI_DECODE_TIME 0x4
#define SCI_AUDATA      0x5
#define SCI_WRAM        0x6
#define SCI_WRAMADDR    0x7
#define SCI_HDAT0       0x8
#define SCI_HDAT1       0x9
#define SCI_AIADDR      0xA
#define SCI_VOL         0xB
#define SCI_AICTRL0     0xC
#define SCI_AICTRL1     0xD
#define SCI_AICTRL2     0xE
#define SCI_AICTRL3     0xF


/* Macro used for going in data,command or reset mode */
#define RESET_MODE   palSetPad(GPIOE,GPIOE_SPI4_XDCS);palSetPad(GPIOE,GPIOE_SPI4_XCS)
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
  
  /* Wait until the writing operaion is done */
  while(palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0){
    chThdSleepMilliseconds(10);
  }
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

void sendData(const uint8_t * data){
  int size = sizeof(data);
  int i;
  int j;
  
  while(j < size){
    /* Wait until the it's possible to send data (must be checked every 32 bytes) */
    while(palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0){
      chThdSleepMilliseconds(10);
    }
    for(i = 0 ; i < 32 ; i++){
      spiSend(&SPID4,1,data++);
      j++;
      if(j == size)
	break;
    }
  }
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
