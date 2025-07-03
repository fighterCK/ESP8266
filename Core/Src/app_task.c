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
#include "tim1_us.h"
#include "my_printf.h"
#include "oled.h"


/* Private variables ---------------------------------------------------------*/
/* Task handles */
osThreadId_t MonitorTaskHandle;
osThreadId_t ESP8266TaskHandle;
osThreadId_t MQTTPublishTaskHandle;
osThreadId_t DataProcessTaskHandle;
/* OLED任务句柄 */
osThreadId oledTaskHandle;
/* Queue handles */
osMessageQueueId_t uart2QueueHandle;
osMessageQueueId_t mqttQueueHandle;
osMessageQueueId_t dht11dQueueHandle;
/* Mutex handles */
osMutexId_t uart2MutexHandle;

/* Timer handles */
osTimerId_t keepAliveTimerHandle;

/* Private function prototypes -----------------------------------------------*/
void KeepAliveTimer_Callback(void *argument);
/* Private variables ---------------------------------------------------------*/

/**
  * @brief  Initialize all tasks and RTOS objects
  * @retval None
  */
void Tasks_Init(void)
{
    /* Create queues */
    uart2QueueHandle = osMessageQueueNew(4, sizeof(UartMessage_t), NULL);
    mqttQueueHandle = osMessageQueueNew(4, sizeof(MQTT_Message_t), NULL);
    dht11dQueueHandle = osMessageQueueNew(4, sizeof(DHT11_Data_t), NULL);
    /* Create mutex */
    uart2MutexHandle = osMutexNew(NULL);

    /* Create timer */
    keepAliveTimerHandle = osTimerNew(KeepAliveTimer_Callback, osTimerPeriodic, NULL, NULL);

    /* Create threads */
    const osThreadAttr_t defaultTask_attributes = {
            .name = "vMonitorTask",
            .stack_size = 320 * 4,
            .priority = (osPriority_t) osPriorityNormal,
    };
    MonitorTaskHandle = osThreadNew(vMonitorTask, NULL, &defaultTask_attributes);

    const osThreadAttr_t ESP8266Task_attributes = {
            .name = "ESP8266Task",
            .stack_size = 384 * 4,
            .priority = (osPriority_t) osPriorityHigh,
    };
    ESP8266TaskHandle = osThreadNew(StartESP8266Task, NULL, &ESP8266Task_attributes);

    const osThreadAttr_t MQTTPublishTask_attributes = {
            .name = "MQTTPublishTask",
            .stack_size = 384 * 4,
            .priority = (osPriority_t) osPriorityNormal,
    };
    MQTTPublishTaskHandle = osThreadNew(StartMQTTPublishTask, NULL, &MQTTPublishTask_attributes);

    const osThreadAttr_t DataProcessTask_attributes = {
            .name = "DataProcessTask",
            .stack_size = 256 * 4,
            .priority = (osPriority_t) osPriorityNormal,
    };
    DataProcessTaskHandle = osThreadNew(StartDataProcessTask, NULL, &DataProcessTask_attributes);

    const osThreadAttr_t vMonitorTask_attributes = {
            .name = "OLED_Task",
            .stack_size = 256 * 4,
            .priority = (osPriority_t) osPriorityNormal,
    };
    oledTaskHandle = osThreadNew(OLED_Task, NULL, &vMonitorTask_attributes);
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
        size_t freeHeap = xPortGetFreeHeapSize();
        size_t minFreeHeap = xPortGetMinimumEverFreeHeapSize();
        UBaseType_t uxHighWaterMark = 0;
        my_printf("freeHeap:%d byte\r\n", freeHeap);
        my_printf("minFreeHeap:%d byte\r\n", minFreeHeap);
        uxHighWaterMark = uxTaskGetStackHighWaterMark(ESP8266TaskHandle);
        my_printf("StartESP8266Task is %d\r\n", uxHighWaterMark);
        uxHighWaterMark = uxTaskGetStackHighWaterMark(MQTTPublishTaskHandle);
        my_printf("MQTTPublishTask is %d\r\n", uxHighWaterMark);
        uxHighWaterMark = uxTaskGetStackHighWaterMark(DataProcessTaskHandle);
        my_printf("DataProcessTask is %d\r\n", uxHighWaterMark);
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
        if(ESP8266_SendCommand("AT+CWJAP?", "+CWJAP:", 5000) == ESP8266_OK)
        {
            wifi_connected = 1;
        }
        else
        {
            wifi_connected = 0;
        }
        if(ESP8266_SendCommand("AT+CIPSTATUS", "STATUS:3", 5000) == ESP8266_OK)
        {
            mqtt_connected = 1;
        }
        else
        {
            mqtt_connected = 0;
        }

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
void StartMQTTPublishTask(void *argument) {
    char payload[128];
    uint32_t counter = 0;
    DHT11_Data_t data;
    for (;;) {
        if (mqtt_connected) {
            if (DHT11_Read_Raw_Data(&data) == DHT11_OK) {
                // Create sensor data payload (example)
                snprintf(payload, sizeof(payload),
                         "{\"timestamp\":%lu,\"temperature\":%d,\"humidity\":%d,\"counter\":%lu}",
                         osKernelGetTickCount(), data.temperature_int, data.humidity_int, counter++);

                // Publish data
                MQTT_Publish(MQTT_TOPIC_PUB, payload);
                osMessageQueuePut(dht11dQueueHandle, &data, 0, 0);
            }
        }

        osDelay(5000); // Publish every 30 seconds
    }
}



/**
  * @brief  Data processing task
  * @param  argument: Not used
  * @retval None
  */
void StartDataProcessTask(void *argument)
{
    UBaseType_t uxHighWaterMark = 0;
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


void OLED_Task(void  * argument)
{
    uint32_t counter = 0;
    char str[32];

    /* 初始化OLED */
    OLED_Init();
    /* 清屏 */
    OLED_Clear();
    DHT11_Data_t sensor_data;
    /* 显示标题 */
    snprintf(str, sizeof(str), "temperature:  °C");
    OLED_ShowString(0, 0, str, 16);
    snprintf(str, sizeof(str), "humidity:  %RH");
    OLED_ShowString(0, 2, str, 16);
    /* 显示计数器 */
    snprintf(str, sizeof(str), "Count: %lu", counter++);
    OLED_ShowString(0, 4, str, 16);
    /* 显示系统节拍 */
    snprintf(str, sizeof(str), "Tick: %lu", osKernelGetTickCount());
    OLED_ShowString(0, 6, str, 16);
    for(;;)
    {
        if(osMessageQueueGet(dht11dQueueHandle, &sensor_data, NULL, 100) == osOK)
        {
            /* 显示标题 */
            snprintf(str, sizeof(str), "temperature: %d °C", sensor_data.temperature_int);
            OLED_ShowString(0, 0, str, 16);
            snprintf(str, sizeof(str), "humidity: %d %RH", sensor_data.humidity_int);
            OLED_ShowString(0, 2, str, 16);
            /* 显示计数器 */
            snprintf(str, sizeof(str), "Count: %lu", counter++);
            OLED_ShowString(0, 4, str, 16);
            /* 显示系统节拍 */
            snprintf(str, sizeof(str), "Tick: %lu", osKernelGetTickCount());
            OLED_ShowString(0, 6, str, 16);
        }
    }
}

void show_task_list(void)
{
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    vTaskList(buffer);
    size_t freeHeap = xPortGetFreeHeapSize();
    size_t minFreeHeap = xPortGetMinimumEverFreeHeapSize();
    my_printf("Name\t\tState\tPrio\tStack\tNum\tfreeHeap\tminFreeHeap\r\n");
    my_printf("%s\t\t\t\t\t\t%d\t\t%d\r\n", buffer, freeHeap, minFreeHeap);
//    my_printf("freeHeap:%d byte\r\n", freeHeap);
//    my_printf("minFreeHeap:%d byte\r\n", minFreeHeap);
}

void vMonitorTask(void *pvParameters)
{
    while (1)
    {
        show_task_list();
        vTaskDelay(pdMS_TO_TICKS(1000));  // 每 5 秒打印一次
    }
}


void StackHogTask(void *arg)
{
    char waste[200];  // 占用栈空间
    (void)waste;

    StackHogTask(NULL);  // 递归调用自己制造溢出
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