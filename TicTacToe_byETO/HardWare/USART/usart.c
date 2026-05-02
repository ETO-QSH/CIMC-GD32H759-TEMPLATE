/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：usart.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/9     V0.01    original
************************************************************/


/************************* 头文件 *************************/
#include "usart.h"

/************************* 宏定义 *************************/


/************************ 变量定义 ************************/
uint8_t recv_buf[128] = { 0 };
uint8_t recv_len = 0;

uint8_t recv_real_buf[128] = { 0 };
uint8_t recv_real_len = 0;

uint8_t recv_flag = 0;

/************************************************************
 * Function :       my_usart_init
 * Comment  :       用于初始化USART0
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-09 V0.1 original
************************************************************/
void my_usart_init(void)
{

	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
	nvic_irq_enable(UART3_IRQn , 3 , 0);

	//! 使能USART时钟
	rcu_periph_clock_enable(RCU_UART3);
	rcu_periph_clock_enable(RCU_GPIOA);

	// 对TX配置成为复用模式
	gpio_af_set(GPIOA , GPIO_AF_8 , GPIO_PIN_0);

	// 设置GPIO引脚模式
	gpio_mode_set(GPIOA , GPIO_MODE_AF , GPIO_PUPD_NONE , GPIO_PIN_0);
	// 设置GPIO引脚参数
	gpio_output_options_set(GPIOA , GPIO_OTYPE_PP , GPIO_OSPEED_100_220MHZ , GPIO_PIN_0);

	// 对RX配置成为复用模式
	gpio_af_set(GPIOA , GPIO_AF_8 , GPIO_PIN_1);

	// 设置GPIO引脚模式
	gpio_mode_set(GPIOA , GPIO_MODE_AF , GPIO_PUPD_NONE , GPIO_PIN_1);
	// 设置GPIO引脚参数
	gpio_output_options_set(GPIOA , GPIO_OTYPE_PP , GPIO_OSPEED_100_220MHZ , GPIO_PIN_1);

	// 配置USART
	usart_deinit(UART3);
	usart_word_length_set(UART3 , USART_WL_8BIT);
	usart_stop_bit_set(UART3 , USART_STB_1BIT);
	usart_parity_config(UART3 , USART_PM_NONE);
	usart_baudrate_set(UART3 , 115200U);
	usart_receive_config(UART3 , USART_RECEIVE_ENABLE);
	usart_transmit_config(UART3 , USART_TRANSMIT_ENABLE);

	usart_enable(UART3);

	usart_interrupt_enable(UART3 , USART_INT_RBNE);
	usart_interrupt_enable(UART3 , USART_INT_IDLE);
	usart_interrupt_flag_clear(UART3 , USART_INT_FLAG_RBNE);
	usart_interrupt_flag_clear(UART3 , USART_INT_FLAG_IDLE);
}
/************************************************************
 * Function :       USART0_IRQHandler
 * Comment  :       用于发送字符串到USART0的中断处理函数
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-09 V0.1 original
************************************************************/
void UART3_IRQHandler(void)
{
	if (usart_interrupt_flag_get(UART3 , USART_INT_FLAG_RBNE) != RESET)
	{
		recv_buf[recv_len++] = usart_data_receive(UART3);
	}
	//! 进行不定长数据的接收
	if (usart_interrupt_flag_get(UART3 , USART_INT_FLAG_IDLE) != RESET)
	{
		//! 清除IDLE中断标志位
		usart_interrupt_flag_clear(UART3 , USART_INT_FLAG_IDLE);
		if (recv_len != 0)
		{
			memcpy(recv_real_buf , recv_buf , recv_len);
			recv_real_len = recv_len;
			recv_len = 0;
			recv_flag = 1;
		}
	}

}

/************************************************************
 * Function :       fputc
 * Comment  :       用于发送字符到USART0
 * Parameter:       ch  要发送的字符
 * Return   :       发送的字符
 * Author   :       Jialei Zhao
 * Date     :       2026-02-09 V0.1 original
************************************************************/
int fputc(int ch , FILE* f)
{
	usart_data_transmit(UART3 , (uint8_t)ch);
	while (RESET == usart_flag_get(UART3 , USART_FLAG_TBE));

	return ch;
}
/****************************End*****************************/
