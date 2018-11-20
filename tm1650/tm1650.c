#include "tm1650.h"

//数码管显示缓存
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


//产生IIC起始信号
void IIC_Start(void)
{
    SDA_OUT();     //sda线输出
    IIC_SDA = 1;
    IIC_SCL = 1;
    delay_us(4);
    IIC_SDA = 0; //START:when CLK is high,DATA change form high to low
    delay_us(4);
    IIC_SCL = 0; //钳住I2C总线，准备发送或接收数据
}

//产生IIC停止信号
void IIC_Stop(void)
{
    SDA_OUT();//sda线输出
    IIC_SCL = 0;
    IIC_SDA = 0; //STOP:when CLK is high DATA change form low to high
    delay_us(4);
    IIC_SCL = 1;
    IIC_SDA = 1; //发送I2C总线结束信号
    delay_us(4);
}


//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    SDA_IN();      //SDA设置为输入
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
    IIC_SCL = 0; //时钟输出0
    return 0;
}

//产生ACK应答
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

//不产生ACK应答
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

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void IIC_Send_Byte(u8 txd)
{
    u8 t;
    SDA_OUT();
    IIC_SCL = 0; //拉低时钟开始数据传输
    for(t = 0; t < 8; t++)
    {
        IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1;
        delay_us(2);   //对TEA5767这三个延时都是必须的
        IIC_SCL = 1;
        delay_us(2);
        IIC_SCL = 0;
        delay_us(2);
    }
}

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
u8 IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    SDA_IN();//SDA设置为输入
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
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK
    return receive;
}


//向TM1650指定地址写入1Byte的数据
void TM1650_WriteData(u8 addr, u8 data)
{
		IIC_Start();
    IIC_Send_Byte(addr);			//地址
    IIC_Wait_Ack();
    IIC_Send_Byte(data);			//数据
    IIC_Wait_Ack();
		IIC_Stop();
}


void TM1650_Init(void)
{
    IIC_Init();
	
    IIC_Start();
    IIC_Send_Byte(0X48);			//设置模式为数码管驱动
    IIC_Wait_Ack();
    IIC_Send_Byte(0X51);			//5级亮度，8段显示，开启数码管
    IIC_Wait_Ack();
		IIC_Stop();
	
		
		TM1650_OFF();
}


//关闭数码管
void TM1650_OFF(void)
{
		//向数码管的四位均写入0X00，等效于全部熄灭
    TM1650_WriteData(BIT1_ADDR, 0X00);
		TM1650_WriteData(BIT2_ADDR, 0X00);
		TM1650_WriteData(BIT3_ADDR, 0X00);
		TM1650_WriteData(BIT4_ADDR, 0X00);
}



//在数码管显示一个两位小数部分的浮点数
void TM1650_SHOW_Float(float data)
{
    int tmp = data * 100;
		u8 bit1 = (tmp / 1000);
    u8 bit2 = (tmp / 100 % 10);		//多显示1个小数点
    u8 bit3 = (tmp / 10 % 10);
    u8 bit4 = (tmp % 10);
	
		TM1650_WriteData(BIT1_ADDR, TM1650Buff[bit1]);		//第一位数码管写高位
		TM1650_WriteData(BIT2_ADDR, TM1650Buff[bit2] | 0X80);		//多显示一个小数点
		TM1650_WriteData(BIT3_ADDR, TM1650Buff[bit3]);
		TM1650_WriteData(BIT4_ADDR, TM1650Buff[bit4]);
}


void TM1650_SHOW_Int(int data)
{
    u8 bit1 = (data / 1000);
    u8 bit2 = (data / 100 % 10);		
    u8 bit3 = (data / 10 % 10);
    u8 bit4 = (data % 10);

		TM1650_WriteData(BIT1_ADDR, TM1650Buff[bit1]);		//第一位数码管写高位
		TM1650_WriteData(BIT2_ADDR, TM1650Buff[bit2]);
		TM1650_WriteData(BIT3_ADDR, TM1650Buff[bit3]);
		TM1650_WriteData(BIT4_ADDR, TM1650Buff[bit4]);

}





