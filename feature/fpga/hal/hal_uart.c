#include "mconfig.h"
#include "fpga_debug.h"

#include "hal_uart.h"
#include "hal_time.h"
#include "delay.h"


void HalUartIoInit(unsigned int dev)
{
	GPIO_InitTypeDef	gpioinit;
	switch (dev)
	{
#if ((defined HAL_UART1) && HAL_UART1)
		case HAL_UART1:
			RCC_AHB1PeriphClockCmd(UART1RXPERIPH | UART1RXPERIPH, ENABLE);
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
			gpioinit.GPIO_PuPd = GPIO_PuPd_UP;
			gpioinit.GPIO_OType = GPIO_OType_PP;
			gpioinit.GPIO_Mode = GPIO_Mode_AF;
			gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
			gpioinit.GPIO_Pin = UART1TXPIN;
			GPIO_Init(UART1TXPORT, &gpioinit);
			gpioinit.GPIO_Pin = UART1RXPIN;
			GPIO_Init(UART1RXPORT, &gpioinit);
			
			USART_OverSampling8Cmd(USART1, ENABLE);
			break;
#endif
#if ((defined HAL_UART4) && HAL_UART4)
		case HAL_UART4:
			RCC_AHB1PeriphClockCmd(UART4RXPERIPH | UART4RXPERIPH, ENABLE);
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4);
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4);
			gpioinit.GPIO_PuPd = GPIO_PuPd_UP;
			gpioinit.GPIO_OType = GPIO_OType_PP;
			gpioinit.GPIO_Mode = GPIO_Mode_AF;
			gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
			gpioinit.GPIO_Pin = UART4TXPIN;
			GPIO_Init(UART4TXPORT, &gpioinit);
			gpioinit.GPIO_Pin = UART4RXPIN;
			GPIO_Init(UART4RXPORT, &gpioinit);
			break;
#endif
#if ((defined HAL_UART6) && HAL_UART6)
		case HAL_UART6:
			RCC_AHB1PeriphClockCmd(UART6RXPERIPH | UART6RXPERIPH, ENABLE);
			gpioinit.GPIO_PuPd = GPIO_PuPd_UP;
			gpioinit.GPIO_OType = GPIO_OType_PP;
			gpioinit.GPIO_Mode = GPIO_Mode_AF;
			gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
			gpioinit.GPIO_Pin = UART6TXPIN;
			GPIO_Init(UART6TXPORT, &gpioinit);
			gpioinit.GPIO_Pin = UART6RXPIN;
			GPIO_Init(UART6RXPORT, &gpioinit);
			break;
#endif
		default:
			break;
	}
}


void UartxInit(uartxinfop uartcfp, unsigned int interrupt)
{
	USART_TypeDef *uartctl;
	USART_InitTypeDef uartinit;
	NVIC_InitTypeDef uartnvic;
	switch (uartcfp->devid)
	{
#if ((defined HAL_UART1) && HAL_UART1)
		case HAL_UART1:
			uartctl = USART1;
			uartnvic.NVIC_IRQChannel = USART1_IRQn;
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
			break;
#endif
#if ((defined HAL_UART4) && HAL_UART4)
		case HAL_UART4:
			uartctl = UART4;
			uartnvic.NVIC_IRQChannel = UART4_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
			break;
#endif
#if ((defined HAL_UART6) && HAL_UART6)
		case HAL_UART6:
			uartctl = USART6;
			uartnvic.NVIC_IRQChannel = USART6_IRQn;
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
			break;
#endif
	}
	/* Tx, Rx pin */
	HalUartIoInit(uartcfp->devid);

	uartinit.USART_BaudRate = uartcfp->baudrate;
	uartinit.USART_StopBits = uartcfp->stopbit;
	uartinit.USART_WordLength = (uartcfp->parity) ? (uartcfp->bitlen | USART_WordLength_9b) : uartcfp->bitlen;
	uartinit.USART_Parity = uartcfp->parity;
	uartinit.USART_HardwareFlowControl = uartcfp->hdfwctl;
	uartinit.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(uartctl, &uartinit);
	if (interrupt)
	{
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		uartnvic.NVIC_IRQChannelPreemptionPriority = 0;
		uartnvic.NVIC_IRQChannelSubPriority = 0;
		uartnvic.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&uartnvic);
		USART_ITConfig(uartctl, USART_IT_RXNE, ((TXDERXEN & interrupt) ? ENABLE : DISABLE));
		USART_ITConfig(uartctl, USART_IT_TXE, ((TXENRXDE & interrupt) ? ENABLE : DISABLE));
	}
	/* start uartx */
	USART_Cmd(uartctl, ENABLE);
}


unsigned int UartxPollPutc(USART_TypeDef *USARTx, unsigned char *buf, unsigned int size)
{
	unsigned char i;
    
	for (i = 0; i < size; i++)
	{
		USARTx->DR = (*buf & (unsigned short)0x01FF);
		/* data register is empty */
		/* Loop until USARTy DR register is empty */
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
        
        buf++;     
	}
    
    /* Loop until USARTy DR transmission complete */
	while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
    USART_ClearFlag(USARTx, USART_FLAG_TC);
    
	return i;
}

unsigned int UartxPollGetc(USART_TypeDef *USARTx, unsigned char *value)
{
	if (USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) != RESET)
	{
		*value = (unsigned char)USARTx->DR;
		return 1;
	}
	return 0;
}



#if ((defined HAL_UART1) && HAL_UART1)

static srlrxcexe uart1callback = NULL;

void Uart1RxCallBack(void *fun)
{
	uart1callback = (srlrxcexe)fun;
}

unsigned int Uart1PollGetc(unsigned char *buf)
{
	unsigned int ret;
	ret = UartxPollGetc(USART1, buf);
	return ret;
}

unsigned int Uart1Pollputc(unsigned char *buf, unsigned int size)
{
	unsigned int ret;
#ifdef PCDEBUG
	Delay_5Us(10);
#endif
	ret = UartxPollPutc(USART1, buf, size);
    st_fpga_debug(ST_FPGA_DEBUG,"response message length£º%d \r\n" ,size);
    for(unsigned short i=0; i<size; i++)
    {
        st_fpga_debug(ST_FPGA_DEBUG,"%02x ",buf[i]);  
    }  
    st_fpga_debug(ST_FPGA_DEBUG,"\r\n");
#ifdef PCDEBUG
	Delay_5Us(100);
#endif
	return ret;
}


#if BOARD_GV3S_HONUE_QM
void USART1_IRQHandler(void)
{
	unsigned char redata;
	/* check rx interrupt flag */
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		/* clear rx interrupt flag */
		USART_ClearITPendingBit(USART1,   USART_IT_RXNE);
		redata = USART_ReceiveData(USART1);
		if (uart1callback)
		{
			uart1callback(redata);
		}
	}
	else if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
	{
		/* clear rx interrupt flag */
		USART_ClearITPendingBit(USART1,   USART_IT_TC);
	}
}
#endif /* BOARD_GV3S_HONUE_QM */
#endif /* (defined HAL_UART1) && HAL_UART1 */


#if ((defined HAL_UART2) && HAL_UART2)

static srlrxcexe uart2callback = NULL;

void Uart2RxCallBack(void *fun)
{
	uart2callback = (srlrxcexe)fun;
}

unsigned int Uart2PollGetc(unsigned char *buf)
{
	unsigned int ret;
	ret = UartxPollGetc(USART2, buf);
	return ret;
}

unsigned int Uart2Pollputc(unsigned char *buf, unsigned int size)
{
	unsigned int ret;
	RS4852CTL(1);
#ifdef PCDEBUG
	Delay_5Us(10);
#endif
	ret = UartxPollPutc(USART2, buf, size);
#ifdef PCDEBUG
	Delay_5Us(100);
#endif
	RS4852CTL(0);
	return ret;
}


void USART2_IRQHandler(void)
{
	unsigned char redata;
	/* check rx interrupt flag */
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		/* clear rx interrupt flag */
		USART_ClearITPendingBit(USART2,   USART_IT_RXNE);
		redata = USART_ReceiveData(USART2);
		if (uart2callback)
		{
			uart2callback(redata);
		}
	}
	else if (USART_GetITStatus(USART2, USART_IT_TC) != RESET)
	{
		/* clear rx interrupt flag */
		USART_ClearITPendingBit(USART2,   USART_IT_TC);
	}
}

#endif


#if ((defined HAL_UART3) && HAL_UART3)

static srlrxcexe uart3callback = NULL;

void Uart3RxCallBack(void *fun)
{
	uart3callback = (srlrxcexe)fun;
}


unsigned int Uart3PollGetc(unsigned char *buf, unsigned int size)
{
	unsigned int ret;
	ret = UartxPollGetc(USART3, buf, size, UARTTIMEOUT);
	return ret;
}


unsigned int Uart3Pollputc(unsigned char *buf, unsigned int size)
{
	unsigned int ret;
	RS4853CTL(1);
#ifdef PCDEBUG
	Delay_5Us(10);
#endif
	ret = UartxPollPutc(USART3, buf, size);
#ifdef PCDEBUG
	Delay_5Us(100);
#endif
	RS4853CTL(0);
	return ret;
}

#endif



#if ((defined HAL_UART4) && HAL_UART4)

static srlrxcexe uart4callback = NULL;

void Uart4RxCallBack(void *fun)
{
	uart4callback = (srlrxcexe)fun;
}

unsigned int Uart4PollGetc(unsigned char *buf)
{
	unsigned int ret;
	ret = UartxPollGetc(UART4, buf);
	return ret;
}

unsigned int Uart4Pollputc(unsigned char *buf, unsigned int size)
{
	unsigned int ret;
#ifdef PCDEBUG
	Delay_5Us(10);
#endif
	ret = UartxPollPutc(UART4, buf, size);
#ifdef PCDEBUG
	Delay_5Us(100);
#endif
	return ret;
}

void UART4_IRQHandler(void)
{
	unsigned char redata;
	/* check rx interrupt flag */
	if (USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{
		/* clear rx interrupt flag */
		USART_ClearITPendingBit(UART4,   USART_IT_RXNE);
		redata = USART_ReceiveData(UART4);
		if (uart4callback)
		{
			uart4callback(redata);
		}
	}
	else if (USART_GetITStatus(UART4, USART_IT_TC) != RESET)
	{
		/* clear rx interrupt flag */
		USART_ClearITPendingBit(UART4,   USART_IT_TC);
	}
}


#endif







/* set console port */
#define	FpgaConsolePutChar(ch)			Uart2Pollputc((unsigned char *)ch, 1)
#define	FpgaConsoleGetChar(ch)			Uart2PollGetc((unsigned char *)ch)


int ConsoleCheckRxChar(char c)
{
	unsigned char ret;
	FpgaConsoleGetChar(&ret);
	return ((ret == c) ? 1 : 0);
}


#if 0
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif 

PUTCHAR_PROTOTYPE
{
	FpgaConsolePutChar(&ch);
	return ch;
}
#endif






