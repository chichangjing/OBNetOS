

#include "misc.h"
#include "timer.h"

unsigned int TimerMsTick = 0;

int gTimeoutFlag=0; 

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);		
		TimerMsTick++;
		gTimeoutFlag=1;
	}
	return;
}


unsigned int TimerGetMsTick(void)
{
	return TimerMsTick;
}


void TimerDisable(void)
{
	TIM_Cmd(TIM3, DISABLE);
	TIM_ITConfig(TIM3,TIM_IT_Update, DISABLE);
	return;
}

void TimerInit(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = 10-1;
    TIM_TimeBaseStructure.TIM_Prescaler = 6000-1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_ClearITPendingBit(TIM3,TIM_IT_Update);

    TIM_ITConfig(TIM3,TIM_IT_Update, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM3, ENABLE);	
}

void TimerDelayMs(unsigned int DelayTime)
{
	static unsigned int TickPre = 0;
	TickPre = TimerMsTick;
	while(1){
		if((TimerMsTick - TickPre) >= DelayTime){
			break;
		}
	}
	return;	
}


