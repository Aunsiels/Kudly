#include "hal.h"
#include "ch.h"
#include "camera.h"
#include "chprintf.h"
#include "sd_perso.h"
#include "sccb.h"
#include "ov2640_regs.h"

#define JPEG_SIZE   3

#define IMG_HEIGHT  200
#define IMG_WIDTH   120
#define BPP         1       /* bytes per pixel */
#define IMG_SIZE    IMG_HEIGHT*IMG_WIDTH*BPP

/* The image buffer */
static uint8_t imgBuf[IMG_SIZE];
static uint8_t* imgBuf0 = imgBuf;
static uint8_t* imgBuf1 = &imgBuf[IMG_SIZE/2];

/* Source to indicate if a frame ends or dma ends */
static EventSource dmaEvS, frameEvS;
/* The listeners linked */
static EventListener dmaEvL, frameEvL;

/* Callback frame end */
static void frameEndCb(DCMIDriver* dcmip) {
    chSysLockFromIsr();
    /* 
     * As I do not know the size of the image, I stop the dma when there is the
     * end of the frame
     */
    dmaStreamDisable(dcmip->dmarx);
    chEvtBroadcastI(&frameEvS);
    chSysUnlockFromIsr();
}

/* Callback dma tx ends */
static void dmaTxferEndCb(DCMIDriver* dcmip) {
    (void) dcmip;
    chSysLockFromIsr();
    chEvtBroadcastI(&dmaEvS);
    chSysUnlockFromIsr();
}

/* QQVGA configuration */
const char OV2640_QQVGA[][2]=
{
    {0xff, 0x00},
    {0x2c, 0xff},
    {0x2e, 0xdf},
    {0xff, 0x01},
    {0x3c, 0x32},
    {0x11, 0x00},
    {0x09, 0x02},
    {0x03, 0xcf},
    {0x04, 0x08},
    {0x13, 0xe5},
    {0x14, 0x48},
    {0x2c, 0x0c},
    {0x33, 0x78},
    {0x3a, 0x33},
    {0x3b, 0xfb},
    {0x3e, 0x00},
    {0x43, 0x11},
    {0x16, 0x10},
    {0x39, 0x02},
    {0x35, 0x88},
    {0x22, 0x0a},
    {0x37, 0x40},
    {0x23, 0x00},
    {0x34, 0xa0},
    {0x36, 0x1a},
    {0x06, 0x02},
    {0x07, 0xc0},
    {0x0d, 0xb7},
    {0x0e, 0x01},
    {0x4c, 0x00},
    {0x4a, 0x81},
    {0x21, 0x99},
    {0x24, 0x3a},
    {0x25, 0x32},
    {0x26, 0x82},
    {0x5c, 0x00},
    {0x63, 0x00},
    {0x5d, 0x55},
    {0x5e, 0x7d},
    {0x5f, 0x7d},
    {0x60, 0x55},
    {0x61, 0x70},
    {0x62, 0x80},
    {0x7c, 0x05},
    {0x20, 0x80},
    {0x28, 0x30},
    {0x6c, 0x00},
    {0x6d, 0x80},
    {0x6e, 0x00},
    {0x70, 0x02},
    {0x71, 0x96},
    {0x73, 0xe1},
    {0x3d, 0x34},
    {0x5a, 0x57},
    {0x4f, 0xbb},
    {0x50, 0x9c},
    {0x0f, 0x43},
    {0xff, 0x00},
    {0xe5, 0x7f},
    {0xf9, 0xc0},
    {0x41, 0x24},
    {0xe0, 0x14},
    {0x76, 0xff},
    {0x33, 0xa0},
    {0x42, 0x20},
    {0x43, 0x18},
    {0x4c, 0x00},
    {0x87, 0xd0},
    {0x88, 0x3f},
    {0xd7, 0x03},
    {0xd9, 0x10},
    {0xd3, 0x82},
    {0xc8, 0x08},
    {0xc9, 0x80},
    {0x7c, 0x00},
    {0x7d, 0x02},
    {0x7c, 0x03},
    {0x7d, 0x48},
    {0x7d, 0x48},
    {0x7c, 0x08},
    {0x7d, 0x20},
    {0x7d, 0x10},
    {0x7d, 0x0e},
    {0x90, 0x00},
    {0x91, 0x0e},
    {0x91, 0x1a},
    {0x91, 0x31},
    {0x91, 0x5a},
    {0x91, 0x69},
    {0x91, 0x75},
    {0x91, 0x7e},
    {0x91, 0x88},
    {0x91, 0x8f},
    {0x91, 0x96},
    {0x91, 0xa3},
    {0x91, 0xaf},
    {0x91, 0xc4},
    {0x91, 0xd7},
    {0x91, 0xe8},
    {0x91, 0x20},
    {0x92, 0x00},
    {0x93, 0x06},
    {0x93, 0xe3},
    {0x93, 0x05},
    {0x93, 0x05},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x96, 0x00},
    {0x97, 0x08},
    {0x97, 0x19},
    {0x97, 0x02},
    {0x97, 0x0c},
    {0x97, 0x24},
    {0x97, 0x30},
    {0x97, 0x28},
    {0x97, 0x26},
    {0x97, 0x02},
    {0x97, 0x98},
    {0x97, 0x80},
    {0x97, 0x00},
    {0x97, 0x00},
    {0xc3, 0xed},
    {0xa4, 0x00},
    {0xa8, 0x00},
    {0xbf, 0x00},
    {0xba, 0xf0},
    {0xbc, 0x64},
    {0xbb, 0x02},
    {0xb6, 0x3d},
    {0xb8, 0x57},
    {0xb7, 0x38},
    {0xb9, 0x4e},
    {0xb3, 0xe8},
    {0xb4, 0xe1},
    {0xb5, 0x66},
    {0xb0, 0x67},
    {0xb1, 0x5e},
    {0xb2, 0x04},
    {0xc7, 0x00},
    {0xc6, 0x51},
    {0xc5, 0x11},
    {0xc4, 0x9c},
    {0xcf, 0x02},
    {0xa6, 0x00},
    {0xa7, 0xe0},
    {0xa7, 0x10},
    {0xa7, 0x1e},
    {0xa7, 0x21},
    {0xa7, 0x00},
    {0xa7, 0x28},
    {0xa7, 0xd0},
    {0xa7, 0x10},
    {0xa7, 0x16},
    {0xa7, 0x21},
    {0xa7, 0x00},
    {0xa7, 0x28},
    {0xa7, 0xd0},
    {0xa7, 0x10},
    {0xa7, 0x17},
    {0xa7, 0x21},
    {0xa7, 0x00},
    {0xa7, 0x28},
    {0xc0, 0xc8},
    {0xc1, 0x96},
    {0x86, 0x1d},
    {0x50, 0x00},
    {0x51, 0x90},
    {0x52, 0x18},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x88},
    {0x57, 0x00},
    {0x5a, 0x90},
    {0x5b, 0x18},
    {0x5c, 0x05},
    {0xc3, 0xef},
    {0x7f, 0x00},
    {0xda, 0x00},
    {0xe5, 0x1f},
    {0xe1, 0x67},
    {0xe0, 0x00},
    {0xdd, 0xff},
    {0x05, 0x00},
    {0xff, 0x01},
    {0xff, 0x01},
    {0x12, 0x00},
    {0x17, 0x11},
    {0x18, 0x75},
    {0x19, 0x01},
    {0x1a, 0x97},
    {0x32, 0x36},
    {0x4f, 0xbb},
    {0x6d, 0x80},
    {0x3d, 0x34},
    {0x39, 0x02},
    {0x35, 0x88},
    {0x22, 0x0a},
    {0x37, 0x40},
    {0x23, 0x00},
    {0x34, 0xa0},
    {0x36, 0x1a},
    {0x06, 0x02},
    {0x07, 0xc0},
    {0x0d, 0xb7},
    {0x0e, 0x01},
    {0x4c, 0x00},
    {0xff, 0x00},
    {0xe0, 0x04},
    {0x8c, 0x00},
    {0x87, 0xd0},
    {0xe0, 0x00},
    {0xff, 0x00},
    {0xe0, 0x14},
    {0xe1, 0x77},
    {0xe5, 0x1f},
    {0xd7, 0x03},
    {0xda, 0x10},
    {0xe0, 0x00},
    {0xff, 0x00},
    {0xe0, 0x04},
    {0xc0, 0xc8},
    {0xc1, 0x96},
    {0x86, 0x1d},
    {0x50, 0x00},
    {0x51, 0x90},
    {0x52, 0x2c},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x88},
    {0x57, 0x00},
    {0x5a, 0x90},
    {0x5b, 0x2c},
    {0x5c, 0x05},
    {0xe0, 0x00},
    {0xd3, 0x04},
    {0xff, 0x00},
    {0xc3, 0xef},
    {0xa6, 0x00},
    {0xa7, 0xdd},
    {0xa7, 0x78},
    {0xa7, 0x7e},
    {0xa7, 0x24},
    {0xa7, 0x00},
    {0xa7, 0x25},
    {0xa6, 0x06},
    {0xa7, 0x20},
    {0xa7, 0x58},
    {0xa7, 0x73},
    {0xa7, 0x34},
    {0xa7, 0x00},
    {0xa7, 0x25},
    {0xa6, 0x0c},
    {0xa7, 0x28},
    {0xa7, 0x58},
    {0xa7, 0x6d},
    {0xa7, 0x34},
    {0xa7, 0x00},
    {0xa7, 0x25},
    {0xff, 0x00},
    {0xe0, 0x04},
    {0xe1, 0x67},
    {0xe5, 0x1f},
    {0xd7, 0x01},
    {0xda, 0x08},
    {0xda, 0x09},
    {0xe0, 0x00},
    {0x98, 0x00},
    {0x99, 0x00},
    {0xff, 0x01},
    {0x04, 0x28},
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
    {0xe0, 0x00},
    {0xff, 0x01},
    {0x11, 0x00},
    {0x3d, 0x38},
    {0x2d, 0x00},
    {0x50, 0x65},
    {0xff, 0x00},
    {0xd3, 0x04},
    {0x7c, 0x00},
    {0x7d, 0x04},
    {0x7c, 0x09},
    {0x7d, 0x28},
    {0x7d, 0x00},
};

/* QVGA 320x240 */
const unsigned char OV2640_QVGA[][2]=
{
    {0xff, 0x00},
    {0x2c, 0xff},
    {0x2e, 0xdf},
    {0xff, 0x01},
    {0x3c, 0x32},
    {0x11, 0x00},
    {0x09, 0x02},
    {0x04, 0x28},
    {0x13, 0xe5},
    {0x14, 0x48},
    {0x2c, 0x0c},
    {0x33, 0x78},
    {0x3a, 0x33},
    {0x3b, 0xfB},
    {0x3e, 0x00},
    {0x43, 0x11},
    {0x16, 0x10},
    {0x4a, 0x81},
    {0x21, 0x99},
    {0x24, 0x40},
    {0x25, 0x38},
    {0x26, 0x82},
    {0x5c, 0x00},
    {0x63, 0x00},
    {0x46, 0x3f},
    {0x0c, 0x3c},
    {0x61, 0x70},
    {0x62, 0x80},
    {0x7c, 0x05},
    {0x20, 0x80},
    {0x28, 0x30},
    {0x6c, 0x00},
    {0x6d, 0x80},
    {0x6e, 0x00},
    {0x70, 0x02},
    {0x71, 0x94},
    {0x73, 0xc1},
    {0x3d, 0x34},
    {0x5a, 0x57},
    {0x12, 0x00},
    {0x11, 0x00},
    {0x17, 0x11},
    {0x18, 0x75},
    {0x19, 0x01},
    {0x1a, 0x97},
    {0x32, 0x36},
    {0x03, 0x0f},
    {0x37, 0x40},
    {0x4f, 0xbb},
    {0x50, 0x9c},
    {0x5a, 0x57},
    {0x6d, 0x80},
    {0x6d, 0x38},
    {0x39, 0x02},
    {0x35, 0x88},
    {0x22, 0x0a},
    {0x37, 0x40},
    {0x23, 0x00},
    {0x34, 0xa0},
    {0x36, 0x1a},
    {0x06, 0x02},
    {0x07, 0xc0},
    {0x0d, 0xb7},
    {0x0e, 0x01},
    {0x4c, 0x00},
    {0xff, 0x00},
    {0xe5, 0x7f},
    {0xf9, 0xc0},
    {0x41, 0x24},
    {0xe0, 0x14},
    {0x76, 0xff},
    {0x33, 0xa0},
    {0x42, 0x20},
    {0x43, 0x18},
    {0x4c, 0x00},
    {0x87, 0xd0},
    {0x88, 0x3f},
    {0xd7, 0x03},
    {0xd9, 0x10},
    {0xd3, 0x82},
    {0xc8, 0x08},
    {0xc9, 0x80},
    {0x7d, 0x00},
    {0x7c, 0x03},
    {0x7d, 0x48},
    {0x7c, 0x08},
    {0x7d, 0x20},
    {0x7d, 0x10},
    {0x7d, 0x0e},
    {0x90, 0x00},
    {0x91, 0x0e},
    {0x91, 0x1a},
    {0x91, 0x31},
    {0x91, 0x5a},
    {0x91, 0x69},
    {0x91, 0x75},
    {0x91, 0x7e},
    {0x91, 0x88},
    {0x91, 0x8f},
    {0x91, 0x96},
    {0x91, 0xa3},
    {0x91, 0xaf},
    {0x91, 0xc4},
    {0x91, 0xd7},
    {0x91, 0xe8},
    {0x91, 0x20},
    {0x92, 0x00},
    {0x93, 0x06},
    {0x93, 0xe3},
    {0x93, 0x02},
    {0x93, 0x02},
    {0x93, 0x00},
    {0x93, 0x04},
    {0x93, 0x00},
    {0x93, 0x03},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x96, 0x00},
    {0x97, 0x08},
    {0x97, 0x19},
    {0x97, 0x02},
    {0x97, 0x0c},
    {0x97, 0x24},
    {0x97, 0x30},
    {0x97, 0x28},
    {0x97, 0x26},
    {0x97, 0x02},
    {0x97, 0x98},
    {0x97, 0x80},
    {0x97, 0x00},
    {0x97, 0x00},
    {0xc3, 0xef},
    {0xff, 0x00},
    {0xba, 0xdc},
    {0xbb, 0x08},
    {0xb6, 0x24},
    {0xb8, 0x33},
    {0xb7, 0x20},
    {0xb9, 0x30},
    {0xb3, 0xb4},
    {0xb4, 0xca},
    {0xb5, 0x43},
    {0xb0, 0x5c},
    {0xb1, 0x4f},
    {0xb2, 0x06},
    {0xc7, 0x00},
    {0xc6, 0x51},
    {0xc5, 0x11},
    {0xc4, 0x9c},
    {0xbf, 0x00},
    {0xbc, 0x64},
    {0xa6, 0x00},
    {0xa7, 0x1e},
    {0xa7, 0x6b},
    {0xa7, 0x47},
    {0xa7, 0x33},
    {0xa7, 0x00},
    {0xa7, 0x23},
    {0xa7, 0x2e},
    {0xa7, 0x85},
    {0xa7, 0x42},
    {0xa7, 0x33},
    {0xa7, 0x00},
    {0xa7, 0x23},
    {0xa7, 0x1b},
    {0xa7, 0x74},
    {0xa7, 0x42},
    {0xa7, 0x33},
    {0xa7, 0x00},
    {0xa7, 0x23},
    {0xc0, 0xc8},
    {0xc1, 0x96},
    {0x8c, 0x00},
    {0x86, 0x3d},
    {0x50, 0x92},
    {0x51, 0x90},
    {0x52, 0x2c},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x88},
    {0x5a, 0x50},
    {0x5b, 0x3c},
    {0x5c, 0x00},
    {0xd3, 0x04},
    {0x7f, 0x00},
    {0xda, 0x00},
    {0xe5, 0x1f},
    {0xe1, 0x67},
    {0xe0, 0x00},
    {0xdd, 0x7f},
    {0x05, 0x00},
    {0xff, 0x00},
    {0xe0, 0x04},
    {0xc0, 0xc8},
    {0xc1, 0x96},
    {0x86, 0x3d},
    {0x50, 0x92},
    {0x51, 0x90},
    {0x52, 0x2c},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x88},
    {0x57, 0x00},
    {0x5a, 0x50},
    {0x5b, 0x3c},
    {0x5c, 0x00},
    {0xd3, 0x04},
    {0xe0, 0x00},
    {0xFF, 0x00},
    {0x05, 0x00},
    {0xDA, 0x08},
    {0xda, 0x09},
    {0x98, 0x00},
    {0x99, 0x00},
    {0x00, 0x00},
};

/* JPEG init */
const unsigned char OV2640_JPEG_INIT[][2]=
{
    {0xff, 0x00},
    {0x2c, 0xff},
    {0x2e, 0xdf},
    {0xff, 0x01},
    {0x3c, 0x32},
    {0x11, 0x00},
    {0x09, 0x02},
    {0x04, 0x28},
    {0x13, 0xe5},
    {0x14, 0x48},
    {0x2c, 0x0c},
    {0x33, 0x78},
    {0x3a, 0x33},
    {0x3b, 0xfB},
    {0x3e, 0x00},
    {0x43, 0x11},
    {0x16, 0x10},
    {0x39, 0x92},
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
    {0x48, 0x00},
    {0x5B, 0x00},
    {0x42, 0x03},
    {0x4a, 0x81},
    {0x21, 0x99},
    {0x24, 0x40},
    {0x25, 0x38},
    {0x26, 0x82},
    {0x5c, 0x00},
    {0x63, 0x00},
    {0x61, 0x70},
    {0x62, 0x80},
    {0x7c, 0x05},
    {0x20, 0x80},
    {0x28, 0x30},
    {0x6c, 0x00},
    {0x6d, 0x80},
    {0x6e, 0x00},
    {0x70, 0x02},
    {0x71, 0x94},
    {0x73, 0xc1},
    {0x12, 0x40},
    {0x17, 0x11},
    {0x18, 0x43},
    {0x19, 0x00},
    {0x1a, 0x4b},
    {0x32, 0x09},
    {0x37, 0xc0},
    {0x4f, 0x60},
    {0x50, 0xa8},
    {0x6d, 0x00},
    {0x3d, 0x38},
    {0x46, 0x3f},
    {0x4f, 0x60},
    {0x0c, 0x3c},
    {0xff, 0x00},
    {0xe5, 0x7f},
    {0xf9, 0xc0},
    {0x41, 0x24},
    {0xe0, 0x14},
    {0x76, 0xff},
    {0x33, 0xa0},
    {0x42, 0x20},
    {0x43, 0x18},
    {0x4c, 0x00},
    {0x87, 0xd5},
    {0x88, 0x3f},
    {0xd7, 0x03},
    {0xd9, 0x10},
    {0xd3, 0x82},
    {0xc8, 0x08},
    {0xc9, 0x80},
    {0x7c, 0x00},
    {0x7d, 0x00},
    {0x7c, 0x03},
    {0x7d, 0x48},
    {0x7d, 0x48},
    {0x7c, 0x08},
    {0x7d, 0x20},
    {0x7d, 0x10},
    {0x7d, 0x0e},
    {0x90, 0x00},
    {0x91, 0x0e},
    {0x91, 0x1a},
    {0x91, 0x31},
    {0x91, 0x5a},
    {0x91, 0x69},
    {0x91, 0x75},
    {0x91, 0x7e},
    {0x91, 0x88},
    {0x91, 0x8f},
    {0x91, 0x96},
    {0x91, 0xa3},
    {0x91, 0xaf},
    {0x91, 0xc4},
    {0x91, 0xd7},
    {0x91, 0xe8},
    {0x91, 0x20},
    {0x92, 0x00},
    {0x93, 0x06},
    {0x93, 0xe3},
    {0x93, 0x05},
    {0x93, 0x05},
    {0x93, 0x00},
    {0x93, 0x04},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x96, 0x00},
    {0x97, 0x08},
    {0x97, 0x19},
    {0x97, 0x02},
    {0x97, 0x0c},
    {0x97, 0x24},
    {0x97, 0x30},
    {0x97, 0x28},
    {0x97, 0x26},
    {0x97, 0x02},
    {0x97, 0x98},
    {0x97, 0x80},
    {0x97, 0x00},
    {0x97, 0x00},
    {0xc3, 0xed},
    {0xa4, 0x00},
    {0xa8, 0x00},
    {0xc5, 0x11},
    {0xc6, 0x51},
    {0xbf, 0x80},
    {0xc7, 0x10},
    {0xb6, 0x66},
    {0xb8, 0xA5},
    {0xb7, 0x64},
    {0xb9, 0x7C},
    {0xb3, 0xaf},
    {0xb4, 0x97},
    {0xb5, 0xFF},
    {0xb0, 0xC5},
    {0xb1, 0x94},
    {0xb2, 0x0f},
    {0xc4, 0x5c},
    {0xc0, 0x64},
    {0xc1, 0x4B},
    {0x8c, 0x00},
    {0x86, 0x3D},
    {0x50, 0x00},
    {0x51, 0xC8},
    {0x52, 0x96},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x00},
    {0x5a, 0xC8},
    {0x5b, 0x96},
    {0x5c, 0x00},
    {0xd3, 0x7f},
    {0xc3, 0xed},
    {0x7f, 0x00},
    {0xda, 0x00},
    {0xe5, 0x1f},
    {0xe1, 0x67},
    {0xe0, 0x00},
    {0xdd, 0x7f},
    {0x05, 0x00},

    {0x12, 0x40},
    {0xd3, 0x7f},
    {0xc0, 0x16},
    {0xC1, 0x12},
    {0x8c, 0x00},
    {0x86, 0x3d},
    {0x50, 0x00},
    {0x51, 0x2C},
    {0x52, 0x24},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x00},
    {0x5A, 0x2c},
    {0x5b, 0x24},
    {0x5c, 0x00},
};

/* YUV422 configuration */
const unsigned char OV2640_YUV422[][2]= 
{
    {0xFF, 0x00},
    {0x05, 0x00},
    {0xDA, 0x10},
    {0xD7, 0x03},
    {0xDF, 0x00},
    {0x33, 0x80},
    {0x3C, 0x40},
    {0xe1, 0x77},
    {0x00, 0x00},
};

/* JPEG configuration */
const unsigned char OV2640_JPEG[][2]=
{
    {0xe0, 0x14},
    {0xe1, 0x77},
    {0xe5, 0x1f},
    {0xd7, 0x03},
    {0xda, 0x10},
    {0xe0, 0x00},
    {0xFF, 0x01},
    {0x04, 0x08},
};

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
    {0xe0, 0x00},
};

/* JPG, 0x176x144 */
const unsigned char OV2640_176x144_JPEG[][2]=
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
    {0x5a, 0x2c},
    {0x5b, 0x24},
    {0x5c, 0x00},
    {0xe0, 0x00},
};

/* JPG 320x240 */
const unsigned char OV2640_320x240_JPEG[][2]=
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
    {0x50, 0x89},
    {0x51, 0xc8},
    {0x52, 0x96},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x00},
    {0x57, 0x00},
    {0x5a, 0x50},
    {0x5b, 0x3c},
    {0x5c, 0x00},
    {0xe0, 0x00},
};

/* JPG 352x288 */
const unsigned char OV2640_352x288_JPEG[][2]=
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
    {0x50, 0x89},
    {0x51, 0xc8},
    {0x52, 0x96},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x00},
    {0x57, 0x00},
    {0x5a, 0x58},
    {0x5b, 0x48},
    {0x5c, 0x00},
    {0xe0, 0x00},
};

static const DCMIConfig dcmicfg = {
    frameEndCb, /* Callback frame */
    dmaTxferEndCb,
    /* Callaback dma */
    DCMI_CR_PCKPOL   /* Rising edge */
};

/*
 * Initializes the QVGA
 */
void initializeQVGA(void)
{
    uint32_t i;

    /* Reset all registers */
    sccbWrite(BANK_SEL, BANK_SEL_SENSOR);
    sccbWrite(COM7, COM7_SRST);
    chThdSleepMilliseconds(200);

    /* Initialize OV2640 */
    for(i=0; i<(sizeof(OV2640_QVGA)/2); i++){
        sccbWrite(OV2640_QVGA[i][0], OV2640_QVGA[i][1]);
    }
}

/*
 * Initializes the QQVGA
 */
void initilizeQQVGA(void)
{
    uint32_t i;

    /* Reset all registers */
    sccbWrite(BANK_SEL, BANK_SEL_SENSOR);
    sccbWrite(COM7, COM7_SRST);
    chThdSleepMilliseconds(200);

    /* Initialize OV2640 */
    for(i=0; i<(sizeof(OV2640_QQVGA)/2); i++){
        sccbWrite(OV2640_QQVGA[i][0], OV2640_QQVGA[i][1]);
    }
}

/*
 * Initilize JPEG for the camera
 */
void initializeJPEG (void){
    /* Reset all registers */
    sccbWrite(BANK_SEL, BANK_SEL_SENSOR);
    sccbWrite(COM7, COM7_SRST);

    chThdSleepMilliseconds(10);
    uint32_t i;

    /* Initialize OV2640 */
    for(i=0; i<(sizeof(OV2640_JPEG_INIT)/2); i++){
        sccbWrite(OV2640_JPEG_INIT[i][0], OV2640_JPEG_INIT[i][1]);
    }

    /* Set to output YUV422 */
    for(i=0; i<(sizeof(OV2640_YUV422)/2); i++){
        sccbWrite(OV2640_YUV422[i][0], OV2640_YUV422[i][1]);
    }

    sccbWrite(0xff, 0x01);
    sccbWrite(0x15, 0x00);

    /* Set to output JPEG */
    for(i=0; i<(sizeof(OV2640_JPEG)/2); i++){
        sccbWrite(OV2640_JPEG[i][0], OV2640_JPEG[i][1]);
    }

    chThdSleepMilliseconds(100);

    /* Define the size */
#if JPEG_SIZE == 0
    for(i=0; i<(sizeof(OV2640_160x120_JPEG)/2); i++){
        sccbWrite(OV2640_160x120_JPEG[i][0], OV2640_160x120_JPEG[i][1]);
    }
#elif JPEG_SIZE == 1
    for(i=0; i<(sizeof(OV2640_176x144_JPEG)/2); i++){
        sccbWrite(OV2640_176x144_JPEG[i][0], OV2640_176x144_JPEG[i][1]);
    }
#elif JPEG_SIZE == 2
    for(i=0; i<(sizeof(OV2640_320x240_JPEG)/2); i++){
        sccbWrite(OV2640_320x240_JPEG[i][0], OV2640_320x240_JPEG[i][1]);
    }
#elif JPEG_SIZE == 3
    for(i=0; i<(sizeof(OV2640_352x288_JPEG)/2); i++){
        sccbWrite(OV2640_352x288_JPEG[i][0], OV2640_352x288_JPEG[i][1]);
    }
#endif
}

/* Is the camera ready to be used  ?*/
static int cameraReady = 0;

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
    palSetPadMode(GPIOA, GPIOA_CAMERA_HSYNC, PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP );
    palSetPadMode(GPIOA, GPIOA_CAMERA_PIXCLK, PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL);
    palSetPadMode(GPIOC, GPIOC_CAMERA_D0, PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(GPIOC, GPIOC_CAMERA_D1, PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(GPIOC, GPIOC_CAMERA_D4, PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(GPIOB, GPIOB_CAMERA_D5, PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(GPIOB, GPIOB_CAMERA_VSYNC, PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(GPIOB, GPIOB_CAMERA_D6, PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(GPIOB, GPIOB_CAMERA_D7, PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(GPIOE, GPIOE_CAMERA_D2, PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(GPIOE, GPIOE_CAMERA_D3, PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_PULLUP);

    chThdSleepMilliseconds(100);
    /* Start camera */
    palClearPad(GPIOC, GPIOC_CAMERA_ENABLE);

    /* Initializes the events */
    chEvtInit(&dmaEvS);
    chEvtInit(&frameEvS);

    /* Initialize the DCMI driver */
    dcmiObjectInit(&DCMID1);

    /* Start DMCI */
    dcmiStart(&DCMID1, &dcmicfg);

    /* Initializes the jpeg mode */
    initializeJPEG();

    /* Camera is ready */
    cameraReady = 1;
}

/*
 * Command to take a picture
 */
void cmdCamera(BaseSequentialStream *chp, int argc, char *argv[]){
    (void) argv;
    if (argc != 1){
        chprintf(chp, "Usage : camera filename\r\n");
        return;
    }

    if(!cameraReady){
        chprintf(chp, "The camera is not ready\r\n");
        return;
    }

    if (!sdIsReady()) {
        chprintf(chp, "The sd card is not ready\r\n");
        return;
    }

    /* File object */
    FIL fil;
    FRESULT res;

    res = f_open(&fil, argv[0], FA_WRITE | FA_OPEN_ALWAYS);
    if(res) {
        chprintf(chp, "Problem while creating the file\r\n");
        return;
    }

    chprintf(chp, "Begin photo\r\n");

    chEvtRegisterMask(&dmaEvS, &dmaEvL, EVENT_MASK(1));
    chEvtRegisterMask(&frameEvS, &frameEvL, EVENT_MASK(2));
    chprintf(chp, "Beginning transfer:\n\r");

    /*
     * using synchronous API for simplicity, single buffer.
     * limits max image size to available SRAM. Note that max DMA transfers in one go is 65535.
     * i.e. IMG_SIZE cannot be larger than 65535 here.
     */
    dcmiStartReceiveOneShot(&DCMID1, IMG_SIZE/2, imgBuf0, imgBuf1);

    eventmask_t currentEv;
    int currentBuf = 0;

    /* Receive all datas*/
    do {
        chprintf(chp, "Wait for event\n\r");
        currentEv = chEvtWaitOne(EVENT_MASK(1) | EVENT_MASK(2));
        chprintf(chp, "Got DMA interrupt, and DCMI interrupt.\n\r");

        /* Write the file */
        UINT written = 0;
        /* Write in the right buffer */
        if (currentBuf)
            f_write(&fil, (char *) imgBuf1, sizeof(imgBuf)/2, &written);
        else
            f_write(&fil, (char *) imgBuf0, sizeof(imgBuf)/2, &written);
        /* Change buffer */
        currentBuf = !currentBuf;
        if (written != sizeof(imgBuf)/2) {
            f_unlink(argv[0]);
            chprintf(chp, "Problem while writting\r\n");
            dmaStreamDisable(DCMID1.dmarx);
            return;
        }

    } while (currentEv == EVENT_MASK(1));

    /* Unregister the listeners */
    chEvtUnregister(&dmaEvS, &dmaEvL);
    chEvtUnregister(&frameEvS, &frameEvL);

    res = f_close(&fil);
    if (res) {
        f_unlink(argv[0]);
        chprintf(chp, "Problem while closing the file\r\n");
        return;
    }
}

/*
 * Configures the brightness of the camera
 *     0x40 for Brightness +2,
 *     0x30 for Brightness +1,
 *     0x20 for Brightness 0,
 *     0x10 for Brightness -1,
 *     0x00 for Brightness -2,
 */
void cameraSetBrightness(uint8_t brightness){
    sccbWrite(0xff, 0x00);
    sccbWrite(0x7c, 0x00);
    sccbWrite(0x7d, 0x04);
    sccbWrite(0x7c, 0x09);
    sccbWrite(0x7d, brightness);
    sccbWrite(0x7d, 0x00);
}

/*
 * Configures the black and white mode.
 *     0x18 for B&W,
 *     0x40 for Negative,
 *     0x58 for B&W negative,
 *     0x00 for Normal,
 */
void cameraSetBW(uint8_t blackWhite){
    sccbWrite(0xff, 0x00);
    sccbWrite(0x7c, 0x00);
    sccbWrite(0x7d, blackWhite);
    sccbWrite(0x7c, 0x05);
    sccbWrite(0x7d, 0x80);
    sccbWrite(0x7d, 0x80);
}

/*
 * Configures the colors effects.
 *     value1 = 0x40, value2 = 0xa6 for Antique,
 *     value1 = 0xa0, value2 = 0x40 for Bluish,
 *     value1 = 0x40, value2 = 0x40 for Greenish,
 *     value1 = 0x40, value2 = 0xc0 for Reddish,
 */
void cameraSetColorEffect(uint8_t value1, uint8_t value2){
    sccbWrite(0xff, 0x00);
    sccbWrite(0x7c, 0x00);
    sccbWrite(0x7d, 0x18);
    sccbWrite(0x7c, 0x05);
    sccbWrite(0x7d, value1);
    sccbWrite(0x7d, value2);
}

/*
 * Configures the contraste of the image.
 *     value1 = 0x28, value2 = 0x0c for Contrast +2,
 *     value1 = 0x24, value2 = 0x16 for Contrast +1,
 *     value1 = 0x20, value2 = 0x20 for Contrast 0,
 *     value1 = 0x1c, value2 = 0x2a for Contrast -1,
 *     value1 = 0x18, value2 = 0x34 for Contrast -2,
 */
void cameraSetContrast(uint8_t value1, uint8_t value2){
    sccbWrite(0xff, 0x00);
    sccbWrite(0x7c, 0x00);
    sccbWrite(0x7d, 0x04);
    sccbWrite(0x7c, 0x07);
    sccbWrite(0x7d, 0x20);
    sccbWrite(0x7d, value1);
    sccbWrite(0x7d, value2);
    sccbWrite(0x7d, 0x06);
}

