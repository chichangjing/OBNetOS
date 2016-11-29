



#ifndef _HAL_UART_H
#define _HAL_UART_H


#include "stm32f2xx_usart.h"
#include "stm32f2xx_gpio.h"


#include "stm32f2xx_rcc.h"
#include "stdio.h"
#include "fifo.h"


#ifdef __DEBUG

#define DEBUG(fmt, ...)	printf(fmt, ##__VA_ARGS__)

#else

#define DEBUG(fmt, ...)

#endif




/* device num (base 1) */
#define NONEDEV		0
#define HAL_UART1	1
#define HAL_UART2	NONEDEV
#define HAL_UART3	NONEDEV
#define HAL_UART4	NONEDEV


#define SRLMAXBUFF		256
#define UARTTIMEOUT	0x100000


/* stm32f100xx?¦Ì¨¢D maxdev = 3 */
#if ((defined HAL_UART1) && (HAL_UART1))

#define	UART1TXPIN			GPIO_Pin_9
#define	UART1TXPORT			GPIOA
#define	UART1TXPERIPH		RCC_AHB1Periph_GPIOA

#define	UART1RXPIN			GPIO_Pin_10
#define	UART1RXPORT			GPIOA
#define	UART1RXPERIPH		RCC_AHB1Periph_GPIOA


#define	UT1MAXNUM		160

#endif


#if	((defined HAL_UART6) && (HAL_UART6))

#define  UART6TXPIN			GPIO_Pin_6
#define  UART6TXPORT		GPIOC
#define  UART6TXPERIPH		RCC_AHB1Periph_GPIOC

#define  UART6RXPIN			GPIO_Pin_7
#define  UART6RXPORT		GPIOC
#define  UART6RXPERIPH		RCC_AHB1Periph_GPIOC

#define	UT2MAXNUM		160

#endif

#if	((defined HAL_UART4) && (HAL_UART4))

#define  UART4TXPIN			GPIO_Pin_0
#define  UART4TXPORT		GPIOA
#define  UART4TXPERIPH		RCC_AHB1Periph_GPIOA

#define  UART4RXPIN			GPIO_Pin_1
#define  UART4RXPORT		GPIOA
#define  UART4RXPERIPH		RCC_AHB1Periph_GPIOA

#endif



typedef enum INTFLAG
{
    TXDERXDE = 0x00,
    TXDERXEN = 0x01,
    TXENRXDE = 0x10,
    TXENRXEN = 0x11
} intflag;


typedef struct
{
	unsigned int	devid;
	unsigned int	baudrate;
	unsigned int	bitlen;
	unsigned int	stopbit;
	unsigned int	parity;
	unsigned int	hdfwctl;		//hardwareflowcontrol
} uartxinfo, *uartxinfop;



typedef void (*srlrxcexe)(unsigned char);


typedef struct
{
	fifo_p fp;
	unsigned short int  cnt;
	unsigned char buff[SRLMAXBUFF];
} serialbufftype;







/* variable */




/* func */
void UartxInit(uartxinfop uartcfp, unsigned int interrupt);
unsigned int UartxPollPutc(USART_TypeDef *USARTx, unsigned char *buf, unsigned int size);
unsigned int Uart1Pollputc(unsigned char *buf, unsigned int size);
unsigned int Uart1PollGetc(unsigned char *buf);
unsigned int Uart2Pollputc(unsigned char *buf, unsigned int size);
unsigned int Uart2PollGetc(unsigned char *buf);
unsigned int Uart4Pollputc(unsigned char *buf, unsigned int size);
unsigned int Uart4PollGetc(unsigned char *buf);


void Uart1RxCallBack(void *fun);
void Uart2RxCallBack(void *fun);
void Uart3RxCallBack(void *fun);
void Uart4RxCallBack(void *fun);



int ConsoleCheckRxChar(char c);

/* set console port */
#define	ConsolePutChar(ch)			Uart1Pollputc((unsigned char *)ch, 1)
#define	ConsoleGetChar(ch)			Uart1PollGetc((unsigned char *)ch)






#endif //end uart.h
