#include "hal.h"
#include "ch.h"
#include "camera.h"
#include "chprintf.h"
#include "sd_perso.h"
#include "sccb.h"
#include "ov2640_regs.h"

#define SVGA_HSIZE     (200)
#define SVGA_VSIZE     (150)

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

static const uint8_t default_regs[][2] = {
    { BANK_SEL, BANK_SEL_DSP },
    { 0x2c,     0xff },
    { 0x2e,     0xdf },
    { BANK_SEL, BANK_SEL_SENSOR },
    { 0x3c,     0x32 },
    { CLKRC,    0x80 }, /* Set PCLK divider */
    { COM2,     COM2_OUT_DRIVE_3x }, /* Output drive x2 */
#ifdef OPENMV2
    { REG04,    0xF8}, /* Mirror/VFLIP/AEC[1:0] */
#else
    { REG04_SET(REG04_HREF_EN)},
#endif
    { COM8,     COM8_SET(COM8_BNDF_EN | COM8_AGC_EN | COM8_AEC_EN) },
    { COM9,     COM9_AGC_SET(COM9_AGC_GAIN_8x)},
    { 0x2c,     0x0c },
    { 0x33,     0x78 },
    { 0x3a,     0x33 },
    { 0x3b,     0xfb },
    { 0x3e,     0x00 },
    { 0x43,     0x11 },
    { 0x16,     0x10 },
    { 0x39,     0x02 },
    { 0x35,     0x88 },
    { 0x22,     0x0a },
    { 0x37,     0x40 },
    { 0x23,     0x00 },
    { ARCOM2,   0xa0 },
    { 0x06,     0x02 },
    { 0x06,     0x88 },
    { 0x07,     0xc0 },
    { 0x0d,     0xb7 },
    { 0x0e,     0x01 },
    { 0x4c,     0x00 },
    { 0x4a,     0x81 },
    { 0x21,     0x99 },
    { AEW,      0x40 },
    { AEB,      0x38 },
    /* AGC/AEC fast mode operating region */
    { VV,       VV_AGC_TH_SET(0x08, 0x02) },
    { COM19,    0x00 }, /* Zoom control 2 MSBs */
    { ZOOMS,    0x00 }, /* Zoom control 8 MSBs */
    { 0x5c,     0x00 },
    { 0x63,     0x00 },
    { FLL,      0x00 },
    { FLH,      0x00 },

    /* Set banding filter */
    { COM3,     COM3_BAND_SET(COM3_BAND_AUTO) },
    { REG5D,    0x55 },
    { REG5E,    0x7d },
    { REG5F,    0x7d },
    { REG60,    0x55 },
    { HISTO_LOW,   0x70 },
    { HISTO_HIGH,  0x80 },
    { 0x7c,     0x05 },
    { 0x20,     0x80 },
    { 0x28,     0x30 },
    { 0x6c,     0x00 },
    { 0x6d,     0x80 },
    { 0x6e,     0x00 },
    { 0x70,     0x02 },
    { 0x71,     0x94 },
    { 0x73,     0xc1 },
    { 0x3d,     0x34 },
    //{ COM7,   COM7_RES_UXGA | COM7_ZOOM_EN },
    { 0x5a,     0x57 },
    { BD50,     0xbb },
    { BD60,     0x9c },

    { BANK_SEL, BANK_SEL_DSP },
    { 0xe5,     0x7f },
    { MC_BIST,  MC_BIST_RESET | MC_BIST_BOOT_ROM_SEL },
    { 0x41,     0x24 },
    { RESET,    RESET_JPEG | RESET_DVP },
    { 0x76,     0xff },
    { 0x33,     0xa0 },
    { 0x42,     0x20 },
    { 0x43,     0x18 },
    { 0x4c,     0x00 },
    { CTRL3,    CTRL3_BPC_EN | CTRL3_WPC_EN | 0x10 },
    { 0x88,     0x3f },
    { 0xd7,     0x03 },
    { 0xd9,     0x10 },
    { R_DVP_SP , R_DVP_SP_AUTO_MODE | 0x2 },
    { 0xc8,     0x08 },
    { 0xc9,     0x80 },
    { BPADDR,   0x00 },
    { BPDATA,   0x00 },
    { BPADDR,   0x03 },
    { BPDATA,   0x48 },
    { BPDATA,   0x48 },
    { BPADDR,   0x08 },
    { BPDATA,   0x20 },
    { BPDATA,   0x10 },
    { BPDATA,   0x0e },
    { 0x90,     0x00 },
    { 0x91,     0x0e },
    { 0x91,     0x1a },
    { 0x91,     0x31 },
    { 0x91,     0x5a },
    { 0x91,     0x69 },
    { 0x91,     0x75 },
    { 0x91,     0x7e },
    { 0x91,     0x88 },
    { 0x91,     0x8f },
    { 0x91,     0x96 },
    { 0x91,     0xa3 },
    { 0x91,     0xaf },
    { 0x91,     0xc4 },
    { 0x91,     0xd7 },
    { 0x91,     0xe8 },
    { 0x91,     0x20 },
    { 0x92,     0x00 },
    { 0x93,     0x06 },
    { 0x93,     0xe3 },
    { 0x93,     0x03 },
    { 0x93,     0x03 },
    { 0x93,     0x00 },
    { 0x93,     0x02 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x96,     0x00 },
    { 0x97,     0x08 },
    { 0x97,     0x19 },
    { 0x97,     0x02 },
    { 0x97,     0x0c },
    { 0x97,     0x24 },
    { 0x97,     0x30 },
    { 0x97,     0x28 },
    { 0x97,     0x26 },
    { 0x97,     0x02 },
    { 0x97,     0x98 },
    { 0x97,     0x80 },
    { 0x97,     0x00 },
    { 0x97,     0x00 },
    { 0xa4,     0x00 },
    { 0xa8,     0x00 },
    { 0xc5,     0x11 },
    { 0xc6,     0x51 },
    { 0xbf,     0x80 },
    { 0xc7,     0x10 },
    { 0xb6,     0x66 },
    { 0xb8,     0xA5 },
    { 0xb7,     0x64 },
    { 0xb9,     0x7C },
    { 0xb3,     0xaf },
    { 0xb4,     0x97 },
    { 0xb5,     0xFF },
    { 0xb0,     0xC5 },
    { 0xb1,     0x94 },
    { 0xb2,     0x0f },
    { 0xc4,     0x5c },
    { 0xa6,     0x00 },
    { 0xa7,     0x20 },
    { 0xa7,     0xd8 },
    { 0xa7,     0x1b },
    { 0xa7,     0x31 },
    { 0xa7,     0x00 },
    { 0xa7,     0x18 },
    { 0xa7,     0x20 },
    { 0xa7,     0xd8 },
    { 0xa7,     0x19 },
    { 0xa7,     0x31 },
    { 0xa7,     0x00 },
    { 0xa7,     0x18 },
    { 0xa7,     0x20 },
    { 0xa7,     0xd8 },
    { 0xa7,     0x19 },
    { 0xa7,     0x31 },
    { 0xa7,     0x00 },
    { 0xa7,     0x18 },
    { 0x7f,     0x00 },
    { 0xe5,     0x1f },
    { 0xe1,     0x77 },
    { 0xdd,     0x7f },
    { CTRL0,    CTRL0_YUV422 | CTRL0_YUV_EN | CTRL0_RGB_EN },
    { 0x00,     0x00 }
};

static const uint8_t jpeg_regs[][2] = {
        { BANK_SEL, BANK_SEL_DSP },
        { RESET,   RESET_DVP},
        { IMAGE_MODE, IMAGE_MODE_JPEG_EN|IMAGE_MODE_RGB565 },
        { 0xD7,     0x03 },
        { 0xE1,     0x77 },
        { QS,       0x0C },
        { RESET,    0x00 },
        {0, 0},
};

static const uint8_t svga_regs[][2] = {
        { BANK_SEL, BANK_SEL_SENSOR },
        /* DSP input image resoultion and window size control */
        { COM7,    COM7_RES_SVGA},
        { COM1,    0x0A }, /* UXGA=0x0F, SVGA=0x0A, CIF=0x06 */
        { REG32,   0x09 }, /* UXGA=0x36, SVGA/CIF=0x09 */

        { HSTART,  0x11 }, /* UXGA=0x11, SVGA/CIF=0x11 */
        { HSTOP,   0x43 }, /* UXGA=0x75, SVGA/CIF=0x43 */

        { VSTART,  0x00 }, /* UXGA=0x01, SVGA/CIF=0x00 */
        { VSTOP,   0x4b }, /* UXGA=0x97, SVGA/CIF=0x4b */
        { 0x3d,    0x38 }, /* UXGA=0x34, SVGA/CIF=0x38 */

        { 0x35,    0xda },
        { 0x22,    0x1a },
        { 0x37,    0xc3 },
        { 0x34,    0xc0 },
        { 0x06,    0x88 },
        { 0x0d,    0x87 },
        { 0x0e,    0x41 },
        { 0x42,    0x03 },

        /* Set DSP input image size and offset.
           The sensor output image can be scaled with OUTW/OUTH */
        { BANK_SEL, BANK_SEL_DSP },
        { R_BYPASS, R_BYPASS_DSP_BYPAS },

        { RESET,   RESET_DVP },
        { HSIZE8,  (SVGA_HSIZE>>3)}, /* Image Horizontal Size HSIZE[10:3] */
        { VSIZE8,  (SVGA_VSIZE>>3)}, /* Image Vertiacl Size VSIZE[10:3] */

        /* {HSIZE[11], HSIZE[2:0], VSIZE[2:0]} */
        { SIZEL,   ((SVGA_HSIZE>>6)&0x40) | ((SVGA_HSIZE&0x7)<<3) | (SVGA_VSIZE&0x7)},

        { XOFFL,   0x00 }, /* OFFSET_X[7:0] */
        { YOFFL,   0x00 }, /* OFFSET_Y[7:0] */
        { HSIZE,   ((SVGA_HSIZE>>2)&0xFF) }, /* H_SIZE[7:0]= HSIZE/4 */
        { VSIZE,   ((SVGA_VSIZE>>2)&0xFF) }, /* V_SIZE[7:0]= VSIZE/4 */

        /* V_SIZE[8]/OFFSET_Y[10:8]/H_SIZE[8]/OFFSET_X[10:8] */
        { VHYX,    ((SVGA_VSIZE>>3)&0x80) | ((SVGA_HSIZE>>7)&0x08) },
        { TEST,    (SVGA_HSIZE>>4)&0x80}, /* H_SIZE[9] */

        { CTRL2,   CTRL2_DCW_EN | CTRL2_SDE_EN |
          CTRL2_UV_AVG_EN | CTRL2_CMX_EN | CTRL2_UV_ADJ_EN },

        /* H_DIVIDER/V_DIVIDER */
        { CTRLI,   CTRLI_LP_DP | 0x00},
        /* DVP prescalar */
        { R_DVP_SP, R_DVP_SP_AUTO_MODE},

        { R_BYPASS, R_BYPASS_DSP_EN },
        { RESET,    0x00 },
        {0, 0},
};

/* Picture buffer for test */
static uint32_t photo[SVGA_HSIZE*SVGA_VSIZE/4];

void cmdDcmi(BaseSequentialStream *chp, int argc, char *argv[]){
    (void) argv;
    if (argc > 0){
        chprintf(chp, "Usage : dcmi\r\n");
        return;
    }
    int i=0;
    const uint8_t (*regs)[2];

    /* Reset all registers */
    sccbWrite(BANK_SEL, BANK_SEL_SENSOR);
    sccbWrite(COM7, COM7_SRST);

    /* delay n ms */
    chThdSleepMilliseconds(10);

    i = 0;
    regs = default_regs;
    /* Write initial regsiters */
    while (regs[i][0]) {
        sccbWrite(regs[i][0], regs[i][1]);
        i++;
    }
    chprintf(chp, "Write default\r\n");

 //   i = 0;
 //   regs = svga_regs;
 //   /* Write DSP input regsiters */
 //   while (regs[i][0]) {
 //       sccbWrite(regs[i][0], regs[i][1]);
 //       i++;
 //   }

 //   chprintf(chp, "Write vga\r\n");

    i = 0;
    regs = jpeg_regs;
    /* Write DSP input regsiters */
    while (regs[i][0]) {
        sccbWrite(regs[i][0], regs[i][1]);
        i++;
    }

    chprintf(chp, "Write jpeg\r\n");

    chThdSleepMilliseconds(100);

    chprintf(chp, "Begin photo\r\n");

    dcmiStartConversion(photo, sizeof(photo));
    chprintf(chp, "End of the photo\r\n");
    int error = writeFile("test.jpg", (char *) photo, sizeof(photo)*4);
    if (error)
       chprintf(chp, "A problem occured while writting in the file\r\n");
}
