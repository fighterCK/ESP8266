#include "task_monitor.h"

/* 静态任务句柄 */
static TaskHandle_t xMonitorTaskHandle = NULL;

/* 将任务状态枚举转换为字符串 */
const char* pcGetTaskStateName(eTaskState eState) {
    switch(eState) {
        case eRunning:   return "Running";
        case eReady:     return "Ready";
        case eBlocked:   return "Blocked";
        case eSuspended: return "Suspended";
        case eDeleted:   return "Deleted";
        default:         return "Invalid";
    }
}

/* 打印所有任务状态 */
void vPrintTaskStatus(void) {
    UBaseType_t uxArraySize, x;
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxCurrentNumberOfTasks;
    uint32_t ulTotalRunTime;

    printf("\r\n=== Task Status Report ===\r\n");
    printf("Task Name\t\tState\t\tPrio\tStack\tNumber\r\n");
    printf("----------------------------------------------------------\r\n");

    /* 获取当前任务数量 */
    uxCurrentNumberOfTasks = uxTaskGetNumberOfTasks();

    /* 分配内存存储任务状态 */
    pxTaskStatusArray = pvPortMalloc(uxCurrentNumberOfTasks * sizeof(TaskStatus_t));

    if(pxTaskStatusArray != NULL) {
        /* 获取所有任务状态 */
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray,
                                           uxCurrentNumberOfTasks,
                                           &ulTotalRunTime);

        /* 打印每个任务的信息 */
        for(x = 0; x < uxArraySize; x++) {
            printf("%-16s\t%-10s\t%lu\t%u\t%lu\r\n",
                   pxTaskStatusArray[x].pcTaskName,
                   pcGetTaskStateName(pxTaskStatusArray[x].eCurrentState),
                   pxTaskStatusArray[x].uxCurrentPriority,
                   pxTaskStatusArray[x].usStackHighWaterMark,
                   pxTaskStatusArray[x].xTaskNumber);
        }

        printf("Total Runtime: %lu\r\n", ulTotalRunTime);
        printf("Free Heap: %u bytes\r\n", xPortGetFreeHeapSize());
        printf("==========================\r\n\r\n");

        /* 释放内存 */
        vPortFree(pxTaskStatusArray);
    } else {
        printf("Failed to allocate memory for task status\n");
    }
}

/* 打印单个任务状态 */
void vPrintSingleTaskStatus(TaskHandle_t xTask) {
    if(xTask != NULL) {
        eTaskState eState = eTaskGetState(xTask);
        UBaseType_t uxPriority = uxTaskPriorityGet(xTask);
        UBaseType_t uxStackHighWaterMark = uxTaskGetStackHighWaterMark(xTask);

        printf("Task State: %s, Priority: %lu, Stack Free: %u\n",
               pcGetTaskStateName(eState), uxPriority, uxStackHighWaterMark);
    }
}

/* 监控任务函数 */
static void vMonitorTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for(;;) {
        /* 每10秒打印一次任务状态 */
        vPrintTaskStatus();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10000));
    }
}

/* 初始化监控功能 */
void vTaskMonitorInit(void) {
    BaseType_t xResult;

    /* 创建监控任务 */
    xResult = xTaskCreate(vMonitorTask,
                          "Monitor",
                          MONITOR_TASK_STACK_SIZE,
                          NULL,
                          MONITOR_TASK_PRIORITY,
                          &xMonitorTaskHandle);

    if(xResult == pdPASS) {
        printf("Task Monitor initialized successfully\n");
    } else {
        printf("Failed to create Monitor task\n");
    }
}