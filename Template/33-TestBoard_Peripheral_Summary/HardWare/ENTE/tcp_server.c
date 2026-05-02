/************************************************************
* 版权：2025CIMC Copyright。
* 文件：tcp_server.c
* 作者: Jialei Zhao
* 平台: 2025CIMC IHD-V04
* 版本: Jialei Zhao     2026/3/10     V0.01    original
************************************************************/
/************************* 头文件 *************************/
#include "tcp_server.h"
#include "app_main.h"
#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip/sockets.h"
/************************* 宏定义 *************************/
#define MAX_NAME_SIZE      32
#define GREETING           "\n\r======= HelloGigaDevice =======\
                            \n\r== GD32 ==\
                            \n\r== Telnet SUCCESS==\
                            \n\rHello. What is your name?\r\n"
#define HELLO              "\n\rGigaDevice 8000 PORT Hello "

/************************ 变量定义 ************************/
struct recev_packet
{
	int length;
	char bytes[MAX_NAME_SIZE];
	int done;
} name_recv;

/************************ 函数定义 ************************/
void hello_task(void* pvParameters);
static err_t hello_gigadevice_recv(int fd , void* data , int len);

/************************************************************
 * Function :       my_tcp_server_init
 * Comment  :       初始化TCP服务器
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-03-10 V0.1 original
************************************************************/
void my_tcp_server_init(void)
{
	xTaskCreate(hello_task , "HELLO" , DEFAULT_THREAD_STACKSIZE , NULL , Priority4 , NULL);
}


/************************************************************
 * Function :       hello_task
 * Comment  :       任务函数(处理用户业务的任务)
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-03-10 V0.1 original
************************************************************/
void hello_task(void* pvParameters)
{
	// printf("server_task\r\n");
	int ret;
	int sockfd = -1 , newfd = -1;
	uint32_t len;
	int tcp_port = 8000;
	int recvnum;
	struct sockaddr_in svr_addr , clt_addr;
	char buf[50];

	/* bind to port 8000 at any interface */
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(tcp_port);
	svr_addr.sin_addr.s_addr = htons(INADDR_ANY);

	name_recv.length = 0;
	name_recv.done = 0;

	while (1)
	{
		/* create a TCP socket */
		sockfd = socket(AF_INET , SOCK_STREAM , 0);
		if (sockfd < 0)
		{
			continue;
		}

		ret = bind(sockfd , (struct sockaddr*)&svr_addr , sizeof(svr_addr));
		if (ret < 0)
		{
			lwip_close(sockfd);
			sockfd = -1;
			continue;
		}

		/* listen for incoming connections (TCP listen backlog = 1) */
		ret = listen(sockfd , 1);
		if (ret < 0)
		{
			lwip_close(sockfd);
			continue;
		}

		len = sizeof(clt_addr);

		/* grab new connection */
		newfd = accept(sockfd , (struct sockaddr*)&clt_addr , (socklen_t*)&len);
		if (-1 != newfd)
		{
			send(newfd , (void*)&GREETING , sizeof(GREETING) , 0);
		}

		while (-1 != newfd)
		{
			/* reveive packets, and limit a reception to MAX_NAME_SIZE bytes */
			recvnum = recv(newfd , buf , MAX_NAME_SIZE , 0);
			if (recvnum <= 0)
			{
				lwip_close(newfd);
				newfd = -1;
				break;
			}
			hello_gigadevice_recv(newfd , buf , recvnum);
		}

		lwip_close(sockfd);
		sockfd = -1;
	}
}

/************************************************************
 * Function :       hello_gigadevice_recv
 * Comment  :       处理接收到的数据，将其后面添加"\r\n"
 * Parameter:       int fd, void *data, int len
 * Return   :       err_t
 * Author   :       Jialei Zhao
 * Date     :       2026-03-10 V0.1 original
************************************************************/
static err_t hello_gigadevice_recv(int fd , void* data , int len)
{
	char* c;
	int i;
	int done;

	done = 0;
	c = (char*)data;

	/* a telnet communication packet is ended with an enter key,
	   in socket, here is to check whether the last packet is complete */
	for (i = 0; i < len && !done; i++)
	{
		done = ((c[i] == '\r') || (c[i] == '\n'));
	}

	/* when the packet length received is no larger than MAX_NAME_SIZE */
	if (0 == name_recv.done)
	{
		/* havn't received the end of the packet, so the received data length
		   is the configured socket reception limit--MAX_NAME_SIZE */
		if (0 == done)
		{
			memcpy(name_recv.bytes , data , MAX_NAME_SIZE);
			name_recv.length = MAX_NAME_SIZE;
			name_recv.done = 1;
			/* have received the end of the packet */
		}
		else
		{
			memcpy(name_recv.bytes , data , len);
			name_recv.length = len;
		}
	}

	if (1 == done)
	{
		if (c[len - 2] != '\r' || c[len - 1] != '\n')
		{
			/* limit the received data length to MAX_NAME_SIZE - 2('\r' and '\n' will be put into the buffer) */
			if ((c[len - 1] == '\r' || c[len - 1] == '\n') && (len + 1 <= MAX_NAME_SIZE))
			{
				/* calculate the buffer size to be sent(including '\r' and '\n') */
				name_recv.length += 1;
			}
			else if (len + 2 <= MAX_NAME_SIZE)
			{
				name_recv.length += 2;
			}
			else
			{
				name_recv.length = MAX_NAME_SIZE;
			}

			/* save the received data to name_recv.bytes */
			name_recv.bytes[name_recv.length - 2] = '\r';
			name_recv.bytes[name_recv.length - 1] = '\n';
		}

		send(fd , (void*)&HELLO , sizeof(HELLO) , 0);
		send(fd , name_recv.bytes , name_recv.length , 0);

		name_recv.done = 0;
		name_recv.length = 0;
	}

	return ERR_OK;
}

/****************************End*****************************/
