# 2025年CIMC中国智能制造挑战赛-工业嵌入式系统开发赛项

# Program：19_OLED_driver

## 程序简介
- 工程名称：GD32H759 DEMO 程序模板
- 实验平台: CIMC IHD V0.4
- MDK版本：5.25
19_OLED_driver
## 板载资源

 - GD32H759IMK6 MCU
 
 
## 功能简介

程序模板，可用于拷贝并快速建立工程。
基于 GD32H759IMK6 单片机实现：通过 I2C 实现OLED的信息显示

## 实验操作
下载程序并复位开发板后，打开串口调试工具。系统启动后，MCU 会通过 模拟I2C 向OLED 发送数据，实现信息显示。

注 : 本项目实现的I2C时通过软件模拟实现的

## 引脚分配

usart0_tx <---> PA9
usart0_rx <---> PA10

i2c_scl <---> PD13
i2c_sda <---> PD12

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
│  ├─EEPROM		EEPROM驱动
│  ├─I2C		I2C驱动
│  ├─LED		LED驱动
│  └─USART		串口驱动
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
