#include "spi.h"

void my_spi_init(void)
{
	//!¿ªÊ±ÖÓ
	rcu_periph_clock_enable(RCU_GPIOH);
	rcu_periph_clock_enable(RCU_GPIOF);
	rcu_periph_clock_enable(RCU_SPI4);
	rcu_spi_clock_config(IDX_SPI4 , RCU_SPISRC_APB2);

	//!ÅäÖÃGPIO
	/* connect port to SPI4_NSS->PF10
					   SPI4_SCK->PH6
					   SPI4_MISO->PF8
					   SPI4_MOSI->PF9 */
					   //CSÒý½Å
	gpio_mode_set(GPIOF , GPIO_MODE_OUTPUT , GPIO_PUPD_NONE , GPIO_PIN_10);
	gpio_output_options_set(GPIOF , GPIO_OTYPE_PP , GPIO_OSPEED_60MHZ , GPIO_PIN_10);

	//SCKÒý½Å
	gpio_af_set(GPIOH , GPIO_AF_5 , GPIO_PIN_6);
	gpio_mode_set(GPIOH , GPIO_MODE_AF , GPIO_PUPD_NONE , GPIO_PIN_6);
	gpio_output_options_set(GPIOH , GPIO_OTYPE_PP , GPIO_OSPEED_60MHZ , GPIO_PIN_6);
	//MISOÒý½Å
	gpio_af_set(GPIOF , GPIO_AF_5 , GPIO_PIN_8 | GPIO_PIN_9);
	gpio_mode_set(GPIOF , GPIO_MODE_AF , GPIO_PUPD_NONE , GPIO_PIN_8 | GPIO_PIN_9);
	gpio_output_options_set(GPIOF , GPIO_OTYPE_PP , GPIO_OSPEED_60MHZ , GPIO_PIN_8 | GPIO_PIN_9);


	//!ÅäÖÃSPI4
	spi_parameter_struct spi_init_struct;
	/* deinitialize SPI and the parameters */
	spi_i2s_deinit(SPI4);

	spi_struct_para_init(&spi_init_struct);

	/* SPI0 parameter configuration */
	spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
	spi_init_struct.device_mode = SPI_MASTER;
	spi_init_struct.data_size = SPI_DATASIZE_8BIT;
	spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
	spi_init_struct.nss = SPI_NSS_SOFT;
	spi_init_struct.prescale = SPI_PSC_16;
	spi_init_struct.endian = SPI_ENDIAN_MSB;
	spi_init(SPI4 , &spi_init_struct);

	/* enable SPI byte access */
	spi_byte_access_enable(SPI4);

	/* SPI NSS pin high level in software mode */
	spi_nss_internal_high(SPI4);

	/* configure SPI current data number  */
	spi_current_data_num_config(SPI4 , 1);

	SET_SPI0_NSS_HIGH;

	/* SPI enable */
	spi_enable(SPI4);
}

uint8_t my_spi_send_byte(uint8_t byte)
{

	while (RESET == spi_i2s_flag_get(SPI4 , SPI_FLAG_TP));
	spi_i2s_data_transmit(SPI4 , byte);
	spi_master_transfer_start(SPI4 , SPI_TRANS_START);

	while (RESET == spi_i2s_flag_get(SPI4 , SPI_FLAG_RP));
	uint8_t receive_byte = spi_i2s_data_receive(SPI4);

	return receive_byte;

}

