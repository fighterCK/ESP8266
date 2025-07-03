# ESP8266 MQTT 物联网节点

基于 STM32F103C8T6 和 ESP8266 的物联网节点，实现环境数据采集和 MQTT 数据传输。

## 功能特性

- 通过 DHT11 传感器采集温湿度数据
- 通过 ESP8266 WiFi 模块连接 MQTT 服务器
- 实时数据显示在 OLED 屏幕上
- 基于 FreeRTOS 的多任务处理
- 支持 MQTT 协议进行数据上传
- 看门狗任务监控系统状态

## 硬件要求

- STM32F103C8T6 开发板 (Blue Pill)
- ESP8266-01 WiFi 模块
- DHT11 温湿度传感器
- 0.96" OLED 显示屏 (I2C 接口)
- USB 转 TTL 串口模块

## 软件依赖

- STM32CubeMX
- STM32CubeIDE 或 CLion + STM32 插件
- FreeRTOS
- MQTT 协议栈

## 项目结构

```
ESP8266/
├── Core/
│   ├── Inc/                # 头文件
│   │   ├── app_task.h      # 应用任务定义
│   │   ├── config.h        # 配置文件
│   │   ├── dht11.h         # DHT11 驱动
│   │   ├── esp8266.h       # ESP8266 驱动
│   │   ├── mqtt.h          # MQTT 协议实现
│   │   ├── oled.h          # OLED 显示驱动
│   │   └── ...
│   └── Src/                # 源文件
│       ├── app_task.c      # 应用任务实现
│       ├── dht11.c         # DHT11 驱动实现
│       ├── esp8266.c       # ESP8266 驱动实现
│       ├── mqtt.c          # MQTT 协议实现
│       ├── oled.c          # OLED 显示驱动实现
│       └── ...
└── Drivers/                # STM32 HAL 库
```

## 快速开始

1. 克隆本仓库
   ```bash
   git clone <repository-url>
   ```

2. 使用 STM32CubeMX 打开项目并生成代码

3. 配置 `Core/Inc/config.h` 中的 WiFi 和 MQTT 参数：
   ```c
   #define WIFI_SSID           "your_wifi_ssid"
   #define WIFI_PASSWORD       "your_wifi_password"
   #define MQTT_SERVER         "mqtt.broker.address"
   #define MQTT_PORT           1883
   #define MQTT_USERNAME       "mqtt_username"
   #define MQTT_PASSWORD       "mqtt_password"
   #define MQTT_TOPIC         "sensor/data"
   ```

4. 编译并烧录到开发板

5. 连接硬件：
   - 将 ESP8266 连接到 USART2
   - 将 DHT11 连接到指定的 GPIO 引脚
   - 将 OLED 显示屏连接到 I2C 接口

## 任务说明

- **ESP8266 任务**：处理 WiFi 连接和 MQTT 通信
- **数据发布任务**：定期发布传感器数据到 MQTT 服务器
- **数据处理任务**：处理接收到的传感器数据
- **OLED 显示任务**：在 OLED 上显示系统状态和传感器数据
- **监控任务**：监控系统状态和任务运行情况

## 调试

- 使用串口1(USART1)进行调试输出，波特率115200
- 使用 `my_printf` 函数进行调试信息输出
- 可以通过 MQTT 客户端订阅 `sensor/data` 主题接收传感器数据

## 效果图
![f097840999ce7dac9e6b904e271b3f1d](https://github.com/user-attachments/assets/195bb331-a40b-4fb6-947f-16497125b17c)
![fc20c95041c2dadb2952b2bfce6dd839](https://github.com/user-attachments/assets/e427938f-3a5f-46da-a7ea-2fe0c1c17f3b)
![image](https://github.com/user-attachments/assets/f1902de3-414a-4ff6-a22c-49600a55c8dc)


  

## 许可证

MIT License

## 贡献

欢迎提交 Issue 和 Pull Request
