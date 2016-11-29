
#include "mconfig.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "semphr.h"

/* BSP includes */
#include "misc.h"
#include "timer.h"
#if MODULE_UART_SERVER
#include "usart_if.h"
#endif
#include "obring.h"

unsigned int TimerMsTick = 0;
static unsigned int RebootDelay = 0;
#if MODULE_UART_SERVER
extern unsigned int uart_tim[];
extern unsigned int uart_led_tim[];
extern xSemaphoreHandle xSemUartRx[];
#endif

#if OBRING_DEV
//extern void obring_timer_tick(void);
#endif

static int TrafficInterval = 0;
extern xSemaphoreHandle xSemTraffic;

void TIM3_IRQHandler(void)
{
	int i;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);		
		TimerMsTick++;
		TrafficInterval++;
#if OBRING_DEV
		//obring_timer_tick();
#endif		
		if(TrafficInterval > 5 * 1000) {
			if(xSemTraffic != NULL)
				xSemaphoreGiveFromISR(xSemTraffic, &xHigherPriorityTaskWoken);
			TrafficInterval = 0;
		}
			
		if(RebootDelay > 0) {
			RebootDelay--;
			if(RebootDelay == 0)
				NVIC_SystemReset();
		}

		
#if MODULE_UART_SERVER
		for(i=0; i<COM_PORT_MAX; i++) {
			if(uart_tim[i] > 0) { 
				uart_tim[i]--;
				if(uart_tim[i] == 0) {
					if(xSemUartRx[i] != NULL)
						xSemaphoreGiveFromISR(xSemUartRx[i], &xHigherPriorityTaskWoken );
				}
			}

			if(uart_led_tim[i] > 0) {
				uart_led_tim[i]--;
				if(uart_led_tim[i] == 0) {
					UartLedOff(i);
				}
			}
		}
#endif		
	}
	
	if( xHigherPriorityTaskWoken != pdFALSE ) {
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
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

/***************************************************************************
	= ( (1+TIM_Prescaler)/60M ) * ( 1+TIM_Period )
	= ( (1+6000-1)/60M) * (1+10-1)
	= 1 ms
 ***************************************************************************/

void TimerInit(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
    
    /* 2 bit for pre-emption priority, 2 bits for subpriority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM3, ENABLE);	
}

void TimerDelayMs(unsigned int DelayTime)
{
	static unsigned int TickPre = 0;
	TickPre = TimerMsTick;
	while(1){
		if((TimerMsTick - TickPre) >= DelayTime) {
			break;
		}
	}
	return;	
}

void RebootDelayMs(int ms)
{
	__disable_interrupt();
	RebootDelay = ms;
	__enable_interrupt();	
}



