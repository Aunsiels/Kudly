#include "hal.h"
#include "ch.h"
#include "dcmi.h"
#include "chprintf.h"
#include "sd_perso.h"
#include "sccb.h"

/*
 * Initializes the dcmi
 */

void dcmiInit() {
    chSysLock();
    /* Activate the RCC clock for DCMI */
    rccEnableAPB2(RCC_AHB2ENR_DCMIEN, false);
    rccResetAPB2(RCC_AHB2RSTR_DCMIRST);

    /* Configure CR */
    /* Configuration :
     *     DCMI disable
     *     8-bit
     *     All frames captured
     *     VSYNC active high
     *     HSYNC active high
     *     PXCLK folling edge active
     *     Hardware synchronization
     *     JPEG format
     *     Crop : full image
     *     Snapshot mode
     *     Capture disable
     */
    DCMI->CR = DCMI_CR_VSPOL |
               DCMI_CR_HSPOL |
               DCMI_CR_JPEG  |
               DCMI_CR_CM;

    /* DMA Initialization */
    dmaStreamAllocate(STM32_DMA2_STREAM1, 0, NULL,  NULL);

    /* Enable DCMI */
    DCMI->CR |= DCMI_CR_ENABLE;
    chSysUnlock();
}

void dcmiStartConversion(uint32_t * buf, int nbrData){
    /* dma configuration */
    dmaStreamSetPeripheral(STM32_DMA2_STREAM1,
                           (uint32_t) (DCMI_BASE + 0x28));
    dmaStreamSetMemory0(STM32_DMA2_STREAM1,(uint32_t) buf);
    dmaStreamSetTransactionSize(STM32_DMA2_STREAM1, nbrData);
    dmaStreamSetMode(STM32_DMA2_STREAM1,
                     STM32_DMA_CR_CHSEL(1)      |
                     STM32_DMA_CR_DIR_P2M       |
                     STM32_DMA_CR_MINC          |
                     STM32_DMA_CR_MSIZE_WORD    |
                     STM32_DMA_CR_PSIZE_WORD    |
                     STM32_DMA_CR_CIRC          |
                     STM32_DMA_CR_MBURST_SINGLE |
                     STM32_DMA_CR_PBURST_SINGLE);
    dmaStreamSetFIFO(STM32_DMA2_STREAM1,
                     STM32_DMA_FCR_FTH_FULL);
    dmaStreamEnable(STM32_DMA2_STREAM1);

    /* dcmi start conversion */
    DCMI->CR |= DCMI_CR_CAPTURE;

    /* Wait end conversion */
    dmaWaitCompletion(STM32_DMA2_STREAM1);

}

/* JPG 160x120 */
const unsigned char OV2640_160x120_JPEG[][2]=
{
    {0xff, 0x01},
    {0x12, 0x40},
    {0x17, 0x11},
    {0x18, 0x43},
    {0x19, 0x00},
    {0x1a, 0x4b},
    {0x32, 0x09},
    {0x4f, 0xca},
    {0x50, 0xa8},
    {0x5a, 0x23},
    {0x6d, 0x00},
    {0x39, 0x12},
    {0x35, 0xda},
    {0x22, 0x1a},
    {0x37, 0xc3},
    {0x23, 0x00},
    {0x34, 0xc0},
    {0x36, 0x1a},
    {0x06, 0x88},
    {0x07, 0xc0},
    {0x0d, 0x87},
    {0x0e, 0x41},
    {0x4c, 0x00},
    {0xff, 0x00},
    {0xe0, 0x04},
    {0xc0, 0x64},
    {0xc1, 0x4b},
    {0x86, 0x35},
    {0x50, 0x92},
    {0x51, 0xc8},
    {0x52, 0x96},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x00},
    {0x57, 0x00},
    {0x5a, 0x28},
    {0x5b, 0x1e},
    {0x5c, 0x00},
    {0xe0, 0x00}
};

/* Picture buffer for test */
static uint32_t photo[160*120/4];

void cmdDcmi(BaseSequentialStream *chp, int argc, char *argv[]){
    (void) argv;
    if (argc > 0){
        chprintf(chp, "Usage : dcmi\r\n");
        return;
    }
    uint32_t i;
    /* JPEG Init */
    for (i=0 ; i < sizeof(OV2640_160x120_JPEG)/2; i++){
        sccbWrite(OV2640_160x120_JPEG[i][0],OV2640_160x120_JPEG[i][1]);
    }
    dcmiStartConversion(photo, sizeof(photo));
    chprintf(chp, "End of the photo\r\n");
    int error = writeFile("test.jpg", (char *) photo, sizeof(photo)*4);
    if (error)
       chprintf(chp, "A problem occured while writting in the file");
}
