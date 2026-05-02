/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：timer.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/5     V0.01    original
************************************************************/

/************************* 头文件 *************************/
#include "timer.h"
/************************ 全局变量定义 ************************/

uint8_t led_state_flag = 0;

/************************************************************
 * Function :       my_timer_init
 * Comment  :       用于初始化定时器6
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void my_timer_init(void)
{
	//!Timer6
	timer_parameter_struct timer_initpara;

	rcu_periph_clock_enable(RCU_TIMER6);
	// !150 * 4 = 600M
	// rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);

	timer_deinit(TIMER6);

	//!500ms触发中断
	timer_initpara.prescaler = 6000 - 1;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = 100000 - 1;
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER6 , &timer_initpara);

	timer_enable(TIMER6);
}

uint32_t my_timer_get_counter(void) {
	return timer_counter_read(TIMER6);
}

/****************************End*****************************/

