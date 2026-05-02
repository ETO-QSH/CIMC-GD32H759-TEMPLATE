/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：led.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/5     V0.01    original
************************************************************/

/************************* 头文件 *************************/

#include "LED.h"

/************************ 全局变量定义 ************************/


/************************************************************
 * Function :       LED_Init
 * Comment  :       用于初始化LED端口
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-05 V0.1 original
************************************************************/

void LED_Init(void)
{

	rcu_periph_clock_enable(RCU_GPIOA);    // 初始化GPIO_A总线时钟
	rcu_periph_clock_enable(RCU_GPIOF);    // 初始化GPIO_F总线时钟

	//初始化LED1端口
	gpio_mode_set(GPIOA , GPIO_MODE_OUTPUT , GPIO_PUPD_NONE , GPIO_PIN_6);   			// GPIO模式设置为输出
	gpio_output_options_set(GPIOA , GPIO_OTYPE_PP , GPIO_OSPEED_60MHZ , GPIO_PIN_6);   // 输出参数设置
	gpio_bit_reset(GPIOA , GPIO_PIN_6);  											// 引脚初始电平为低电平

	//初始化LED2~4端口（批量初始化端口）
	gpio_mode_set(GPIOF , GPIO_MODE_OUTPUT , GPIO_PUPD_NONE , GPIO_PIN_10);   			// GPIO模式设置为输出
	gpio_output_options_set(GPIOF , GPIO_OTYPE_PP , GPIO_OSPEED_60MHZ , GPIO_PIN_10);     // 输出参数设置
	gpio_bit_reset(GPIOF , GPIO_PIN_10);  											// 引脚初始电平为低电平

}

/************************************************************
 * Function :       LED_Toggle
 * Comment  :       用于切换LED端口电平
 * Parameter:       gpio_port: LED端口号
 * Parameter:       gpio_pin: LED引脚号
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-05 V0.1 original
************************************************************/
void LED_Toggle(uint32_t gpio_port , uint32_t gpio_pin)
{
	if (gpio_output_bit_get(gpio_port , gpio_pin) == SET)
	{
		gpio_bit_reset(gpio_port , gpio_pin);
	}
	else
	{
		gpio_bit_set(gpio_port , gpio_pin);
	}
}
/****************************End*****************************/

