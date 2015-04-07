/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    STM32F4xx/dcmi_lld.c
 * @brief   STM32 DCMI subsystem low level driver source.
 *
 * @addtogroup DCMI
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#if HAL_USE_DCMI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define DCMI1_RX_DMA_CHANNEL                                                 \
  STM32_DMA_GETCHANNEL(STM32_DCMI_DCMI1_RX_DMA_STREAM,                        \
                       STM32_DCMI1_RX_DMA_CHN)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief DCMI1 driver identifier.*/
#if STM32_DCMI_USE_DCMI1 || defined(__DOXYGEN__)
DCMIDriver DCMID1;
#endif

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Shared end-of-dma-rx service routine.
 *
 * @param[in] dcmip      pointer to the @p DCMIDriver object
 * @param[in] flags      pre-shifted content of the ISR register
 */
static void dcmi_lld_serve_dma_rx_interrupt(DCMIDriver *dcmip, uint32_t flags) {

  /* DMA errors handling.*/
#if defined(STM32_DCMI_DMA_ERROR_HOOK)
  if ((flags & (STM32_DMA_ISR_TEIF | STM32_DMA_ISR_DMEIF)) != 0) {
    STM32_DCMI_DMA_ERROR_HOOK(dcmip);
  }
#else
  (void)flags;
#endif

  if( dcmip->config->transfer_complete_cb != NULL ) {
    dcmip->config->transfer_complete_cb(dcmip);
  }
}



/*===========================================================================*/
/* Driver interrupt handlers                               */
/*===========================================================================*/

/**
 * @brief   DCMI interrupt handler
 *
 * @isr
 */
CH_IRQ_HANDLER(DCMI_IRQHandler) {
  CH_IRQ_PROLOGUE();

  palSetPad(GPIOA, GPIOA_LED2);
  #if STM32_DCMI_USE_DCMI1
    DCMI->ICR |= (1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0)|STM32_DCMI_ICR_FRAME_ISC;
    _dcmi_isr_code(&DCMID1);
  #endif

  CH_IRQ_EPILOGUE();
}


/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level DCMI driver initialization.
 *
 * @notapi
 */
void dcmi_lld_init(void) {

#if STM32_DCMI_USE_DCMI1
  dcmiObjectInit(&DCMID1);
  DCMID1.dcmi      = DCMI;
  DCMID1.dmarx     = STM32_DMA_STREAM(STM32_DCMI_DCMI1_RX_DMA_STREAM);
  DCMID1.rxdmamode = STM32_DMA_CR_CHSEL(1) |
                    STM32_DMA_CR_PL(STM32_DCMI_DCMI1_DMA_PRIORITY) |
                    STM32_DMA_CR_DIR_P2M |
                    STM32_DMA_CR_TCIE |
                    STM32_DMA_CR_DMEIE |
                    STM32_DMA_CR_TEIE |
                    STM32_DMA_CR_PBURST_SINGLE |
                    STM32_DMA_CR_MBURST_SINGLE |
                    STM32_DMA_CR_PSIZE_WORD |
                    STM32_DMA_CR_MSIZE_WORD |
                    STM32_DMA_CR_MINC ;
#endif

}

/**
 * @brief   Configures and activates the DCMI peripheral.
 *
 * @param[in] dcmip      pointer to the @p DCMIDriver object
 *
 * @notapi
 */
void dcmi_lld_start(DCMIDriver *dcmip) {
  /* If in stopped state then enables the DCMI and DMA clocks.*/
  rccEnableDCMI(FALSE);
  if (dcmip->state == DCMI_STOP) {
#if STM32_DCMI_USE_DCMI1
    if (&DCMID1 == dcmip) {
      bool_t b;
      b = dmaStreamAllocate(dcmip->dmarx,
                            STM32_DCMI_DCMI1_DMA_IRQ_PRIORITY,
                            (stm32_dmaisr_t)dcmi_lld_serve_dma_rx_interrupt,
                            (void *)dcmip);
      chDbgAssert(!b, "dcmi_lld_start(), #1", "stream already allocated");
    }
#endif
    /* DMA setup.*/
    dmaStreamSetPeripheral(dcmip->dmarx, &dcmip->dcmi->DR);
  }

  /* Configuration-specific DMA setup.*/
  dcmip->rxdmamode = (dcmip->rxdmamode & ~STM32_DMA_CR_SIZE_MASK) |
                    STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD;

  nvicEnableVector( DCMI_IRQn, 
             CORTEX_PRIORITY_MASK(STM32_DCMI_DCMI1_DCMI_IRQ_PRIORITY));

  dcmip->dcmi->IER |= STM32_DCMI_IER_FRAME_IE | STM32_DCMI_IER_VSYNC_IE;
  dcmip->dcmi->CR  |= (dcmip->config->cr & 
                      ~(STM32_DCMI_CR_CAPTURE | STM32_DCMI_CR_ENABLE));
}

/**
 * @brief   Deactivates the DCMI peripheral.
 *
 * @param[in] dcmip      pointer to the @p DCMIDriver object
 *
 * @notapi
 */
void dcmi_lld_stop(DCMIDriver *dcmip) {
  /* If in ready state then disables the DCMI clock.*/
  if (dcmip->state == DCMI_READY) {
    /* DCMI disable.*/
    dcmip->dcmi->CR &= ~(STM32_DCMI_CR_CAPTURE|STM32_DCMI_CR_ENABLE);
    dmaStreamRelease(dcmip->dmarx);

#if STM32_DCMI_USE_DCMI1
    if (&DCMID1 == dcmip)
      rccDisableDCMI(FALSE);
#endif
    nvicDisableVector(DCMI_IRQn);
 }
}

/**
 * @brief   Begins reception of frame over DCMI.
 * @details This function enables the DCMI 'Capture' bit and DMA stream
 * @post    During operation, the user-specified transfer_complete_cb will
 *          be invoked whenever n transfers have completed.
 *          The user-specified frame_end_cb will be invoked upon frame end.
 * @note    The buffers are organized as uint8_t arrays for data sizes
 *          equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] dcmip     pointer to the @p DCMIDriver object
 * @param[in] n         Size of each receive buffer, in DCMI words.
 * @param[in] oneShot   If true, the DCMI will be placed in snapshot mode.
 * @param[out] rxbuf0   The pointer to the first receive buffer. Non-NULL.
 * @param[out] rxbuf1   The pointer to the second receive buffer. May be NULL.
 *
 * @notapi
 */
void dcmi_lld_receive(DCMIDriver *dcmip, size_t n, bool_t oneShot,
                      void* rxbuf0, void* rxbuf1) {
  /* Get DMA ready first */
  uint32_t dmaMode;
  dmaStreamSetMemory0(dcmip->dmarx, rxbuf0);
  dmaStreamSetMemory1(dcmip->dmarx, rxbuf1);
  dmaStreamSetTransactionSize(dcmip->dmarx, n/4);
  //dmaMode = dcmip->rxdmamode | STM32_DMA_CR_MINC;
  dmaMode = STM32_DMA_CR_PL(2) |
    STM32_DMA_CR_PSIZE_WORD |
    STM32_DMA_CR_MSIZE_WORD |
    STM32_DMA_CR_PBURST_SINGLE |
    STM32_DMA_CR_MBURST_SINGLE |
    STM32_DMA_CR_MINC |
    STM32_DMA_CR_CHSEL(1);
  /* if second buffer not given, turn off double buffering */
  if ( rxbuf1 == NULL ) {
    dmaMode &= (~STM32_DMA_CR_DBM);
  } else {
    dmaMode |= STM32_DMA_CR_DBM;
  }
  dmaStreamSetMode(dcmip->dmarx, dmaMode);
  dmaStreamSetFIFO(dcmip->dmarx,
    STM32_DMA_FCR_FTH_HALF |
    STM32_DMA_FCR_DMDIS );
  dmaStreamEnable(dcmip->dmarx);

  /* Now the DCMI */
  //dcmip->dcmi->IER |= STM32_DCMI_IER_FRAME_IE;
  dcmip->dcmi->CR = (oneShot==TRUE) ? dcmip->dcmi->CR | STM32_DCMI_CR_CM
                            : dcmip->dcmi->CR & (~STM32_DCMI_CR_CM);
  dcmip->dcmi->CR |= STM32_DCMI_CR_ENABLE;
  dcmip->dcmi->CR |= STM32_DCMI_CR_CAPTURE;
}

#endif /* HAL_USE_DCMI */

/** @} */
