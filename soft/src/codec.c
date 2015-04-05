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
  0
};

/* Buffer used for construcion of read and write command instructions */
static const uint8_t writeCommand = 2;
static const uint8_t readCommand = 3;
static uint8_t readData1 = 1;
static uint8_t readData2 = 1;



static void writeRegister(uint8_t adress, uint16_t command){
  
  uint8_t command1 = (command << 8);
  uint8_t command2 = (command & 0xff);
  
  /* Wait until the writing operation is done */
  while(palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0);
  
  COMMAND_MODE;
  
  /* Construction of instruction (Write opcode, adress, command) */
  spiSend(&SPID4,1,&writeCommand);
  spiSend(&SPID4,1,&adress);
  spiSend(&SPID4,1,&command1);
  spiSend(&SPID4,1,&command2);

  RESET_MODE;

}

static uint16_t readRegister(uint8_t adress){

  uint16_t data;
  
  /* Wait until it's possible to read from SCI */
  while(palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0);
  writeSerial("Read Command : 0x%x\r\n",readCommand); 
  writeSerial("Adress : 0x%x\r\n",adress); 
  COMMAND_MODE;

  /* Construction of instruction (Read opcode, adress) */
  spiSend(&SPID4,1,&readCommand);
  spiSend(&SPID4,1,&adress);
  spiReceive(&SPID4,1,&readData1);
  spiReceive(&SPID4,1,&readData2);
  
  RESET_MODE;

  data = ((readData1) << 8);
  data |= readData2;
  /* Return data from the register */
  return (data);
}

void sendData(const uint8_t * data){
  int size = sizeof(data);
  int i;
  int j = 0;

  DATA_MODE;

  while(j < size){
    /* Wait until it's possible to send data (must be checked every 32 bytes) */
    while(palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0);
      for(i = 0 ; i < 32 ; i++){
	spiSend(&SPID4,1,data++);
	j++;
	if(j == size)
	  break;
      }
    }

  RESET_MODE;
}

void codecReset(void){
  
  RESET_MODE;

  /* Software reset of the codec */
  writeRegister(SCI_MODE,0x4);
  /* Wait until reset is complete */
  while(palReadPad(GPIOE,GPIOE_CODEC_DREQ) == 0);

  // XXXXXXXXXXXXXXX LOAD THE PATCH

  /* Use native SPI modes and use both XDCS ans XCS for chip select */
  writeRegister(SCI_MODE,0x800);
  /* Set Clock settings : x4.5 multiplier (+ x1 when needed, to encode in Ogg Vorbis)  */
  writeRegister(SCI_CLOCKF,0xC800);
  /* Set encoding samplerate to 16000Hz, in mono mode */
  writeRegister(SCI_AUDATA,0x3E80);
  /* Both left and right volumes are 0x24 * -0.5 = -18.0 dB */
  writeRegister(SCI_VOL,0x2424);
  
  int i;

  while(1){
    palTogglePad(GPIOE,8);chThdSleepMilliseconds(500);
    for(i = 0 ; i < 16 ; i++ )
      writeSerial("Registre %u : %x\r\n",i,readRegister(i));
  }
  
}

void codecInit(){
  /* Change the mode of the pins used for the codec and his SPI bus */

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

  codecReset();


}

void codecLowPower(){
  /* Set clock settings : x1.0 to disable the PLL and save power */
  writeRegister(SCI_CLOCKF,0x0000);
  /* Reduce the samplerate, the VSDSP core will just wait for an interrupt, thus saving power */
  writeRegister(SCI_AUDATA,0x0010);
  /* Set the attenuation to his maximum */
  writeRegister(SCI_VOL,0xffff);

}
