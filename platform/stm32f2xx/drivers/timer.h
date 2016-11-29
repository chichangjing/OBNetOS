
#ifndef __TIMER_H
#define __TIMER_H

void TimerInit(void);
void TimerDelayMs(unsigned int DelayTime);
unsigned int TimerGetMsTick(void);
void TimerDisable(void);
void RebootDelayMs(int ms);

#endif

