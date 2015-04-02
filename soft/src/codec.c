#include "ch.h"
#include "hal.h"
#include "codec.h"

#define COMMAND_MODE palSetPad(GPIOE,GPIOE_SPI4_XDCS);palClearPad(GPIOE,GPIOE_SPI4_XCS);
#define DATA_MODE    palSetPad(GPIOE,GPIOE_SPI4_XCS);palClearPad(GPIOE,GPIOE_SPI4_XDCS);


/* SPI configuration (21MHz, CPHA=0, CPOL=0, MSb first) */
static const SPIConfig hs_spicfg = {
  NULL,
  GPIOB,
  12,
  0
};

/*  SPI initialisation commands */
static const uint8_t initbuf[] = {};


void codecInit(){
  /* Change the mode of the pins used for the codec */
  palSetPadMode(GPIOE,GPIOE_SPI4_XDCS,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOE,GPIOE_SPI4_XCS,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOE,GPIOE_CODEC_DREQ,PAL_MODE_INPUT_PULLUP);
  palSetPadMode(GPIOE,GPIOE_SPI4_SCK,PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOE,GPIOE_SPI4_MISO,PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOE,GPIOE_SPI4_MOSI,PAL_MODE_ALTERNATE(5));

  /* Start of SPI bus */
  spiAcquireBus(&SPID4);
  spiStart(&SPID4, &hs_spicfg);
  spiSelect(&SPID4);
  
  /* Go in commande mode */
  COMMAND_MODE

  /* Send initialisation commands */
  spiSend(&SPID4,sizeof(initbuf),initbuf);
  
}
