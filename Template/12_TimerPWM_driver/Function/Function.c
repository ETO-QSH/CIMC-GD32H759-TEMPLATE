/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：Function.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/5     V0.01    original
************************************************************/


/************************* 头文件 *************************/

#include "Function.h"
#include "timer.h"
#include "LED.h"

/************************* 宏定义 *************************/


/************************ 变量定义 ************************/


/************************ 函数定义 ************************/

static void cache_enable(void);

/************************************************************
 * Function :       System_Init
 * Comment  :       用于初始化MCU
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-05 V0.1 original
************************************************************/

void System_Init(void)
{

	cache_enable();

	systick_config();     // 时钟配置

	LED_Init();

	my_timer_init();
}

/************************************************************
 * Function :       UsrFunction
 * Comment  :       用户程序功能: 呼吸灯效果
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-05 V0.1 original
************************************************************/

void UsrFunction(void)
{
	uint8_t led_state = 0;
	uint8_t pwm_value = 0;
	while (1)
	{
		if (!led_state)
		{
			pwm_value++;
		}
		else
		{
			pwm_value--;
		}
		if (pwm_value >= 100)
		{
			led_state = 1;
		}
		if (pwm_value <= 0)
		{
			led_state = 0;
		}
		my_timer_pwm_config(pwm_value);
		delay_1ms(10);
	}
}

/*!
	\brief      enable the CPU Chache
	\param[in]  none
	\param[out] none
	\retval     none
*/
static void cache_enable(void)
{
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}

/****************************End*****************************/

