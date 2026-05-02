/************************************************************
* 版权：2025CIMC Copyright。
* 文件：tcp_client.c
* 作者: Jialei Zhao
* 平台: 2025CIMC IHD-V04
* 版本: Jialei Zhao     2026/3/10     V0.01    original
************************************************************/
/************************* 头文件 *************************/
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "tcp_client.h"
#include "gd32h7xx_enet_eval.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "lwip/api.h"
#include "lwip/sockets.h"
#include "app_main.h"


/************************* 宏定义 *************************/
#define MAX_BUF_SIZE                    50

/************************ 变量定义 ************************/


/************************ 函数定义 ************************/
void tcp_client_task(void* pvParameters);

/************************************************************
 * Function :       my_tcp_client_init
 * Comment  :       用于初始化TCP客户端
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-3-10 V0.01 original
************************************************************/
void my_tcp_client_init(void)
{
	xTaskCreate(tcp_client_task , "TCP_CLIENT" , TASK_STACK_SIZE , NULL , Priority4 , NULL);
}

/************************************************************
 * Function :       tcp_client_task
 * Comment  :       TCP客户端任务函数(处理用户业务的任务)
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-03-10 V0.1 original
************************************************************/
void tcp_client_task(void* pvParameters)
{
	// printf("tcp_client_task\r\n");
	while (1)
	{
		int  ret , recvnum , sockfd = -1;
		int tcp_port = 10260;
		struct sockaddr_in svr_addr , clt_addr;
		char buf[100];
		ip_addr_t ipaddr;

		IP4_ADDR(&ipaddr , IP_S_ADDR0 , IP_S_ADDR1 , IP_S_ADDR2 , IP_S_ADDR3);

		/* set up address to connect to */
		svr_addr.sin_family = AF_INET;
		svr_addr.sin_port = htons(tcp_port);
		svr_addr.sin_addr.s_addr = ipaddr.addr;

		clt_addr.sin_family = AF_INET;
		clt_addr.sin_port = htons(tcp_port);
		clt_addr.sin_addr.s_addr = htons(INADDR_ANY);

		while (1)
		{
			/* create the socket */
			sockfd = socket(AF_INET , SOCK_STREAM , 0);
			if (sockfd < 0)
			{
				continue;
			}

			ret = bind(sockfd , (struct sockaddr*)&clt_addr , sizeof(clt_addr));
			if (ret < 0)
			{
				lwip_close(sockfd);
				sockfd = -1;
				continue;
			}

			/* connect */
			ret = connect(sockfd , (struct sockaddr*)&svr_addr , sizeof(svr_addr));
			if (ret < 0)
			{
				lwip_close(sockfd);
				sockfd = -1;
				continue;
			}

			while (-1 != sockfd)
			{
				/* reveive packets, and limit a reception to MAX_BUF_SIZE bytes */
				recvnum = recv(sockfd , buf , MAX_BUF_SIZE , 0);
				if (recvnum <= 0)
				{
					lwip_close(sockfd);
					sockfd = -1;
					break;
				}
				send(sockfd , buf , recvnum , 0);
			}

			lwip_close(sockfd);
			sockfd = -1;
		}

	}
}

/****************************End*****************************/
