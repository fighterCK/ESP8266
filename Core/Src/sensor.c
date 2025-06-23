#include "sensor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "stdlib.h"
/* 模拟传感器初始化 */
bool Sensor_Init(void) {
    printf("传感器初始化...\r\n");

    /* 这里是传感器初始化代码 */
    /* 实际项目中应替换为真实传感器的初始化代码 */

    /* 模拟初始化成功 */
    return true;
}

/* 读取传感器数据 */
bool Sensor_ReadData(float *temperature, float *humidity, uint16_t *pressure) {
    if (temperature == NULL || humidity == NULL || pressure == NULL) {
        return false;
    }

    /* 这里是读取传感器数据的代码 */
    /* 实际项目中应替换为真实传感器的读取代码 */

    /* 模拟传感器数据 */
    *temperature = 25.5 + (float)(rand() % 100) / 10.0;  // 25.5~35.4°C
    *humidity = 40.0 + (float)(rand() % 300) / 10.0;   // 40.0~69.9%
    *pressure = 1013 + (rand() % 20);                // 1013~1033 hPa

    printf("读取传感器数据 - 温度: %.1f°C, 湿度: %.1f%%, 气压: %u hPa\r\n",
           *temperature, *humidity, *pressure);

    return true;
}