/*
================================================================================
tasks.c - FreeRTOS任务实现文件
================================================================================
*/
#include "app_task.h"
#include "esp8266.h"
#include "mqtt.h"
#include "uart.h"
#include "mqtt.h"
#include "config.h"
#include "dht11.h"

/* Private variables ---------------------------------------------------------*/
/* Task handles */
osThreadId_t defaultTaskHandle;
osThreadId_t ESP8266TaskHandle;
osThreadId_t MQTTPublishTaskHandle;
osThreadId_t DataProcessTaskHandle;

/* Queue handles */
osMessageQueueId_t uart2QueueHandle;
osMessageQueueId_t mqttQueueHandle;

/* Mutex handles */
osMutexId_t uart2MutexHandle;

/* Timer handles */
osTimerId_t keepAliveTimerHandle;

/* Private function prototypes -----------------------------------------------*/
void KeepAliveTimer_Callback(void *argument);
/* Private variables ---------------------------------------------------------*/
static DHT11_Data_t sensor_data;
static char json_buffer[256];

/**
  * @brief  Initialize all tasks and RTOS objects
  * @retval None
  */
void Tasks_Init(void)
{
    /* Create queues */
    uart2QueueHandle = osMessageQueueNew(8, sizeof(UartMessage_t), NULL);
    mqttQueueHandle = osMessageQueueNew(8, sizeof(MQTT_Message_t), NULL);

    /* Create mutex */
    uart2MutexHandle = osMutexNew(NULL);

    /* Create timer */
    keepAliveTimerHandle = osTimerNew(KeepAliveTimer_Callback, osTimerPeriodic, NULL, NULL);

    /* Create threads */
//    const osThreadAttr_t defaultTask_attributes = {
//            .name = "defaultTask",
//            .stack_size = 32 * 4,
//            .priority = (osPriority_t) osPriorityNormal,
//    };
//    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

    const osThreadAttr_t ESP8266Task_attributes = {
            .name = "ESP8266Task",
            .stack_size = 256 * 4,
            .priority = (osPriority_t) osPriorityHigh,
    };
    ESP8266TaskHandle = osThreadNew(StartESP8266Task, NULL, &ESP8266Task_attributes);

    const osThreadAttr_t MQTTPublishTask_attributes = {
            .name = "MQTTPublishTask",
            .stack_size = 288 * 4,
            .priority = (osPriority_t) osPriorityNormal,
    };
    MQTTPublishTaskHandle = osThreadNew(StartSensorTask, NULL, &MQTTPublishTask_attributes);

    const osThreadAttr_t DataProcessTask_attributes = {
            .name = "DataProcessTask",
            .stack_size = 256 * 4,
            .priority = (osPriority_t) osPriorityNormal,
    };
    DataProcessTaskHandle = osThreadNew(StartDataProcessTask, NULL, &DataProcessTask_attributes);
}

/**
  * @brief  Default task function
  * @param  argument: Not used
  * @retval None
  */
void StartDefaultTask(void *argument)
{
    for(;;)
    {
        // Toggle LED to indicate system is running
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
        osDelay(1000);
    }
}

/**
  * @brief  ESP8266 management task
  * @param  argument: Not used
  * @retval None
  */
void StartESP8266Task(void *argument)
{
    // Initialize ESP8266
    ESP8266_Init();

    // Connect to WiFi
    while(ESP8266_ConnectWiFi(WIFI_SSID, WIFI_PASSWORD) != ESP8266_OK)
    {
        osDelay(5000); // Retry every 5 seconds
    }
    // Connect to MQTT broker
    while(MQTT_Connect() != MQTT_OK)
    {
        osDelay(5000); // Retry every 5 seconds
    }

    // Subscribe to control topic
    MQTT_Subscribe(MQTT_TOPIC_SUB);

    // Start keep-alive timer
    osTimerStart(keepAliveTimerHandle, 60000); // 60 seconds

    for(;;)
    {
        // Check connections periodically
        if(!wifi_connected)
        {
            ESP8266_ConnectWiFi(WIFI_SSID, WIFI_PASSWORD);
        }

        if(!mqtt_connected && wifi_connected)
        {
            MQTT_Connect();
            MQTT_Subscribe(MQTT_TOPIC_SUB);
        }

        osDelay(10000); // Check every 10 seconds
    }
}

/**
  * @brief  MQTT publish task
  * @param  argument: Not used
  * @retval None
  */
void StartMQTTPublishTask(void *argument)
{
    char payload[128];
    uint32_t counter = 0;

    for(;;)
    {
        if(mqtt_connected)
        {
            // Create sensor data payload (example)
            snprintf(payload, sizeof(payload),
                     "{\"timestamp\":%lu,\"temperature\":25.6,\"humidity\":60.3,\"counter\":%lu}",
                     osKernelGetTickCount(), counter++);

            // Publish data
            MQTT_Publish(MQTT_TOPIC_PUB, payload);
        }

        osDelay(30000); // Publish every 30 seconds
    }
}
/**
  * @brief  传感器数据采集任务
  * @param  argument: Not used
  * @retval None
  */
void StartSensorTask(void *argument)
{
    uint32_t last_read_time = 0;
    uint32_t read_interval = 5000; // 5秒采集一次

    // 初始化DHT11传感器
    DHT11_Init();

    for(;;)
    {
        uint32_t current_time = osKernelGetTickCount();


            // 读取DHT11数据
            if(DHT11_ReadData(&sensor_data) == DHT11_OK)
            {
                // 格式化JSON数据
                snprintf(json_buffer, sizeof(json_buffer),
                         "{"
                         "\"device_id\":\"STM32_DHT11\","
                         "\"timestamp\":%lu,"
                         "\"temperature\":%.1f,"
                         "\"humidity\":%.1f,"
                         "\"status\":\"ok\""
                         "}",
                         current_time,
                         sensor_data.temperature,
                         sensor_data.humidity
                );

                // 通过MQTT发布数据
                if(mqtt_connected)
                {
                    MQTT_Publish(MQTT_TOPIC_PUB, json_buffer);

                    // 可选：发送到串口调试
//                    printf("Temperature: %.1f°C, Humidity: %.1f%%\r\n",
//                           sensor_data.temperature, sensor_data.humidity);
                }

                // 指示灯闪烁表示数据采集成功
                HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
            }
            else
            {
                // 读取失败，发送错误信息
                snprintf(json_buffer, sizeof(json_buffer),
                         "{"
                         "\"device_id\":\"STM32_DHT11\","
                         "\"timestamp\":%lu,"
                         "\"status\":\"error\","
                         "\"message\":\"DHT11 read failed\""
                         "}",
                         current_time
                );

                if(mqtt_connected)
                {
                    MQTT_Publish("home/error/stm32", json_buffer);
                }

                //printf("DHT11 read error!\r\n");
            }

            last_read_time = current_time;


        osDelay(3000); // 短暂延时，避免占用过多CPU
    }
}
/**
  * @brief  Data processing task
  * @param  argument: Not used
  * @retval None
  */
void StartDataProcessTask(void *argument)
{
    for(;;)
    {
        // Process UART data
        //UART2_ProcessData();
        //UART2_ProcessDMAData();

        // Process MQTT messages
        MQTT_Message_t mqtt_msg;
        if(osMessageQueueGet(mqttQueueHandle, &mqtt_msg, NULL, 100) == osOK)
        {
            // Handle received MQTT command
            if(strstr(mqtt_msg.payload + 10, "LED_ON") != NULL)
            {
                HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_RESET);
            }
            else if(strstr(mqtt_msg.payload + 10, "LED_OFF") != NULL)
            {
                HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_SET);
            }
        }
    }
}

/**
  * @brief  Keep-alive timer callback
  * @param  argument: Not used
  * @retval None
  */
void KeepAliveTimer_Callback(void *argument)
{
    MQTT_KeepAlive();
}