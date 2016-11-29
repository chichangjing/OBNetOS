
/********************************************************************************
  * @file    console.c
  * @author  OB networks
  * @brief   Console driver
  *******************************************************************************/ 
  
#include "mconfig.h"

/* Standard includes */
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/* BSP includes. */
#include "misc.h"

/* Other includes. */
#include "console.h"

#define CONSOLE_QUEUE_LENGTH 128

tConsoleDev ConsoleDev;
tConsoleDev *pConsoleDev = &ConsoleDev;
xQueueHandle xRxedChars;
int UartRxFlag = 0;

xSemaphoreHandle xSemConsoleRxIdle;

static void USART2_Init(unsigned int BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	xRxedChars = xQueueCreate(CONSOLE_QUEUE_LENGTH, (unsigned portBASE_TYPE)sizeof(signed portCHAR));

	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(COM_PORT1_RX_GPIO_CLK | COM_PORT1_TX_GPIO_CLK, ENABLE);	
	/* Enable USART clock */	
	RCC_APB1PeriphClockCmd(COM_PORT1_CLK, ENABLE);		

	/* Connect the pin to the Alternate Function (AF) */
	GPIO_PinAFConfig(COM_PORT1_TX_GPIO_PORT, COM_PORT1_TX_SOURCE, COM_PORT1_TX_AF);
	GPIO_PinAFConfig(COM_PORT1_RX_GPIO_PORT, COM_PORT1_RX_SOURCE, COM_PORT1_RX_AF);

	/* Configure USART Tx as alternate function  */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = COM_PORT1_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(COM_PORT1_TX_GPIO_PORT, &GPIO_InitStructure);

	/* Configure USART Rx as alternate function  */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = COM_PORT1_RX_PIN;
	GPIO_Init(COM_PORT1_RX_GPIO_PORT, &GPIO_InitStructure);
	
	/* USART configuration, BaudRate, WordLength, StopBit, Parity, FlowControl */
	USART_InitStructure.USART_BaudRate = BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( COM_PORT1, &USART_InitStructure );	
	
	/* Enable the USART interrupt in the NVIC. */
	NVIC_InitStructure.NVIC_IRQChannel = COM_PORT1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	/* Enable the rx interrupt */
	USART_ITConfig(COM_PORT1, USART_IT_RXNE, ENABLE);
	
	USART_Cmd(COM_PORT1, ENABLE);
}

static void USART3_Init(unsigned int BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	xRxedChars = xQueueCreate(CONSOLE_QUEUE_LENGTH, (unsigned portBASE_TYPE)sizeof(signed portCHAR));

	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(COM_PORT2_RX_GPIO_CLK | COM_PORT2_TX_GPIO_CLK, ENABLE);	
	/* Enable USART clock */	
	RCC_APB1PeriphClockCmd(COM_PORT2_CLK, ENABLE);		

	/* Connect the pin to the Alternate Function (AF) */
	GPIO_PinAFConfig(COM_PORT2_TX_GPIO_PORT, COM_PORT2_TX_SOURCE, COM_PORT2_TX_AF);
	GPIO_PinAFConfig(COM_PORT2_RX_GPIO_PORT, COM_PORT2_RX_SOURCE, COM_PORT2_RX_AF);

	/* Configure USART Tx as alternate function  */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = COM_PORT2_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(COM_PORT2_TX_GPIO_PORT, &GPIO_InitStructure);

	/* Configure USART Rx as alternate function  */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = COM_PORT2_RX_PIN;
	GPIO_Init(COM_PORT2_RX_GPIO_PORT, &GPIO_InitStructure);
	
	/* USART configuration, BaudRate, WordLength, StopBit, Parity, FlowControl */
	USART_InitStructure.USART_BaudRate = BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( COM_PORT2, &USART_InitStructure );	
	
	/* Enable the USART interrupt in the NVIC. */
	NVIC_InitStructure.NVIC_IRQChannel = COM_PORT2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	/* Enable the rx interrupt */
	USART_ITConfig(COM_PORT2, USART_IT_RXNE, ENABLE);
	if(pConsoleDev->Mode == RS485) {
		USART_ITConfig(COM_PORT2, USART_IT_IDLE, ENABLE);
	}
	
	USART_Cmd(COM_PORT2, ENABLE);
}

static void UART5_Init(unsigned int BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	xRxedChars = xQueueCreate(CONSOLE_QUEUE_LENGTH, (unsigned portBASE_TYPE)sizeof(signed portCHAR));
		
	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(COM_PORT4_RX_GPIO_CLK | COM_PORT4_TX_GPIO_CLK, ENABLE);	
	/* Enable UART clock */	
	RCC_APB1PeriphClockCmd(COM_PORT4_CLK, ENABLE);		

	/* Connect the pin to the Alternate Function (AF) */
	GPIO_PinAFConfig(COM_PORT4_TX_GPIO_PORT, COM_PORT4_TX_SOURCE, COM_PORT4_TX_AF);
	GPIO_PinAFConfig(COM_PORT4_RX_GPIO_PORT, COM_PORT4_RX_SOURCE, COM_PORT4_RX_AF);

	/* Configure UART Tx as alternate function  */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = COM_PORT4_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(COM_PORT4_TX_GPIO_PORT, &GPIO_InitStructure);

	/* Configure UART Rx as alternate function  */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = COM_PORT4_RX_PIN;
	GPIO_Init(COM_PORT4_RX_GPIO_PORT, &GPIO_InitStructure);

	/* UART configuration, BaudRate, WordLength, StopBit, Parity, FlowControl */
	USART_InitStructure.USART_BaudRate = BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(COM_PORT4, &USART_InitStructure );	

	/* Enable the UART interrupt in the NVIC. */
	NVIC_InitStructure.NVIC_IRQChannel = COM_PORT4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure );

	/* Enable the rx interrupt */
	USART_ITConfig(COM_PORT4, USART_IT_RXNE, ENABLE);
	if(pConsoleDev->Mode == RS485) {
		USART_ITConfig(COM_PORT4, USART_IT_IDLE, ENABLE);
	}
	
	USART_Cmd(COM_PORT4, ENABLE);	
}

static void USART6_Init(unsigned int BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	xRxedChars = xQueueCreate(CONSOLE_QUEUE_LENGTH, (unsigned portBASE_TYPE)sizeof(signed portCHAR));

	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(COM_PORT5_RX_GPIO_CLK | COM_PORT5_TX_GPIO_CLK, ENABLE);	
	/* Enable USART clock */	
	RCC_APB2PeriphClockCmd(COM_PORT5_CLK, ENABLE);		

	/* Connect the pin to the Alternate Function (AF) */
	GPIO_PinAFConfig(COM_PORT5_TX_GPIO_PORT, COM_PORT5_TX_SOURCE, COM_PORT5_TX_AF);
	GPIO_PinAFConfig(COM_PORT5_RX_GPIO_PORT, COM_PORT5_RX_SOURCE, COM_PORT5_RX_AF);

	/* Configure USART Tx as alternate function  */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = COM_PORT5_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(COM_PORT5_TX_GPIO_PORT, &GPIO_InitStructure);

	/* Configure USART Rx as alternate function  */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = COM_PORT5_RX_PIN;
	GPIO_Init(COM_PORT5_RX_GPIO_PORT, &GPIO_InitStructure);
	
	/* USART configuration, BaudRate, WordLength, StopBit, Parity, FlowControl */
	USART_InitStructure.USART_BaudRate = BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(COM_PORT5, &USART_InitStructure );	
	
	/* Enable the USART interrupt in the NVIC. */
	NVIC_InitStructure.NVIC_IRQChannel = COM_PORT5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure );

	/* Enable the rx interrupt */
	USART_ITConfig(COM_PORT5, USART_IT_RXNE, ENABLE);
	
	USART_Cmd(COM_PORT5, ENABLE);
}

static void RS485_GpioInit(int port)
{
	GPIO_InitTypeDef  GPIO_InitStructure; 
#if BOARD_GE22103MA
	extern u8 gHardwareVer87;
    extern u16 gHardwareAddr;
#endif

	switch(port) {  
		case COM_USART1:
		/* RS485-EN, USART1, PA11 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOA, &GPIO_InitStructure); 			
		break;

		case COM_USART3:
#if BOARD_GE22103MA
        if(gHardwareAddr == 0x007F){
            if(gHardwareVer87 != 0x03) {
                /* RS485-EN, USART3, PD10 */
                RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
                GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
                GPIO_Init(GPIOD, &GPIO_InitStructure);
            }
        }else{ 
            /* RS485-EN, USART3, PD10 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
            GPIO_Init(GPIOD, &GPIO_InitStructure);
        }	
#elif BOARD_GV3S_HONUE_QM
        /* RS485-EN, USART3, PD10 */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
        GPIO_Init(GPIOD, &GPIO_InitStructure);
#endif
		break;
		
		case COM_UART5:
#if BOARD_GE22103MA
		/* RS485-EN, UART5, PD1 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOD, &GPIO_InitStructure); 		
#elif BOARD_GE20023MA
		/* RS485-EN, UART5, PD0 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOD, &GPIO_InitStructure); 		
#endif		
		break;

		default:
		break;
	}
	
}

static void RS485_TxEnable(int port)
{
#if BOARD_GE22103MA
	extern u8 gHardwareVer87;
    extern u16 gHardwareAddr;
#endif

	switch(port) {  
		case COM_USART1:
		GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_SET);
		break;

		case COM_USART3:
#if BOARD_GE22103MA
        if(gHardwareAddr == 0x007F){
            if(gHardwareVer87 != 0x03)
                GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_SET);
        }else{
            GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_SET);
        }
#elif BOARD_GV3S_HONUE_QM
    GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_SET);
#endif
		break;
		
		case COM_UART5:
#if BOARD_GE22103MA	
		GPIO_WriteBit(GPIOD, GPIO_Pin_1, Bit_SET);
#elif BOARD_GE20023MA
		GPIO_WriteBit(GPIOD, GPIO_Pin_0, Bit_SET);
#endif
		break;

		default:
		break;
	}
}

static void RS485_RxEnable(int port)
{
#if BOARD_GE22103MA
	extern u8 gHardwareVer87;
    extern u16 gHardwareAddr;
#endif

	switch(port) {  
		case COM_USART1:
		GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);
		break;

		case COM_USART3:
#if BOARD_GE22103MA
        if(gHardwareAddr == 0x007F){
            if(gHardwareVer87 != 0x03)
                GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_RESET);
        }else{
			GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_RESET);
        }
#elif BOARD_GV3S_HONUE_QM
        GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_RESET);
#endif
		break;

		case COM_UART5:
#if BOARD_GE22103MA	
		GPIO_WriteBit(GPIOD, GPIO_Pin_1, Bit_RESET);
#elif BOARD_GE20023MA
		GPIO_WriteBit(GPIOD, GPIO_Pin_0, Bit_RESET);
#endif			
		break;

		default:
		break;
	}
}

void ConsoleInit(int Port, unsigned int BaudRate)
{
#if BOARD_GE22103MA
	extern u8 gHardwareVer87;
    extern u16 gHardwareAddr;
#endif

#if BOARD_GE22103MA
	pConsoleDev->Mode = RS485;
	if(pConsoleDev->Mode == RS485) {
        if(gHardwareAddr == 0x007F){
            if(gHardwareVer87 != 0x03) {
                RS485_GpioInit(COM_USART3);
                RS485_RxEnable(COM_USART3);	
            } else {
                RS485_GpioInit(COM_UART5);
                RS485_RxEnable(COM_UART5);
            }
        }else{  
            RS485_GpioInit(COM_USART3);
            RS485_RxEnable(COM_USART3);	
        }

		UartRxFlag = 1;
		vSemaphoreCreateBinary(xSemConsoleRxIdle);
	}
#elif BOARD_GV3S_HONUE_QM
    pConsoleDev->Mode = RS485;
	if(pConsoleDev->Mode == RS485) {
		RS485_GpioInit(COM_USART3);
		RS485_RxEnable(COM_USART3);	
		UartRxFlag = 1;
		vSemaphoreCreateBinary(xSemConsoleRxIdle);
	}
#elif BOARD_GE20023MA
	if(pConsoleDev->Mode == RS485) {
		RS485_GpioInit(COM_UART5);
		RS485_RxEnable(COM_UART5);	
		UartRxFlag = 1;
		vSemaphoreCreateBinary(xSemConsoleRxIdle);
	}
#else
	pConsoleDev->Mode = RS232;
#endif	
	switch(Port){	
		case COM_USART2:
			USART2_Init(BaudRate); 
			pConsoleDev->UARTx = USART2;
			pConsoleDev->ComPort = COM_USART2;
			break;
		case COM_USART3:
			USART3_Init(BaudRate); 
			pConsoleDev->UARTx = USART3;
			pConsoleDev->ComPort = COM_USART3;
			break;			
		case COM_UART5:	
			UART5_Init(BaudRate); 
			pConsoleDev->UARTx = UART5;
			pConsoleDev->ComPort = COM_UART5;
			break;	
		case COM_USART6:
			USART6_Init(BaudRate); 
			pConsoleDev->UARTx = USART6;
			pConsoleDev->ComPort = COM_USART6;
			break;				
		default:			
			break;
	}
}

void ConsolePutChar(char c)
{
	if(pConsoleDev->Mode == RS485) {
		if(UartRxFlag == 1) {
			if(xSemaphoreTake(xSemConsoleRxIdle, 500) == pdTRUE) {
				USART_Cmd(pConsoleDev->UARTx, DISABLE);	
				RS485_TxEnable(pConsoleDev->ComPort);
				USART_Cmd(pConsoleDev->UARTx, ENABLE);	
				UartRxFlag = 0;
			}
		} else {
			RS485_TxEnable(pConsoleDev->ComPort);
		}
	}

    while(!(pConsoleDev->UARTx->SR & USART_SR_TXE));
	pConsoleDev->UARTx->DR = c;
	while(!(pConsoleDev->UARTx->SR & USART_SR_TC));
	
	if(pConsoleDev->Mode == RS485) {
		RS485_RxEnable(pConsoleDev->ComPort);
	}
}

void ConsoleEchoReady(void)
{
	if(pConsoleDev->Mode == RS485) {
		while(!(pConsoleDev->UARTx->SR & USART_SR_IDLE));	
	}
}

void ConsolePutString(const char * s)
{
	if(pConsoleDev->Mode == RS485) {
		RS485_TxEnable(pConsoleDev->ComPort);
	}
	
	while(*s != '\0') {
		while(!(pConsoleDev->UARTx->SR & USART_SR_TXE));
		pConsoleDev->UARTx->DR = *s++;
		while(!(pConsoleDev->UARTx->SR & USART_SR_TC));
	}

	if(pConsoleDev->Mode == RS485) {
		RS485_RxEnable(pConsoleDev->ComPort);	
	}	
}

char ConsoleGetChar(void)
{
	while(!(pConsoleDev->UARTx->SR & USART_SR_RXNE));
	if(pConsoleDev->Mode == RS485) {
		while(!(pConsoleDev->UARTx->SR & USART_SR_IDLE));	
	}
	return (char)(pConsoleDev->UARTx->DR);
}

void ConsoleIRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	portCHAR cChar;

	if(pConsoleDev->Mode == RS485) {
		if(USART_GetITStatus(pConsoleDev->UARTx, USART_IT_IDLE) == SET) {
			USART_ReceiveData(pConsoleDev->UARTx);
			if(UartRxFlag==1)
				xSemaphoreGiveFromISR(xSemConsoleRxIdle, &xHigherPriorityTaskWoken);
		}
	}
	
	if( USART_GetITStatus(pConsoleDev->UARTx, USART_IT_RXNE) == SET) {
		if(pConsoleDev->Mode == RS485) {
			UartRxFlag=1;
		}		
		cChar = USART_ReceiveData(pConsoleDev->UARTx);
		xQueueSendFromISR(xRxedChars, &cChar, &xHigherPriorityTaskWoken);
	}

	#if 0
	if(USART_GetFlagStatus(pConsoleDev->UARTx, USART_FLAG_ORE) == SET) {
		USART_ReceiveData(pConsoleDev->UARTx);
	}
	#endif
	
	if(xHigherPriorityTaskWoken != pdFALSE) {
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}	
}

#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

/* Retargets the C library printf function. */
PUTCHAR_PROTOTYPE {
	ConsolePutChar((uint8_t)ch);
	return ch;
}

