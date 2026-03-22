/*
================================================================================
uart_handler.c - UART DMA IDLE处理实现文件
================================================================================
*/
#include "uart.h"
#include "task.h"
#include "esp8266.h"
#include "uart.h"
#include "app_task.h"
#include "my_printf.h"

/* Private variables ---------------------------------------------------------*/
uint8_t uart2_dma_buffer[UART_DMA_BUFFER_SIZE];
uint8_t uart2_process_buffer[UART_DMA_BUFFER_SIZE];
volatile uint16_t uart2_last_dma_size = 0;
volatile uint8_t uart2_rx_complete_flag = 0;
static volatile uint8_t uart2_tx_busy = 0;
static uint8_t uart2_tx_buffer[UART_DMA_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void UART2_ParseReceivedData(uint8_t* data, uint16_t length);

/**
  * @brief  Initialize UART2 DMA handler
  * @retval None
  */
void UART2_Init(void)
{
    // 清空缓冲区
    memset(uart2_dma_buffer, 0, UART_DMA_BUFFER_SIZE);
    memset(uart2_process_buffer, 0, UART_DMA_BUFFER_SIZE);

    // 启用UART2空闲中断
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

    // 启动DMA接收
    UART2_DMA_Start();
}

/**
  * @brief  启动UART2 DMA接收
  * @retval None
  */
void UART2_DMA_Start(void)
{
    // 启动DMA循环接收
    HAL_UART_Receive_DMA(&huart2, uart2_dma_buffer, UART_DMA_BUFFER_SIZE);
    uart2_last_dma_size = UART_DMA_BUFFER_SIZE;
}

/**
  * @brief  Send string through UART2
  * @param  str: String to send
  * @retval None
  */
void UART2_SendString(const char* str)
{
    if(osMutexAcquire(uart2MutexHandle, osWaitForever) == osOK)
    {
        /* Wait for previous DMA TX to finish */
        uint32_t timeout = 1000;
        while(uart2_tx_busy && timeout--)
        {
            osDelay(1);
        }

        uint16_t len = strlen(str);
        if(len > UART_DMA_BUFFER_SIZE)
            len = UART_DMA_BUFFER_SIZE;

        memcpy(uart2_tx_buffer, str, len);
        uart2_tx_busy = 1;
        HAL_UART_Transmit_DMA(&huart2, uart2_tx_buffer, len);

        osMutexRelease(uart2MutexHandle);
    }
}

/**
  * @brief  HAL DMA发送完成回调
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        uart2_tx_busy = 0;
    }
}

/**
  * @brief  UART空闲中断回调函数
  * @param  huart: UART handle
  * @retval None
  */
void HAL_UART_IdleCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        // 停止DMA传输
        HAL_UART_DMAStop(&huart2);

        // 计算接收到的数据长度
        uint16_t recv_len = UART_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);

        if(recv_len > 0)
        {
            // 复制数据到处理缓冲区
            memset(uart2_process_buffer, 0, UART_DMA_BUFFER_SIZE);
            memcpy(uart2_process_buffer, uart2_dma_buffer, recv_len);
            //HAL_UART_Transmit(&huart1, uart2_process_buffer, sizeof(uart2_process_buffer), 100);
            // 解析接收到的数据
            UART2_ParseReceivedData(uart2_process_buffer, recv_len);

            // 清空DMA缓冲区
            memset(uart2_dma_buffer, 0, UART_DMA_BUFFER_SIZE);
        }

        // 重新启动DMA接收
        UART2_DMA_Start();
    }
}

/**
  * @brief  UART中断处理函数
  * @param  huart: UART handle
  * @retval None
  */
void HAL_UART_IRQHandler_Custom(UART_HandleTypeDef *huart)
{
    // 检查空闲中断标志
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET)
    {
        // 清除空闲中断标志
        __HAL_UART_CLEAR_IDLEFLAG(huart);

        // 调用空闲中断回调
        HAL_UART_IdleCallback(huart);
    }
}

/**
  * @brief  解析接收到的数据
  * @param  data: 接收数据指针
  * @param  length: 数据长度
  * @retval None
  */
static void UART2_ParseReceivedData(uint8_t* data, uint16_t length)
{
    if(length == 0)
        return;

    // 截断保护 + 添加结束符
    if(length >= UART_DMA_BUFFER_SIZE)
        length = UART_DMA_BUFFER_SIZE - 1;
    data[length] = '\0';

    // 检查是否为+IPD数据（MQTT收到的报文）
    char *ipd = strstr((char*)data, "+IPD,");
    if(ipd != NULL)
    {
        char *colon = strchr(ipd, ':');
        if(colon != NULL)
        {
            MQTT_Message_t mqtt_msg;
            uint16_t payload_len = length - (uint16_t)(colon + 1 - (char*)data);
            if(payload_len >= sizeof(mqtt_msg.payload))
                payload_len = sizeof(mqtt_msg.payload) - 1;
            memcpy(mqtt_msg.payload, colon + 1, payload_len);
            mqtt_msg.payload[payload_len] = '\0';
            osMessageQueuePut(mqttQueueHandle, &mqtt_msg, 0, 0);
            uart2_rx_complete_flag = 1;
            return;
        }
    }

    // 普通AT响应放入uart2队列
    if(length < sizeof(((UartMessage_t*)0)->data))
    {
        UartMessage_t uart_msg;
        memcpy(uart_msg.data, data, length + 1);  // 含'\0'
        uart_msg.length = length;
        osMessageQueuePut(uart2QueueHandle, &uart_msg, 0, 0);
    }

    uart2_rx_complete_flag = 1;
}

/**
  * @brief  等待UART响应
  * @param  expected_response: 期望的响应字符串
  * @param  timeout: 超时时间(ms)
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef UART2_WaitForResponse(const char* expected_response, uint32_t timeout)
{
    uint32_t start_time = osKernelGetTickCount();
    UartMessage_t uart_msg;

    uart2_rx_complete_flag = 0;

    while((osKernelGetTickCount() - start_time) < timeout)
    {
        // 尝试从队列获取消息
        if(osMessageQueueGet(uart2QueueHandle, &uart_msg, NULL, 100) == osOK)
        {
            //HAL_UART_Transmit(&huart1, uart_msg.data, sizeof(uart_msg.data), 100);
            // 检查是否包含期望的响应
            if(strstr(uart_msg.data, expected_response) != NULL)
            {
                return HAL_OK;
            }
        }

        osDelay(10);
    }

    return HAL_TIMEOUT;
}

/**
  * @brief  处理DMA接收的数据
  * @retval None
  */
void UART2_ProcessDMAData(void)
{
    UartMessage_t uart_msg;

    // 从队列中获取UART消息
    while(osMessageQueueGet(uart2QueueHandle, &uart_msg, NULL, 0) == osOK)
    {
        // 处理接收到的数据
        //printf("Received: %s (Length: %d)\n", uart_msg.data, uart_msg.length);

        // 可以在这里添加更多的数据处理逻辑
        // 例如：MQTT消息处理、AT命令响应等
    }
}
