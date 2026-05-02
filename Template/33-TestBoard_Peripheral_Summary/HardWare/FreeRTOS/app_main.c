/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：app_main.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/3/10     V0.01    original
************************************************************/
//!跟示例不一样的地方是示例为200MHZ ， 但是该项目模板为240MHZ

/************************* 头文件 *************************/
#include "app_main.h"
#include "usart.h"
#include "netconf.h"
#include "tcp_client.h"
#include "udp_tmp.h"
#include "tcp_server.h"
#include "gd32h7xx_enet_eval.h"
#include "usart.h"

#include "adc.h"
#include "can.h"
#include "extRTC.h"
#include "oled.h"
#include "rtc.h"
#include "exmc_sdram.h"
#include "SPI_FLASH.h"
#include "usart485_232.h"



/************************* 宏定义 *************************/

#define  SFLASH_ID                     0xC84017
#define BUFFER_SIZE                    256
#define TX_BUFFER_SIZE                 BUFFER_SIZE
#define RX_BUFFER_SIZE                 BUFFER_SIZE
#define  FLASH_WRITE_ADDRESS           0x000000
#define  FLASH_READ_ADDRESS            FLASH_WRITE_ADDRESS


#define BUFFER_SIZE1                ((uint32_t)0x8000)
#define WRITE_READ_ADDR            ((uint32_t)0x7FFF00)

/************************ 变量定义 ************************/
uint32_t flash_id = 0;
uint8_t  tx_buffer[TX_BUFFER_SIZE];
uint8_t  rx_buffer[TX_BUFFER_SIZE];
uint8_t  is_successful = 0;


uint32_t writereadstatus = 0;
//!32位对齐的缓冲区
__ALIGNED(32) uint8_t txbuffer1[BUFFER_SIZE1] = { 0 };
__ALIGNED(32) uint8_t rxbuffer1[BUFFER_SIZE1] = { 0 };

/************************ 函数定义 ************************/
void task_init(void* pvParameters);

//!可以做其他的任务(示例)
void led_task(void* pvParameters);

ErrStatus memory_compare(uint8_t* src , uint8_t* dst , uint16_t length);

/************************************************************
 * Function :       my_app_main_init
 * Comment  :       用户程序任务的创建初始化
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-03-10 V0.1 original
************************************************************/
void my_app_main_init(void)
{

	/* configure 4 bits pre-emption priority */
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

	xTaskCreate(task_init , "task_init" , TASK_STACK_SIZE , NULL , Priority0 , NULL);

	vTaskStartScheduler();

	while (1)
	{
		vTaskDelay(1000);
	}

}

/************************************************************
 * Function :       task_init
 * Comment  :       任务初始化函数
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-03-10 V0.1 original
************************************************************/
void task_init(void* pvParameters)
{
	//!可以初始化自己的一些驱动(或者说在function中进行初始化)
	my_usart_init();
	usart485_232_init();

	/* start toogle LED task every 500ms */
	xTaskCreate(led_task , "LED" , TASK_STACK_SIZE , NULL , Priority1 , NULL);

	//!初始化完成删除初始化任务
	vTaskDelete(NULL);
}

/************************************************************
 * Function :       led_task
 * Comment  :       LED任务函数(处理用户业务的任务)
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-03-10 V0.1 original
************************************************************/
void led_task(void* pvParameters)
{
	while (1)
	{
		//===========OLED TEST=====================
		OLED_Init();
		printf("-------OLED TEST-------------\r\n");
		uint8_t data[16] = { 0 };
		printf("请观察屏幕是否有显示字!!!\r\n");
		for (uint8_t i = 0; i < 5; i++)
		{
			sprintf((char*)data , "Value = %d" , i);
			OLED_ShowString(0 , 0 , "**Hello World**" , 16);
			OLED_ShowString(0 , 16 , data , 16);
			vTaskDelay(1000);
			OLED_Refresh();
		}

		//===========RTC TEST=====================
		my_rtc_init();
		printf("-------RTC TEST-------------\r\n");
		rtc_setup();
		for (uint8_t i = 0; i < 5; i++)
		{
			rtc_show_time();
			vTaskDelay(1000);
		}

		//===========CAN TEST BY ETO =====================
		my_can_init();
		printf("-------CAN TEST-------------\r\n");
		my_can_send_message(); 
		vTaskDelay(5);
		my_can_receive_message(); 
		my_can_deinit();

		//===========EXTRTC TEST=====================
		printf("-------EXTRTC TEST-------------\r\n");
		my_extRTC_init();
		uint8_t time[32] = { 0 };
		for (uint8_t i = 0;i < 5; i++)
		{
			my_extRTC_ReadTime(0x02 , time , 32);
			printf("time = %s\r\n" , time);
			vTaskDelay(1000);
		}

		//=============adc_in0,adc_in1 TEST=====================
		my_adc_init();
		printf("-------adc_in0,adc_in1 TEST-------------\r\n");

		for (uint8_t i = 0; i < 5; i++)
		{
			printf("adc_in0 = %d\r\n" , my_adc_read(0));
			printf("adc_in1 = %d\r\n" , my_adc_read(1));
			vTaskDelay(1000);
		}

		//===========NORFLASH TEST=====================
		spi_flash_init();
		printf("-------NORFLASH TEST-------------\r\n");

		/* get flash id */
		flash_id = spi_flash_read_id();
		printf("\r\nThe Flash_ID:0x%X\r\n" , flash_id);

		/* flash id is correct */
		if (SFLASH_ID == flash_id)
		{
			printf("\r\n******************************erases flash sector*************************\r\n");
			/* erases the specified flash sector */
			spi_flash_sector_erase(FLASH_WRITE_ADDRESS);


			printf("\r\n******************************Write to tx_buffer:*************************\r\n");
			/* printf tx_buffer value */
			for (uint16_t i = 0; i < BUFFER_SIZE; i++)
			{
				tx_buffer[i] = i;
				printf("0x%02X " , tx_buffer[i]);

				if (15 == i % 16)
					printf("\r\n");
			}

			/* write tx_buffer data to the flash */
			spi_flash_buffer_write(tx_buffer , FLASH_WRITE_ADDRESS , TX_BUFFER_SIZE);

			vTaskDelay(10);

			printf("\r\n\r\n******************************Read from tx_buffer:*************************\r\n\r\n");
			/* read a block of data from the flash to rx_buffer */
			spi_flash_buffer_read(rx_buffer , FLASH_READ_ADDRESS , RX_BUFFER_SIZE);
			/* printf rx_buffer value */
			for (uint16_t i = 0; i <= 255; i++)
			{
				printf("0x%02X " , rx_buffer[i]);
				if (15 == i % 16)
					printf("\r\n");
			}


			/*比对读出和写入的数据*/
			if (ERROR == memory_compare(tx_buffer , rx_buffer , 256))
			{
				printf("Err:Data Read and Write aren't Matching.\r\n");
				is_successful = 1;
			}

			/* spi flash test passed */
			if (0 == is_successful)
			{
				printf("\r\nSPI-GD25Q60ESIGR Test Passed!\r\n");
			}
			else
			{
				/* spi flash read id fail */
				printf("\r\nSPI Flash: Read ID Fail!\r\n");
			}
		}
		else
		{
			printf("\r\nSPI Flash: ID Fail!\r\n");
		}

		//====================SDRAM TEST=====================

		/* configure the EXMC access mode */
		exmc_synchronous_dynamic_ram_init(EXMC_SDRAM_DEVICE0);

		printf("-------SDRAM TEST-------------\r\n");

		vTaskDelay(1000);

		/* fill txbuffer */
		fill_buffer(txbuffer1 , BUFFER_SIZE1 , 0x0000);

		/* write data to SDRAM */
		sdram_writebuffer_8(EXMC_SDRAM_DEVICE0 , txbuffer1 , WRITE_READ_ADDR , BUFFER_SIZE1);

		printf("SDRAM write data completed!\r\n");
		vTaskDelay(1000);

		/* read data from SDRAM */
		sdram_readbuffer_8(EXMC_SDRAM_DEVICE0 , rxbuffer1 , WRITE_READ_ADDR , BUFFER_SIZE1);

		printf("SDRAM read data completed!\r\n");
		vTaskDelay(1000);

		printf("Check the data!\r\n");
		vTaskDelay(1000);

		/* compare two buffers */
		for (uint32_t i = 0; i < BUFFER_SIZE1; i++)
		{
			if (rxbuffer1[i] != txbuffer1[i])
			{
				writereadstatus++;
				break;
			}
		}

		//!打印出读取的数据，因为很多所以只打印了前512个字节(0000~00FF,0000~00FF)
		// printf("Recv data: \r\n");
		// for (uint32_t i = 0; i < 512; i++)
		// {
		// 	printf("%04X " , rxbuffer1[i]);
		// }
		if (writereadstatus)
		{
			printf("SDRAM test failed!\r\n");
		}
		else
		{
			printf("SDRAM test succeeded!\r\n");
		}


		//====================ENET TEST=====================
		printf("-------ENET TEST-------------\r\n");
		/* configure ethernet (GPIOs, clocks, MAC, DMA) */
		enet_system_setup();
		/* initilaize the LwIP stack */
		lwip_stack_init();
		printf("ENTE Init Success\r\n");


		//====================485 232 TEST=====================
		printf("-------485 232 TEST-------------\r\n");

		while (1)
		{
			for (uint8_t i = 0; i < 50; i++)
			{
				// 485接收数据
				usart485_232_recv_buf();
				vTaskDelay(20);
			}
			// 485发送数据
			usart485_232_send_str("Hello World!\r\n" , 14);
		}


	}
}
void lwip_netif_status_callback(struct netif* netif)
{
	if (((netif->flags & NETIF_FLAG_UP) != 0) && (0 != netif->ip_addr.addr))
	{
		/* initilaize the tcp server: telnet 8000 */
		my_tcp_client_init();
		/* initilaize the tcp client: echo 10260 */
		my_tcp_server_init();
		/* initilaize the udp: echo 1025 */
		my_udp_init();
	}
}

/*!
	\brief      memory compare function
	\param[in]  src: source data pointer
	\param[in]  dst: destination data pointer
	\param[in]  length: the compare data length
	\param[out] none
	\retval     ErrStatus: ERROR or SUCCESS
*/
ErrStatus memory_compare(uint8_t* src , uint8_t* dst , uint16_t length)
{
	while (length--)
	{
		if (*src++ != *dst++)
			return ERROR;
	}
	return SUCCESS;
}


/****************************End*****************************/
