/*
================================================================================
dht11.h - DHT11温湿度传感器驱动头文件
================================================================================
*/
#ifndef __DHT11_H
#define __DHT11_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
typedef struct {
    float temperature;      // 温度值 (℃)
    float humidity;         // 湿度值 (%)
    uint8_t check_sum;      // 校验和
    uint8_t status;         // 读取状态：0-成功，1-失败
} DHT11_Data_t;

/* Exported constants --------------------------------------------------------*/

#define DHT11_PIN                   GPIO_PIN_0
#define DHT11_GPIO_PORT            GPIOA

#define DHT11_OK                   0
#define DHT11_ERROR                1
#define DHT11_TIMEOUT              2

/* Exported macro ------------------------------------------------------------*/
#define DHT11_DATA_HIGH()          HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_PIN, GPIO_PIN_SET)
#define DHT11_DATA_LOW()           HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_PIN, GPIO_PIN_RESET)
#define DHT11_DATA_READ()          HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_PIN)

/* Exported functions prototypes ---------------------------------------------*/
void DHT11_Init(void);
uint8_t DHT11_ReadData(DHT11_Data_t* data);
void DHT11_SetOutput(void);
void DHT11_SetInput(void);
uint8_t DHT11_ReadByte(void);
uint8_t DHT11_ReadBit(void);

#ifdef __cplusplus
}
#endif

#endif /* __DHT11_H */