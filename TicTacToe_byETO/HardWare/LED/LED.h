#ifndef __LED_H
#define __LED_H

#include "HeaderFiles.h"

//定义第一颗LED状态函数
#define    LED1_OFF()      gpio_bit_set(GPIOE, GPIO_PIN_2) 	 // 低电平   关灯 
#define    LED1_ON()       gpio_bit_reset(GPIOE, GPIO_PIN_2)       // 高电平   开灯

//定义第二颗LED状态函数
#define    LED2_OFF()      gpio_bit_set(GPIOE, GPIO_PIN_5) 	 // 低电平   关灯 
#define    LED2_ON()       gpio_bit_reset(GPIOE, GPIO_PIN_5)       // 高电平   开灯

//定义第三颗LED状态函数
#define    LED3_OFF()      gpio_bit_set(GPIOH, GPIO_PIN_7)
#define    LED3_ON()       gpio_bit_reset(GPIOH, GPIO_PIN_7)

void LED_Init(void);     // LED 初始化

void LED_Toggle(uint32_t gpio_port, uint32_t gpio_pin);
				    
#endif
