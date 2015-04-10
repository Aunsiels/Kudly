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
 * @file    dcmi.h
 * @brief   DCMI Driver macros and structures.
 *
 * @addtogroup DCMI
 * @{
 */

#ifndef _DCMI_H_
#define _DCMI_H_

#if HAL_USE_DCMI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    DCMI configuration options
 * @{
 */
/**
 * @brief   Enables synchronous APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(DCMI_USE_WAIT) || defined(__DOXYGEN__)
#define DCMI_USE_WAIT                TRUE
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  DCMI_UNINIT = 0,                   /**< Not initialized.                   */
  DCMI_STOP = 1,                     /**< Stopped.                           */
  DCMI_READY = 2,                    /**< Ready to begin listening.          */
  DCMI_LISTEN = 3,                   /**< Listening for frames, continuous   */
  DCMI_LISTEN_ONCE = 4,              /**< Listening for frames, one shot     */
  DCMI_COMPLETE = 5                  /**< Frame complete, callback pending   */
} dcmistate_t;

#include "dcmi_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Begins reception of frames on the DCMI.
 * @details This asynchronous function starts a receive operation.
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
 * @iclass
 */
#define dcmiStartReceiveI(dcmip, n, rxbuf0, rxbuf1) {                         \
  (dcmip)->state = DCMI_LISTEN;                                               \
  dcmi_lld_receive(dcmip, n, FALSE, rxbuf0, rxbuf1);                          \
}

/**
 * @brief   Receives a single frame over the DCMI
 * @details This asynchronous function starts a single shot receive operation.
 * @post    Upon either of the two buffers being filled, the configured callback
 *          (transfer_complete_cb) is invoked.
 *          At the end of the operation the configured callback
 *          (frame_end_cb) is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] dcmip     pointer to the @p DCMIDriver object
 * @param[in] n         Size of each receive buffer, in DCMI words.
 * @param[in] rxbuf0    the pointer to the first receive buffer
 * @param[in] rxbuf1    the pointer to the second receive buffer
 *
 * @iclass
 */
#define dcmiStartReceiveOneShotI(dcmip, n, rxbuf0, rxbuf1) {                  \
  (dcmip)->state = DCMI_LISTEN_ONCE;                                          \
  dcmi_lld_receive(dcmip, n, TRUE, rxbuf0, rxbuf1);                                 \
}

/**
 * @name    Low Level driver helper macros
 * @{
 */
#if DCMI_USE_WAIT || defined(__DOXYGEN__)
/**
 * @brief   Waits for operation completion.
 * @details This function waits for the driver to complete the current
 *          operation.
 * @pre     An operation must be running while the function is invoked.
 * @note    No more than one thread can wait on a DCMI driver using
 *          this function.
 *
 * @param[in] dcmip      pointer to the @p DCMIDriver object
 *
 * @notapi
 */
#define _dcmi_wait_s(dcmip) {                                                 \
  chDbgAssert((dcmip)->thread == NULL,                                       \
              "_dcmi_wait(), #1", "already waiting");                        \
  (dcmip)->thread = chThdSelf();                                             \
  chSchGoSleepS(THD_STATE_SUSPENDED);                                       \
}

/**
 * @brief   Wakes up the waiting thread.
 *
 * @param[in] dcmip      pointer to the @p DCMIDriver object
 *
 * @notapi
 */
#define _dcmi_wakeup_isr(dcmip) {                                             \
  if ((dcmip)->thread != NULL) {                                             \
    Thread *tp = (dcmip)->thread;                                            \
    (dcmip)->thread = NULL;                                                 \
    chSysLockFromIsr();                                                     \
    chSchReadyI(tp);                                                        \
    chSysUnlockFromIsr();                                                   \
  }                                                                         \
}
#else /* !DCMI_USE_WAIT */
#define _dcmi_wait_s(dcmip)
#define _dcmi_wakeup_isr(dcmip)
#endif /* !DCMI_USE_WAIT */

/**
 * @brief   Common DCMI Frame Complete ISR code
 * @details This code handles the portable part of the ISR code:
 *          - Callback invocation.
 *          - Waiting thread wakeup, if any.
 *          - Driver state transitions.
 *          .
 * @note    This macro is meant to be used in the low level drivers
 *          implementation only.
 *
 * @param[in] dcmip      pointer to the @p DCMIDriver object
 *
 * @notapi
 */
#define _dcmi_isr_code(dcmip) {                                               \
  bool_t oneShot = ((dcmip)->state == DCMI_LISTEN_ONCE);                      \
  if ((dcmip)->config->frame_end_cb != NULL) {                                \
    (dcmip)->state = DCMI_COMPLETE;                                           \
    (dcmip)->config->frame_end_cb(dcmip);                                     \
    if ((dcmip)->state == DCMI_COMPLETE)                                      \
      (dcmip)->state = oneShot ? DCMI_READY : DCMI_LISTEN;                    \
  }                                                                           \
  else {                                                                      \
    (dcmip)->state = oneShot ? DCMI_READY : DCMI_LISTEN;                      \
  }                                                                           \
  _dcmi_wakeup_isr(dcmip);                                                    \
}
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void dcmiInit(void);
  void dcmiObjectInit(DCMIDriver *dcmip);
  void dcmiStart(DCMIDriver *dcmip, const DCMIConfig *config);
  void dcmiStop(DCMIDriver *dcmip);
  void dcmiStartReceive(DCMIDriver *dcmip, size_t n,
                        void* rxbuf0, void* rxbuf1);
  void dcmiStartReceiveOneShot(DCMIDriver *dcmip, size_t n,
                        void* rxbuf0, void* rxbuf1);
#if DCMI_USE_WAIT
  void dcmiReceiveOneShot(DCMIDriver *dcmip, size_t n,
                        void* rxbuf0, void* rxbuf1);
#endif /* DCMI_USE_WAIT */

#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_DCMI */

#endif /* _DCMI_H_ */

/** @} */
