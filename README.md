##  qcloud-iot-sdk-for-stm32withfreeRTOS-example 

### 介绍

qcloud-iot-sdk-for-stm32withfreeRTOS-example 是[腾讯云C-SDK](https://github.com/tencentyun/qcloud-iot-sdk-embedded-c.git)基于STM32MCU+FreeRTOS软硬件环境的移植示例。

### SDK架构图
![](https://i.imgur.com/tNoOACV.png)

### 目录结构

| 名称            | 说明 |
| ----            | ---- |
| doc            | 文档目录,包含硬件原理图 |
| Drivers        | STM32 F1xx HAL驱动|
| Inc            | 头文件 |
| MDK-ARM        | Keil工程目录 |
| Src    		 | 板级驱动文件 |
| Middlewares    | 第三方软件包，腾讯云SDK作为第三方软件包 |
|   ├── Third_Party\qcloud-iot-sdk-embedded-c  |
|   		├── qcloud-iot-sdk-embedded-c\src\platform|  |具体的软硬件平台腾讯云SDK需要移植适配的目录|
|   		└── Third_Party\qcloud-iot-sdk-embedded-c\samples|  |示例|
|  		├─── Third_Party\qcloud-iot-sdk-embedded-c\scenarized\exhibitor_shadow_sample.c | 参会证应用示例|
| README.md       | 软件包使用说明 |



### 许可证

MIT License 协议许可。


### SDK接口说明
#### 关于 SDK 的更多使用方式及接口了解, 请访问腾讯云C-SDK[文档平台](https://cloud.tencent.com/document/product/634)及[官方WiKi](https://github.com/tencentyun/qcloud-iot-sdk-embedded-c/wiki)
