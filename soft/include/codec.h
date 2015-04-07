#ifndef CODEC_H
#define CODEC_H

void codecInit(void);
void codecLowPower(void);
void codecEncodeSound(int);

void writeRam(uint16_t,uint16_t);
void writeRam32(uint16_t,uint32_t);
uint16_t readRam(uint16_t);
uint32_t readRam32(uint16_t);

#endif /* CODEC_H */
