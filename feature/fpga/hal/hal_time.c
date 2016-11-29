#include "mconfig.h"

#include "stm32f2xx.h"

extern unsigned int systick;

void SysTickGet(unsigned int *time)
{
	*time = systick;
}

void SysTickSet(unsigned int *time)
{
	systick = *time;
}

#if 0
void SysTick_Handler(void)
{
	systick++;
}
#endif

//end hal_time.c
