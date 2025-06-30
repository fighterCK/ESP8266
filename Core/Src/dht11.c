/*
================================================================================
dht11.c - DHT11温湿度传感器驱动实现文件
================================================================================
*/
#include "dht11.h"

/* Private variables ---------------------------------------------------------*/
static DHT11_Data_t dht11_data;

/**
  * @brief  初始化DHT11传感器
  * @retval None
  */
void DHT11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin : DHT11_PIN */
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);

    /* 初始状态设为高电平 */
    DHT11_DATA_HIGH();
}

/**
  * @brief  设置DHT11 IO为输出模式
  * @retval None
  */
void DHT11_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  设置DHT11 IO为输入模式
  * @retval None
  */
void DHT11_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  微秒级延时函数
  * @param  us: 延时微秒数
  * @retval None
  */
void DHT11_Delay_us(uint32_t us)
{
    uint32_t delay = us * (SystemCoreClock / 1000000) / 4;
    while(delay--);
}

/**
  * @brief  读取DHT11数据
  * @param  data: 数据结构指针
  * @retval 读取状态
  */
uint8_t DHT11_ReadData(DHT11_Data_t* data)
{
    uint8_t buf[5] = {0};
    uint8_t i;

    /* 主机发送开始信号 */
    DHT11_SetOutput();
    DHT11_DATA_LOW();          // 拉低电平
    osDelayUntil(20);             // 延时至少18ms
    DHT11_DATA_HIGH();         // 拉高电平
    DHT11_Delay_us(30);        // 延时20-40us

    /* 切换为输入模式 */
    DHT11_SetInput();

    /* 等待DHT11响应 */
    if(DHT11_DATA_READ() == 0)
    {
        /* 等待DHT11拉高 */
        while(!DHT11_DATA_READ());
        /* 等待DHT11拉低 */
        while(DHT11_DATA_READ());

        /* 开始接收40位数据 */
        for(i = 0; i < 5; i++)
        {
            buf[i] = DHT11_ReadByte();
        }

        /* 恢复输出模式 */
        DHT11_SetOutput();
        DHT11_DATA_HIGH();

        /* 校验数据 */
        if(buf[4] == (buf[0] + buf[1] + buf[2] + buf[3]))
        {
            data->humidity = buf[0] + buf[1] * 0.1f;
            data->temperature = buf[2] + buf[3] * 0.1f;
            data->check_sum = buf[4];
            data->status = DHT11_OK;
            return DHT11_OK;
        }
    }

    /* 读取失败，恢复输出模式 */
    DHT11_SetOutput();
    DHT11_DATA_HIGH();
    data->status = DHT11_ERROR;
    return DHT11_ERROR;
}

/**
  * @brief  读取一个字节数据
  * @retval 读取到的字节数据
  */
uint8_t DHT11_ReadByte(void)
{
    uint8_t i, byte = 0;

    for(i = 0; i < 8; i++)
    {
        byte <<= 1;
        byte |= DHT11_ReadBit();
    }

    return byte;
}

/**
  * @brief  读取一位数据
  * @retval 读取到的位数据
  */
uint8_t DHT11_ReadBit(void)
{
    uint16_t timeout = 0;

    /* 等待低电平结束 */
    while(!DHT11_DATA_READ())
    {
        timeout++;
        if(timeout > 1000) return 0;
        DHT11_Delay_us(1);
    }

    /* 延时40us */
    DHT11_Delay_us(40);

    /* 判断此时电平，高电平表示1，低电平表示0 */
    if(DHT11_DATA_READ())
    {
        /* 等待高电平结束 */
        while(DHT11_DATA_READ())
        {
            timeout++;
            if(timeout > 1000) return 1;
            DHT11_Delay_us(1);
        }
        return 1;
    }
    else
    {
        return 0;
    }
}
