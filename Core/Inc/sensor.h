#ifndef __SENSOR_H
#define __SENSOR_H

#include "main.h"
#include "stdbool.h"

/* 初始化传感器 */
bool Sensor_Init(void);

/* 读取传感器数据 */
bool Sensor_ReadData(float *temperature, float *humidity, uint16_t *pressure);

#endif /* __SENSOR_H */