#ifndef __OLED_H
#define __OLED_H
#include "sys.h"

#define			OLED_CMD		0		//д����
#define			OLED_DATA		1		//д����

#define			OLED_CLK		PBout(12)		//CLKʱ�ӣ�D0
#define			OLED_MOSI		PBout(13)		//MOSI		D1
#define			OLED_RST		PBout(14)		//RET��λ  ret
#define			OLED_DC			PBout(15)		//����/����  DC(0��ʾ�������1��ʾ��������)



void OLED_Init(void);//��ʼ��OLED
void OLED_ON(void);//����OLED
void OLED_OFF(void);//OLED����
void OLED_Refresh_Gram(void);//�����Դ浽OLED
void OLED_Clear(void);//����
void OLED_DrawPoint(u8 x,u8 y,u8 t);//����
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);//���
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size, u8 mode);//��ʾ�ַ�
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);//��ʾ2������
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size);//��ʾ�ַ���
void OLED_ShowFloat(u8 x, u8 y, float num, u8 intlen, u8 floatlen, u8 size);		//��ʾС��


#endif

