//////////////////////////////////////////////////////////////////////////////////
//±¾³ÌÐòÖ»¹©Ñ§Ï°Ê¹ÓÃ£¬Î´¾­×÷ÕßÐí¿É£¬²»µÃÓÃÓÚÆäËüÈÎºÎÓÃÍ¾
//ÖÐ¾°Ô°µç×Ó
//µêÆÌµØÖ·£ºhttp://shop73023976.taobao.com/?spm=2013.1.0.0.M4PqC2
//
//  ÎÄ ¼þ Ãû   : main.c
//  °æ ±¾ ºÅ   : v2.0
//  ×÷    Õß   : Evk123
//  Éú³ÉÈÕÆÚ   : 2014-0101
//  ×î½üÐÞ¸Ä   :
//  ¹¦ÄÜÃèÊö   : 0.69´çOLED ½Ó¿ÚÑÝÊ¾Àý³Ì(STM32F103ZEÏµÁÐIIC)
//              ËµÃ÷:
//              ----------------------------------------------------------------
//              GND   µçÔ´µØ
//              VCC   ½Ó5V»ò3.3vµçÔ´
//              SCL   ½ÓPD6£¨SCL£©
//              SDA   ½ÓPD7£¨SDA£©
//              ----------------------------------------------------------------
//Copyright(C) ÖÐ¾°Ô°µç×Ó2014/3/16
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////£


#include "stdlib.h"
#include "oledfont.h"
#include "oled.h"
#include "soft_i2c.h"

static uint8_t g_oled_i2c_addr = 0x78;

static void OLED_I2C_Delay(void)
{
    volatile uint16_t i;
    for(i = 0; i < 60; i++)
    {
        __NOP();
    }
}

static uint8_t OLED_I2C_WriteFrame(uint8_t addr, uint8_t control, uint8_t payload)
{
    I2C_Start();
    I2C_Send_Byte(addr);
    if(I2C_Wait_Ack())
    {
        I2C_Stop();
        return 1;
    }
    I2C_Send_Byte(control);
    if(I2C_Wait_Ack())
    {
        I2C_Stop();
        return 1;
    }
    I2C_Send_Byte(payload);
    if(I2C_Wait_Ack())
    {
        I2C_Stop();
        return 1;
    }
    I2C_Stop();
    return 0;
}
//OLEDµÄÏÔ´æ
//´æ·Å¸ñÊ½ÈçÏÂ.
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127
/**********************************************
//IIC Start
**********************************************/
/**********************************************
//IIC Start
**********************************************/
void IIC_Start()
{

    OLED_SCLK_Set() ;
    OLED_SDIN_Set();
    OLED_I2C_Delay();
    OLED_SDIN_Clr();
    OLED_I2C_Delay();
    OLED_SCLK_Clr();
    OLED_I2C_Delay();
}

/**********************************************
//IIC Stop
**********************************************/
void IIC_Stop()
{
    OLED_SCLK_Clr();
    OLED_SDIN_Clr();
    OLED_I2C_Delay();
    OLED_SCLK_Set() ;
    OLED_I2C_Delay();
    OLED_SDIN_Set();
    OLED_I2C_Delay();

}

void IIC_Wait_Ack()
{

    //GPIOB->CRH &= 0XFFF0FFFF;	//ÉèÖÃPB12ÎªÉÏÀ­ÊäÈëÄ£Ê½
    //GPIOB->CRH |= 0x00080000;
//	OLED_SDA = 1;
//	delay_us(1);
    //OLED_SCL = 1;
    //delay_us(50000);
/*	while(1)
	{
		if(!OLED_SDA)				//ÅÐ¶ÏÊÇ·ñ½ÓÊÕµ½OLED Ó¦´ðÐÅºÅ
		{
			//GPIOB->CRH &= 0XFFF0FFFF;	//ÉèÖÃPB12ÎªÍ¨ÓÃÍÆÃâÊä³öÄ£Ê½
			//GPIOB->CRH |= 0x00030000;
			return;
		}
	}
*/
    OLED_SCLK_Set() ;
    OLED_I2C_Delay();
    OLED_SCLK_Clr();
    OLED_I2C_Delay();
}
/**********************************************
// IIC Write byte
**********************************************/

void Write_IIC_Byte(unsigned char IIC_Byte)
{
    unsigned char i;
    unsigned char m,da;
    da=IIC_Byte;
    OLED_SCLK_Clr();
    OLED_I2C_Delay();
    for(i=0;i<8;i++)
    {
        m=da;
        //	OLED_SCLK_Clr();
        m=m&0x80;
        if(m==0x80)
        {OLED_SDIN_Set();}
        else OLED_SDIN_Clr();
        da=da<<1;
        OLED_I2C_Delay();
        OLED_SCLK_Set();
        OLED_I2C_Delay();
        OLED_SCLK_Clr();
        OLED_I2C_Delay();
    }


}
/**********************************************
// IIC Write Command
**********************************************/
void Write_IIC_Command(unsigned char IIC_Command)
{
    if(OLED_I2C_WriteFrame(g_oled_i2c_addr, 0x00, IIC_Command) != 0)
    {
        // Try alternate SSD1306 address (0x3D << 1 = 0x7A)
        if(g_oled_i2c_addr == 0x78)
        {
            g_oled_i2c_addr = 0x7A;
        }
        else
        {
            g_oled_i2c_addr = 0x78;
        }
        (void)OLED_I2C_WriteFrame(g_oled_i2c_addr, 0x00, IIC_Command);
    }
}
/**********************************************
// IIC Write Data
**********************************************/
void Write_IIC_Data(unsigned char IIC_Data)
{
    if(OLED_I2C_WriteFrame(g_oled_i2c_addr, 0x40, IIC_Data) != 0)
    {
        // Keep behavior resilient if module uses alternate address.
        if(g_oled_i2c_addr == 0x78)
        {
            g_oled_i2c_addr = 0x7A;
        }
        else
        {
            g_oled_i2c_addr = 0x78;
        }
        (void)OLED_I2C_WriteFrame(g_oled_i2c_addr, 0x40, IIC_Data);
    }
}
void OLED_WR_Byte(unsigned dat,unsigned cmd)
{
    if(cmd)
    {

        Write_IIC_Data(dat);

    }
    else {
        Write_IIC_Command(dat);

    }


}


/********************************************
// fill_Picture
********************************************/
void fill_picture(unsigned char fill_Data)
{
    unsigned char m,n;
    for(m=0;m<8;m++)
    {
        OLED_WR_Byte(0xb0+m,0);		//page0-page1
        OLED_WR_Byte(0x00,0);		//low column start address
        OLED_WR_Byte(0x10,0);		//high column start address
        for(n=0;n<128;n++)
        {
            OLED_WR_Byte(fill_Data,1);
        }
    }
}


/***********************Delay****************************************/
void Delay_50ms(unsigned int Del_50ms)
{
    unsigned int m;
    for(;Del_50ms>0;Del_50ms--)
        for(m=6245;m>0;m--);
}

void Delay_1ms(unsigned int Del_1ms)
{
    unsigned char j;
    while(Del_1ms--)
    {
        for(j=0;j<123;j++);
    }
}

//×ø±êÉèÖÃ

void OLED_Set_Pos(unsigned char x, unsigned char y)
{ 	OLED_WR_Byte(0xb0+y,OLED_CMD);
    OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
    OLED_WR_Byte((x&0x0f),OLED_CMD);
}
//¿ªÆôOLEDÏÔÊ¾
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDCÃüÁî
    OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
    OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//¹Ø±ÕOLEDÏÔÊ¾
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDCÃüÁî
    OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
    OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}
//ÇåÆÁº¯Êý,ÇåÍêÆÁ,Õû¸öÆÁÄ»ÊÇºÚÉ«µÄ!ºÍÃ»µãÁÁÒ»Ñù!!!
void OLED_Clear(void)
{
    uint8_t i,n;
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);    //ÉèÖÃÒ³µØÖ·£¨0~7£©
        OLED_WR_Byte (0x00,OLED_CMD);      //ÉèÖÃÏÔÊ¾Î»ÖÃ¡ªÁÐµÍµØÖ·
        OLED_WR_Byte (0x10,OLED_CMD);      //ÉèÖÃÏÔÊ¾Î»ÖÃ¡ªÁÐ¸ßµØÖ·
        for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA);
    } //¸üÐÂÏÔÊ¾
}
void OLED_On(void)
{
    uint8_t i,n;
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);    //ÉèÖÃÒ³µØÖ·£¨0~7£©
        OLED_WR_Byte (0x00,OLED_CMD);      //ÉèÖÃÏÔÊ¾Î»ÖÃ¡ªÁÐµÍµØÖ·
        OLED_WR_Byte (0x10,OLED_CMD);      //ÉèÖÃÏÔÊ¾Î»ÖÃ¡ªÁÐ¸ßµØÖ·
        for(n=0;n<128;n++)OLED_WR_Byte(1,OLED_DATA);
    } //¸üÐÂÏÔÊ¾
}
//ÔÚÖ¸¶¨Î»ÖÃÏÔÊ¾Ò»¸ö×Ö·û,°üÀ¨²¿·Ö×Ö·û
//x:0~127
//y:0~63
//mode:0,·´°×ÏÔÊ¾;1,Õý³£ÏÔÊ¾
//size:Ñ¡Ôñ×ÖÌå 16/12
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size)
{
    unsigned char c=0,i=0;
    c=chr-' ';//µÃµ½Æ«ÒÆºóµÄÖµ
    if(x>Max_Column-1){x=0;y=y+2;}
    if(Char_Size ==16)
    {
        OLED_Set_Pos(x,y);
        for(i=0;i<8;i++)
            OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
        OLED_Set_Pos(x,y+1);
        for(i=0;i<8;i++)
            OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
    }
    else {
        OLED_Set_Pos(x,y);
        for(i=0;i<6;i++)
            OLED_WR_Byte(F6x8[c][i],OLED_DATA);

    }
}
//m^nº¯Êý
uint32_t oled_pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;
    while(n--)result*=m;
    return result;
}
//ÏÔÊ¾2¸öÊý×Ö
//x,y :Æðµã×ø±ê
//len :Êý×ÖµÄÎ»Êý
//size:×ÖÌå´óÐ¡
//mode:Ä£Ê½	0,Ìî³äÄ£Ê½;1,µþ¼ÓÄ£Ê½
//num:ÊýÖµ(0~4294967295);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size2)
{
    uint8_t t,temp;
    uint8_t enshow=0;
    for(t=0;t<len;t++)
    {
        temp=(num/oled_pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                OLED_ShowChar(x+(size2/2)*t,y,' ',size2);
                continue;
            }else enshow=1;

        }
        OLED_ShowChar(x+(size2/2)*t,y,temp+'0',size2);
    }
}
//ÏÔÊ¾Ò»¸ö×Ö·ûºÅ´®
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t Char_Size)
{
    unsigned char j=0;
    while (chr[j]!='\0')
    {		OLED_ShowChar(x,y,chr[j],Char_Size);
        x+=8;
        if(x>120){x=0;y+=2;}
        j++;
    }
}
//ÏÔÊ¾ºº×Ö
void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t no)
{
    uint8_t t,adder=0;
    OLED_Set_Pos(x,y);
    for(t=0;t<16;t++)
    {
        OLED_WR_Byte(Hzk[2*no][t],OLED_DATA);
        adder+=1;
    }
    OLED_Set_Pos(x,y+1);
    for(t=0;t<16;t++)
    {
        OLED_WR_Byte(Hzk[2*no+1][t],OLED_DATA);
        adder+=1;
    }
}
/***********¹¦ÄÜÃèÊö£ºÏÔÊ¾ÏÔÊ¾BMPÍ¼Æ¬128¡Á64ÆðÊ¼µã×ø±ê(x,y),xµÄ·¶Î§0¡«127£¬yÎªÒ³µÄ·¶Î§0¡«7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{
    unsigned int j=0;
    unsigned char x,y;

    if(y1%8==0) y=y1/8;
    else y=y1/8+1;
    for(y=y0;y<y1;y++)
    {
        OLED_Set_Pos(x0,y);
        for(x=x0;x<x1;x++)
        {
            OLED_WR_Byte(BMP[j++],OLED_DATA);
        }
    }
}

//³õÊ¼»¯SSD1306
void OLED_Init(void)
{
    Soft_I2C_Init();
    g_oled_i2c_addr = 0x78;


    osDelay(800);
    OLED_WR_Byte(0xAE,OLED_CMD);//--display off
    OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address
    OLED_WR_Byte(0xB0,OLED_CMD);//--set page address
    OLED_WR_Byte(0x81,OLED_CMD); // contract control
    OLED_WR_Byte(0xFF,OLED_CMD);//--128
    OLED_WR_Byte(0xA1,OLED_CMD);//set segment remap
    OLED_WR_Byte(0xA6,OLED_CMD);//--normal / reverse
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3F,OLED_CMD);//--1/32 duty
    OLED_WR_Byte(0xC8,OLED_CMD);//Com scan direction
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset
    OLED_WR_Byte(0x00,OLED_CMD);//

    OLED_WR_Byte(0xD5,OLED_CMD);//set osc division
    OLED_WR_Byte(0x80,OLED_CMD);//

    OLED_WR_Byte(0xD8,OLED_CMD);//set area color mode off
    OLED_WR_Byte(0x05,OLED_CMD);//

    OLED_WR_Byte(0xD9,OLED_CMD);//Set Pre-Charge Period
    OLED_WR_Byte(0xF1,OLED_CMD);//

    OLED_WR_Byte(0xDA,OLED_CMD);//set com pin configuartion
    OLED_WR_Byte(0x12,OLED_CMD);//

    OLED_WR_Byte(0xDB,OLED_CMD);//set Vcomh
    OLED_WR_Byte(0x30,OLED_CMD);//

    OLED_WR_Byte(0x8D,OLED_CMD);//set charge pump enable
    OLED_WR_Byte(0x14,OLED_CMD);//

    OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel

    // Power-on self-test pattern: full on for a short period, then clear.
    fill_picture(0xFF);
    osDelay(200);
    OLED_Clear();
}





























