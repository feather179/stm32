#ifndef __TM1650_H
#define __TM1650_H
#include "delay.h"
#include "sys.h"

//SCL		PC14
//SDA		PC13
#define SDA_IN()		{GPIOC->CRH&=0XFF0FFFFF;GPIOC->CRH|=(u32)8<<20;}
#define SDA_OUT()		{GPIOC->CRH&=0XFF0FFFFF;GPIOC->CRH|=(u32)3<<20;}
	
#define IIC_SCL		PCout(14)
#define IIC_SDA		PCout(13)
#define READ_SDA	PCin(13)

#define BIT1_ADDR		0X68
#define BIT2_ADDR		0X6A
#define BIT3_ADDR		0X6C
#define BIT4_ADDR		0X6E




void TM1650_WriteData(u8 addr, u8 data);	//向数码管指定显存写入一个数据
void TM1650_Init(void);										//数码管初始化
void TM1650_OFF(void);										//关闭数码管显示
void TM1650_SHOW_Float(float data);				//显示两位小数的小数
void TM1650_SHOW_Int(int data);						//显示四位整数

#endif

