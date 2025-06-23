/*
================================================================================
mqtt_client.h - MQTT客户端头文件
================================================================================
*/
#ifndef __MQTT_CLIENT_H
#define __MQTT_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "esp8266.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
    MQTT_OK = 0,
    MQTT_ERROR,
    MQTT_TIMEOUT,
    MQTT_NOT_CONNECTED
} MQTT_StatusTypeDef;

typedef struct {
    char topic[64];
    char payload[256];
} MQTT_Message_t;

/* Exported constants --------------------------------------------------------*/
#define MQTT_BROKER                 "broker.emqx.io"
#define MQTT_PORT                   "1883"
#define MQTT_CLIENT_ID              "STM32_Client"
#define MQTT_USERNAME               "your_username"
#define MQTT_PASSWORD               "your_password"
#define MQTT_TOPIC_PUB              "stm32/sensor/data"
#define MQTT_TOPIC_SUB              "stm32/control/cmd"

#define MQTT_KEEP_ALIVE             60
#define MQTT_BUFFER_SIZE            512

/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
extern volatile uint8_t mqtt_connected;
extern uint16_t mqtt_message_id;

/* Exported functions prototypes ---------------------------------------------*/
MQTT_StatusTypeDef MQTT_Connect(void);
MQTT_StatusTypeDef MQTT_Disconnect(void);
MQTT_StatusTypeDef MQTT_Publish(const char* topic, const char* payload);
MQTT_StatusTypeDef MQTT_Subscribe(const char* topic);
MQTT_StatusTypeDef MQTT_Unsubscribe(const char* topic);
void MQTT_ProcessMessage(const char* data, uint16_t length);
void MQTT_KeepAlive(void);

#ifdef __cplusplus
}
#endif

#endif /* __MQTT_CLIENT_H */

