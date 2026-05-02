/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：Function.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/5     V0.01    original
************************************************************/


/************************* 头文件 *************************/

#include "Function.h"
#include "extRTC.h"
#include "usart.h"


/************************* 宏定义 *************************/


/************************ 变量定义 ************************/
uint8_t extRTC_data[32] = { 0 };

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

	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

	systick_config();     // 时钟配置

	my_usart_init();

	my_extRTC_init();
}

/************************************************************
 * Function :       UsrFunction
 * Comment  :       用户程序功能:
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-05 V0.1 original
************************************************************/

void UsrFunction(void)
{

	while (1)
	{


		my_extRTC_ReadTime(0x02, extRTC_data, 32);
		printf("extRTC_data: %s\r\n" , extRTC_data);

		// test_send();

		delay_1ms(1000);
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

