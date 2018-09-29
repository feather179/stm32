#include "ledspi.h"
uint16_t PixelBuffer[4000] = {0};			//150颗WS2811级联共计需要3600个u16的数据，再加上20位的低电平复位信号
uint16_t PixelPointer = 0;

const unsigned long WSDataInitial[nWS] = {0};
const unsigned long WSData1[3] = {0xFF0000, 0X00FF00, 0X0000FF};
const unsigned long WSData2[3] = {0x00AA00, 0X0000BB, 0XCC0000};
const unsigned long WSData3[3] = {0xABC000, 0X000ABC, 0X00ABC0};
const unsigned long WSData4[3] = {0xDEF000, 0X000DEF, 0X0DEF00};


//PA7,SPI1 MOSI,对应DMA1通道3
void LED_SPI_Init(void)
{
    u16 i = 0;
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef   SPI_InitStructure;
    DMA_InitTypeDef   DMA_InitStructure;

    //时钟初始化
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    //DMA初始化
    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)PixelBuffer;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (SPI1->DR);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    //IO口初始化
    /*MOSI*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //SPI初始化
    SPI_I2S_DeInit(SPI1);
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_Cmd(SPI1, ENABLE);



    for(i = 0; i < nWS; i++)
    {
        PixelBuffer[i] = 0xAAAA;
    }
    PixelPointer = 0;
		
		LED_SPI_Update(WSDataInitial, nWS);
}


void LED_SPI_WriteByte(u16 data)
{
    PixelBuffer[PixelPointer] = data;
    PixelPointer++;
}

void LED_SPI_SendBits(u8 bits)
{
    u16 zero = 0XF800;		//1111 1000 0000 0000
    u16 one	 = 0XFFE0;		//1111 1111 1110 0000
    u8 i = 0X00;

    for(i = 0X80; i >= 0X01; i >>= 1)
    {
        LED_SPI_WriteByte((bits & i) ? one : zero);
    }
}

void LED_SPI_SendPixel(u32 color)
{
    /*
    r7,r6,r5,r4,r3,r2,r1,r0,g7,g6,g5,g4,g3,g2,g1,g0,b7,b6,b5,b4,b3,b2,b1,b0
    \_____________________________________________________________________/
                           |      _________________...
                           |     /   __________________...
                           |    /   /   ___________________...
                           |   /   /   /
                          RGB,RGB,RGB,RGB,...,STOP
    */

    /*
    	BUG Fix : Actual is GRB,datasheet is something wrong.
    */
    u8 Red, Green, Blue;		//三原色
    Red = color >> 8;
    Green = color >> 16;
    Blue = color;

    LED_SPI_SendBits(Green);
    LED_SPI_SendBits(Red);
    LED_SPI_SendBits(Blue);
}

void LED_SPI_Update(const unsigned long buffer[], u16 length)
{
    u16 i = 0;

    if(DMA_GetCurrDataCounter(DMA1_Channel3) == 0)
    {
        for(i = 0; i < length; i++)
        {
            LED_SPI_SendPixel(buffer[i]);
        }

        for(i = 0; i < 20; i++)
        {
            LED_SPI_WriteByte(0X00);
        }
        PixelPointer = 0;

        DMA_SetCurrDataCounter(DMA1_Channel3, 24 * length + 20);
        DMA_Cmd(DMA1_Channel3, ENABLE);							//使能
        while(!DMA_GetFlagStatus(DMA1_FLAG_TC3));		//等待传送完成
        DMA_Cmd(DMA1_Channel3, DISABLE);						//失能
        DMA_ClearFlag(DMA1_FLAG_TC3);								//清除转换完成标志位


    }
}





