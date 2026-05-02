/************************************************************
* 版权：2025CIMC Copyright。
* 文件：usart.c
* 作者: Jialei Zhao
* 平台: 2025CIMC IHD-V04
* 版本: Jialei Zhao     2026/2/10     V0.01    original
************************************************************/


/************************* 头文件 *************************/
#include "rtc.h"

/************************* 宏定义 *************************/
#define BKP_VALUE    0x32F0

/************************ 变量定义 ************************/
uint32_t RTCSRC_FLAG = 0;
__IO uint32_t prescaler_a = 0 , prescaler_s = 0;
rtc_parameter_struct  rtc_initpara;

/************************ 函数定义 ************************/

void rtc_pre_config(void);


/************************************************************
 * Function :       my_rtc_init
 * Comment  :       用于初始化RTC
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void my_rtc_init(void)
{
	/* enable PMU clock */
	rcu_periph_clock_enable(RCU_PMU);
	/* enable the access of the RTC registers */
	pmu_backup_write_enable();
	/* get RTC clock entry selection */
	rcu_periph_clock_enable(RCU_RTC);
	rtc_register_sync_wait();

	/* 关闭RTC功能防止init失败 */
	rtc_alarm_disable(RTC_ALARM0);
	rtc_alarm_disable(RTC_ALARM1);
	rtc_wakeup_disable();
	//!判断是否配置了时钟源
	RTCSRC_FLAG = GET_BITS(RCU_BDCTL , 8 , 9);
	rtc_pre_config();
}


/************************************************************
 * Function :       rtc_pre_config
 * Comment  :       用于初始化RTC时钟源
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void rtc_pre_config(void)
{
	rcu_osci_on(RCU_LXTAL);
	rcu_osci_stab_wait(RCU_LXTAL);
	rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
	prescaler_s = 0xFF;
	prescaler_a = 0x7F;

	rcu_periph_clock_enable(RCU_RTC);
	rtc_register_sync_wait();
}
/************************************************************
 * Function :       rtc_setup
 * Comment  :       用于初始化RTC时间
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void rtc_setup(void)
{
	rtc_initpara.factor_asyn = prescaler_a;
	rtc_initpara.factor_syn = prescaler_s;

	rtc_initpara.year = 0x16;
	rtc_initpara.day_of_week = RTC_SATURDAY;
	rtc_initpara.month = RTC_APR;
	rtc_initpara.date = 0x30;
	rtc_initpara.display_format = RTC_24HOUR;
	rtc_initpara.am_pm = RTC_AM;

	rtc_initpara.year = 0x26;
	rtc_initpara.month = 0x04;
	rtc_initpara.date = 0x16;
	rtc_initpara.hour = 0x00;
	rtc_initpara.minute = 0x00;
	rtc_initpara.second = 0x00;

	//! 初始化RTC
	if (rtc_init(&rtc_initpara) == ERROR)
	{
		printf("RTC init false\r\n");
		return;
	}
	// printf("RTC init success\r\n");

	//! 备份校验值
	RTC_BKP0 = BKP_VALUE;
}


/************************************************************
 * Function :       rtc_show_time
 * Comment  :       用于显示RTC时间
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void rtc_show_time(void)
{
	rtc_current_time_get(&rtc_initpara);

	printf("Current time: 20%0.2x-%0.2x-%0.2x" , \
		rtc_initpara.year , rtc_initpara.month , rtc_initpara.date);

	printf("  %0.2x:%0.2x:%0.2x \r\n" , \
		rtc_initpara.hour , rtc_initpara.minute , rtc_initpara.second);
}


/****************************End*****************************/
