

#ifndef __LED_DRV_H
#define __LED_DRV_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"

typedef enum {
	BLINK_1HZ		= 0,
    BLINK_2HZ		= 1,
    BLINK_5HZ		= 2,
    BLINK_10HZ		= 3
} eBlinkRate;

void Test1_LED_On(void);
void Test1_LED_Off(void);
void Test2_LED_On(void);
void Test2_LED_Off(void);

void PortA_LED_On(void);
void PortA_LED_Off(void);
void PortA_LED_Blink(void);

void PortB_LED_On(void);
void PortB_LED_Off(void);
void PortB_LED_Blink(void);

void Set_RingLED_BlinkMode(eBlinkRate rate);
void LED_GPIO_config(void);

#if defined(OS_FREERTOS)
#if BOARD_GE220044MD
void RUN_LED_Task(void * arg);
void RING_LED_Task(void * arg);
#else /* NOT BOARD_GE220044MD */
void LED_Task(void *arg);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif


