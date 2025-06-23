/*
================================================================================
uart_handler.h - UART DMA IDLE处理头文件
================================================================================
*/
#ifndef __UART_HANDLER_H
#define __UART_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
typedef struct {
    char data[512];
    uint16_t length;
} UartMessage_t;

/* Exported constants --------------------------------------------------------*/
#define UART_BUFFER_SIZE            1024
#define UART_DMA_BUFFER_SIZE        512

/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
extern uint8_t uart2_dma_buffer[UART_DMA_BUFFER_SIZE];
extern uint8_t uart2_process_buffer[UART_DMA_BUFFER_SIZE];
extern volatile uint16_t uart2_last_dma_size;

/* Exported functions prototypes ---------------------------------------------*/
void UART2_Init(void);
void UART2_SendString(const char* str);
void UART2_ProcessDMAData(void);
HAL_StatusTypeDef UART2_WaitForResponse(const char* expected_response, uint32_t timeout);
void UART2_DMA_Start(void);
#ifdef __cplusplus
}
#endif

#endif /* __UART_HANDLER_H */
