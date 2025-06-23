#ifndef TASK_MONITOR_H
#define TASK_MONITOR_H

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

/* 函数声明 */
void vTaskMonitorInit(void);
void vPrintTaskStatus(void);
void vPrintSingleTaskStatus(TaskHandle_t xTask);
const char* pcGetTaskStateName(eTaskState eState);

/* 监控任务优先级定义 */
#define MONITOR_TASK_PRIORITY    (tskIDLE_PRIORITY + 1)
#define MONITOR_TASK_STACK_SIZE  (256)

#endif /* TASK_MONITOR_H */