/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：Function.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/5     V0.01    original
************************************************************/


/************************* 头文件 *************************/

#include "Function.h"
#include "app_main.h"

/************************* 宏定义 *************************/


/************************ 变量定义 ************************/


/************************ 函数定义 ************************/

static void cache_enable(void);

static void mpu_config(void);

/************************************************************
 * Function :       System_Init
 * Comment  :       用于初始化MCU
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-05 V0.1 original
************************************************************/

void System_Init(void)
{
	mpu_config();

	cache_enable();

	systick_config();     // 时钟配置

}

/************************************************************
 * Function :       UsrFunction
 * Comment  :       用户程序功能:
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-05 V0.1 original
************************************************************/

void UsrFunction(void)
{

	my_app_main_init();

	while (1)
	{


	}
}

/*!
	\brief      enable the CPU Chache
	\param[in]  none
	\param[out] none
	\retval     none
*/
static void cache_enable(void)
{
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}


static void mpu_config(void)
{
	mpu_region_init_struct mpu_init_struct;
	mpu_region_struct_para_init(&mpu_init_struct);

	/* disable the MPU */
	ARM_MPU_SetRegion(0U , 0U);
	/* configure the MPU attributes for Reserved, no access */
	mpu_init_struct.region_base_address = 0x0;
	mpu_init_struct.region_size = MPU_REGION_SIZE_4GB;
	mpu_init_struct.access_permission = MPU_AP_NO_ACCESS;
	mpu_init_struct.access_bufferable = MPU_ACCESS_NON_BUFFERABLE;
	mpu_init_struct.access_cacheable = MPU_ACCESS_NON_CACHEABLE;
	mpu_init_struct.access_shareable = MPU_ACCESS_SHAREABLE;
	mpu_init_struct.region_number = MPU_REGION_NUMBER0;
	mpu_init_struct.subregion_disable = 0x87;
	mpu_init_struct.instruction_exec = MPU_INSTRUCTION_EXEC_NOT_PERMIT;
	mpu_init_struct.tex_type = MPU_TEX_TYPE0;
	mpu_region_config(&mpu_init_struct);
	mpu_region_enable();

	/* configure the MPU attributes for SDRAM */
	mpu_init_struct.region_base_address = 0xC0000000;
	mpu_init_struct.region_size = MPU_REGION_SIZE_32MB;
	mpu_init_struct.access_permission = MPU_AP_FULL_ACCESS;
	mpu_init_struct.access_bufferable = MPU_ACCESS_NON_BUFFERABLE;
	mpu_init_struct.access_cacheable = MPU_ACCESS_CACHEABLE;
	mpu_init_struct.access_shareable = MPU_ACCESS_NON_SHAREABLE;
	mpu_init_struct.region_number = MPU_REGION_NUMBER1;
	mpu_init_struct.subregion_disable = 0x0;
	mpu_init_struct.instruction_exec = MPU_INSTRUCTION_EXEC_NOT_PERMIT;
	mpu_init_struct.tex_type = MPU_TEX_TYPE0;
	mpu_region_config(&mpu_init_struct);
	mpu_region_enable();

	/* Configure the DMA descriptors and Rx/Tx buffer*/
	mpu_init_struct.region_base_address = 0x30000000;
	mpu_init_struct.region_size = MPU_REGION_SIZE_16KB;
	mpu_init_struct.access_permission = MPU_AP_FULL_ACCESS;
	mpu_init_struct.access_bufferable = MPU_ACCESS_BUFFERABLE;
	mpu_init_struct.access_cacheable = MPU_ACCESS_NON_CACHEABLE;
	mpu_init_struct.access_shareable = MPU_ACCESS_NON_SHAREABLE;
	mpu_init_struct.region_number = MPU_REGION_NUMBER0;
	mpu_init_struct.subregion_disable = MPU_SUBREGION_ENABLE;
	mpu_init_struct.instruction_exec = MPU_INSTRUCTION_EXEC_PERMIT;
	mpu_init_struct.tex_type = MPU_TEX_TYPE0;
	mpu_region_config(&mpu_init_struct);
	mpu_region_enable();

	/* Configure the LwIP RAM heap */
	mpu_init_struct.region_base_address = 0x30004000;
	mpu_init_struct.region_size = MPU_REGION_SIZE_16KB;
	mpu_init_struct.access_permission = MPU_AP_FULL_ACCESS;
	mpu_init_struct.access_bufferable = MPU_ACCESS_NON_BUFFERABLE;
	mpu_init_struct.access_cacheable = MPU_ACCESS_NON_CACHEABLE;
	mpu_init_struct.access_shareable = MPU_ACCESS_SHAREABLE;
	mpu_init_struct.region_number = MPU_REGION_NUMBER1;
	mpu_init_struct.subregion_disable = MPU_SUBREGION_ENABLE;
	mpu_init_struct.instruction_exec = MPU_INSTRUCTION_EXEC_PERMIT;
	mpu_init_struct.tex_type = MPU_TEX_TYPE1;
	mpu_region_config(&mpu_init_struct);
	mpu_region_enable();



	/* enable the MPU */
	ARM_MPU_Enable(MPU_MODE_PRIV_DEFAULT);
}

/****************************End*****************************/

