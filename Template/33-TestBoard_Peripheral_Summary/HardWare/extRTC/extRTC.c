/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：usart.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/4/15     V0.01    original
************************************************************/
/************************* 头文件 *************************/
#include "extRTC.h"
#include "i2c.h"

/************************* 宏定义 *************************/
#define ADDR_WRITE 0xA2
#define ADDR_READ  0xA3

/************************ 变量定义 ************************/

static void extRTC_WriteCMD(uint8_t addr , uint8_t cmd);

static void extRTC_Config(void);
void my_extRTC_init(void)
{
	my_I2C_Init();

	extRTC_Config();
}

/************************************************************
 * Function :       extRTC_Config
 * Comment  :       用户程序功能: 配置RTC寄存器
 * Parameter:       无
 * Return   :       无
 * Author   :       Jialei Zhao
 * Date     :       2026-04-15 V0.1 original
************************************************************/
static void extRTC_Config(void)
{
	// 控制寄存器1：普通模式，时钟运行(STOP=0)
	extRTC_WriteCMD(0x00 , 0x00);

	// 控制寄存器2：清除所有中断标志
	extRTC_WriteCMD(0x01 , 0x00);

	// 时间寄存器（BCD格式）
	extRTC_WriteCMD(0x02 , 0x00);   // 秒=00, VL=0（清除掉电标志）

	// 3. CLKOUT配置(0x0D): FE=1使能输出, FD1=1 FD0=1 → 1Hz
	// extRTC_WriteCMD(0x0D , 0x83);

	extRTC_WriteCMD(0x03 , 0x00);   // 分=00
	extRTC_WriteCMD(0x04 , 0x00);   // 时=00

	extRTC_WriteCMD(0x05 , 0x01);   // 日=01

	extRTC_WriteCMD(0x06 , 0x00);   // 星期=0

	extRTC_WriteCMD(0x07 , 0x01);   // 月=01

	extRTC_WriteCMD(0x08 , 0x26);   // 年=26（2026年，可选）
}



/************************************************************
 * Function :       extRTC_WriteCMD
 * Comment  :       用户程序功能: 写入RTC寄存器
 * Parameter:       addr - 寄存器地址
 * cmd - 要写入的命令字节
 * Return   :       无
 * Author   :       Jialei Zhao
 * Date     :       2026-04-15 V0.1 original
************************************************************/
static void extRTC_WriteCMD(uint8_t addr , uint8_t cmd)
{
	I2C_Start();
	my_I2C_Send_Byte(ADDR_WRITE);
	my_I2C_Send_Byte(addr);
	my_I2C_Send_Byte(cmd);
	I2C_Stop();
}

/************************************************************
 * Function :       extRTC_ReadTime
 * Comment  :       用户程序功能: 读取RTC时间
 * Parameter:       addr - 寄存器地址
 * data - 存储读取到的时间字符串的缓冲区
 * len - 缓冲区长度
 * Return   :       无
 * Author   :       Jialei Zhao
 * Date     :       2026-04-15 V0.1 original
************************************************************/
void my_extRTC_ReadTime(uint8_t addr , uint8_t* data , uint8_t len)
{

	if (len < 20)
	{
		printf("len error\n");
		return;
	}

	I2C_Start();
	my_I2C_Send_Byte(ADDR_WRITE);   // 0xA2
	my_I2C_Send_Byte(addr);         // 设置起始寄存器地址 0x02
	// I2C_Stop();

	I2C_Start();                    // 重复起始信号
	my_I2C_Send_Byte(ADDR_READ);    // 0xA3



	uint8_t data_tmp[7] = { 0 };

	//读取顺序 秒 分  时  日  星期 月 年
	for (int8_t i = 0; i < 7; i++)
	{
		data_tmp[i] = my_I2C_Read_Byte();
		if (i == 7 - 1)
		{
			I2C_Respond(1);
		}
		else
		{
			I2C_Respond(0);
		}
	}

	I2C_Stop();

	sprintf((char*)data , "20%02x-%02x-%02x %02x:%02x:%02x" ,
		data_tmp[6] , data_tmp[5] & 0x1F , data_tmp[3] & 0x3F , data_tmp[2] & 0x3F , data_tmp[1] & 0x7F , data_tmp[0] & 0x7F);
}

/************************************************************
 * Function :       extRTC_Test
 * Comment  :       用户程序功能: 测试RTC功能
 * Parameter:       无
 * Return   :       无
 * Author   :       Jialei Zhao
 * Date     :       2026-04-15 V0.1 original
************************************************************/
void my_extRTC_Test(void)
{
	I2C_Start();
	uint8_t ret = my_I2C_Send_Byte(0xA2);
	printf("ACK = %d\r\n" , ret);
	I2C_Stop();
}


/****************************End*****************************/
