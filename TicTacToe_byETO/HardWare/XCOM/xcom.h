#ifndef __USART_xcom_H__
#define __USART_xcom_H__

#include "HeaderFiles.h"

#define USART USART1
#define USARTX_RCU RCU_USART1

#define USART_TX_PORT GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_PIN_RCU_TX RCU_GPIOA

#define USART_RX_PORT GPIOD
#define USART_RX_Pin GPIO_PIN_6
#define USART_PIN_RCU_RX RCU_GPIOD


#define USART_xcom_CS_RCU RCU_GPIOD
#define USART_xcom_CS_PORT GPIOD
#define USARTX_xcom_CS_Pin GPIO_PIN_4

#define USARTX_xcom_Send 1
#define USARTX_xcom_Receive 0

void usart_xcom_init(void);
void usart_xcom_send_str(uint8_t* str, uint8_t len);
void usart_xcom_recv_buf(void);

#endif // !__USART_H__
