#ifndef __OLED_H
#define __OLED_H

#include "sys.h"
#include "delay.h"

//IO方向设置
#define OLEDSDA_IN()  {GPIOB->CRH&=0XFFFFFFF0;GPIOB->CRH|=(u32)8<<0;}	//PB8输入模式
#define OLEDSDA_OUT() {GPIOB->CRH&=0XFFFFFFF0;GPIOB->CRH|=(u32)3<<0;} //PB8输出模式
//IO操作函数	 
#define OLEDIIC_SCL    PBout(9) //SCL
#define OLEDIIC_SDA    PBout(8) //SDA	 
#define OLEDREAD_SDA   PBin(8)  //输入SDA 


#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


#define OLED_MODE 0
#define SIZE 8
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	

void OLED_Init(void);
void OLED_Clear(void);

void show_size8float4_2f(unsigned char X,unsigned char Y,float sum);
void show_size8float3_1f(unsigned char X,unsigned char Y,float sum);
void show_english_size8string(unsigned char X,unsigned char Y,u8 *s);
void show_size8float4_6f(unsigned char X,unsigned char Y,double sum);
void show_size16float4_2f(unsigned char X,unsigned char Y,float sum);
void show_size16float3_1f(unsigned char X,unsigned char Y,float sum);
void show_english_size16string(unsigned char X,unsigned char Y,u8 *s);
void show_size16float4_6f(unsigned char X,unsigned char Y,double sum);



#endif




