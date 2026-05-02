# 2025年CIMC中国智能制造挑战赛-工业嵌入式系统开发赛项

# Program：CIMC_GD32_KEY_driver

## 程序简介
- 工程名称：GD32H759IMK6 KEY驱动
- 实验平台: CIMC IHD V0.4
- MDK版本：5.25
CIMC_GD32_KEY_driver

## 板载资源

 - GD32H759IMK6 MCU
 - LED * 2
 - KEY * 1
 
## 功能简介

程序模板，可用于拷贝并快速建立工程。
基于 GD32H759IMK6 单片机实现：通过按键控制 LED 点亮功能（采用 轮询方式 检测按键状态）。

## 实验操作

下载程序并复位开发板后，按下 KEY1 按键， LED1 将被点亮。按下 KEY2 按键， LED2 将被点亮。

## 引脚分配

		KEY1   <--->     PC13
		KEY2   <--->     PG3

		LED1   <--->     PE2
		LED2   <--->     PE5


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
│  |─LED		LED驱动
│  └─KEY		KEY驱动
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
