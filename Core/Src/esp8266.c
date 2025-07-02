/*
================================================================================
esp8266.c - ESP8266驱动实现文件
================================================================================
*/
#include "esp8266.h"
#include "uart.h"
#include "app_task.h"

/* Private variables ---------------------------------------------------------*/
volatile uint8_t esp8266_ready = 0;
volatile uint8_t wifi_connected = 0;

/* Private function prototypes -----------------------------------------------*/
static ESP8266_StatusTypeDef ESP8266_WaitResponse(const char* expected, uint32_t timeout);

/**
  * @brief  Initialize ESP8266 module
  * @retval ESP8266_StatusTypeDef
  */
ESP8266_StatusTypeDef ESP8266_Init(void)
{
    // Test AT command
    if(ESP8266_SendCommand("AT+RESTORE", "OK", 2000) != ESP8266_OK)
        return ESP8266_ERROR;

    osDelay(1000);

    // Reset module
    ESP8266_SendCommand("AT+RST", "OK", 5000);
    osDelay(3000);

    // Set WiFi mode to Station
    if(ESP8266_SendCommand("AT+CWMODE=1", "OK", 2000) != ESP8266_OK)
        return ESP8266_ERROR;

    osDelay(1000);

    // Set multiple connection mode
    if(ESP8266_SendCommand("AT+CIPMUX=1", "OK", 2000) != ESP8266_OK)
        return ESP8266_ERROR;

    osDelay(1000);

    esp8266_ready = 1;
    return ESP8266_OK;
}

/**
  * @brief  Send AT command to ESP8266
  * @param  cmd: Command string
  * @param  expected_response: Expected response
  * @param  timeout: Timeout in milliseconds
  * @retval ESP8266_StatusTypeDef
  */
ESP8266_StatusTypeDef ESP8266_SendCommand(const char* cmd, const char* expected_response, uint32_t timeout)
{
    // Send command
    UART2_SendString(cmd);
    UART2_SendString("\r\n");

    // Wait for response
    if(UART2_WaitForResponse(expected_response, timeout) == HAL_OK)
        return ESP8266_OK;

    return ESP8266_TIMEOUT;
}

/**
  * @brief  Connect to WiFi network
  * @param  ssid: WiFi SSID
  * @param  password: WiFi password
  * @retval ESP8266_StatusTypeDef
  */
ESP8266_StatusTypeDef ESP8266_ConnectWiFi(const char* ssid, const char* password)
{
    char cmd_buffer[128];

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);

    if(ESP8266_SendCommand(cmd_buffer, "OK", 15000) == ESP8266_OK)
    {
        wifi_connected = 1;
        return ESP8266_OK;
    }

    return ESP8266_ERROR;
}

/**
  * @brief  Connect to TCP server
  * @param  host: Server hostname or IP
  * @param  port: Server port
  * @retval ESP8266_StatusTypeDef
  */
ESP8266_StatusTypeDef ESP8266_ConnectTCP(const char* host, const char* port)
{
    char cmd_buffer[128];

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CIPSTART=0,\"TCP\",\"%s\",%s", host, port);

    return ESP8266_SendCommand(cmd_buffer, "OK", 10000);
}

/**
  * @brief  Send data through ESP8266
  * @param  data: Data to send
  * @param  length: Data length
  * @retval ESP8266_StatusTypeDef
  */
ESP8266_StatusTypeDef ESP8266_SendData(const uint8_t* data, uint16_t length)
{
    char cmd_buffer[32];

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CIPSEND=0,%d", length);

    if(ESP8266_SendCommand(cmd_buffer, ">", 5000) == ESP8266_OK)
    {
        if(osMutexAcquire(uart2MutexHandle, osWaitForever) == osOK)
        {
            HAL_UART_Transmit(&huart2, data, length, HAL_MAX_DELAY);
            osMutexRelease(uart2MutexHandle);
            osDelay(1000);
            return ESP8266_OK;
        }
    }

    return ESP8266_ERROR;
}

/**
  * @brief  Check ESP8266 connection status
  * @retval ESP8266_StatusTypeDef
  */
ESP8266_StatusTypeDef ESP8266_CheckConnection(void)
{
    return ESP8266_SendCommand("AT+CIPSTATUS", "OK", 2000);
}

/**
  * @brief  Process ESP8266 response
  * @param  response: Response string
  * @retval None
  */
void ESP8266_ProcessResponse(const char* response)
{
    // Handle different responses
    if(strstr(response, "WIFI DISCONNECT") != NULL)
    {
        wifi_connected = 0;
    }
    else if(strstr(response, "WIFI CONNECTED") != NULL)
    {
        wifi_connected = 1;
    }
}
