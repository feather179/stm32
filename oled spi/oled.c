#include "oled.h"
#include "font.h"

void OLED_SPI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}



//模拟SPI协议向模块写入一个字节
void SPI_WriteByte(u8 data, u8 cmd)
{
    u8 i = 0;
    OLED_DC = cmd;
    OLED_CLK = 0;
    for(i = 0; i < 8; i++)
    {
        OLED_CLK = 0;
        if(data & 0X80)
        {
            OLED_MOSI = 1;		//从高位到低位
        }
        else
        {
            OLED_MOSI = 0;
        }
        OLED_CLK = 1;
        data <<= 1;
    }
    OLED_CLK = 1;
    OLED_DC = 1;
}



//写命令
void WriteCmd(u8 cmd)
{
    SPI_WriteByte(cmd, OLED_CMD);
}

//写数据
void WriteData(u8 data)
{
    SPI_WriteByte(data, OLED_DATA);
}




//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127

u8 OLED_GRAM[128][8] = {0X00};

//软延时
void OLED_DLY_MS(u8 nms)
{
    u16 a = 0;
    while(nms)
    {
        a = 1335;
        while(a--);
        nms--;
    }
}



void OLED_Init(void)
{
    OLED_SPI_Init();
    OLED_RST = 1;
    OLED_DLY_MS(200);
    OLED_RST = 0;
    OLED_DLY_MS(200);
    OLED_RST = 1;

    //从上电到下面开始初始化要有足够的时间，即等待RC复位完毕
    WriteCmd(0xAE);         // Display Off (0x00)
    WriteCmd(0xD5);
    WriteCmd(0x80);         // Set Clock as 100 Frames/Sec
    WriteCmd(0xA8);
    WriteCmd(0x3F);         // 1/64 Duty (0x0F~0x3F)
    WriteCmd(0xD3);
    WriteCmd(0x00);         // Shift Mapping RAM Counter (0x00~0x3F)


    WriteCmd(0x40);  // Set Mapping RAM Display Start Line (0x00~0x3F)
    WriteCmd(0x8D);
    WriteCmd(0x14);  // Enable Embedded DC/DC Converter (0x00/0x04)
    WriteCmd(0x20);
    WriteCmd(0x02);         // Set Page Addressing Mode (0x00/0x01/0x02)
    WriteCmd(0xA1);  // Set SEG/Column Mapping
    WriteCmd(0xC0);  // Set COM/x Scan Direction
    WriteCmd(0xDA);
    WriteCmd(0x12);  // Set Sequential Configuration (0x00/0x10)


    WriteCmd(0x81);
    WriteCmd(0xCF);         // Set SEG Output Current
    WriteCmd(0xD9);
    WriteCmd(0xF1);         // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    WriteCmd(0xDB);
    WriteCmd(0x40);         // Set VCOM Deselect Level

    WriteCmd(0xA4);  // Disable Entire Display On (0x00/0x01)
    WriteCmd(0xA6);  // Disable Inverse Display On (0x00/0x01)
    WriteCmd(0xAF);  // Display On (0x01)


    OLED_Clear();  //初始清屏
}


/*************************************************************************/
/*函数功能: 将OLED从休眠中唤醒                                            */
/*************************************************************************/
void OLED_ON(void)
{
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X14);  //开启电荷泵
    WriteCmd(0XAF);  //OLED唤醒
}

/*************************************************************************/
/*函数功能: 将OLED休眠 -- 休眠模式下,OLED功耗不到10uA                      */
/*************************************************************************/
void OLED_OFF(void)
{
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X10);  //关闭电荷泵
    WriteCmd(0XAE);  //OLED休眠
}


//设置显示坐标
void OLED_Set_Pos(u8 x, u8 y)
{
    WriteCmd(0XB0 + y);		//列偏移
    WriteCmd(((x & 0XF0) >> 4) | 0X10);		//行高四位地址
    WriteCmd((x & 0X0F) | 0X01);					//行低四位地址
}
/*************************************************************************/
/*函数功能: 更新显存到OLED                                                 */
/*************************************************************************/
void OLED_Refresh_Gram(void)
{
    u8 i, n;
    for(i = 0; i < 8; i++)
    {
        WriteCmd(0xb0 + i); //设置页地址（0~7）
        WriteCmd(0x00);      //设置显示位置—列低地址
        WriteCmd(0x10);      //设置显示位置—列高地址
        for(n = 0; n < 128; n++)
        {
            WriteData(OLED_GRAM[n][i]);
        }
    }
}


/*************************************************************************/
/*函数功能: 清屏                                                          */
/*************************************************************************/
void OLED_Clear(void)
{
    u8 i, n;
    for(i = 0; i < 8; i++)
    {
        for(n = 0; n < 128; n++)
        {
            OLED_GRAM[n][i] = 0X00;
        }
    }
    OLED_Refresh_Gram();//更新显示
}

//



/*************************************************************************/
/*函数功能: 画点                                                          */
/*入口参数：                                                             */
/*                      x：横坐标       0~127                               */
/*                      y：纵坐标               0~63                     */
/*                      t:0,清空;1,填充                                                                                */
/*************************************************************************/
void OLED_DrawPoint(u8 x, u8 y, u8 t)
{
    u8 pos, bx, temp = 0;
    if(x > 127 || y > 63)return; //超出范围了.
    pos = 7 - y / 8;
    bx = y % 8;
    temp = 1 << (7 - bx);
    if(t)OLED_GRAM[x][pos] |= temp;
    else OLED_GRAM[x][pos] &= ~temp;
    //OLED_Refresh_Gram();//更新显示
}

/*************************************************************************/
/*函数功能: 填充                                                          */
/*入口参数：                                                                                                                          */
/*                      x1,y1,x2,y2 填充区域的对角坐标                              */
/*                      确保x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63                                         */
/*                      dot:0,清空;1,填充                                                                                */
/*************************************************************************/
void OLED_Fill(u8 x1, u8 y1, u8 x2, u8 y2, u8 dot)
{
    u8 x, y;
    for(x = x1; x <= x2; x++)
    {
        for(y = y1; y <= y2; y++)OLED_DrawPoint(x, y, dot);
    }
    OLED_Refresh_Gram();//更新显示
}

/*************************************************************************/
/*函数功能: 在指定位置显示一个字符,包括部分字符                             */
/*入口参数：                                                                       */
/*                      x:0~127                                                     */
/*                      y:0~63                                                      */
/*                      chr:字符																										*/
/*					            size:选择字体 16/12                                        */
/*************************************************************************/

void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size, u8 mode)
{
    u8 temp, t, t1;
    u8 y0 = y;
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); //得到字体一个字符对应点阵集所占的字节数
    chr = chr - ' '; //得到偏移后的值
    for(t = 0; t < csize; t++)
    {
        if(size == 12)temp = asc2_1206[chr][t];     //调用1206字体
        else if(size == 16)temp = asc2_1608[chr][t]; //调用1608字体
        else if(size == 24)temp = asc2_2412[chr][t]; //调用2412字体
        else return;                                //没有的字库
        for(t1 = 0; t1 < 8; t1++)
        {
            if(temp & 0x80)OLED_DrawPoint(x, y, mode);
            else OLED_DrawPoint(x, y, !mode);
            temp <<= 1;
            y++;
            if((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}




//m^n函数
u32 mypow(u8 m, u8 n)
{
    u32 result = 1;
    while(n--)result *= m;
    return result;
}

/*************************************************************************/
/*函数功能: 显示2个数字                                                   */
/*入口参数：                                                             */
/*                      x,y :起点坐标                                     */
/*                      len :数字的位数                                   */
/*                      size:字体大小                                        */
/*            mode:模式   0,填充模式;1,叠加模式                             */
/*            num:数值(0~4294967295)                                     */
/*************************************************************************/
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size)
{
    u8 t, temp;
    u8 enshow = 0;
    for(t = 0; t < len; t++)
    {
        temp = (num / mypow(10, len - t - 1)) % 10;
        if(enshow == 0 && t < (len - 1))
        {
            if(temp == 0)
            {
                OLED_ShowChar(x + (size / 2)*t, y, '0', size, 1);
                continue;
            } else enshow = 1;

        }
        OLED_ShowChar(x + (size / 2)*t, y, temp + '0', size, 1);
    }
}



/*************************************************************************/
/*函数功能: 显示浮点数                                                   */
/*入口参数：                                                             */
/*                      x,y :起点坐标                                    */
/*                      intlen :整数部分的位数                           */
/*											floatlen:小数部分的位数													 */
/*                      size:字体大小                                    */
/*            					num:数值 		                                     */
/*************************************************************************/
void OLED_ShowFloat(u8 x, u8 y, float num, u8 intlen, u8 floatlen, u8 size)
{
//	u32 tmp = num * mypow(10, floatlen);			//先将小数扩大一定的倍数变为整数
    u32 intnum = (int)num;																//整数部分
    u32 floatnum = (int)((num - intnum) * mypow(10, floatlen));	//小数部分


    OLED_ShowNum(x, y, intnum, intlen, size);							//整数部分
    OLED_DrawPoint(x + 6 * intlen + 2, y + 8, 1);											//显示小数点
    OLED_DrawPoint(x + 6 * intlen + 3, y + 8, 1);											//显示小数点
    OLED_DrawPoint(x + 6 * intlen + 2, y + 9, 1);											//显示小数点
    OLED_DrawPoint(x + 6 * intlen + 3, y + 9, 1);											//显示小数点
    OLED_ShowNum(x + 6 * (intlen + 1), y, floatnum, floatlen, size); //小数部分
}





/*************************************************************************/
/*函数功能: 显示字符串                                                                            */
/*入口参数：                                                                                                                          */
/*                      x,y:起点坐标                                                */
/*                      size:字体大小                                                          */
/*                      *p:字符串起始地址                                                               */
/*************************************************************************/
void OLED_ShowString(u8 x, u8 y, const u8 *p, u8 size)
{
    while((*p <= '~') && (*p >= ' ')) //判断是不是非法字符!
    {
        if(x > (128 - (size / 2))) {
            x = 0;
            y += size;
        }
        if(y > (64 - size)) {
            y = x = 0;
            OLED_Clear();
        }
        OLED_ShowChar(x, y, *p, size, 1);
        x += size / 2;
        p++;
    }

}



