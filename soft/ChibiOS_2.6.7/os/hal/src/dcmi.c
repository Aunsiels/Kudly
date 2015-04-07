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
 * @file    dcmi.c
 * @brief   DCMI Driver code.
 *
 * @addtogroup DCMI
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_DCMI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   DCMI Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void dcmiInit(void) {

  dcmi_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p DCMIDriver structure.
 *
 * @param[out] dcmip     pointer to the @p DCMIDriver object
 *
 * @init
 */
void dcmiObjectInit(DCMIDriver *dcmip) {
  dcmip->state = DCMI_STOP;
  dcmip->config = NULL;
#if DCMI_USE_WAIT
  dcmip->thread = NULL;
#endif /* DCMI_USE_WAIT */
#if defined(DCMI_DRIVER_EXT_INIT_HOOK)
  DCMI_DRIVER_EXT_INIT_HOOK(dcmip);
#endif
}


/**
 * @brief   Configures and activates the DCMI peripheral.
 *
 * @param[in] dcmip      pointer to the @p DCMIDriver object
 * @param[in] config    pointer to the @p DCMIConfig object
 *
 * @api
 */
void dcmiStart(DCMIDriver *dcmip, const DCMIConfig *config) {
  chDbgCheck((dcmip != NULL) && (config != NULL), "dcmiStart");

  chSysLock();
  chDbgAssert((dcmip->state == DCMI_STOP) || (dcmip->state == DCMI_READY),
              "dcmiStart(), #1", "invalid state");
  dcmip->config = config;
  dcmi_lld_start(dcmip);
  dcmip->state = DCMI_READY;
  chSysUnlock();
}


/**
 * @brief Deactivates the DCMI peripheral.
 *
 * @param[in] dcmip      pointer to the @p DCMIDriver object
 *
 * @api
 */
void dcmiStop(DCMIDriver *dcmip) {

  chDbgCheck(dcmip != NULL, "dcmiStop");

  chSysLock();
  chDbgAssert((dcmip->state == DCMI_STOP) || (dcmip->state == DCMI_READY),
              "dcmiStop(), #1", "invalid state");
  dcmi_lld_stop(dcmip);
  dcmip->state = DCMI_STOP;
  chSysUnlock();
}


/**
 * @brief   Begins reception of frames on the DCMI.
 * @details This asynchronous function starts a continuous receive operation.
 * @post    Upon either of the two buffers being filled, the configured callback
 *          (transfer_complete_cb) is invoked.
 *          At the end of each frame the configured callback 
 *          (frame_end_cb) is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes equal to 
 *          8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] dcmip     pointer to the @p DCMIDriver object
 * @param[in] n         Size of each receive buffer, in DCMI words.
 * @param[out] rxbuf0   the pointer to the first receive buffer
 * @param[out] rxbuf1   the pointer to the second receive buffer
 *
 */
void dcmiStartReceive(DCMIDriver *dcmip, size_t n, void* rxbuf0, void* rxbuf1) {

  chDbgCheck((dcmip != NULL) && (n > 0) && (n < 65536) && (rxbuf0 != NULL),
             "dcmiStartReceive");

  chSysLock();
  chDbgAssert(dcmip->state == DCMI_READY, "dcmiStartReceive(), #1", "not ready");
  dcmiStartReceiveI(dcmip, n, rxbuf0, rxbuf1);
  chSysUnlock();
}


/**
 * @brief   Receives a single frame over the DCMI
 * @details This asynchronous function starts a single shot receive operation.
 * @post    Upon either of the two receive buffers being filled, the configured callback
 *          (transfer_complete_cb) is invoked.
 *          At the end of the frame the configured callback (frame_end_cb) is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] dcmip     pointer to the @p DCMIDriver object
 * @param[in] n         Size of each receive buffer, in DCMI words.
 * @param[in] rxbuf0    the pointer to the first receive buffer
 * @param[in] rxbuf1    the pointer to the second receive buffer
 *
 */
void dcmiStartReceiveOneShot(DCMIDriver *dcmip, size_t n, void* rxbuf0, void* rxbuf1) {
   chDbgCheck((dcmip != NULL) && ( n > 0 ) && ( n < 65536 ) && (rxbuf0 != NULL),
              "dcmiStartReceiveOneShot");

   chSysLock();
   chDbgAssert(dcmip->state == DCMI_READY, "dcmiStartReceiveOneShot(), #1", "not ready");
   dcmiStartReceiveOneShotI(dcmip, n, rxbuf0, rxbuf1);
   chSysUnlock();
}



#if DCMI_USE_WAIT || defined(__DOXYGEN__)
/**
 * @brief   Receives a single frame over the DCMI
 * @details This synchronous function performs a single shot receive operation.
 * @pre     A frame_end_cb callback must not be configured (DMA transfer_complete_cb may be non-null).
 * @post    Upon either of the two receive buffers being filled, the configured callback
 *          (transfer_complete_cb) is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] dcmip     pointer to the @p DCMIDriver object
 * @param[in] n         Size of each receive buffer, in DCMI words.
 * @param[in] rxbuf0    the pointer to the first receive buffer
 * @param[in] rxbuf1    the pointer to the second receive buffer
 *
 */
void dcmiReceiveOneShot(DCMIDriver *dcmip, size_t n, void* rxbuf0, void* rxbuf1) {
  chDbgCheck((dcmip != NULL) && (n > 0) && (n < 65536) && (rxbuf0 != NULL),
             "dcmiReceive");

  chSysLock();
  chDbgAssert(dcmip->state == DCMI_READY, "dcmiReceive(), #1", "not ready");
  chDbgAssert(dcmip->config->frame_end_cb == NULL,
              "dcmiReceive(), #2", "has callback");
  dcmiStartReceiveOneShotI(dcmip, n, rxbuf0, rxbuf1);
  chSysUnlock();
  _dcmi_wait_s(dcmip);
}
#endif /* DCMI_USE_WAIT */

#endif /* HAL_USE_DCMI */

/** @} */
