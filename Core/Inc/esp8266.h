/*
================================================================================
esp8266.h - ESP8266驱动头文件
================================================================================
*/
#ifndef __ESP8266_H
#define __ESP8266_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal_uart.h"
#include <string.h>
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
typedef enum {
    ESP8266_OK = 0,
    ESP8266_ERROR,
    ESP8266_TIMEOUT,
    ESP8266_BUSY
} ESP8266_StatusTypeDef;

typedef struct {
    char data[512];
    uint16_t length;
} ESP8266_Message_t;

/* Exported constants --------------------------------------------------------*/
#define ESP8266_BUFFER_SIZE         1024
#define ESP8266_TIMEOUT_DEFAULT     5000

/* WiFi配置 */
#define WIFI_SSID                   "ChinaNet-Rqtw"
#define WIFI_PASSWORD               "hp4qbyq7"

/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern volatile uint8_t esp8266_ready;
extern volatile uint8_t wifi_connected;

/* Exported functions prototypes ---------------------------------------------*/
ESP8266_StatusTypeDef ESP8266_Init(void);
ESP8266_StatusTypeDef ESP8266_SendCommand(const char* cmd, const char* expected_response, uint32_t timeout);
ESP8266_StatusTypeDef ESP8266_ConnectWiFi(const char* ssid, const char* password);
ESP8266_StatusTypeDef ESP8266_ConnectTCP(const char* host, const char* port);
ESP8266_StatusTypeDef ESP8266_SendData(const uint8_t* data, uint16_t length);
ESP8266_StatusTypeDef ESP8266_CheckConnection(void);
void ESP8266_ProcessResponse(const char* response);

#ifdef __cplusplus
}
#endif

#endif /* __ESP8266_H */