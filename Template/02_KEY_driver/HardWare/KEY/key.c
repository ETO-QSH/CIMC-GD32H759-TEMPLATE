/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：led.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/5     V0.01    original
************************************************************/

/************************* 头文件 *************************/
#include "key.h"

/************************ 全局变量定义 ************************/


/************************************************************
 * Function :       my_key_init
 * Comment  :       用于初始化按键端口
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-05 V0.1 original
************************************************************/
void my_key_init(void)
{
	/* enable the key GPIO clock */
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOG);
	/* configure key pin as input */
	gpio_mode_set(GPIOC , GPIO_MODE_INPUT , GPIO_PUPD_PULLDOWN, GPIO_PIN_13);
	gpio_mode_set(GPIOG , GPIO_MODE_INPUT , GPIO_PUPD_PULLDOWN, GPIO_PIN_3);

}

/************************************************************
 * Function :       my_key_scan
 * Comment  :       用于扫描按键状态
 * Parameter:       null
 * Return   :       uint8_t: 按键状态
 * Author   :       Jialei Zhao
 * Date     :       2026-02-05 V0.1 original
************************************************************/
uint8_t my_key_scan(void)
{

	/* check if the key pin is low */
	if (gpio_input_bit_get(GPIOC , GPIO_PIN_13) == SET)
	{
		delay_1ms(20);
		if (gpio_input_bit_get(GPIOC , GPIO_PIN_13) == SET)
		{
			return 1;
		}
		delay_1ms(20);
	}
	else if (gpio_input_bit_get(GPIOG , GPIO_PIN_3) == SET)
	{
		delay_1ms(20);
		if (gpio_input_bit_get(GPIOG , GPIO_PIN_3) == SET)
		{
			return 2;
		}
		delay_1ms(20);
	}

	return 0;
}

/****************************End*****************************/
