/************************************************************
* 版权：2025CIMC Copyright。
* 文件：timer.c
* 作者: Jialei Zhao
* 平台: 2025CIMC IHD-V04
* 版本: Jialei Zhao     2026/2/10     V0.01    original
************************************************************/

/************************* 头文件 *************************/
#include "timer.h"

/************************* 宏定义 *************************/
#define TIMER0_CH3_PWM_PERIOD 15000

/************************ 全局变量定义 ************************/



/************************************************************
 * Function :       my_timer_init
 * Comment  :       用于初始化定时器
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void my_timer_init(void)
{
	//初始化输出PWM脉冲的输出引脚
	rcu_periph_clock_enable(RCU_GPIOC);
	/*configure PC7(TIMER0 CH0) as alternate function*/
	gpio_mode_set(GPIOC , GPIO_MODE_AF , GPIO_PUPD_NONE , GPIO_PIN_7);
	gpio_output_options_set(GPIOC , GPIO_OTYPE_PP , GPIO_OSPEED_60MHZ , GPIO_PIN_7);
	gpio_af_set(GPIOC , GPIO_AF_1 , GPIO_PIN_7);

	/* TIMER0 configuration: generate PWM signals with different duty cycles:
		  TIMER0CLK = 300MHz / (299+1) = 1MHz */

	timer_oc_parameter_struct timer_ocintpara;
	timer_parameter_struct timer_initpara;

	//初始化定时器0
	rcu_periph_clock_enable(RCU_TIMER0);
	timer_deinit(TIMER0);

	/* TIMER0 configuration */
	timer_initpara.prescaler = 299;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = TIMER0_CH3_PWM_PERIOD;
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER0 , &timer_initpara);

	/* CH0 configuration in PWM mode */
	timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
	timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
	timer_ocintpara.ocpolarity = TIMER_OC_POLARITY_LOW;
	timer_ocintpara.ocnpolarity = TIMER_OCN_POLARITY_LOW;
	timer_ocintpara.ocidlestate = TIMER_OC_IDLE_STATE_HIGH;
	timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_HIGH;
	timer_channel_output_config(TIMER0 , TIMER_CH_3 , &timer_ocintpara);

	timer_channel_output_pulse_value_config(TIMER0 , TIMER_CH_3 , TIMER0_CH3_PWM_PERIOD / 2);
	timer_channel_output_mode_config(TIMER0 , TIMER_CH_3 , TIMER_OC_MODE_PWM0);
	timer_channel_output_shadow_config(TIMER0 , TIMER_CH_3 , TIMER_OC_SHADOW_DISABLE);

	timer_primary_output_config(TIMER0 , ENABLE);
	/* auto-reload preload enable */
	timer_auto_reload_shadow_enable(TIMER0);
	timer_enable(TIMER0);

}

/************************************************************
 * Function :       my_timer_pwm_config
 * Comment  :       用于配置定时器PWM输出
 * Parameter:       pulse_value_Percentage: 占空比百分比	范围1~100	因为当前的PWM模式是
 * 																	PWM0模式所以范围只能在1~100之间
 * 																	如果要是PWM1模式则范围为0~99之间
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void my_timer_pwm_config(uint8_t pulse_value_Percentage)
{

	if (pulse_value_Percentage > 100)
	{
		pulse_value_Percentage = 100;
	}
	uint32_t pulse_value = pulse_value_Percentage * TIMER0_CH3_PWM_PERIOD / 100;
	//配置定时器PWM输出占空比
	timer_channel_output_pulse_value_config(TIMER0 , TIMER_CH_3 , pulse_value);
}

/****************************End*****************************/
