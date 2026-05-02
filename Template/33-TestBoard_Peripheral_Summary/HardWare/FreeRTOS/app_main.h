/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：app_main.h
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/3/10     V0.01    original
************************************************************/
#ifndef __APP_MAIN_H__
#define __APP_MAIN_H__

/************************* 头文件 *************************/

#include "HeaderFiles.h"

/************************* 宏定义 *************************/

#define TASK_STACK_SIZE 256

/************************ 变量定义 ************************/
typedef enum
{
	Priority0 = 1 ,
	Priority1 ,
	Priority2 ,
	Priority3 ,
	Priority4
} TaskPriority;

/************************ 函数定义 ************************/
void my_app_main_init(void);


#endif // !__APP_MAIN_H__

/****************************End*****************************/

