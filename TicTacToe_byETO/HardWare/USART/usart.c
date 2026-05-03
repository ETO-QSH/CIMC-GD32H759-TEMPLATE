#include "usart.h"

uint8_t recv_buf[128] = { 0 };
uint8_t recv_len = 0;

uint8_t recv_real_buf[128] = { 0 };
uint8_t recv_real_len = 0;

uint8_t recv_flag = 0;

void my_usart_init(void)
{
	//! 使能USART时钟
	rcu_periph_clock_enable(UARTx_RCU);
	rcu_periph_clock_enable(GPIOx_RCU);

	// 对TX配置成为复用模式
	gpio_af_set(UARTx_GPIOx_PORT , UARTx_AF , UARTx_TX);

	// 设置GPIO引脚模式
	gpio_mode_set(UARTx_GPIOx_PORT , GPIO_MODE_AF , GPIO_PUPD_NONE , UARTx_TX);
	// 设置GPIO引脚参数
	gpio_output_options_set(UARTx_GPIOx_PORT , GPIO_OTYPE_PP , GPIO_OSPEED_100_220MHZ , UARTx_TX);

	// 对RX配置成为复用模式
	gpio_af_set(UARTx_GPIOx_PORT , UARTx_AF , UARTx_RX);

	// 设置GPIO引脚模式
	gpio_mode_set(UARTx_GPIOx_PORT , GPIO_MODE_AF , GPIO_PUPD_NONE , UARTx_RX);
	// 设置GPIO引脚参数
	gpio_output_options_set(UARTx_GPIOx_PORT , GPIO_OTYPE_PP , GPIO_OSPEED_100_220MHZ , UARTx_RX);

	// 配置USART
	usart_deinit(UARTx);
	usart_word_length_set(UARTx , USART_WL_8BIT);
	usart_stop_bit_set(UARTx , USART_STB_1BIT);
	usart_parity_config(UARTx , USART_PM_NONE);
	usart_baudrate_set(UARTx , 115200U);
	usart_receive_config(UARTx , USART_RECEIVE_ENABLE);
	usart_transmit_config(UARTx , USART_TRANSMIT_ENABLE);

	usart_enable(UARTx);
}

void usart_recv_buf(void)
{
	if (recv_flag)
	{
		printf("recv_real_buf ==%s , recv_real_len ==%d\r\n" , recv_real_buf , recv_real_len);

		recv_flag = 0;
	}
}

int fputc(int ch , FILE* f)
{
	usart_data_transmit(UARTx , (uint8_t)ch);
	while (RESET == usart_flag_get(UARTx , USART_FLAG_TBE));

	return ch;
}
