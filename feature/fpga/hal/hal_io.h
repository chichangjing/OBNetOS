#ifndef _HAL_IO_H
#define _HAL_IO_H

#include "stm32f2xx_gpio.h"
#include "stm32f2xx_rcc.h"


/* GOIOB PORT */
#define	LEDRUNPIN			GPIO_Pin_14
#define	LEDRUNPORT			GPIOB
#define	LEDRUNPERIPH		RCC_AHB1Periph_GPIOB
#define	LEDRUNCTL(mode)	GPIO_WriteBit(LEDRUNPORT,LEDRUNPIN,mode)

#define	LEDDATAPIN			GPIO_Pin_15
#define	LEDDATAPORT		    GPIOB
#define	LEDDATAPERIPH		RCC_AHB1Periph_GPIOB
#define LEDDATACTL(mode) 	GPIO_WriteBit(LEDDATAPORT,LEDDATAPIN,mode)

/* GOIOC PORT */
#define	FPGAPROMPIN		    GPIO_Pin_7
#define	FPGAPROMPORT		GPIOC
#define	FPGAPROMPERIPH		RCC_AHB1Periph_GPIOC
#define FPGAPROMCRL(mode) 	GPIO_WriteBit(FPGAPROMPORT,FPGAPROMPIN,mode)

#define	FPGADONEPIN		    GPIO_Pin_6
#define	FPGADONEPORT		GPIOC
#define	FPGADONEPERIPH		RCC_AHB1Periph_GPIOC
#define FPGADONECRL(mode) 	GPIO_WriteBit(FPGADONEPORT,FPGADONEPIN,mode)

#define	FPGAINITPIN		    GPIO_Pin_8
#define	FPGAINITPORT		GPIOC
#define	FPGAINITPERIPH		RCC_AHB1Periph_GPIOC
#define FPGAINITCRL(mode) 	GPIO_WriteBit(FPGAINITPORT,FPGAINITPIN,mode)

/* GOIOD PORT */
#define	SOFTRST1PIN		    GPIO_Pin_12
#define	SOFTRST1PORT		GPIOD
#define	SOFTRST1PERIPH		RCC_AHB1Periph_GPIOD
#define SOFTRST1CRL(mode) 	GPIO_WriteBit(SOFTRST1PORT,SOFTRST1PIN,mode)

#define	SOFTRST2PIN		    GPIO_Pin_11
#define	SOFTRST2PORT		GPIOD
#define	SOFTRST2PERIPH		RCC_AHB1Periph_GPIOD
#define SOFTRST2CRL(mode) 	GPIO_WriteBit(SOFTRST2PORT,SOFTRST2PIN,mode)

#define	CLK1SELPIN			GPIO_Pin_13
#define	CLK1SELPORT		    GPIOD
#define	CLK1SELPERIPH		RCC_AHB1Periph_GPIOD
#define	CLK1SELCTL(mode)	GPIO_WriteBit(CLK1SELPORT,CLK1SELPIN,mode)

#define	LEDSYSPIN			GPIO_Pin_14
#define	LEDSYSPORT			GPIOD
#define	LEDSYSPERIPH		RCC_AHB1Periph_GPIOD
#define LEDSYSCTL(mode) 	GPIO_WriteBit(LEDSYSPORT,LEDSYSPIN,mode)

/* GOIOE PORT */
#define	SOFTRSTPIN			GPIO_Pin_7
#define	SOFTRSTPORT		    GPIOE
#define	SOFTRSTPERIPH		RCC_AHB1Periph_GPIOE
#define SOFTRSTCRL(mode) 	GPIO_WriteBit(SOFTRSTPORT,SOFTRSTPIN,mode)

//----------------------FPGA1 config Interface pins----------------------------//
#define	FPGAPROMPIN		    GPIO_Pin_7
#define	FPGAPROMPORT		GPIOC
#define	FPGAPROMPERIPH		RCC_AHB1Periph_GPIOC
#define FPGAPROMCRL(mode) 	GPIO_WriteBit(FPGAPROMPORT,FPGAPROMPIN,mode)

#define	FPGADONEPIN		    GPIO_Pin_6
#define	FPGADONEPORT		GPIOC
#define	FPGADONEPERIPH		RCC_AHB1Periph_GPIOC
#define FPGADONECRL(mode) 	GPIO_WriteBit(FPGADONEPORT,FPGADONEPIN,mode)

#define	FPGAINITPIN		    GPIO_Pin_8
#define	FPGAINITPORT		GPIOC
#define	FPGAINITPERIPH		RCC_AHB1Periph_GPIOC
#define FPGAINITCRL(mode) 	GPIO_WriteBit(FPGAINITPORT,FPGAINITPIN,mode)
//----------------------FPGA1 config Interface pins----------------------------//


#if (defined DEBUG_ONLYSWD)

typedef enum
{
    FULLSWJR = 0,
    FULLSWJNR = 1,
    DJTAGESW = 2,
    DJTAGDSW = 4
} debugportmode;
void DebugPortAFSet(debugportmode mode);

#endif

void GpioConfInit(void);
void IdGet(unsigned char *id);
void BlinkRunLed(unsigned int timeblink);

#endif  /* _HAL_IO_H */
