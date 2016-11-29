#include "tsensor.h"
#include "delay.h"
//#include "stm32f10x_adc.h"
//#include "stm32f2xx.h"
//#include "stm32f2xx_adc.h"
//////////////////////////////////////////////////////////////////////////////////	 

////////////////////////////////////////////////////////////////////////////////// 	  
 

void T_Adc_Init(void)  //ADC通道初始化
{
	ADC_InitTypeDef ADC_InitStructure; 
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1	, ENABLE );	  //使能ADC1通道时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
	//RCC_ADCCLKConfig(RCC_PCLK2_Div2);   //24M/2=12,ADC最大时间不能超过14M

	//PA0/1/2/3 作为模拟通道输入引脚                         
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
//	GPIO_Init(GPIOA, &GPIO_InitStructure);	

   	ADC_DeInit();  //将外设 ADC1 的全部寄存器重设为缺省值
	
 
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div6;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	//ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	//ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	//ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器

	ADC_TempSensorVrefintCmd(ENABLE);//开启内部温度传感器
	
 
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1

	//ADC_ResetCalibration(ADC1);	//重置指定的ADC1的校准寄存器

    //while(ADC_GetResetCalibrationStatus(ADC1));	//获取ADC1重置校准寄存器的状态,设置状态则等待

	//ADC_StartCalibration(ADC1);

	//while(ADC_GetCalibrationStatus(ADC1));		//获取指定ADC1的校准程序,设置状态则等待
	
	
}
u16 T_Get_Adc(u8 ch)   
{
	ADC_RegularChannelConfig(ADC1, ch, 1,ADC_SampleTime_144Cycles);	//ADC1,ADC通道3,规则采样顺序值为1,采样时间为239.5周期	  			    
 
	ADC_SoftwareStartConv(ADC1);		//使能指定的ADC1的软件转换启动功能
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}


//得到ADC采样内部温度传感器的值

u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
		{
		temp_val+=T_Get_Adc(ch);	  //TampSensor
		Delay_Ms(5);
		}
	return temp_val/times;
}
