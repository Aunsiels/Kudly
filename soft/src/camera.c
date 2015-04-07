#include "hal.h"
#include "ch.h"
#include "camera.h"
#include "chprintf.h"
#include "sd_perso.h"
#include "sccb.h"
#include "ov2640_regs.h"

#define IMG_HEIGHT  160
#define IMG_WIDTH   120
#define BPP         2       //bytes per pixel
#define IMG_SIZE    IMG_HEIGHT*IMG_WIDTH*BPP


uint8_t imgBuf[IMG_SIZE];
uint8_t* imgBuf0 = imgBuf;
uint8_t* imgBuf1 = &imgBuf[IMG_SIZE/2];

EventSource es1, es2;
EventListener el1, el2;

void frameEndCb(DCMIDriver* dcmip) {
   (void) dcmip;
   chSysLockFromIsr();
   chEvtBroadcastI(&es2);
   chSysUnlockFromIsr();
}

void dmaTxferEndCb(DCMIDriver* dcmip) {
   (void) dcmip;
   chSysLockFromIsr();
   chEvtBroadcastI(&es1);
   chSysUnlockFromIsr();
}

static const DCMIConfig dcmicfg = {
   frameEndCb,
   dmaTxferEndCb,
   DCMI_CR_VSPOL |
   DCMI_CR_HSPOL |
   DCMI_CR_JPEG 
};

/*
 * Initializes the camera
 */

void cameraInit() {
    /* XCLK */
    palSetPadMode(GPIOA, GPIOA_CAMERA_XCLK, PAL_MODE_ALTERNATE(0));

    /* Camera pins PWDN */
    palSetPadMode(GPIOC, GPIOC_CAMERA_ENABLE, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPad(GPIOC, GPIOC_CAMERA_ENABLE);

    /* Camera pin */
    palSetPadMode(GPIOA, GPIOA_CAMERA_HSYNC, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOA, GPIOA_CAMERA_PIXCLK, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOC, GPIOC_CAMERA_D0, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOC, GPIOC_CAMERA_D1, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOC, GPIOC_CAMERA_D4, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, GPIOB_CAMERA_D5, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, GPIOB_CAMERA_VSYNC, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, GPIOB_CAMERA_D6, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, GPIOB_CAMERA_D7, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOE, GPIOE_CAMERA_D2, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOE, GPIOE_CAMERA_D3, PAL_MODE_ALTERNATE(13));

    chThdSleepMilliseconds(100);
    palClearPad(GPIOC, GPIOC_CAMERA_ENABLE);

    chEvtInit(&es1);
    chEvtInit(&es2);

    dcmiObjectInit(&DCMID1);

    dcmiStart(&DCMID1, &dcmicfg);

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

void cmdCamera(BaseSequentialStream *chp, int argc, char *argv[]){
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

    chEvtRegisterMask(&es1, &el1, EVENT_MASK(1));
    chEvtRegisterMask(&es2, &el2, EVENT_MASK(2));
    chThdSleepMilliseconds(1000);
    chprintf(chp, "Beginning transfer:\n\r");
    //using synchronous API for simplicity, single buffer.
    //limits max image size to available SRAM. Note that max DMA transfers in one go is 65535.
    // i.e. IMG_SIZE cannot be larger than 65535 here.
    dcmiStartReceiveOneShot(&DCMID1, IMG_SIZE/2, imgBuf0, imgBuf1);
    chprintf(chp, "Wait for event\n\r");
    chEvtWaitOne(EVENT_MASK(1));
    chprintf(chp, "Got first DMA interrupt\n\r");
    chEvtWaitAll(EVENT_MASK(1) | EVENT_MASK(2));
    chprintf(chp, "Got second DMA interrupt, and DCMI interrupt, dumping BMP:\n\r");
    chprintf(chp, "End of the photo\r\n");
    int error = writeFile("test.jpg", (char *) imgBuf, sizeof(imgBuf));
    if (error)
       chprintf(chp, "A problem occured while writting in the file\r\n");
}
