#ifndef __TSENSOR_H
#define __TSENSOR_H	
//#include "stm32f10x.h"
#include "stm32f2xx.h"
#include "stm32f2xx_adc.h"
//////////////////////////////////////////////////////////////////////////////////	 

////////////////////////////////////////////////////////////////////////////////// 	  

/*
#define T_ADC_CH0  0  //ͨ��0
#define T_ADC_CH1  1  //ͨ��1
#define T_ADC_CH2  2  //ͨ��2
#define T_ADC_CH3  3  //ͨ��3
#define T_TEMP_CH  16 //�¶ȴ�����ͨ��
*/
	   
//u16 Get_Adc_Average(u8 ch,u8 times);  //ȡ���¶�ֵ
void T_Adc_Init(void); //ADCͨ����ʼ��
u16  T_Get_Adc(u8 ch); //���ĳ��ͨ��ֵ  	
u16 Get_Adc_Average(u8 ch,u8 times);  //ȡ���¶�ֵ

#endif 
