/*
 * DHT11 FreeRTOS驱动程序 - 源文件
 * 适用于STM32 + FreeRTOS环境
 */

#include "dht11.h"
#include <string.h>

// ================================ 私有变量 ================================
static SemaphoreHandle_t xDHT11_Mutex = NULL;
static DHT11_Data_t g_dht11_data = {0};
static TaskHandle_t xDHT11_TaskHandle = NULL;

// ================================ 私有函数声明 ================================
static void DHT11_GPIO_Init(void);
static void DHT11_Set_Output(void);
static void DHT11_Set_Input(void);
static void DHT11_Pin_High(void);
static void DHT11_Pin_Low(void);
static uint8_t DHT11_Pin_Read(void);
static void DHT11_Delay_us(uint32_t us);
static DHT11_Status_t DHT11_Start_Signal(void);
static DHT11_Status_t DHT11_Wait_Response(void);
static uint8_t DHT11_Read_Bit(void);
static uint8_t DHT11_Read_Byte(void);

static void DHT11_Task(void *pvParameters);

// ================================ 硬件抽象层实现 ================================
static void DHT11_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

            DHT11_GPIO_CLK();

    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);
}

static void DHT11_Set_Output(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

static void DHT11_Set_Input(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

static void DHT11_Pin_High(void) {
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);
}

static void DHT11_Pin_Low(void) {
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_RESET);
}

static uint8_t DHT11_Pin_Read(void) {
    return HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN);
}

// 微秒级精确延时
static void DHT11_Delay_us(uint32_t us) {
    taskENTER_CRITICAL();

//    uint32_t start = DWT->CYCCNT;
//    uint32_t cycles = us * (SystemCoreClock / 1000000U);
//    while ((DWT->CYCCNT - start) < cycles) {
//        __NOP();
//    }
/* Microsecond delay function */

        uint32_t delay = us * (SystemCoreClock / 1000000) / 5;
        while (delay--);

    taskEXIT_CRITICAL();
}

// ================================ DHT11通信协议实现 ================================
static DHT11_Status_t DHT11_Start_Signal(void) {
    DHT11_Set_Output();
    DHT11_Pin_High();
    DHT11_Delay_us(10);

    // 主机发送起始信号：拉低至少18ms
    DHT11_Pin_Low();
    vTaskDelay(pdMS_TO_TICKS(20));

    // 主机拉高20-40us，然后释放总线
    DHT11_Pin_High();
    DHT11_Delay_us(30);
    DHT11_Set_Input();

    return DHT11_OK;
}

static DHT11_Status_t DHT11_Wait_Response(void) {
    uint32_t timeout = 0;

    // 等待DHT11拉低（响应信号开始）
    while (DHT11_Pin_Read() && timeout < 100) {
        DHT11_Delay_us(1);
        timeout++;
    }
    if (timeout >= 100) return DHT11_ERROR_NO_RESPONSE;

    timeout = 0;
    // 等待DHT11拉高（响应信号结束）
    while (!DHT11_Pin_Read() && timeout < 100) {
        DHT11_Delay_us(1);
        timeout++;
    }
    if (timeout >= 100) return DHT11_ERROR_NO_RESPONSE;

    timeout = 0;
    // 等待DHT11再次拉低（数据传输准备）
    while (DHT11_Pin_Read() && timeout < 100) {
        DHT11_Delay_us(1);
        timeout++;
    }
    if (timeout >= 100) return DHT11_ERROR_NO_RESPONSE;

    return DHT11_OK;
}

static uint8_t DHT11_Read_Bit(void) {
    uint32_t timeout = 0;

    // 等待低电平结束（每个bit开始都是50us低电平）
    while (!DHT11_Pin_Read() && timeout < 100) {
        DHT11_Delay_us(1);
        timeout++;
    }

    // 延时30us后采样
    DHT11_Delay_us(30);

    if (DHT11_Pin_Read()) {
        // 如果还是高电平，说明是'1'（高电平持续70us）
        // 等待高电平结束
        timeout = 0;
        while (DHT11_Pin_Read() && timeout < 100) {
            DHT11_Delay_us(1);
            timeout++;
        }
        return 1;
    } else {
        // 如果是低电平，说明是'0'（高电平持续26-28us）
        return 0;
    }
}

static uint8_t DHT11_Read_Byte(void) {
    uint8_t byte_data = 0;

    for (int i = 7; i >= 0; i--) {
        byte_data |= (DHT11_Read_Bit() << i);
    }

    return byte_data;
}

// ================================ 主要接口函数实现 ================================
 DHT11_Status_t DHT11_Read_Raw_Data(DHT11_Data_t *data) {
    if (data == NULL) return DHT11_ERROR_TIMEOUT;

    uint8_t raw_data[5] = {0};

    //vPortEnterCritical();
    // 发送起始信号
    if (DHT11_Start_Signal() != DHT11_OK) {
        return DHT11_ERROR_TIMEOUT;
    }

    // 等待DHT11响应
    if (DHT11_Wait_Response() != DHT11_OK) {
        return DHT11_ERROR_NO_RESPONSE;
    }

    // 读取40位数据
    for (int i = 0; i < 5; i++) {
        raw_data[i] = DHT11_Read_Byte();
    }
    //vPortExitCritical();
    // 校验和检查
    uint8_t checksum = raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3];
    if (checksum != raw_data[4]) {
        return DHT11_ERROR_CHECKSUM;
    }

    // 数据解析
    data->humidity_int = raw_data[0];
    data->humidity_dec = raw_data[1];
    data->temperature_int = raw_data[2];
    data->temperature_dec = raw_data[3];
    data->checksum = raw_data[4];

    data->humidity = (float)data->humidity_int + (float)data->humidity_dec / 10.0f;
    data->temperature = (float)data->temperature_int + (float)data->temperature_dec / 10.0f;

    data->timestamp = xTaskGetTickCount();
    data->valid = 1;

    return DHT11_OK;
}

// ================================ FreeRTOS任务实现 ================================
static void DHT11_Task(void *pvParameters) {
    DHT11_Data_t temp_data;
    DHT11_Status_t status;
    uint8_t retry_count = 0;

    while (1) {
        // 尝试读取数据
        status = DHT11_Read_Raw_Data(&temp_data);

        if (status == DHT11_OK) {
            // 读取成功，更新全局数据
            if (xSemaphoreTake(xDHT11_Mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                memcpy(&g_dht11_data, &temp_data, sizeof(DHT11_Data_t));
                xSemaphoreGive(xDHT11_Mutex);
                retry_count = 0;
            }
        } else {
            retry_count++;

            if (retry_count >= DHT11_MAX_RETRY) {
                // 标记数据无效
                if (xSemaphoreTake(xDHT11_Mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                    g_dht11_data.valid = 0;
                    xSemaphoreGive(xDHT11_Mutex);
                }
                retry_count = 0;
            }
        }

        // 等待下次读取
        vTaskDelay(pdMS_TO_TICKS(DHT11_READ_INTERVAL_MS));
    }
}

// ================================ 公共接口函数实现 ================================
DHT11_Status_t DHT11_Init(void) {
    // 初始化DWT计数器（用于微秒延时）
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }

    // 创建互斥锁
//    xDHT11_Mutex = xSemaphoreCreateMutex();
//    if (xDHT11_Mutex == NULL) {
//        return DHT11_ERROR_TIMEOUT;
//    }

    // 初始化GPIO
    DHT11_GPIO_Init();

//    // 创建读取任务
//    BaseType_t result = xTaskCreate(
//            DHT11_Task,
//            "DHT11_Task",
//            512,  // 栈大小
//            NULL,
//            tskIDLE_PRIORITY + 2,  // 任务优先级
//            &xDHT11_TaskHandle
//    );
//
//    if (result != pdPASS) {
//        return DHT11_ERROR_TIMEOUT;
//    }

    return DHT11_OK;
}

DHT11_Status_t DHT11_Get_Data(float *temperature, float *humidity) {
    if (temperature == NULL || humidity == NULL) {
        return DHT11_ERROR_TIMEOUT;
    }

    if (xSemaphoreTake(xDHT11_Mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (g_dht11_data.valid) {
            *temperature = g_dht11_data.temperature;
            *humidity = g_dht11_data.humidity;
            xSemaphoreGive(xDHT11_Mutex);
            return DHT11_OK;
        } else {
            xSemaphoreGive(xDHT11_Mutex);
            return DHT11_ERROR_NO_RESPONSE;
        }
    }

    return DHT11_ERROR_BUSY;
}

DHT11_Status_t DHT11_Get_Full_Data(DHT11_Data_t *data) {
    if (data == NULL) return DHT11_ERROR_TIMEOUT;

    if (xSemaphoreTake(xDHT11_Mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        memcpy(data, &g_dht11_data, sizeof(DHT11_Data_t));
        xSemaphoreGive(xDHT11_Mutex);
        return DHT11_OK;
    }

    return DHT11_ERROR_BUSY;
}

uint8_t DHT11_Is_Data_Valid(void) {
    uint8_t valid = 0;

    if (xSemaphoreTake(xDHT11_Mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        valid = g_dht11_data.valid;
        xSemaphoreGive(xDHT11_Mutex);
    }

    return valid;
}