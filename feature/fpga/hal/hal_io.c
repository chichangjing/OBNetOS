
#include "hal_io.h"
#include "hal_time.h"
#include "delay.h"



void GpioConfInit(void)
{
  	GPIO_InitTypeDef gpioinit;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOB | 
                           RCC_AHB1Periph_GPIOE | FPGAPROMPERIPH, ENABLE);

    //fpga1 config pin init
	gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
	gpioinit.GPIO_Mode = GPIO_Mode_OUT;
	gpioinit.GPIO_OType = GPIO_OType_PP;
	gpioinit.GPIO_PuPd = GPIO_PuPd_DOWN;
	gpioinit.GPIO_Pin = FPGAPROMPIN;
	GPIO_Init(FPGAPROMPORT, &gpioinit);
	
	gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
	gpioinit.GPIO_Mode = GPIO_Mode_IN;
	gpioinit.GPIO_OType = GPIO_OType_PP;
	gpioinit.GPIO_PuPd = GPIO_PuPd_DOWN;
	gpioinit.GPIO_Pin = FPGADONEPIN | FPGAINITPIN;
	GPIO_Init(FPGAINITPORT, &gpioinit);
	
	FPGAPROMCRL(1);  
}

//#define	TIMEBLINK	200

void BlinkRunLed(unsigned int timeblink)
{
	static unsigned int otime;
	static BitAction mode;
	unsigned int ctime;
	SysTickGet(&ctime);
	U32TIMEOUT(ctime, otime);
	if (ctime >= timeblink)
	{
		SysTickGet(&otime);
		mode = !mode;
		LEDRUNCTL(mode);
//		LEDDATACTL(~mode);
	}
}

void IdGet(unsigned char *id)
{
	*id = 0;
	*id  |= (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7)) ? (1 << 7) : 0;
	*id  |= (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_8)) ? (1 << 6) : 0;
	*id  |= (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9)) ? (1 << 5) : 0;
	*id  |= (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_10)) ? (1 << 4) : 0;
	*id  |= (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11)) ? (1 << 3) : 0;
	*id  |= (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_12)) ? (1 << 2) : 0;
	*id  |= (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_13)) ? (1 << 1) : 0;
	*id  |= (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_14)) ? (1 << 0) : 0;
}


#if (defined DEBUG_ONLYSWD)

void DebugPortAFSet(debugportmode mode)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//	temp = AFIO->MAPR & 0xf8ffffff;
//	temp |= mode << 24;
//	AFIO->MAPR |= temp;
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, DISABLE);
}

#endif













//end hal_io.c
