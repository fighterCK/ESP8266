/*
================================================================================
tasks.h - FreeRTOS任务头文件
================================================================================
*/
#ifndef __TASKS_H
#define __TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os2.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
/* Task handles */
extern osThreadId_t defaultTaskHandle;
extern osThreadId_t ESP8266TaskHandle;
extern osThreadId_t MQTTPublishTaskHandle;
extern osThreadId_t DataProcessTaskHandle;

/* Queue handles */
extern osMessageQueueId_t uart2QueueHandle;
extern osMessageQueueId_t mqttQueueHandle;

/* Mutex handles */
extern osMutexId_t uart2MutexHandle;

/* Exported functions prototypes ---------------------------------------------*/
void StartDefaultTask(void *argument);
void StartESP8266Task(void *argument);
void StartMQTTPublishTask(void *argument);
void StartDataProcessTask(void *argument);
void StartSensorTask(void *argument);
void Tasks_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __TASKS_H */
