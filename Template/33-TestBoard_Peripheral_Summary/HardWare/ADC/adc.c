/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：adc.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/5     V0.01    original
************************************************************/

/************************* 头文件 *************************/
#include "adc.h"

/************************ 全局变量定义 ************************/
/************************************************************
 * Function :       my_adc_init
 * Comment  :       用户程序功能: 初始化ADC
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-05 V0.1 original
************************************************************/
void my_adc_init(void)
{
    rcu_periph_clock_enable(RCU_SYSCFG);

    rcu_periph_clock_enable(RCU_ADC0);
    syscfg_analog_switch_enable(SYSCFG_PA0_ANALOG_SWITCH);
    syscfg_analog_switch_enable(SYSCFG_PA1_ANALOG_SWITCH);

    adc_deinit(ADC0);

    adc_clock_config(ADC0, ADC_CLK_SYNC_HCLK_DIV8);

    adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);

    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);

    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);

    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_DISABLE);

    adc_enable(ADC0);

    vTaskDelay(1);  

    adc_calibration_mode_config(ADC0, ADC_CALIBRATION_OFFSET);
    adc_calibration_number(ADC0, ADC_CALIBRATION_NUM1);
    adc_calibration_enable(ADC0);  


}

/************************************************************
 * Function :       my_adc_read
 * Comment  :       用户程序功能: 读取ADC转换结果
 * Parameter:       null
 * Return   :       uint16_t 转换结果
 * Author   :       Jialei Zhao
 * Date     :       2026-02-05 V0.1 original
************************************************************/
uint16_t my_adc_read(uint8_t channel)
{
	adc_regular_channel_config(ADC0 , 0 , channel , 333);	//  配置ADC通道和采样时间

	adc_flag_clear(ADC0 , ADC_FLAG_EOC);  				//  清除结束标志
	adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
	while (SET != adc_flag_get(ADC0 , ADC_FLAG_EOC)) {}  	//  获取转换结束标志
	return adc_regular_data_read(ADC0);
}



/****************************End*****************************/

