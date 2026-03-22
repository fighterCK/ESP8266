/*=================================================================
 * 文件: soft_i2c.h
 * 描述: 模拟I2C头文件
 * 作者: 开发者
 * 日期: 2025
 *=================================================================*/

#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

#include "main.h"
#include "stm32f1xx_hal.h"

/* GPIO定义 */
#define I2C_SCL_GPIO_Port   GPIOB
#define I2C_SCL_Pin         GPIO_PIN_1
#define I2C_SDA_GPIO_Port   GPIOB
#define I2C_SDA_Pin         GPIO_PIN_0

/* OLED I2C地址 */
#define OLED_ADDRESS        0x78  // 0x3C << 1

/* GPIO操作宏 */
#define SCL_H()     HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET)
#define SCL_L()     HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_RESET)
#define SDA_H()     HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_SET)
#define SDA_L()     HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_RESET)
#define READ_SDA()  HAL_GPIO_ReadPin(I2C_SDA_GPIO_Port, I2C_SDA_Pin)

/* 函数声明 */
void Soft_I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
uint8_t I2C_Wait_Ack(void);
void I2C_Ack(void);
void I2C_NAck(void);
void I2C_Send_Byte(uint8_t data);
uint8_t I2C_Read_Byte(void);
void OLED_Write_Cmd(uint8_t cmd);
void OLED_Write_Data(uint8_t data);

#endif /* __SOFT_I2C_H */
