




#ifndef _CONFIG_H
#define _CONFIG_H


#include "stm32f10x.h"
#include "stm32f10x_conf.h"




#include "i2c_ee.h"
#include "delay.h"



/* func  */

void RCC_Configuration(void);
void Gpio_Configuration(void);
void NVIC_Configuration(void);
void Uartx_Configuration(uartxconfig_t *uartconf);





#endif //end config.h
