# 2025年CIMC中国智能制造挑战赛-工业嵌入式系统开发赛项

# Program：CIMC_GD32_Template

## 程序简介
- 工程名称：GD32H759 DEMO 程序模板
- 实验平台: CIMC IHD V0.4
- MDK版本：5.25
CIMC_GD32_Template
## 板载资源

 - GD32H759IMK6 MCU
 
## 功能简介

本项目为基于 GD32 平台的外部 RTC（实时时钟）驱动示例工程。  
通过 I2C 总线对外部 RTC 芯片进行初始化、时间配置及读取操作，实现基本的时间获取功能。

工程实现内容包括：
- I2C 通信初始化
- RTC 寄存器配置（时间初始化）
- RTC 时间读取（BCD 转字符串格式输出）
- 串口打印当前时间信息

## 实验操作

下载程序并复位开发板后，系统开始运行。  

打开串口调试工具（波特率与工程配置一致），观察输出信息。  

程序执行流程如下：
1. 初始化 Cache、NVIC、SysTick
2. 初始化串口
3. 初始化 I2C 及外部 RTC
4. 配置 RTC 初始时间（默认：2026-01-01 00:00:00）
5. 周期性读取 RTC 时间
6. 通过串口输出当前时间（每 1 秒更新一次）

## 实验现象

上电后，串口会周期性打印当前 RTC 时间


## 引脚分配

uart3_tx --- PB5
uart3_rx --- PB13

RTC_SDA --- PD13
RTC_SCL --- PD12


## 程序版本

- 程序版本：V0.1
- 发布日期：2025-03-22

## 联系我们

- Copyright   : CIMC中国智能制造挑战赛
- Author      ：Lingyu Meng
- Website     ：www.siemenscup-cimc.org.cn
- Phone       ：15801122380

## 声明

**严禁商业用途，仅供学习使用。 **


## 目录结构

├─01 Readme		工程项目说明
├─CMSIS			内核驱动文件：Cortex Microcontroller Software Interface Standard
├─Function		用户程序
├─HardWare		硬件驱动
├─HeaderFiles	头文件集合
├─Library		库文件
│  ├─GD32F4xx_standard_peripheral
│  ├─GD32F4xx_usb_library
│  └─Third_Party
├─project		工程文件（含生成的连接文件）
├─Protocol		协议程序
├─Startup		启动文件
├─System		
└─User
