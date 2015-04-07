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
 * @file    STM32F4xx/dcmi_lld.h
 * @brief   STM32 DCMI subsystem low level driver header.
 *
 * @addtogroup DCMI
 * @{
 */

#ifndef _DCMI_LLD_H_
#define _DCMI_LLD_H_

#if HAL_USE_DCMI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define STM32_DCMI_CR_CM        DCMI_CR_CM
#define STM32_DCMI_CR_CROP      DCMI_CR_CROP
#define STM32_DCMI_CR_ESS       DCMI_CR_ESS
#define STM32_DCMI_CR_PCKPOL    DCMI_CR_PCKPOL
#define STM32_DCMI_CR_HSPOL     DCMI_CR_HSPOL
#define STM32_DCMI_CR_VSPOL     DCMI_CR_VSPOL
#define STM32_DCMI_CR_FCRC_0    DCMI_CR_FCRC_0
#define STM32_DCMI_CR_FCRC_1    DCMI_CR_FCRC_1
#define STM32_DCMI_CR_EDM_0     DCMI_CR_EDM_0
#define STM32_DCMI_CR_EDM_1     DCMI_CR_EDM_1
#define STM32_DCMI_CR_CAPTURE   DCMI_CR_CRE
#define STM32_DCMI_CR_ENABLE    DCMI_CR_ENABLE

#define STM32_DCMI_CR_EDM_MASK (STM32_DCMI_CR_EDM_0|STM32_DCMI_CR_EDM_1)

#define STM32_DCMI_IER_FRAME_IE DCMI_IER_FRAME_IE
#define STM32_DCMI_IER_OVF_IE   DCMI_IER_OVF_IE
#define STM32_DCMI_IER_ERR_IE   DCMI_IER_ERR_IE
#define STM32_DCMI_IER_VSYNC_IE DCMI_IER_VSYNC_IE
#define STM32_DCMI_IER_LINE_IE  DCMI_IER_LINE_IE

#define STM32_DCMI_ICR_FRAME_ISC DCMI_ICR_FRAME_ISC

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   DCMI1 driver enable switch.
 * @details If set to @p TRUE the support for DCMI1 is included.
 * @note    The default is @p TRUE, since only DCMI1 is currently supported.
 */
#if !defined(STM32_DCMI_USE_DCMI1) || defined(__DOXYGEN__)
#define STM32_DCMI_USE_DCMI1                  TRUE
#endif

/**
 * @brief   DCMI1 DCMI interrupt priority level setting.
 */
#if !defined(STM32_DCMI_DCMI1_DCMI_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DCMI_DCMI1_DCMI_IRQ_PRIORITY         11
#endif

/**
 * @brief   DCMI1 DMA interrupt priority level setting.
 */
#if !defined(STM32_DCMI_DCMI1_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DCMI_DCMI1_DMA_IRQ_PRIORITY         10
#endif

/**
 * @brief   DCMI1 DMA priority (0..3|lowest..highest).
 */
#if !defined(STM32_DCMI_DCMI1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DCMI_DCMI1_DMA_PRIORITY         2
#endif

/**
 * @brief   DCMI DMA error hook.
 */
#if !defined(STM32_DCMI_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define STM32_DCMI_DMA_ERROR_HOOK(dcmip)      chSysHalt()
#endif

#if STM32_ADVANCED_DMA || defined(__DOXYGEN__)

/**
 * @brief   DMA stream used for DCMI1 RX operations.
 * @note    This option is only available on platforms with enhanced DMA.
 */
#if !defined(STM32_DCMI_DCMI1_RX_DMA_STREAM) || defined(__DOXYGEN__)
#define STM32_DCMI_DCMI1_RX_DMA_STREAM        STM32_DMA_STREAM_ID(2, 1)
#endif
#else /* !STM32_ADVANCED_DMA */
  #error "DCMI driver activated on platform without Advanced DMA - all platforms with DCMI have Advanced DMA"
#endif /* !STM32_ADVANCED_DMA*/
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if STM32_DCMI_USE_DCMI1 && !STM32_HAS_DCMI1
#error "DCMI1 not present in the selected device"
#endif

#if !STM32_DCMI_USE_DCMI1
#error "DCMI driver activated but no DCMI peripheral assigned"
#endif

#if STM32_DCMI_USE_DCMI1 &&                                                   \
   !CORTEX_IS_VALID_KERNEL_PRIORITY(STM32_DCMI_DCMI1_DCMI_IRQ_PRIORITY)
#error "Invalid DCMI IRQ priority assigned to DCMI1"
#endif

#if STM32_DCMI_USE_DCMI1 &&                                                   \
   !CORTEX_IS_VALID_KERNEL_PRIORITY(STM32_DCMI_DCMI1_DMA_IRQ_PRIORITY)
#error "Invalid DMA IRQ priority assigned to DCMI1"
#endif

#if STM32_DCMI_USE_DCMI1 &&                                                   \
    !STM32_DMA_IS_VALID_PRIORITY(STM32_DCMI_DCMI1_DMA_PRIORITY)
#error "Invalid DMA priority assigned to DCMI1"
#endif

#if STM32_DCMI_USE_DCMI1 &&                                                   \
    !STM32_DMA_IS_VALID_ID(STM32_DCMI_DCMI1_RX_DMA_STREAM, STM32_DCMI1_RX_DMA_MSK)
#error "invalid DMA stream associated to DCMI1 RX"
#endif

#if !defined(STM32_DMA_REQUIRED)
#define STM32_DMA_REQUIRED
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an DCMI driver.
 */
typedef struct DCMIDriver DCMIDriver;

/**
 * @brief   DCMI notification callback type.
 *
 * @param[in] dcmip      pointer to the @p DCMIDriver object triggering the
 *                      callback
 */
typedef void (*dcmicallback_t)(DCMIDriver *dcmip);

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /**
   * @brief DCMI frame complete callback or @p NULL.
   */
  dcmicallback_t             frame_end_cb;
  /**
   * @brief DMA transfer complete callback or @p NULL
   */
  dcmicallback_t             transfer_complete_cb;
  /* End of the mandatory fields.*/

  /**
   * @brief DCMI CR1 register initialization data.
   */
  uint32_t                  cr;
} DCMIConfig;

/**
 * @brief   Structure representing a DCMI driver.
 */
struct DCMIDriver{
  /**
   * @brief Driver state.
   */
  dcmistate_t                state;
  /**
   * @brief Current configuration data.
   */
  const DCMIConfig           *config;
#if DCMI_USE_WAIT || defined(__DOXYGEN__)
  /**
   * @brief Waiting thread.
   */
  Thread                    *thread;
#endif /* DCMI_USE_WAIT */

#if defined(DCMI_DRIVER_EXT_FIELDS)
  DCMI_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the DCMIx registers block.
   */
  DCMI_TypeDef               *dcmi;
  /**
   * @brief Receive DMA stream.
   */
  const stm32_dma_stream_t  *dmarx;
  /**
   * @brief RX DMA mode bit mask.
   */
  uint32_t                  rxdmamode;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_DCMI_USE_DCMI1 && !defined(__DOXYGEN__)
extern DCMIDriver DCMID1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void dcmi_lld_init(void);
  void dcmi_lld_start(DCMIDriver *dcmip);
  void dcmi_lld_stop(DCMIDriver *dcmip);
  void dcmi_lld_receive(DCMIDriver *dcmip, size_t n, bool_t oneShot,
                        void* rxbuf0, void* rxbuf1);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_DCMI */

#endif /* _DCMI_LLD_H_ */

/** @} */
