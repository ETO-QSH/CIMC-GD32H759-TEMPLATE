/************************************************************
 * 版权：2025CIMC Copyright。 
 * 文件：led.h
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/5     V0.01    original
************************************************************/
#ifndef __LED_H
#define __LED_H

/************************* 头文件 *************************/

#include "HeaderFiles.h"

/************************* 宏定义 *************************/

//定义第一颗LED状态函数
#define    LED1_OFF()      gpio_bit_reset(GPIOF, GPIO_PIN_10) 	 // 低电平   关灯 
#define    LED1_ON()       gpio_bit_set(GPIOF, GPIO_PIN_10)       // 高电平   开灯

//定义第二颗LED状态函数
#define    LED2_OFF()      gpio_bit_reset(GPIOA, GPIO_PIN_6) 	 // 低电平   关灯 
#define    LED2_ON()       gpio_bit_set(GPIOA, GPIO_PIN_6)       // 高电平   开灯

/************************ 变量定义 ************************/


/************************ 函数定义 ************************/

void LED_Init(void);     // LED 初始化

void LED_Toggle(uint32_t gpio_port, uint32_t gpio_pin);
				    
#endif


/****************************End*****************************/

