#ifndef __USART_H__
#define __USART_H__

#include "gd32h7xx.h"
#include "gd32h7xx_libopt.h"
#include "stdio.h"
#include "string.h" 

#define UARTx_RCU RCU_UART4
#define UARTx UART4
#define GPIOx_RCU RCU_GPIOB

#define UARTx_AF GPIO_AF_14
#define UARTx_GPIOx_PORT GPIOB
#define UARTx_TX GPIO_PIN_5
#define UARTx_RX GPIO_PIN_13

void my_usart_init(void);

void usart_recv_buf(void);

#endif // !__USART_H__
