
/*=================================================================
 * 文件: soft_i2c.c
 * 描述: 模拟I2C实现文件
 * 作者: 开发者
 * 日期: 2025
 *=================================================================*/

#include "soft_i2c.h"

/**
 * @brief  微秒延时函数
 * @param  us: 延时微秒数
 * @retval None
 */
static void I2C_Delay_us(uint32_t us)
{
    uint32_t delay = us * (SystemCoreClock / 1000000U) / 2U;
    while(delay--)
    {
        __NOP();
    }
}

/**
 * @brief  设置SDA为输出模式
 * @param  None
 * @retval None
 */
static void SDA_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = I2C_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_SDA_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief  设置SDA为输入模式
 * @param  None
 * @retval None
 */
static void SDA_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = I2C_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(I2C_SDA_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief  模拟I2C初始化
 * @param  None
 * @retval None
 */
void Soft_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能GPIOB时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 配置SCL引脚为推挽输出 */
    GPIO_InitStruct.Pin = I2C_SCL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_SCL_GPIO_Port, &GPIO_InitStruct);

    /* 配置SDA引脚为推挽输出（初始状态） */
    GPIO_InitStruct.Pin = I2C_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_SDA_GPIO_Port, &GPIO_InitStruct);

    /* 初始状态：SCL和SDA都为高电平 */
    SCL_H();
    SDA_H();
}

/**
 * @brief  I2C起始信号
 * @param  None
 * @retval None
 */
void I2C_Start(void)
{
    SDA_OUT();      /* SDA设为输出 */
    SDA_H();        /* SDA高电平 */
    SCL_H();        /* SCL高电平 */
    I2C_Delay_us(4);
    SDA_L();        /* SDA变低电平 */
    I2C_Delay_us(4);
    SCL_L();        /* SCL变低电平，钳住I2C总线 */
}

/**
 * @brief  I2C停止信号
 * @param  None
 * @retval None
 */
void I2C_Stop(void)
{
    SDA_OUT();      /* SDA设为输出 */
    SCL_L();        /* SCL低电平 */
    SDA_L();        /* SDA低电平 */
    I2C_Delay_us(4);
    SCL_H();        /* SCL高电平 */
    I2C_Delay_us(4);
    SDA_H();        /* SDA高电平 */
    I2C_Delay_us(4);
}

/**
 * @brief  等待应答信号
 * @param  None
 * @retval 0: 有应答, 1: 无应答
 */
uint8_t I2C_Wait_Ack(void)
{
    uint8_t ucErrTime = 0;
    SDA_IN();       /* SDA设为输入模式 */
    SDA_H();
    I2C_Delay_us(1);
    SCL_H();
    I2C_Delay_us(1);

    while(READ_SDA())
    {
        ucErrTime++;
        if(ucErrTime > 250)
        {
            I2C_Stop();
            return 1;   /* 无应答 */
        }
    }
    SCL_L();
    return 0;   /* 有应答 */
}

/**
 * @brief  产生ACK应答
 * @param  None
 * @retval None
 */
void I2C_Ack(void)
{
    SCL_L();
    SDA_OUT();
    SDA_L();
    I2C_Delay_us(2);
    SCL_H();
    I2C_Delay_us(2);
    SCL_L();
}

/**
 * @brief  产生NACK非应答
 * @param  None
 * @retval None
 */
void I2C_NAck(void)
{
    SCL_L();
    SDA_OUT();
    SDA_H();
    I2C_Delay_us(2);
    SCL_H();
    I2C_Delay_us(2);
    SCL_L();
}

/**
 * @brief  I2C发送一个字节
 * @param  data: 要发送的字节
 * @retval None
 */
void I2C_Send_Byte(uint8_t data)
{
    uint8_t t;
    SDA_OUT();
    SCL_L();    /* 拉低时钟开始数据传输 */

    for(t = 0; t < 8; t++)
    {
        if((data & 0x80) >> 7)
            SDA_H();
        else
            SDA_L();
        data <<= 1;
        I2C_Delay_us(2);
        SCL_H();
        I2C_Delay_us(2);
        SCL_L();
        I2C_Delay_us(2);
    }
}

/**
 * @brief  I2C读取一个字节
 * @param  None
 * @retval 读取到的字节
 */
uint8_t I2C_Read_Byte(void)
{
    uint8_t i, receive = 0;
    SDA_IN();   /* SDA设为输入 */

    for(i = 0; i < 8; i++)
    {
        SCL_L();
        I2C_Delay_us(2);
        SCL_H();
        receive <<= 1;
        if(READ_SDA())
            receive++;
        I2C_Delay_us(1);
    }
    return receive;
}

/**
 * @brief  OLED写命令
 * @param  cmd: 命令字节
 * @retval None
 */
void OLED_Write_Cmd(uint8_t cmd)
{
    I2C_Start();
    I2C_Send_Byte(OLED_ADDRESS);    /* 发送设备地址 */
    I2C_Wait_Ack();
    I2C_Send_Byte(0x00);            /* 写命令 */
    I2C_Wait_Ack();
    I2C_Send_Byte(cmd);
    I2C_Wait_Ack();
    I2C_Stop();
}

/**
 * @brief  OLED写数据
 * @param  data: 数据字节
 * @retval None
 */
void OLED_Write_Data(uint8_t data)
{
    I2C_Start();
    I2C_Send_Byte(OLED_ADDRESS);    /* 发送设备地址 */
    I2C_Wait_Ack();
    I2C_Send_Byte(0x40);            /* 写数据 */
    I2C_Wait_Ack();
    I2C_Send_Byte(data);
    I2C_Wait_Ack();
    I2C_Stop();
}