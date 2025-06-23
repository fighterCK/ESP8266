/*
================================================================================
main.h - 主头文件
================================================================================
*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
#include "esp8266.h"
#include "mqtt_client.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define LED_PIN                     GPIO_PIN_13
#define LED_GPIO_PORT              GPIOC

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */