/************************************************************
 * 版权：2025CIMC Copyright。 
 * 文件：main.c
 * 作者: Jialei Zhao
 * 平台: 2025 CIMC IHD V04
 * 版本: Jialei Zhao     2026/2/5     V0.01    original
************************************************************/

/************************* 头文件 *************************/

#include "HeaderFiles.h"

/************************ 执行函数 ************************/

int main(void)
{
	
	System_Init();   // 系统初始化
	
	UsrFunction();   // 执行用户函数
	
}

// 定义空的 vApplicationStackOverflowHook
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // 空实现
    while(1);
}

// 定义空的 vApplicationIdleHook
void vApplicationIdleHook(void)
{
    // 空实现
}

// 定义空的 vApplicationTickHook
void vApplicationTickHook(void)
{
    // 空实现
}

// 定义空的 vApplicationMallocFailedHook
void vApplicationMallocFailedHook(void)
{
    // 空实现
    while(1);
}
/****************************End*****************************/
