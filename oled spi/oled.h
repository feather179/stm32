#ifndef __OLED_H
#define __OLED_H
#include "sys.h"

#define			OLED_CMD		0		//写命令
#define			OLED_DATA		1		//写数据

#define			OLED_CLK		PBout(12)		//CLK时钟，D0
#define			OLED_MOSI		PBout(13)		//MOSI		D1
#define			OLED_RST		PBout(14)		//RET复位  ret
#define			OLED_DC			PBout(15)		//命令/数据  DC(0表示传输命令，1表示传输数据)



void OLED_Init(void);//初始化OLED
void OLED_ON(void);//唤醒OLED
void OLED_OFF(void);//OLED休眠
void OLED_Refresh_Gram(void);//更新显存到OLED
void OLED_Clear(void);//清屏
void OLED_DrawPoint(u8 x,u8 y,u8 t);//画点
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);//填充
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size, u8 mode);//显示字符
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);//显示2个数字
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size);//显示字符串
void OLED_ShowFloat(u8 x, u8 y, float num, u8 intlen, u8 floatlen, u8 size);		//显示小数


#endif

