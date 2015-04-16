/*
 * bmp.h - contains declaration of Windows BMP format, in order to output data from
 * Toshiba TCM8230MD camera module on the Camera board.
 * Author: Ewan Coldicott
 * Date: 14 June 2012
 */
 
#ifndef _BMP_H_
#define _BMP_H_

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  /*
   * Encapsulates the provided image data (RGB565) into a Windows NT BMP file format.
   * Transmits the image over the BaseSequentialStream I/O channel pointed to by io.
   * Assumes the image data is organised into horizontal scan lines (left to right)
   *   in sequence from the top of the image to the bottom.
   * Width, height are in pixels.
   */
  void send16bppBmpImage( BaseSequentialStream* io, uint16_t* pixels, int32_t width, int32_t height );
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _CAMERA_H_ */
