/*
 * bmp.h - contains implementation of functions useful for outputting image data from
 * Toshiba TCM8230MD camera module on the Camera board in Windows BMP format
 * Author: Ewan Coldicott
 * Date: 14 June 2012
 */
 
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "bmp.h"

 typedef struct _WinBmpFileHeader
{
	uint16_t   FileType;       /* File type, always 4D42h ("BM") */
	uint32_t   FileSize;       /* Size of the file in bytes */
	uint16_t   Reserved1;      /* Always 0 */
	uint16_t   Reserved2;      /* Always 0 */
	uint32_t   BitmapOffset;   /* Starting position of image data in bytes */
} __attribute__((__packed__)) WinBmpFileHeader;

typedef struct _WinNtBitmapHeader
{
	uint32_t  Size;                /* Size of this header in bytes */
	int32_t   Width;               /* Image width in pixels */
	int32_t   Height;              /* Image height in pixels */
	uint16_t  Planes;              /* Number of color planes */
	uint16_t  BitsPerPixel;        /* Number of bits per pixel */
	uint32_t  Compression;         /* Compression methods used */
	uint32_t  SizeOfBitmap;        /* Size of bitmap in bytes */
	int32_t   HorzResolution;      /* Horizontal resolution in pixels per meter */
	int32_t   VertResolution;      /* Vertical resolution in pixels per meter */
	uint32_t  ColoursUsed;          /* Number of colors in the image */
	uint32_t  ColoursImportant;     /* Minimum number of important colors */
} __attribute__((__packed__)) WinNtBitmapHeader;

typedef struct _WinNtBitfieldsMasks
{
	uint32_t  RedMask;         /* Mask identifying bits of red component */
	uint32_t  GreenMask;       /* Mask identifying bits of green component */
	uint32_t  BlueMask;        /* Mask identifying bits of blue component */
	uint32_t  AlphaMask;	     /* Mask identifying alpha component of image */
} __attribute__((__packed__)) WinNtBitfieldsMasks;
 
 void send16bppBmpImage( BaseSequentialStream* io, uint16_t* pixels, int32_t width, int32_t height ) {
  WinBmpFileHeader fileHeader;
  WinNtBitmapHeader bitmapHeader;
  WinNtBitfieldsMasks masks;
  
  fileHeader.FileType = 0x4D42;     /* "BM" in ASCII - indicates Bitmap file */
  fileHeader.FileSize = 14 + 40 + 16 + 2*(width*height);
  fileHeader.Reserved1 = 0;
  fileHeader.Reserved2 = 0;
  fileHeader.BitmapOffset = 14 + 40 + 16;
  
  bitmapHeader.Size = 40 + 16;
  bitmapHeader.Width = width;
  bitmapHeader.Height = -height;    /* Negative heights indicates the image is scanned top-down, rather than the default bottom-up */
  bitmapHeader.Planes = 1;
  bitmapHeader.BitsPerPixel = 16;
  bitmapHeader.Compression = 3;     /* Indicates bitfields encoding (ie colours are not on byte boundaries) */
  bitmapHeader.SizeOfBitmap = 2*(width*height);
  bitmapHeader.HorzResolution = 3150 ;   /* 80 ppi resolution */
  bitmapHeader.VertResolution = 3150 ; 
  bitmapHeader.ColoursUsed = 0;          /* Defaults to 2^16 */
  bitmapHeader.ColoursImportant = 0;     /* All colours important */
  
  /* Following is from TCM8230MD datasheet: data is output as BGR (5b,6b,5b) */
  masks.RedMask = 0x0000F800;       /* 0000 0000 0000 0000 1111 1000 0000 0000 */
  masks.GreenMask = 0x000007E0;     /* 0000 0000 0000 0000 0000 0111 1110 0000 */
  masks.BlueMask = 0x0000001F;      /* 0000 0000 0000 0000 0000 0000 0001 1111 */
  masks.AlphaMask = 0x00000000;
  
  /* Ready to print image */
  chSequentialStreamWrite(io, (uint8_t*)&(fileHeader.FileType), sizeof(fileHeader.FileType));
  chSequentialStreamWrite(io, (uint8_t*)&(fileHeader.FileSize), sizeof(fileHeader.FileSize));
  chSequentialStreamWrite(io, (uint8_t*)&(fileHeader.Reserved1), sizeof(fileHeader.Reserved1));
  chSequentialStreamWrite(io, (uint8_t*)&(fileHeader.Reserved2), sizeof(fileHeader.Reserved2));
  chSequentialStreamWrite(io, (uint8_t*)&(fileHeader.BitmapOffset), sizeof(fileHeader.BitmapOffset));
  chSequentialStreamWrite(io, (uint8_t*)&(bitmapHeader.Size), sizeof(bitmapHeader.Size));
  chSequentialStreamWrite(io, (uint8_t*)&(bitmapHeader.Width), sizeof(bitmapHeader.Width));
  chSequentialStreamWrite(io, (uint8_t*)&(bitmapHeader.Height), sizeof(bitmapHeader.Height));
  chSequentialStreamWrite(io, (uint8_t*)&(bitmapHeader.Planes), sizeof(bitmapHeader.Planes));
  chSequentialStreamWrite(io, (uint8_t*)&(bitmapHeader.BitsPerPixel), sizeof(bitmapHeader.BitsPerPixel));
  chSequentialStreamWrite(io, (uint8_t*)&(bitmapHeader.Compression), sizeof(bitmapHeader.Compression));
  chSequentialStreamWrite(io, (uint8_t*)&(bitmapHeader.SizeOfBitmap), sizeof(bitmapHeader.SizeOfBitmap));
  chSequentialStreamWrite(io, (uint8_t*)&(bitmapHeader.HorzResolution), sizeof(bitmapHeader.HorzResolution));
  chSequentialStreamWrite(io, (uint8_t*)&(bitmapHeader.VertResolution), sizeof(bitmapHeader.VertResolution));
  chSequentialStreamWrite(io, (uint8_t*)&(bitmapHeader.ColoursUsed), sizeof(bitmapHeader.ColoursUsed));
  chSequentialStreamWrite(io, (uint8_t*)&(bitmapHeader.ColoursImportant), sizeof(bitmapHeader.ColoursImportant));
  chSequentialStreamWrite(io, (uint8_t*)&masks, 16);
  chSequentialStreamWrite(io, (uint8_t*)pixels, width*height*2);
  
  return;
 }
