/*
================================================================================
tasks.c - FreeRTOS任务实现文件
================================================================================
*/
#include "app_task.h"
#include "esp8266.h"
#include "mqtt_client.h"
#include "uart.h"

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
    const osThreadAttr_t defaultTask_attributes = {
            .name = "defaultTask",
            .stack_size = 32 * 4,
            .priority = (osPriority_t) osPriorityNormal,
    };
    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

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
    MQTTPublishTaskHandle = osThreadNew(StartMQTTPublishTask, NULL, &MQTTPublishTask_attributes);

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
    osTimerStart(keepAliveTimerHandle, 30000); // 30 seconds

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
        UART2_ProcessDMAData();

        // Process MQTT messages
        MQTT_Message_t mqtt_msg;
        if(osMessageQueueGet(mqttQueueHandle, &mqtt_msg, NULL, 100) == osOK)
        {
            // Handle received MQTT command
            if(strstr(mqtt_msg.payload, "LED_ON") != NULL)
            {
                HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_SET);
            }
            else if(strstr(mqtt_msg.payload, "LED_OFF") != NULL)
            {
                HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_RESET);
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