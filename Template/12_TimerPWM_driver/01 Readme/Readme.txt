# 2025年CIMC中国智能制造挑战赛-工业嵌入式系统开发赛项

# Program：CIMC_GD32_TimerPWM_driver

## 程序简介
- 工程名称：GD32H759 DEMO 定时器PWM驱动
- 实验平台: CIMC IHD V0.4
- MDK版本：5.38
CIMC_GD32_TimerPWM_driver
## 板载资源

 - GD32H759IMK6 MCU
 
 
## 功能简介

程序模板，可用于拷贝并快速建立工程。
基于 GD32H759IMK6 单片机实现：利用 定时器 PWM 输出功能 对外输出 PWM 方波信号，通过周期性改变 PWM 的占空比，使 LED 的亮度逐渐增强和减弱，从而实现 呼吸灯效果。


## 实验操作
下载程序并复位开发板后，将 PWM 方波输出引脚 连接到 LED（任意一个）的控制引脚。程序运行后，定时器会持续输出 PWM 信号，并通过改变占空比实现 LED 亮度的渐变效果，从而可以观察到明显的 呼吸灯现象。

## 引脚分配

PC7 ---> PWM方波输出引脚

## 程序版本

- 程序版本：V0.1
- 发布日期：2025-12-29

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
│  ├─LED        LED驱动
│  └─TIMER       定时器驱动
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
