/*
 * DHT11 FreeRTOS驱动程序 - 头文件
 * 适用于STM32 + FreeRTOS环境
 */

#ifndef __DHT11_H__
#define __DHT11_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "main.h"
#include <stdint.h>

// ================================ 配置定义 ================================
#define DHT11_GPIO_PORT     GPIOA
#define DHT11_GPIO_PIN      GPIO_PIN_0
#define DHT11_GPIO_CLK      __HAL_RCC_GPIOA_CLK_ENABLE

#define DHT11_READ_INTERVAL_MS    2000  // 读取间隔，DHT11最小间隔2秒
#define DHT11_TIMEOUT_MS          100   // 超时时间
#define DHT11_MAX_RETRY           3     // 最大重试次数

// ================================ 数据结构定义 ================================

/**
 * @brief DHT11状态枚举
 */
typedef enum {
    DHT11_OK = 0,                ///< 操作成功
    DHT11_ERROR_CHECKSUM,        ///< 校验和错误
    DHT11_ERROR_TIMEOUT,         ///< 超时错误
    DHT11_ERROR_NO_RESPONSE,     ///< 传感器无响应
    DHT11_ERROR_BUSY             ///< 系统忙碌
} DHT11_Status_t;

/**
 * @brief DHT11数据结构
 */
typedef struct {
    float humidity;              ///< 湿度 (%RH)
    float temperature;           ///< 温度 (°C)
    uint8_t humidity_int;        ///< 湿度整数部分
    uint8_t humidity_dec;        ///< 湿度小数部分
    uint8_t temperature_int;     ///< 温度整数部分
    uint8_t temperature_dec;     ///< 温度小数部分
    uint8_t checksum;            ///< 校验和
    TickType_t timestamp;        ///< 时间戳
    uint8_t valid;               ///< 数据有效标志
} DHT11_Data_t;

// ================================ 公共接口函数声明 ================================

/**
 * @brief 初始化DHT11驱动
 * @note 此函数会创建后台任务和互斥锁，必须在使用其他函数前调用
 * @return DHT11_Status_t 初始化状态
 * @retval DHT11_OK 初始化成功
 * @retval DHT11_ERROR_TIMEOUT 初始化失败
 */
DHT11_Status_t DHT11_Init(void);

/**
 * @brief 获取温湿度数据
 * @param temperature 温度指针，用于存储温度值(°C)
 * @param humidity 湿度指针，用于存储湿度值(%RH)
 * @return DHT11_Status_t 操作状态
 * @retval DHT11_OK 获取成功
 * @retval DHT11_ERROR_TIMEOUT 参数错误
 * @retval DHT11_ERROR_NO_RESPONSE 数据无效
 * @retval DHT11_ERROR_BUSY 系统忙碌
 */
DHT11_Status_t DHT11_Get_Data(float *temperature, float *humidity);

/**
 * @brief 获取完整的DHT11数据
 * @param data DHT11数据结构指针
 * @return DHT11_Status_t 操作状态
 * @retval DHT11_OK 获取成功
 * @retval DHT11_ERROR_TIMEOUT 参数错误
 * @retval DHT11_ERROR_BUSY 系统忙碌
 */
DHT11_Status_t DHT11_Get_Full_Data(DHT11_Data_t *data);

/**
 * @brief 检查当前数据是否有效
 * @return uint8_t 数据有效性
 * @retval 1 数据有效
 * @retval 0 数据无效
 */
uint8_t DHT11_Is_Data_Valid(void);
DHT11_Status_t DHT11_Read_Raw_Data(DHT11_Data_t *data);
#ifdef __cplusplus
}
#endif

#endif /* __DHT11_H__ */