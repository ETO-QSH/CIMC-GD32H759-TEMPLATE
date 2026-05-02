/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：usart.h
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/9     V0.01    original
************************************************************/
#ifndef __USART_H__
#define __USART_H__
/************************* 头文件 *************************/

#include "gd32h7xx.h"
#include "gd32h7xx_libopt.h"
#include "stdio.h"
#include "string.h" 

/************************* 宏定义 *************************/
#define UARTx_RCU RCU_UART4
#define UARTx UART4
#define GPIOx_RCU RCU_GPIOB

#define UARTx_AF GPIO_AF_14
#define UARTx_GPIOx_PORT GPIOB
#define UARTx_TX GPIO_PIN_5
#define UARTx_RX GPIO_PIN_13

/************************ 变量定义 ************************/


/************************ 函数定义 ************************/

void my_usart_init(void);

void usart_recv_buf(void);

#endif // !__USART_H__

/****************************End*****************************/
