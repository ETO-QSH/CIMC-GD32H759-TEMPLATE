#ifndef __SPI_H__
#define __SPI_H__

#include "HeaderFiles.h"

#define SET_SPI0_NSS_HIGH          gpio_bit_set(GPIOF,GPIO_PIN_10);
#define SET_SPI0_NSS_LOW           gpio_bit_reset(GPIOF,GPIO_PIN_10);

void my_spi_init(void);

uint8_t my_spi_send_byte(uint8_t byte);


#endif // !__SPI_H__
