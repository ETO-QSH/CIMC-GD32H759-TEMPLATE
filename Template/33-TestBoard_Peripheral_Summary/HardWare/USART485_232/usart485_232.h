/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：usart.h
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2025/12/30     V0.01    original
************************************************************/

#ifndef __USART485_232_H__
#define __USART485_232_H__

/************************* 头文件 *************************/
#include "HeaderFiles.h"

/************************* 宏定义 *************************/
#define USART USART1
#define USARTX_RCU RCU_USART1

#define USART_TX_PORT GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_PIN_RCU_TX RCU_GPIOA

#define USART_RX_PORT GPIOD
#define USART_RX_Pin GPIO_PIN_6
#define USART_PIN_RCU_RX RCU_GPIOD


#define USART_485_CS_RCU RCU_GPIOD
#define USART_485_CS_PORT GPIOD
#define USARTX_485_CS_Pin GPIO_PIN_4

#define USARTX_485_Send 1
#define USARTX_485_Receive 0



/************************ 变量定义 ************************/


/************************ 函数定义 ************************/

void usart485_232_init(void);
void usart485_232_send_str(uint8_t* str, uint8_t len);
void usart485_232_recv_buf(void);

#endif // !__USART_H__
/****************************End*****************************/
