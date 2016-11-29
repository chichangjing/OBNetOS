
/*************************************************************
 * Filename     : rs_uart.c
 * Description  : uart driver interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#include "mconfig.h"

#if MODULE_RS485
/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* BSP includes */
#include "rs_uart.h"
#include "misc_drv.h"

/* Other includes */
#include "cli_util.h"
#include "sig_srv.h"

/* Need to modify following configuration for your board */
#if BOARD_GE22103MA
#define VALID_COM_PORT_NUM	2
static u8 ComPort[VALID_COM_PORT_NUM] = {COM_PORT_0, COM_PORT_4};
#elif BOARD_GE2C400U
#define VALID_COM_PORT_NUM	2
static u8 ComPort[VALID_COM_PORT_NUM] = {COM_PORT_1, COM_PORT_2};
#elif BOARD_GE1040PU
#define VALID_COM_PORT_NUM	2
static u8 ComPort[VALID_COM_PORT_NUM] = {COM_PORT_0, COM_PORT_2};
#else
#define VALID_COM_PORT_NUM	1
static u8 ComPort[VALID_COM_PORT_NUM] = {COM_PORT_MAX};
#endif

/* Local */
USART_TypeDef*		UartTable[COM_PORT_MAX] = {USART1,USART2,USART3,UART4,UART5,USART6};
static uart_dev_t	UartDevice[COM_PORT_MAX];
static uart_msg_t	UartRxMsg[VALID_COM_PORT_NUM];
static uart_msg_t	RxMsgBuffer[VALID_COM_PORT_NUM];
static uart_msg_t	UartTxMsg[VALID_COM_PORT_NUM];
static uart_msg_t	TxMsgBuffer[VALID_COM_PORT_NUM];
xSemaphoreHandle	xSemUartTx[COM_PORT_MAX] = { NULL };
xSemaphoreHandle	xSemUartRx[COM_PORT_MAX] = { NULL };

static u8 UartTestStartPort = 0;
static u8 UartTestEnable = 0;
static u8 UartTestResult = 0;
static u8 UartTestData[8] = {0x12,0x34,0x56,0x78,0x9a,0xbc,0xde,0xf0};

xQueueHandle UartRxQueue[COM_PORT_MAX];

static void UartRxProcess(u8 com_port, u8 *buf, u16 len);

static void RS485GpioInit(u8 com_port)
{
	GPIO_InitTypeDef  GPIO_InitStructure;   

	switch(com_port) {  
		case COM_PORT_0:
#if (BOARD_GE22103MA || BOARD_GE1040PU)
		/* RS485-EN, USART1, PA11 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOA, &GPIO_InitStructure); 
#endif
		break;

		case COM_PORT_1:
#if BOARD_GE2C400U
		/* RS485-EN, USART2, PD7 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOD, &GPIO_InitStructure); 
#endif		
		break;
		
		
		case COM_PORT_2:
#if (BOARD_GE2C400U || BOARD_GE1040PU)
		/* RS485-EN, USART3, PB15 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOB, &GPIO_InitStructure); 
#endif		
		break;

		case COM_PORT_4:
#if BOARD_GE22103MA
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

static void RS485TxEnable(u8 com_port)
{
	switch(com_port) {  
		case COM_PORT_0:
#if (BOARD_GE22103MA || BOARD_GE1040PU)
		GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_SET);
#endif
		break;

		case COM_PORT_1:
#if BOARD_GE2C400U				
		GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);
#endif
		break;
		
		case COM_PORT_2:
#if (BOARD_GE2C400U || BOARD_GE1040PU)				
		GPIO_WriteBit(GPIOB, GPIO_Pin_15, Bit_SET);
#endif
		break;

		case COM_PORT_4:
#if BOARD_GE22103MA
		GPIO_WriteBit(GPIOD, GPIO_Pin_0, Bit_SET);
#endif
		break;
		
		default:
		break;
	}
}

static void RS485RxEnable(u8 com_port)
{
	switch(com_port) {  
		case COM_PORT_0:
#if (BOARD_GE22103MA || BOARD_GE1040PU)
		GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);
#endif
		break;
		
		case COM_PORT_1:
#if BOARD_GE2C400U			
		GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_RESET);
#endif
		break;
		
		case COM_PORT_2:
#if (BOARD_GE2C400U || BOARD_GE1040PU)		
		GPIO_WriteBit(GPIOB, GPIO_Pin_15, Bit_RESET);
#endif
		break;

		case COM_PORT_4:
#if BOARD_GE22103MA
		GPIO_WriteBit(GPIOD, GPIO_Pin_0, Bit_RESET);
#endif
		break;
		
		default:
		break;
	}
}

void UartLowInit(u8 com_port, USART_InitTypeDef *pUSART_InitStructure)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	switch(com_port) {
		case COM_PORT_0:
			/* Enable GPIO clock */
			RCC_AHB1PeriphClockCmd(COM_PORT0_RX_GPIO_CLK | COM_PORT0_TX_GPIO_CLK, ENABLE);	
			/* Enable USART clock */	
			RCC_APB2PeriphClockCmd(COM_PORT0_CLK, ENABLE);		
		
			/* Connect the pin to the Alternate Function (AF) */
			GPIO_PinAFConfig(COM_PORT0_TX_GPIO_PORT, COM_PORT0_TX_SOURCE, COM_PORT0_TX_AF);
			GPIO_PinAFConfig(COM_PORT0_RX_GPIO_PORT, COM_PORT0_RX_SOURCE, COM_PORT0_RX_AF);

			/* Configure USART Tx as alternate function  */
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
			GPIO_InitStructure.GPIO_Pin = COM_PORT0_TX_PIN;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(COM_PORT0_TX_GPIO_PORT, &GPIO_InitStructure);

			/* Configure USART Rx as alternate function  */
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
			GPIO_InitStructure.GPIO_Pin = COM_PORT0_RX_PIN;
			GPIO_Init(COM_PORT0_RX_GPIO_PORT, &GPIO_InitStructure);

			/* USARTx configuration, BaudRate, WordLength, StopBit, Parity, FlowControl */
			USART_Init(COM_PORT0, pUSART_InitStructure);

			/* Enable the USART interrupt in the NVIC. */ 
			NVIC_InitStructure.NVIC_IRQChannel = COM_PORT0_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init( &NVIC_InitStructure );

			/* Enable the USART interrupt */
			USART_ITConfig(COM_PORT0, USART_IT_RXNE, ENABLE);
			/* Idle line detection interrupt */
			USART_ITConfig(COM_PORT0, USART_IT_IDLE, ENABLE); 
			
			USART_Cmd(COM_PORT0, ENABLE);

			USART_ClearFlag(COM_PORT0, USART_FLAG_TC);
		break;

		case COM_PORT_1:
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
			USART_Init(COM_PORT1, pUSART_InitStructure);

			/* Enable the USART interrupt in the NVIC. */
			NVIC_InitStructure.NVIC_IRQChannel = COM_PORT1_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init( &NVIC_InitStructure );

			/* Enable the USART interrupt */
			USART_ITConfig(COM_PORT1, USART_IT_RXNE, ENABLE);
			/* Idle line detection interrupt */
			USART_ITConfig(COM_PORT1, USART_IT_IDLE, ENABLE); 
			
			USART_Cmd(COM_PORT1, ENABLE);

			USART_ClearFlag(COM_PORT0, USART_FLAG_TC);
		break;

		case COM_PORT_2:
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
			USART_Init(COM_PORT2, pUSART_InitStructure);
			
			/* Enable the USART interrupt in the NVIC. */
			NVIC_InitStructure.NVIC_IRQChannel = COM_PORT2_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&NVIC_InitStructure );

			/* Enable the rx interrupt */
			USART_ITConfig(COM_PORT2, USART_IT_RXNE, ENABLE);
			/* Idle line detection interrupt */
			USART_ITConfig(COM_PORT2, USART_IT_IDLE, ENABLE); 
			
			USART_Cmd(COM_PORT2, ENABLE);

			USART_ClearFlag(COM_PORT0, USART_FLAG_TC);
		break;

		case COM_PORT_4:
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

		/* USART configuration, BaudRate, WordLength, StopBit, Parity, FlowControl */
		USART_Init(COM_PORT4, pUSART_InitStructure);
					
		/* Enable the UART interrupt in the NVIC. */
		NVIC_InitStructure.NVIC_IRQChannel = COM_PORT4_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		/* Enable the rx interrupt */
		USART_ITConfig(COM_PORT4, USART_IT_RXNE, ENABLE);
		/* Idle line detection interrupt */
		USART_ITConfig(COM_PORT4, USART_IT_IDLE, ENABLE); 
		
		USART_Cmd(COM_PORT4, ENABLE);

		USART_ClearFlag(COM_PORT0, USART_FLAG_TC);
		break;
		
		default:
		break;
	}
}

void Uart0_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	USART_TypeDef *uart = UartDevice[COM_PORT_0].uart;
	uart_fifo_t *pfifo = &UartDevice[COM_PORT_0].fifo;	
	u8 valid_lport = UartDevice[COM_PORT_0].lport;
	u8 c, event = 0;
		
	if(USART_GetITStatus(uart, USART_IT_IDLE) == SET) {
		c = (u8)(uart->DR & 0xFF);
		if(pfifo->rxCnt <= pfifo->rxSize) {
			UartRxMsg[valid_lport].msgLen = pfifo->rxCnt;
			xQueueSendFromISR(UartRxQueue[COM_PORT_0], &UartRxMsg[valid_lport], &xHigherPriorityTaskWoken);
			UartDevice[COM_PORT_0].stat.count_rxidle++;
			pfifo->rxCnt = 0;
		}
	}
	
	if(USART_GetITStatus(uart, USART_IT_RXNE) == SET) {
		c = (u8)(uart->DR & 0xFF);
		if(pfifo->rxCnt < pfifo->rxSize) {
			pfifo->rxBuf[pfifo->rxCnt] = c;
			pfifo->rxCnt += 1;
			if(pfifo->rxCnt >= pfifo->rxSize) {
				UartRxMsg[valid_lport].msgLen = pfifo->rxCnt;
				xQueueSendFromISR(UartRxQueue[COM_PORT_0], &UartRxMsg[valid_lport], &xHigherPriorityTaskWoken);
				UartDevice[COM_PORT_0].stat.count_rxovf++;
				pfifo->rxCnt = 0;
			}
		}
	}

	if(USART_GetITStatus(uart, USART_IT_TC) == SET) {
		USART_ITConfig(uart, USART_IT_TC, DISABLE);
		RS485RxEnable(COM_PORT_0);
	}
	
	if(USART_GetITStatus(uart, USART_IT_TXE) == SET) {
		if(pfifo->txCnt) {
			uart->CR1 &= (u16)~0x0080;
			uart->DR = pfifo->txBuf[pfifo->txPop];
			uart->CR1 |= (u16)0x0080;
			pfifo->txPop++;
			pfifo->txCnt--;
			if(pfifo->txCnt == 0) {
				pfifo->txPop = 0;
				USART_ITConfig(uart, USART_IT_TXE, DISABLE);
				USART_ITConfig(uart, USART_IT_TC, ENABLE);
				event |=  UART_TX_DONE;
				UartDevice[COM_PORT_0].stat.count_txdone++;
			}
		}
	}

	if(USART_GetFlagStatus(UartTable[COM_PORT_0], USART_FLAG_ORE) == SET) {
		USART_ClearFlag(UartTable[COM_PORT_0],USART_FLAG_ORE);
		USART_ReceiveData(UartTable[COM_PORT_0]);	
		UartDevice[COM_PORT_0].stat.count_overrun++;
	}

	if(xHigherPriorityTaskWoken != pdFALSE) {
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
}

void Uart1_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	USART_TypeDef *uart = UartDevice[COM_PORT_1].uart;
	uart_fifo_t *pfifo = &UartDevice[COM_PORT_1].fifo;	
	u8 valid_lport = UartDevice[COM_PORT_1].lport;
	u8 c, event = 0;
		
	if(USART_GetITStatus(uart, USART_IT_IDLE) == SET) {
		c = (u8)(uart->DR & 0xFF);
		if(pfifo->rxCnt <= pfifo->rxSize) {
			UartRxMsg[valid_lport].msgLen = pfifo->rxCnt;
			xQueueSendFromISR(UartRxQueue[COM_PORT_1], &UartRxMsg[valid_lport], &xHigherPriorityTaskWoken);
			UartDevice[COM_PORT_1].stat.count_rxidle++;
			pfifo->rxCnt = 0;
		}
	}
	
	if(USART_GetITStatus(uart, USART_IT_RXNE) == SET) {
		c = (u8)(uart->DR & 0xFF);
		if(pfifo->rxCnt < pfifo->rxSize) {
			pfifo->rxBuf[pfifo->rxCnt] = c;
			pfifo->rxCnt += 1;
			if(pfifo->rxCnt >= pfifo->rxSize) {
				UartRxMsg[valid_lport].msgLen = pfifo->rxCnt;
				xQueueSendFromISR(UartRxQueue[COM_PORT_1], &UartRxMsg[valid_lport], &xHigherPriorityTaskWoken);
				UartDevice[COM_PORT_1].stat.count_rxovf++;
				pfifo->rxCnt = 0;
			}
		}
	}

	if(USART_GetITStatus(uart, USART_IT_TC) == SET) {
		USART_ITConfig(uart, USART_IT_TC, DISABLE);
		RS485RxEnable(COM_PORT_1);
	}
	
	if(USART_GetITStatus(uart, USART_IT_TXE) == SET) {
		if(pfifo->txCnt) {
			uart->CR1 &= (u16)~0x0080;
			uart->DR = pfifo->txBuf[pfifo->txPop];
			uart->CR1 |= (u16)0x0080;
			pfifo->txPop++;
			pfifo->txCnt--;
			if(pfifo->txCnt == 0) {
				pfifo->txPop = 0;
				USART_ITConfig(uart, USART_IT_TXE, DISABLE);
				USART_ITConfig(uart, USART_IT_TC, ENABLE);
				event |=  UART_TX_DONE;
				UartDevice[COM_PORT_1].stat.count_txdone++;
			}
		}
	}

	if(USART_GetFlagStatus(UartTable[COM_PORT_1], USART_FLAG_ORE) == SET) {
		USART_ClearFlag(UartTable[COM_PORT_1],USART_FLAG_ORE);
		USART_ReceiveData(UartTable[COM_PORT_1]);	
		UartDevice[COM_PORT_1].stat.count_overrun++;
	}

	if(xHigherPriorityTaskWoken != pdFALSE) {
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
}

void Uart2_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	USART_TypeDef *uart = UartDevice[COM_PORT_2].uart;
	uart_fifo_t *pfifo = &UartDevice[COM_PORT_2].fifo;	
	u8 valid_lport = UartDevice[COM_PORT_2].lport;
	u8 c, event = 0;
		
	if(USART_GetITStatus(uart, USART_IT_IDLE) == SET) {
		c = (u8)(uart->DR & 0xFF);
		if(pfifo->rxCnt <= pfifo->rxSize) {
			UartRxMsg[valid_lport].msgLen = pfifo->rxCnt;
			xQueueSendFromISR(UartRxQueue[COM_PORT_2], &UartRxMsg[valid_lport], &xHigherPriorityTaskWoken);
			UartDevice[COM_PORT_2].stat.count_rxidle++;
			pfifo->rxCnt = 0;
		}
	}
	
	if(USART_GetITStatus(uart, USART_IT_RXNE) == SET) {
		c = (u8)(uart->DR & 0xFF);
		if(pfifo->rxCnt < pfifo->rxSize) {
			pfifo->rxBuf[pfifo->rxCnt] = c;
			pfifo->rxCnt += 1;
			if(pfifo->rxCnt >= pfifo->rxSize) {
				UartRxMsg[valid_lport].msgLen = pfifo->rxCnt;
				xQueueSendFromISR(UartRxQueue[COM_PORT_2], &UartRxMsg[valid_lport], &xHigherPriorityTaskWoken);
				UartDevice[COM_PORT_2].stat.count_rxovf++;
				pfifo->rxCnt = 0;
			}
		}
	}

	if(USART_GetITStatus(uart, USART_IT_TC) == SET) {
		USART_ITConfig(uart, USART_IT_TC, DISABLE);
		RS485RxEnable(COM_PORT_2);
	}
	
	if(USART_GetITStatus(uart, USART_IT_TXE) == SET) {
		if(pfifo->txCnt) {
			uart->CR1 &= (u16)~0x0080;
			uart->DR = pfifo->txBuf[pfifo->txPop];
			uart->CR1 |= (u16)0x0080;
			pfifo->txPop++;
			pfifo->txCnt--;
			if(pfifo->txCnt == 0) {
				pfifo->txPop = 0;
				USART_ITConfig(uart, USART_IT_TXE, DISABLE);
				USART_ITConfig(uart, USART_IT_TC, ENABLE);
				event |=  UART_TX_DONE;
				UartDevice[COM_PORT_2].stat.count_txdone++;
			}
		}
	}

	if(USART_GetFlagStatus(UartTable[COM_PORT_2], USART_FLAG_ORE) == SET) {
		USART_ClearFlag(UartTable[COM_PORT_2],USART_FLAG_ORE);
		USART_ReceiveData(UartTable[COM_PORT_2]);	
		UartDevice[COM_PORT_2].stat.count_overrun++;
	}

	if(xHigherPriorityTaskWoken != pdFALSE) {
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
}

void Uart4_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	USART_TypeDef *uart = UartDevice[COM_PORT_4].uart;
	uart_fifo_t *pfifo = &UartDevice[COM_PORT_4].fifo;	
	u8 valid_lport = UartDevice[COM_PORT_4].lport;
	u8 c, event = 0;
		
	if(USART_GetITStatus(uart, USART_IT_IDLE) == SET) {
		c = (u8)(uart->DR & 0xFF);
		if(pfifo->rxCnt <= pfifo->rxSize) {
			UartRxMsg[valid_lport].msgLen = pfifo->rxCnt;
			xQueueSendFromISR(UartRxQueue[COM_PORT_4], &UartRxMsg[valid_lport], &xHigherPriorityTaskWoken);
			UartDevice[COM_PORT_4].stat.count_rxidle++;
			pfifo->rxCnt = 0;
		}
	}

	if(USART_GetITStatus(uart, USART_IT_RXNE) == SET) {
		c = (u8)(uart->DR & 0xFF);
		if(pfifo->rxCnt < pfifo->rxSize) {
			pfifo->rxBuf[pfifo->rxCnt] = c;
			pfifo->rxCnt += 1;
			if(pfifo->rxCnt >= pfifo->rxSize) {
				UartRxMsg[valid_lport].msgLen = pfifo->rxCnt;
				xQueueSendFromISR(UartRxQueue[COM_PORT_4], &UartRxMsg[valid_lport], &xHigherPriorityTaskWoken);
				UartDevice[COM_PORT_4].stat.count_rxovf++;
				pfifo->rxCnt = 0;
			}
		}
	}

	if(USART_GetITStatus(uart, USART_IT_TC) == SET) {
		USART_ITConfig(uart, USART_IT_TC, DISABLE);
		RS485RxEnable(COM_PORT_4);
	}
	
	if(USART_GetITStatus(uart, USART_IT_TXE) == SET) {
		if(pfifo->txCnt) {
			uart->CR1 &= (u16)~0x0080;
			uart->DR = pfifo->txBuf[pfifo->txPop];
			uart->CR1 |= (u16)0x0080;
			pfifo->txPop++;
			pfifo->txCnt--;
			if(pfifo->txCnt == 0) {
				pfifo->txPop = 0;
				USART_ITConfig(uart, USART_IT_TXE, DISABLE);
				USART_ITConfig(uart, USART_IT_TC, ENABLE);
				event |=  UART_TX_DONE;
				UartDevice[COM_PORT_4].stat.count_txdone++;
			}
		}
	}

	if(USART_GetFlagStatus(UartTable[COM_PORT_4], USART_FLAG_ORE) == SET) {
		USART_ClearFlag(UartTable[COM_PORT_4],USART_FLAG_ORE);
		USART_ReceiveData(UartTable[COM_PORT_4]);	
		UartDevice[COM_PORT_4].stat.count_overrun++;
	}

	if(xHigherPriorityTaskWoken != pdFALSE) {
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
}

void UartIRQHandler(u8 com_port)
{
	//UartIRQ(com_port, &UartDevice[com_port].fifo, UartDevice[com_port].uart);
}

u16 UartRead(u8 com_port, u8 *data, u16 len)
{
	uart_fifo_t	*pfifo = &UartDevice[com_port].fifo;
	USART_TypeDef *uart = UartDevice[com_port].uart;
	u16 n;
	
	if(com_port >= COM_PORT_MAX) {
		return 0;
	} else {
		if(len == 0)
			return 0;

		for(n=0;n<len;n++) {
			if(n < pfifo->rxCnt) {
				*data++ = pfifo->rxBuf[pfifo->rxPop];
				pfifo->rxPop += 1;
				if(pfifo->rxPop >= pfifo->rxSize)
					pfifo->rxPop = 0;
			} else {
				break;
			}
		}
		portDISABLE_INTERRUPTS();
		pfifo->txCnt += n;
		portENABLE_INTERRUPTS();
		
		return n;
	}
}

u16 UartWrite(u8 com_port, u8 *data, u16 len)
{
	uart_fifo_t	*pfifo = &UartDevice[com_port].fifo;
	USART_TypeDef *uart = UartDevice[com_port].uart;
	u16 n, txLen, leftLen, dataOffset;
	
	if(com_port >= COM_PORT_MAX) {
		return 0;
	} else {
		if(len == 0)
			return 0;
			
		leftLen = len;
		dataOffset = 0;
		do {
			if(leftLen > UART_MSG_BUF_SIZE)
				txLen = UART_MSG_BUF_SIZE;
			else
				txLen = leftLen;

			for(n=0;n<txLen;n++) {
				pfifo->txBuf[n] = data[dataOffset];
				dataOffset++;
			}
			leftLen -= txLen;
			UartDevice[com_port].stat.tx_bytes += txLen;	
			
			portDISABLE_INTERRUPTS();
			pfifo->txCnt = txLen;
			portENABLE_INTERRUPTS();

			USART_ITConfig(uart, USART_IT_TXE, ENABLE);
		} while(leftLen > 0);
	}

	return len;
}

void UartRxTask(void *arg)
{
	u8 com_port = *(u8 *)arg;
	u8 valid_lport = UartDevice[com_port].lport;
	u32 count = 0;
	char com_port_str[10];
	
	for(;;) {
		if(UartRxQueue[com_port] != NULL) {
			while(xQueueReceive(UartRxQueue[com_port], &RxMsgBuffer[valid_lport], portMAX_DELAY) == pdTRUE) {

				count += RxMsgBuffer[valid_lport].msgLen;
				
				//cli_debug(DBG_NMS, "\r\nUart-%d rx %d bytes:\r\n", com_port, RxMsgBuffer[valid_lport].msgLen);
				//cli_dump(DBG_NMS, RxMsgBuffer[valid_lport].msgBuf, RxMsgBuffer[valid_lport].msgLen);

				
				//RS485TxEnable(com_port);
				//UartWrite(com_port, RxMsgBuffer[valid_lport].msgBuf, RxMsgBuffer[valid_lport].msgLen);	
				
				if(UartTestEnable == 1) {
					if(memcmp(RxMsgBuffer[valid_lport].msgBuf, UartTestData, 8) == 0) {
						if(com_port == UartTestStartPort) {
							UartTestResult = 1;
							UartTestEnable = 0;
						} else {
							RS485TxEnable(com_port);
							UartWrite(com_port, RxMsgBuffer[valid_lport].msgBuf, RxMsgBuffer[valid_lport].msgLen);	
						}
					} else {
						UartTestResult = 0;
						UartTestEnable = 0;
					}
				} else {
					UartRxProcess(com_port, RxMsgBuffer[valid_lport].msgBuf, RxMsgBuffer[valid_lport].msgLen);
				}
			}
		} else {
			vTaskDelay(100);
		}
	}
}

void UartInit(u8 com_port, uart_property_t *uart_cfg, uart_type_e uart_type)
{
	USART_InitTypeDef USART_InitStructure;
	
	USART_InitStructure.USART_BaudRate = uart_cfg->baudrate;
	USART_InitStructure.USART_WordLength = uart_cfg->databits;
	USART_InitStructure.USART_StopBits = uart_cfg->stopbits;
	USART_InitStructure.USART_Parity = uart_cfg->parity;
	USART_InitStructure.USART_HardwareFlowControl = uart_cfg->flowctrl;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	UartLowInit(com_port, &USART_InitStructure);

	if(uart_type == UART_RS485) {
		RS485GpioInit(com_port);
		RS485RxEnable(com_port);
	}
}

void UartTest(void *pCliEnv)
{
#if BOARD_GE22103MA
	extern u8 gHardwareVer87;
#endif
    extern void TimerDelayMs(unsigned int DelayTime);
	
#if BOARD_GE22103MA
	UartTestStartPort = COM_PORT_0;
#elif BOARD_GE2C400U
	UartTestStartPort = COM_PORT_1;
#elif BOARD_GE1040PU
	UartTestStartPort = COM_PORT_0;
#endif

	cli_printf(pCliEnv, "RS485 loopback test (A->B->A) ... ");
#if BOARD_GE22103MA
	if(gHardwareVer87 == 0x03) {
		cli_printf(pCliEnv, "Not support!\r\n");
		return;
	}
#endif

	UartTestResult = 0;
	UartTestEnable = 1;
	RS485TxEnable(UartTestStartPort);
	UartWrite(UartTestStartPort, UartTestData, 8);
	TimerDelayMs(100);
	if(UartTestResult == 1)
		cli_printf(pCliEnv, "Passed\r\n");
	else
		cli_printf(pCliEnv, "Failed\r\n");

	UartTestResult = 0;
	UartTestEnable = 0;

	return;
}

void UartStart(void)
{
	u8 i, com_port;
	uart_property_t uart_cfg;
	char TaskName[16];
#if BOARD_GE22103MA
	extern u8 gHardwareVer87;
	extern dev_base_info_t	DeviceBaseInfo;
#endif

#if BOARD_GE22103MA
	/* PCB version v1.10 */
	if(gHardwareVer87 == 0x03)
		return;
	
	/*  1SFP(1+7)RJ45+Data or 2SFP(1+7)RJ45+Data */
	if((DeviceBaseInfo.HardwareVer[1] != 4) && (DeviceBaseInfo.HardwareVer[1] != 9))
		return;
#endif
#if MODULE_SIGNAL
    SignalLockInit();
#endif

	for(i=0; i<VALID_COM_PORT_NUM; i++) {
		com_port = ComPort[i];
		if(com_port >= COM_PORT_MAX)
			continue;

		if(xSemUartTx[com_port] == NULL) {
			vSemaphoreCreateBinary(xSemUartTx[com_port]);
			xSemaphoreGive(xSemUartTx[com_port]);
		}
			
		/* Initialize fifo and statistic info */
		UartDevice[com_port].lport			= i;
		UartDevice[com_port].hport			= com_port;
		UartDevice[com_port].uart 			= UartTable[com_port];
		UartDevice[com_port].fifo.rxCnt		= UartDevice[com_port].fifo.txCnt  = 0;
		UartDevice[com_port].fifo.rxPush	= UartDevice[com_port].fifo.txPop  = 0;
		UartDevice[com_port].fifo.txPush	= UartDevice[com_port].fifo.txPop  = 0;
		UartDevice[com_port].fifo.rxSize	= UART_MSG_BUF_SIZE;
		UartDevice[com_port].fifo.rxBuf		= UartRxMsg[i].msgBuf;
		UartDevice[com_port].fifo.txSize	= UART_MSG_BUF_SIZE;
		UartDevice[com_port].fifo.txBuf		= UartTxMsg[i].msgBuf;

		UartDevice[com_port].stat.tx_bytes 			= 0x00;
		UartDevice[com_port].stat.rx_bytes			= 0x00;
		UartDevice[com_port].stat.prv_isr_stat 		= UART_ISR_STAT_IDLE;
		UartDevice[com_port].stat.count_rxidle		= 0x00;
		UartDevice[com_port].stat.count_rxovf		= 0x00;
		UartDevice[com_port].stat.count_rxfull		= 0x00;
		UartDevice[com_port].stat.count_txdone		= 0x00;
		UartDevice[com_port].stat.count_overrun		= 0x00;
		UartDevice[com_port].stat.count_tim_ethtx 	= 0x00;
		UartDevice[com_port].stat.count_ovf_ethtx 	= 0x00;

		UartRxQueue[com_port] = xQueueCreate(5, (unsigned portBASE_TYPE)sizeof(uart_msg_t));

		/* Start task for uart tx/rx */
		sprintf(TaskName,"tUartRx-%d", com_port); 
		xTaskCreate(UartRxTask, (signed char const *)TaskName, configMINIMAL_STACK_SIZE * 2, &ComPort[i], tskIDLE_PRIORITY + 8, NULL);
			
		/* Initialize uart of stm32f2xx */
		uart_cfg.baudrate	= 115200;
		uart_cfg.databits	= USART_WordLength_9b;	/* 8bit Data + 1bit Parity */
		uart_cfg.stopbits	= USART_StopBits_2;
		uart_cfg.parity 	= USART_Parity_Odd;
		uart_cfg.flowctrl	= USART_HardwareFlowControl_None;
		UartInit(com_port, &uart_cfg, UART_RS485);
		printf("\r\nUart-%d rx task is started", com_port);
	}
	printf("\r\n");
}


#if RURAL_CREDIT_PROJECT

u8 HonuPortStatus[2] = {0x40, 0x40};
u8 HonuKinStatus=0;
u8 HonuMasterFlag=0;

static void  CheckAdd(u8 *pBuf, u16 usCheckNum)
{
    u8 ucCheckByte = 0;
    u16 i = 0;

    ucCheckByte = *(pBuf + 1);
    for (i = 2; i <= usCheckNum; i++) {
        ucCheckByte = ucCheckByte ^ (*(pBuf + i));
    }
    if (!(usCheckNum % 2))
        ucCheckByte = ~ucCheckByte;
    ucCheckByte = (ucCheckByte & 0x7f);
    *(pBuf + usCheckNum + 1) = ucCheckByte;
    *(pBuf + usCheckNum + 2) = 0xFB;
}


static u8 OddByteCheck(u8 *pBuf, u16 usCheckNum)
{
    u8 ucCheckByte = 0;
    u16 i = 0;

    ucCheckByte = *(pBuf + 1);
    for (i = 2; i <= usCheckNum; i++) {
        ucCheckByte = ucCheckByte ^ (*(pBuf + i));
    }

    if (!(usCheckNum % 2))
        ucCheckByte = ~ucCheckByte;

    ucCheckByte = (ucCheckByte & 0x7f);

    return ucCheckByte;
}

static u8 GetShelfSlotID(void)
{

	u8 ShelfSlotID = 0;
	
#if BOARD_GE2C400U
	ShelfSlotID =  (u8)(((GPIOD->IDR & GPIO_Pin_0) >> 0) << 7);		/* SW1 */
	ShelfSlotID |= (u8)(((GPIOD->IDR & GPIO_Pin_1) >> 1) << 6);		/* SW2 */
	ShelfSlotID |= (u8)(((GPIOD->IDR & GPIO_Pin_2) >> 2) << 5);		/* SW3 */
	ShelfSlotID |= (u8)(((GPIOD->IDR & GPIO_Pin_3) >> 3) << 4);		/* SW4 */
	ShelfSlotID |= (u8)(((GPIOE->IDR & GPIO_Pin_7) >> 7) << 3);		/* SW5 */
	ShelfSlotID |= (u8)(((GPIOE->IDR & GPIO_Pin_8) >> 8) << 2);		/* SW6 */
	ShelfSlotID |= (u8)(((GPIOE->IDR & GPIO_Pin_9) >> 9) << 1);		/* SW7 */
	ShelfSlotID |= (u8)(((GPIOE->IDR & GPIO_Pin_10) >> 10) << 0);	/* SW8 */
#endif
	return ShelfSlotID;
}

static void UartRxProcess(u8 com_port, u8 *buf, u16 len)
{
#if BOARD_GE2C400U
	u8 OddByte;
	RsHeader_t	RsHeader;
	RsSwitchStatus_t RsSwitchStatus;
	char SwitchName[] = "GE2C400U";
	extern dev_base_info_t	DeviceBaseInfo;
	extern u8 DevMac[];
	u8 valid_lport = UartDevice[com_port].lport;
	u8 offset, ShelfSlotID, ShelfID, SlotID;

	if((buf[0] == 0xFA) && (buf[len-1] == 0xFB)) {
		OddByte = OddByteCheck(buf, len-3);
		ShelfSlotID = GetShelfSlotID();
		ShelfID = ((GetShelfSlotID() & 0xE0) >> 5) | 0x80;
		SlotID = (GetShelfSlotID() & 0x1F) | 0x80;
		if((RxMsgBuffer[valid_lport].msgLen >= 18) && (OddByte == buf[len-2]) && (ShelfID == buf[1]) && (SlotID == buf[2])) {
			
			cli_debug(DBG_UART, "\r\nUart-%d rx %d bytes:\r\n", com_port, RxMsgBuffer[valid_lport].msgLen);
			cli_dump(DBG_UART, RxMsgBuffer[valid_lport].msgBuf, RxMsgBuffer[valid_lport].msgLen);
			
			switch(buf[15]) {
				case 0x07:
				RsSwitchStatus.StartFlag = 0x80;
				memcpy(RsSwitchStatus.SwitchType, DeviceBaseInfo.BoardType, 8);
				memcpy(RsSwitchStatus.SwitchMAC, DevMac, 6);
				RsSwitchStatus.SwitchNameLen = strlen(SwitchName);
				
				TxMsgBuffer[valid_lport].msgLen = sizeof(RsHeader_t) + sizeof(RsSwitchStatus_t) + strlen(SwitchName) + 2;
				RsHeader.StartFlag 		= 0xFA;
				RsHeader.ShelfID 		= ShelfID;
				RsHeader.SlotID 		= SlotID;
				RsHeader.Res1 			= 0x00;
				RsHeader.LenH 			= 0x00;
				RsHeader.LenL 			= TxMsgBuffer[valid_lport].msgLen - 6;
				RsHeader.CmdType 		= 0x02;
				RsHeader.CmdDir 		= 0x00;
				RsHeader.PeerID[0] 		= 0x00;
				RsHeader.PeerID[1] 		= 0x00;
				RsHeader.PeerID[2] 		= 0x00;
				RsHeader.PeerID[3] 		= 0x00;
				RsHeader.SndLportID 	= 0x00;
				RsHeader.SndHportID 	= 0x00;
				RsHeader.ChannelID 		= 0x00;
				RsHeader.CmdID 			= 0x07;

				memcpy(&(TxMsgBuffer[valid_lport].msgBuf[0]), (u8 *)&RsHeader, sizeof(RsHeader_t));
				offset = sizeof(RsHeader_t);
				memcpy(&(TxMsgBuffer[valid_lport].msgBuf[offset]), (u8 *)&RsSwitchStatus, sizeof(RsSwitchStatus_t));
				offset += sizeof(RsSwitchStatus_t);
				memcpy(&(TxMsgBuffer[valid_lport].msgBuf[offset]), SwitchName, strlen(SwitchName));
				offset +=  strlen(SwitchName);
				OddByte = OddByteCheck(TxMsgBuffer[valid_lport].msgBuf, TxMsgBuffer[valid_lport].msgLen-3);
				TxMsgBuffer[valid_lport].msgBuf[offset] = OddByte;
				offset += 1;
				TxMsgBuffer[valid_lport].msgBuf[offset] = 0xFB;
				
				cli_debug(DBG_UART, "\r\nUart-%d tx %d bytes:\r\n", com_port, TxMsgBuffer[valid_lport].msgLen);
				cli_dump(DBG_UART, TxMsgBuffer[valid_lport].msgBuf, TxMsgBuffer[valid_lport].msgLen);
				
				RS485TxEnable(com_port);
				UartWrite(com_port, TxMsgBuffer[valid_lport].msgBuf, TxMsgBuffer[valid_lport].msgLen);
				break;

				default:
				break;
			}
		}
	}

#elif BOARD_GE1040PU
	u8 OddByte;
	RsHeader_t	RsHeader;
	RsMacStatus_t RsMacStatus;
	RsSfpSdiRsp_t RsSfpSdiRsp;
	extern u8 DevMac[];
	u8 valid_lport = UartDevice[com_port].lport;
	u8 offset, ShelfSlotID, ShelfID, SlotID;
	u8 SfpSdiByte, KsigByte;
	
	/* if box_cs = 1'b1, return */
	if(GPIOE->IDR & GPIO_Pin_6)
		return;
		
	if((buf[0] == 0xFA) && (buf[len-1] == 0xFB)) {
		OddByte = OddByteCheck(buf, len-3);
		ShelfSlotID = GetShelfSlotID();
		ShelfID = ((GetShelfSlotID() & 0xE0) >> 5) | 0x80;
		SlotID = (GetShelfSlotID() & 0x1F) | 0x80;
		if(OddByte == buf[len-2]) {
			
			cli_debug(DBG_UART, "\r\nUart-%d rx %d bytes:\r\n", com_port, RxMsgBuffer[valid_lport].msgLen);
			cli_dump(DBG_UART, RxMsgBuffer[valid_lport].msgBuf, RxMsgBuffer[valid_lport].msgLen);
			
			switch(buf[15]) {
				case 0x0C:
				RsMacStatus.StartFlag = 0xEF;
				RsMacStatus.MasterSlave = buf[17];
				memcpy(RsMacStatus.SwitchMAC, DevMac, 6);
				TxMsgBuffer[valid_lport].msgLen = sizeof(RsHeader_t) + sizeof(RsMacStatus_t) + 2;
				RsHeader.StartFlag 		= 0xFA;
				RsHeader.ShelfID 		= ShelfID;
				RsHeader.SlotID 		= SlotID;
				RsHeader.Res1 			= 0x00;
				RsHeader.LenH 			= 0x00;
				RsHeader.LenL 			= TxMsgBuffer[valid_lport].msgLen - 6;
				RsHeader.CmdType 		= 0x02;
				RsHeader.CmdDir 		= 0x00;
				RsHeader.PeerID[0] 		= 0x00;
				RsHeader.PeerID[1] 		= 0x00;
				RsHeader.PeerID[2] 		= 0x00;
				RsHeader.PeerID[3] 		= 0x00;
				RsHeader.SndLportID 	= 0x00;
				RsHeader.SndHportID 	= 0x00;
				RsHeader.ChannelID 		= 0x00;
				RsHeader.CmdID 			= 0x0C;

				memcpy(&(TxMsgBuffer[valid_lport].msgBuf[0]), (u8 *)&RsHeader, sizeof(RsHeader_t));
				offset = sizeof(RsHeader_t);
				memcpy(&(TxMsgBuffer[valid_lport].msgBuf[offset]), (u8 *)&RsMacStatus, sizeof(RsMacStatus_t));
				offset += sizeof(RsMacStatus_t);
				OddByte = OddByteCheck(TxMsgBuffer[valid_lport].msgBuf, TxMsgBuffer[valid_lport].msgLen-3);
				TxMsgBuffer[valid_lport].msgBuf[offset] = OddByte;
				offset += 1;
				TxMsgBuffer[valid_lport].msgBuf[offset] = 0xFB;

				if((RsMacStatus.MasterSlave & 0xC0) == 0x80)
					HonuMasterFlag = 1;
				else
					HonuMasterFlag = 0;
				
				cli_debug(DBG_UART, "\r\nUart-%d tx %d bytes:\r\n", com_port, TxMsgBuffer[valid_lport].msgLen);
				cli_dump(DBG_UART, TxMsgBuffer[valid_lport].msgBuf, TxMsgBuffer[valid_lport].msgLen);
				
				RS485TxEnable(com_port);
				UartWrite(com_port, TxMsgBuffer[valid_lport].msgBuf, TxMsgBuffer[valid_lport].msgLen);
				break;

				case 0x07:
				offset = sizeof(RsHeader_t);
				offset += 1+8;	/* 1Byte (0x80) + 8Byte (SwitchType) */
				offset += buf[offset] + 1;
				offset += 6;
				SfpSdiByte = buf[offset];
				KsigByte = buf[offset+1];

				/* Get SDI-IN Status */
				if(SfpSdiByte & 0x20)
					HonuPortStatus[0] = 0xC0;
				else
					HonuPortStatus[0] = 0x40;
				
				/* Get OPT Status */
				if(SfpSdiByte & 0x80)
					HonuPortStatus[1] = 0xC0;
				else
					HonuPortStatus[1] = 0x40;
                
				if(KsigByte & 0x80)
					HonuKinStatus = 0x01;
				else
					HonuKinStatus = 0x00;
#if MODULE_SIGNAL
                SignalMsgSend(HonuKinStatus);
#endif
					
				cli_debug(DBG_UART, "\r\nSFP-01 status : %s", (SfpSdiByte & 0x80)? "Up":"Down");
				cli_debug(DBG_UART, "\r\nSFP-02 status : %s", (SfpSdiByte & 0x40)? "Up":"Down");
				cli_debug(DBG_UART, "\r\nSDI-IN status : %s", (SfpSdiByte & 0x20)? "Up":"Down");
				cli_debug(DBG_UART, "\r\nKsigIN status : %s\r\n", (KsigByte & 0x80)? "1":"0");
				
				RsSfpSdiRsp.StartFlag = 0x80;
				RsSfpSdiRsp.Res = 0x00;
				RsSfpSdiRsp.RetCode = 0x80;
				
				TxMsgBuffer[valid_lport].msgLen = sizeof(RsHeader_t) + sizeof(RsSfpSdiRsp_t) + 2;
				RsHeader.StartFlag 		= 0xFA;
				RsHeader.ShelfID 		= ShelfID;
				RsHeader.SlotID 		= SlotID;
				RsHeader.Res1 			= 0x00;
				RsHeader.LenH 			= 0x00;
				RsHeader.LenL 			= TxMsgBuffer[valid_lport].msgLen - 6;
				RsHeader.CmdType 		= 0x02;
				RsHeader.CmdDir 		= 0x00;
				RsHeader.PeerID[0] 		= 0x00;
				RsHeader.PeerID[1] 		= 0x00;
				RsHeader.PeerID[2] 		= 0x00;
				RsHeader.PeerID[3] 		= 0x00;
				RsHeader.SndLportID 	= 0x00;
				RsHeader.SndHportID 	= 0x00;
				RsHeader.ChannelID 		= 0x00;
				RsHeader.CmdID 			= 0x07;
				
				memcpy(&(TxMsgBuffer[valid_lport].msgBuf[0]), (u8 *)&RsHeader, sizeof(RsHeader_t));
				offset = sizeof(RsHeader_t);
				memcpy(&(TxMsgBuffer[valid_lport].msgBuf[offset]), (u8 *)&RsSfpSdiRsp, sizeof(RsSfpSdiRsp_t));
				offset += sizeof(RsSfpSdiRsp_t);
				OddByte = OddByteCheck(TxMsgBuffer[valid_lport].msgBuf, TxMsgBuffer[valid_lport].msgLen-3);
				TxMsgBuffer[valid_lport].msgBuf[offset] = OddByte;
				offset += 1;
				TxMsgBuffer[valid_lport].msgBuf[offset] = 0xFB;

				cli_debug(DBG_UART, "\r\nUart-%d tx %d bytes:\r\n", com_port, TxMsgBuffer[valid_lport].msgLen);
				cli_dump(DBG_UART, TxMsgBuffer[valid_lport].msgBuf, TxMsgBuffer[valid_lport].msgLen);

				if((GPIOE->IDR & GPIO_Pin_6) == 0) {
					RS485TxEnable(com_port);
					UartWrite(com_port, TxMsgBuffer[valid_lport].msgBuf, TxMsgBuffer[valid_lport].msgLen);
				}
				break;
				
				default:
				break;
			}
		}
	}
#endif
}

#endif /* RURAL_CREDIT_PROJECT */

#endif


