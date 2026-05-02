/************************************************************
* 版权：2025CIMC Copyright。
* 文件：udp.c
* 作者: Jialei Zhao
* 平台: 2025CIMC IHD-V04
* 版本: Jialei Zhao     2026/3/10     V0.01    original
************************************************************/

/************************* 头文件 *************************/
#include "udp_tmp.h"
#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "gd32h7xx_enet_eval.h"
#include "lwip/api.h"
#include "lwip/sockets.h"
#include "app_main.h"

/************************* 宏定义 *************************/
#define MAX_BUF_SIZE        50


/************************ 变量定义 ************************/


/************************ 函数定义 ************************/
void udp_task(void* pvParameters);

/************************************************************
 * Function :       my_udp_init
 * Comment  :       初始化UDP服务器
 * Parameter:       void
 * Return   :       void
 * Author   :       Jialei Zhao
 * Date     :       2026-03-10 V0.1 original
************************************************************/
void my_udp_init(void)
{
	xTaskCreate(udp_task , "UDP" , DEFAULT_THREAD_STACKSIZE , NULL , Priority4 , NULL);
}

/************************************************************
 * Function :       udp_task
 * Comment  :       UDP任务(处理用户业务的任务)
 * Parameter:       void* pvParameters
 * Return   :       void
 * Author   :       Jialei Zhao
 * Date     :       2026-03-10 V0.1 original
************************************************************/
void udp_task(void* pvParameters)
{
	// printf("udp_task\r\n");
	int  ret , recvnum , sockfd = -1;
	int udp_port = 1025;
	struct sockaddr_in rmt_addr , bod_addr;
	char buf[100];
	u32_t len;
	ip_addr_t ipaddr;

	IP4_ADDR(&ipaddr , IP_S_ADDR0 , IP_S_ADDR1 , IP_S_ADDR2 , IP_S_ADDR3);

	/* set up address to connect to */
	rmt_addr.sin_family = AF_INET;
	rmt_addr.sin_port = htons(udp_port);
	rmt_addr.sin_addr.s_addr = ipaddr.addr;

	bod_addr.sin_family = AF_INET;
	bod_addr.sin_port = htons(udp_port);
	bod_addr.sin_addr.s_addr = htons(INADDR_ANY);

	while (1)
	{
		/* create the socket */
		sockfd = socket(AF_INET , SOCK_DGRAM , 0);
		if (sockfd < 0)
		{
			continue;
		}

		ret = bind(sockfd , (struct sockaddr*)&bod_addr , sizeof(bod_addr));

		if (ret < 0)
		{
			lwip_close(sockfd);
			sockfd = -1;
			continue;
		}

		len = sizeof(rmt_addr);
		/* reveive packets from rmt_addr, and limit a reception to MAX_BUF_SIZE bytes */
		recvnum = recvfrom(sockfd , buf , MAX_BUF_SIZE , 0 , (struct sockaddr*)&rmt_addr , &len);

		while (-1 != sockfd)
		{
			if (recvnum > 0)
			{
				/* send packets to rmt_addr */
				sendto(sockfd , buf , recvnum , 0 , (struct sockaddr*)&rmt_addr , sizeof(rmt_addr));
			}
			recvnum = recvfrom(sockfd , buf , MAX_BUF_SIZE , 0 , (struct sockaddr*)&rmt_addr , &len);
		}

		lwip_close(sockfd);
	}
}
/****************************End*****************************/

