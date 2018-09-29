#include "ledspi.h"
uint16_t PixelBuffer[4000] = {0};			//150��WS2811����������Ҫ3600��u16�����ݣ��ټ���20λ�ĵ͵�ƽ��λ�ź�
uint16_t PixelPointer = 0;

const unsigned long WSDataInitial[nWS] = {0};
const unsigned long WSData1[3] = {0xFF0000, 0X00FF00, 0X0000FF};
const unsigned long WSData2[3] = {0x00AA00, 0X0000BB, 0XCC0000};
const unsigned long WSData3[3] = {0xABC000, 0X000ABC, 0X00ABC0};
const unsigned long WSData4[3] = {0xDEF000, 0X000DEF, 0X0DEF00};


//PA7,SPI1 MOSI,��ӦDMA1ͨ��3
void LED_SPI_Init(void)
{
    u16 i = 0;
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef   SPI_InitStructure;
    DMA_InitTypeDef   DMA_InitStructure;

    //ʱ�ӳ�ʼ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    //DMA��ʼ��
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

    //IO�ڳ�ʼ��
    /*MOSI*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //SPI��ʼ��
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
    u8 Red, Green, Blue;		//��ԭɫ
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
        DMA_Cmd(DMA1_Channel3, ENABLE);							//ʹ��
        while(!DMA_GetFlagStatus(DMA1_FLAG_TC3));		//�ȴ��������
        DMA_Cmd(DMA1_Channel3, DISABLE);						//ʧ��
        DMA_ClearFlag(DMA1_FLAG_TC3);								//���ת����ɱ�־λ


    }
}





