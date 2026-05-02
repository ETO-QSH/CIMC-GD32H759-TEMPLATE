/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：can.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/4/16     V0.01    original
************************************************************/


/************************* 头文件 *************************/

#include "can.h"

/************************* 宏定义 *************************/
#define tx_data_size 8
#define rx_data_size tx_data_size

/************************ 变量定义 ************************/
uint8_t rx_data[rx_data_size] = { 0 };
uint16_t rx_data_len = 0;
static uint8_t can_rx_flag = 0;

uint8_t tx_data[tx_data_size] = { 1,2,3,4,5,6,7,8 };

can_mailbox_descriptor_struct transmit_message;
can_mailbox_descriptor_struct receive_message;


/************************ 函数定义 ************************/
static void can_gpio_init(void);

static void can_config(void);

static void can_receive_transmit_config(void);

/************************************************************
 * Function :       my_can_init
 * Comment  :       用于初始化CAN
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-04-16 V0.1 original
 ************************************************************/
void my_can_init(void)
{
	can_gpio_init();
	can_config();
	can_receive_transmit_config();
}

/************************************************************
 * Function :       can_gpio_init
 * Comment  :       用于初始化CAN GPIO
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-04-16 V0.1 original
 ************************************************************/
static void can_gpio_init(void)
{
	/* configure CAN2 clock source */
	rcu_can_clock_config(IDX_CAN2 , RCU_CANSRC_APB2);
	/* enable CAN clock */
	rcu_periph_clock_enable(RCU_CAN2);
	/* enable CAN port clock */
	rcu_periph_clock_enable(RCU_GPIOF);

	/* configure CAN2_RX GPIO */
	gpio_af_set(GPIOF , GPIO_AF_2 , GPIO_PIN_6);
	gpio_output_options_set(GPIOF , GPIO_OTYPE_PP , GPIO_OSPEED_60MHZ , GPIO_PIN_6);
	gpio_mode_set(GPIOF , GPIO_MODE_AF , GPIO_PUPD_NONE , GPIO_PIN_6);

	/* configure CAN2_TX GPIO */
	gpio_af_set(GPIOF , GPIO_AF_2 , GPIO_PIN_7);
	gpio_output_options_set(GPIOF , GPIO_OTYPE_PP , GPIO_OSPEED_60MHZ , GPIO_PIN_7);
	gpio_mode_set(GPIOF , GPIO_MODE_AF , GPIO_PUPD_PULLUP , GPIO_PIN_7);
}
/************************************************************
 * Function :       can_config
 * Comment  :       用于配置CAN
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-04-16 V0.1 original
 ************************************************************/
static void can_config(void)
{
	// 配置CAN2
	can_parameter_struct can_parameter;

	/* initialize CAN register */
	can_deinit(CAN2);
	/* initialize CAN */
	can_struct_para_init(CAN_INIT_STRUCT , &can_parameter);

	/* initialize CAN parameters */
	can_parameter.internal_counter_source = CAN_TIMER_SOURCE_BIT_CLOCK;
	can_parameter.self_reception = DISABLE;	//!自己接收自己的消息
	// can_parameter.self_reception = ENABLE;	//!自己接收自己的消息
	can_parameter.mb_tx_order = CAN_TX_HIGH_PRIORITY_MB_FIRST;	//!发送邮箱优先级
	can_parameter.mb_tx_abort_enable = ENABLE; //! 发送中止使能
	can_parameter.local_priority_enable = DISABLE; //!本地优先级禁止
	can_parameter.mb_rx_ide_rtr_type = CAN_IDE_RTR_FILTERED; //! IDE/RTR 过滤方式
	can_parameter.mb_remote_frame = CAN_STORE_REMOTE_REQUEST_FRAME; //! 远程帧存储
	can_parameter.rx_private_filter_queue_enable = DISABLE; //!接收私有过滤队列禁止
	can_parameter.edge_filter_enable = DISABLE; //! 边沿滤波禁止	
	can_parameter.protocol_exception_enable = DISABLE; //! 协议异常禁止
	can_parameter.rx_filter_order = CAN_RX_FILTER_ORDER_MAILBOX_FIRST; //! 接收过滤顺序
	can_parameter.memory_size = CAN_MEMSIZE_32_UNIT; //! 内存大小
	/* filter configuration */
	//!公共过滤器掩码：0 表示接受所有报文ID（不过滤任何报文）
	can_parameter.mb_public_filter = 0U;
	/* baud rate 1Mbps, sample point at 80% */
	/* ---- 波特率配置：目标 1Mbps，采样点约 80% ----
	*
	* 位时序公式（以时间份额 Tq 为单位）：
	*   1 bit = SYNC_SEG(1) + PROP_SEG + PHASE_SEG1 + PHASE_SEG2
	*         = 1 + 2 + 5 + 2 = 10 Tq
	*
	* 系统时钟（假设 300MHz）/ prescaler = CAN时钟
	*   300MHz / 30 = 10MHz → 1 Tq = 100ns
	*   1 bit = 10 Tq = 1000ns → 波特率 = 1Mbps
	*
	* 采样点 = (1 + PROP + SEG1) / 总Tq = (1+2+5)/10 = 80%
	*/
	can_parameter.resync_jump_width = 1U;
	can_parameter.prop_time_segment = 2U;
	can_parameter.time_segment_1 = 5U;
	can_parameter.time_segment_2 = 2U;
	can_parameter.prescaler = 30U;

	/* initialize CAN */
	can_init(CAN2 , &can_parameter);

	//!开启CAN2的中断
	/* configure CAN2 NVIC */
	nvic_irq_enable(CAN2_Message_IRQn , 6U , 0U);
	/* enable CAN MB0 interrupt */
	can_interrupt_enable(CAN2 , CAN_INT_MB0);

	// can_operation_mode_enter(CAN2 , CAN_NORMAL_MODE);
	//!回环静默模式 自发自收
	can_operation_mode_enter(CAN2 , CAN_LOOPBACK_SILENT_MODE);
}

/************************************************************
 * Function :       can_receive_transmit_config
 * Comment  :       用于配置CAN接收和发送邮箱
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-04-16 V0.1 original
 ************************************************************/
static void can_receive_transmit_config(void)
{
	//!配置接收邮箱
	//!开启CAN2的接收邮箱0
	receive_message.rtr = 0U;
	receive_message.ide = 0U;
	receive_message.code = CAN_MB_RX_STATUS_EMPTY;
	/* rx mailbox */
	receive_message.id = 0x55U;
	receive_message.data = (uint32_t*)(rx_data);
	can_mailbox_config(CAN2 , 0U , &receive_message);
}

/************************************************************
 * Function :       my_can_send_message
 * Comment  :       用于发送CAN消息
 * Parameter:       data: 消息数据指针
 *                   data_len: 消息数据长度
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-04-16 V0.1 original
 ************************************************************/
void my_can_send_message(void)
{
	//!配置发送邮箱
	/* initialize transmit message */
	transmit_message.rtr = 0U;
	transmit_message.ide = 0U;
	transmit_message.code = CAN_MB_TX_STATUS_DATA;
	transmit_message.brs = 0U;
	transmit_message.fdf = 0U;
	transmit_message.prio = 0U;
	transmit_message.data_bytes = 8;
	/* tx message content */
	transmit_message.data = (uint32_t*)(tx_data);
	transmit_message.id = 0x55U;

	/* transmit message */
	can_mailbox_config(CAN2 , 1U , &transmit_message);

	printf("CAN2 transmit data: \r\n");
	uint8_t* ptr = (uint8_t*)transmit_message.data;
	for (uint8_t i = 0; i < transmit_message.data_bytes; i++)
	{
		printf("%02X " , ptr[i]);
	}
	printf("\r\n");
}

/************************************************************
 * Function :       my_can_receive_message
 * Comment  :       用于接收CAN消息
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-04-16 V0.1 original
 ************************************************************/
void my_can_receive_message(void)
{
	/* CAN2 receive data correctly, the received data is printed */
	if (can_rx_flag)
	{
		memset(rx_data , 0U , sizeof(rx_data));

		can_rx_flag = 0;
		/* check the receive message */
		can_mailbox_receive_data_read(CAN2 , 0U , &receive_message);

		memcpy(rx_data , receive_message.data , receive_message.data_bytes);

		printf("CAN2 receive data: \r\n");
		for (uint8_t i = 0; i < receive_message.data_bytes; i++)
		{
			printf("%02X " , rx_data[i]);
		}
		printf("\r\n");
		return;
	}
}
/************************************************************
 * Function :       CAN2_Message_IRQHandler
 * Comment  :       CAN2中断服务函数
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-04-16 V0.1 original
 ************************************************************/
void CAN2_Message_IRQHandler(void)
{
	if (RESET != can_interrupt_flag_get(CAN2 , CAN_INT_FLAG_MB0))
	{
		can_interrupt_flag_clear(CAN2 , CAN_INT_FLAG_MB0);
		can_rx_flag = 1;
	}
}

/************************************************************
 * Function :       my_can_deinit
 * Comment  :       用于关闭CAN2外设
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-04-16 V0.1 original
 ************************************************************/
void my_can_deinit(void)
{
    /* 1. 关闭 NVIC 中断，防止复位过程中触发中断 */
    nvic_irq_disable(CAN2_Message_IRQn);

    /* 2. 关闭 CAN MB0 中断使能 */
    can_interrupt_disable(CAN2, CAN_INT_MB0);

    /* 3. 清除中断标志位 */
    can_interrupt_flag_clear(CAN2, CAN_INT_FLAG_MB0);

    /* 4. 复位 CAN2 外设寄存器 */
    can_deinit(CAN2);

    /* 5. 关闭 CAN2 外设时钟 */
    rcu_periph_clock_disable(RCU_CAN2);

    /* 6. 清空软件层面的状态标志 */
    can_rx_flag = 0;

    // printf("CAN2 deinit done.\r\n");
}
/****************************End*****************************/
