#include "tm1650.h"

//�������ʾ����
const u8 TM1650Buff[10] = {0X3F, 0X06, 0X5B, 0X4F, 0X66, 0X6D, 0X7D, 0X07, 0X7F, 0X6F};

void IIC_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOC, GPIO_Pin_13 | GPIO_Pin_14);
}


//����IIC��ʼ�ź�
void IIC_Start(void)
{
    SDA_OUT();     //sda�����
    IIC_SDA = 1;
    IIC_SCL = 1;
    delay_us(4);
    IIC_SDA = 0; //START:when CLK is high,DATA change form high to low
    delay_us(4);
    IIC_SCL = 0; //ǯסI2C���ߣ�׼�����ͻ��������
}

//����IICֹͣ�ź�
void IIC_Stop(void)
{
    SDA_OUT();//sda�����
    IIC_SCL = 0;
    IIC_SDA = 0; //STOP:when CLK is high DATA change form low to high
    delay_us(4);
    IIC_SCL = 1;
    IIC_SDA = 1; //����I2C���߽����ź�
    delay_us(4);
}


//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    SDA_IN();      //SDA����Ϊ����
    IIC_SDA = 1;
    delay_us(1);
    IIC_SCL = 1;
    delay_us(1);
    while(READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime > 250)
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL = 0; //ʱ�����0
    return 0;
}

//����ACKӦ��
void IIC_Ack(void)
{
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 0;
    delay_us(2);
    IIC_SCL = 1;
    delay_us(2);
    IIC_SCL = 0;
}

//������ACKӦ��
void IIC_NAck(void)
{
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 1;
    delay_us(2);
    IIC_SCL = 1;
    delay_us(2);
    IIC_SCL = 0;
}

//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��
void IIC_Send_Byte(u8 txd)
{
    u8 t;
    SDA_OUT();
    IIC_SCL = 0; //����ʱ�ӿ�ʼ���ݴ���
    for(t = 0; t < 8; t++)
    {
        IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1;
        delay_us(2);   //��TEA5767��������ʱ���Ǳ����
        IIC_SCL = 1;
        delay_us(2);
        IIC_SCL = 0;
        delay_us(2);
    }
}

//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
u8 IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    SDA_IN();//SDA����Ϊ����
    for(i = 0; i < 8; i++ )
    {
        IIC_SCL = 0;
        delay_us(2);
        IIC_SCL = 1;
        receive <<= 1;
        if(READ_SDA)receive++;
        delay_us(1);
    }
    if (!ack)
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK
    return receive;
}


//��TM1650ָ����ַд��1Byte������
void TM1650_WriteData(u8 addr, u8 data)
{
		IIC_Start();
    IIC_Send_Byte(addr);			//��ַ
    IIC_Wait_Ack();
    IIC_Send_Byte(data);			//����
    IIC_Wait_Ack();
		IIC_Stop();
}


void TM1650_Init(void)
{
    IIC_Init();
	
    IIC_Start();
    IIC_Send_Byte(0X48);			//����ģʽΪ���������
    IIC_Wait_Ack();
    IIC_Send_Byte(0X51);			//5�����ȣ�8����ʾ�����������
    IIC_Wait_Ack();
		IIC_Stop();
	
		
		TM1650_OFF();
}


//�ر������
void TM1650_OFF(void)
{
		//������ܵ���λ��д��0X00����Ч��ȫ��Ϩ��
    TM1650_WriteData(BIT1_ADDR, 0X00);
		TM1650_WriteData(BIT2_ADDR, 0X00);
		TM1650_WriteData(BIT3_ADDR, 0X00);
		TM1650_WriteData(BIT4_ADDR, 0X00);
}



//���������ʾһ����λС�����ֵĸ�����
void TM1650_SHOW_Float(float data)
{
    int tmp = data * 100;
		u8 bit1 = (tmp / 1000);
    u8 bit2 = (tmp / 100 % 10);		//����ʾ1��С����
    u8 bit3 = (tmp / 10 % 10);
    u8 bit4 = (tmp % 10);
	
		TM1650_WriteData(BIT1_ADDR, TM1650Buff[bit1]);		//��һλ�����д��λ
		TM1650_WriteData(BIT2_ADDR, TM1650Buff[bit2] | 0X80);		//����ʾһ��С����
		TM1650_WriteData(BIT3_ADDR, TM1650Buff[bit3]);
		TM1650_WriteData(BIT4_ADDR, TM1650Buff[bit4]);
}


void TM1650_SHOW_Int(int data)
{
    u8 bit1 = (data / 1000);
    u8 bit2 = (data / 100 % 10);		
    u8 bit3 = (data / 10 % 10);
    u8 bit4 = (data % 10);

		TM1650_WriteData(BIT1_ADDR, TM1650Buff[bit1]);		//��һλ�����д��λ
		TM1650_WriteData(BIT2_ADDR, TM1650Buff[bit2]);
		TM1650_WriteData(BIT3_ADDR, TM1650Buff[bit3]);
		TM1650_WriteData(BIT4_ADDR, TM1650Buff[bit4]);

}





