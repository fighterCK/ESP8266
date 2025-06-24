/*
================================================================================
system_config.h - 系统配置文件
================================================================================
*/
#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H

/* WiFi Configuration */
#define WIFI_SSID                   "ChinaNet-Rqtw"
#define WIFI_PASSWORD               "hp4qbyq7"

/* MQTT Configuration */
#define MQTT_BROKER                 "192.168.1.49"
#define MQTT_PORT                   "1883"
#define MQTT_CLIENT_ID              "STM32_Client"
#define MQTT_USERNAME               "ck"
#define MQTT_PASSWORD               "123456"
#define MQTT_TOPIC_PUB              "stm32/sensor/data"
#define MQTT_TOPIC_SUB              "stm32/control/cmd"

#define MQTT_KEEP_ALIVE             60
#define MQTT_BUFFER_SIZE            512
/* System Settings */
#define SYSTEM_CLOCK_FREQ           72000000  // 72MHz
#define UART_BAUD_RATE             115200
#define MQTT_KEEP_ALIVE_INTERVAL   60        // seconds
#define SENSOR_READ_INTERVAL       30000     // milliseconds

#endif /* __SYSTEM_CONFIG_H */