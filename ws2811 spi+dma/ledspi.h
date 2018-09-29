#ifndef __LEDSPI_H
#define __LEDSPI_H
#include "sys.h"
#include "delay.h"

#define		nWS		150		//150¿ÅWS2811¼¶Áª

extern const unsigned long WSData1[3];
extern const unsigned long WSData2[3];
extern const unsigned long WSData3[3];
extern const unsigned long WSData4[3];

void LED_SPI_Init(void);
void LED_SPI_Update(const unsigned long buffer[], u16 length);



#endif

