#include "hal.h"
#include "ch.h"
#include "dcmi.h"

/*
 * Initializes the dcmi
 */

void dcmiUninit() {
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
}
