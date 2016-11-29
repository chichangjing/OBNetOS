
/*************************************************************
 * Filename     : uart_if.c
 * Description  : uart driver interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#include "mconfig.h"

#if MODULE_UART_SERVER

/* Kernel includes. */
#include "FreeRTOS.h"
#include "semphr.h"

/* BSP includes */
#include "usart_if.h"


USART_TypeDef*	UartTable[COM_PORT_MAX] = {USART1,USART2};
uart_dev_t UartDevice[COM_PORT_MAX];
unsigned int uart_led_tim[COM_PORT_MAX] = {0};
extern xSemaphoreHandle xSemUartTx[];
extern xSemaphoreHandle xSemUartRx[];


static u16 _uart_read_isr(uart_fifo_t *pfifo, USART_TypeDef *uart, u8 *data, u16 len)
{
	u32 ie;
	u16 n;
	
	if(len == 0)
		return 0;

	for(n=0;n<len;n++) {
		if(n < pfifo->rxCnt) {
			*data++ = pfifo->rxBuf[pfifo->rxPop];
			pfifo->rxPop += 1;
			if(pfifo->rxPop>=pfifo->rxSize)
				pfifo->rxPop = 0;
		} else {
			break;
		}
	}
#if 1
	portDISABLE_INTERRUPTS();
	pfifo->rxCnt -= n;
	portENABLE_INTERRUPTS();
#else
	ie = __get_PRIMASK();
	__set_PRIMASK(1);
	pfifo->rxCnt -= n;
	__set_PRIMASK(ie);
#endif
	return n;
}

static u16 _uart_write_isr(uart_fifo_t *pfifo, USART_TypeDef *uart, u8 *data, u16 len)
{
	u32 ie;
	u16 n;
	
	if(len == 0)
		return 0;
	
	for(n=0;n<len;n++) {
		if((pfifo->txSize-n) > pfifo->txCnt) {
			pfifo->txBuf[pfifo->txPush] = *data++;
			pfifo->txPush += 1;
			if(pfifo->txPush >= pfifo->txSize)
				pfifo->txPush = 0;
		} else {
			break;
		}
	}
#if 1
	portDISABLE_INTERRUPTS();
	pfifo->txCnt += n;
	portENABLE_INTERRUPTS();
	//__disable_interrupt();
	//__enable_interrupt();
#else
	ie = __get_PRIMASK();
	__set_PRIMASK(1);
	pfifo->txCnt += n;
	__set_PRIMASK(ie);
#endif
	return n;
}

u16 UartRead(u8 port,u8 *data,u16 len)
{
	if(port >= COM_PORT_MAX) {
		return 0;
	} else {
		return _uart_read_isr(&UartDevice[port].fifo, UartDevice[port].uart, data, len);
	}
}

u16 UartWrite(u8 port,u8 *data,u16 len)
{
	u16 tx_count;
	
	if(port >= COM_PORT_MAX) {
		return 0;
	} else {
		USART_TypeDef *pUart = UartDevice[port].uart;
		uart_fifo_t *pfifo = &UartDevice[port].fifo;
		u8 c;
		
		tx_count = _uart_write_isr(&UartDevice[port].fifo, UartDevice[port].uart, data, len);
		USART_ITConfig(UartDevice[port].uart, USART_IT_TXE, ENABLE);
		
		return tx_count;
	}
}

void RS485GpioInit(u8 port)
{
	GPIO_InitTypeDef  GPIO_InitStructure;   

	switch(port) {  
		case COM_PORT_0:
		/* RS485-EN, USART1, PA11 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOA, &GPIO_InitStructure); 			
		break;
		
		case COM_PORT_4:
		/* RS485-EN, UART5, PD0 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOD, &GPIO_InitStructure); 		
		break;

		default:
		break;
	}
	
}

void RS485TxEnable(u8 port)
{
	switch(port) {  
		case COM_PORT_0:
		GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_SET);
		break;
		
		case COM_PORT_4:
		GPIO_WriteBit(GPIOD, GPIO_Pin_0, Bit_SET);
		break;

		default:
		break;
	}
}

void RS485RxEnable(u8 port)
{
	switch(port) {  
		case COM_PORT_0:
		GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);
		break;
		
		case COM_PORT_4:
		GPIO_WriteBit(GPIOD, GPIO_Pin_0, Bit_RESET);
		break;

		default:
		break;
	}
}

void UartLedOn(u8 port)
{
	if(port == 0)
		GPIOE->BSRRH = GPIO_Pin_0;
	else if(port == 1)
		GPIOE->BSRRH = GPIO_Pin_1;
}

void UartLedOff(u8 port)
{
	if(port == 0)
		GPIOE->BSRRL = GPIO_Pin_0;
	else if(port == 1)
		GPIOE->BSRRL = GPIO_Pin_1;
}

void UartLedFlash(u8 port)
{
	if(port >= COM_PORT_MAX)
		return;
	UartLedOn(port);
	uart_led_tim[port] = 10;
}

void UartDevSetup(u8 port, uart_setup_t setup)
{
	UartDevice[port].uart 			= UartTable[port];
	
	UartDevice[port].fifo.rxCnt		= UartDevice[port].fifo.txCnt  = 0;
	UartDevice[port].fifo.rxPush	= UartDevice[port].fifo.txPop  = 0;        
	UartDevice[port].fifo.txPush	= UartDevice[port].fifo.txPop  = 0;
	UartDevice[port].fifo.rxSize	= setup.rxSize;
	UartDevice[port].fifo.rxBuf		= setup.rxBuf;
	UartDevice[port].fifo.txSize	= setup.txSize;
	UartDevice[port].fifo.txBuf		= setup.txBuf;

	UartDevice[port].stat.tx_bytes 		= 0x00;
	UartDevice[port].stat.rx_bytes		= 0x00;
	UartDevice[port].stat.count_rxidle	= 0x00;
	UartDevice[port].stat.count_rxovf	= 0x00;
	UartDevice[port].stat.count_rxfull	= 0x00;
	UartDevice[port].stat.count_txdone	= 0x00;
	UartDevice[port].stat.count_overrun	= 0x00;
	UartDevice[port].stat.count_tim_ethtx = 0x00;
	UartDevice[port].stat.count_ovf_ethtx = 0x00;
}

uart_dev_t *GetUartDev(u8 port)
{
	return &UartDevice[port];
}

void UartLowInit(u8 port, USART_InitTypeDef *pUSART_InitStructure)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	switch(port) {
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
		break;

		default:
		break;
	}
}



void USART1_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	USART_TypeDef *uart = UartDevice[COM_PORT_0].uart;
	uart_fifo_t *pfifo = &UartDevice[COM_PORT_0].fifo;
	u32 iflag;
	u8 c,event = 0;
	__disable_interrupt();
	iflag = (uart->SR & (USART_FLAG_TXE | USART_FLAG_RXNE | USART_FLAG_IDLE));
	__enable_interrupt();
	if(iflag & USART_FLAG_IDLE) {
		c = uart->DR;
		if(pfifo->rxCnt <= (pfifo->rxSize >> 1)) {
			event |=  UART_RX_IDLE;
			UartDevice[COM_PORT_0].stat.count_rxidle++;
		}
	}
	
	if(iflag & USART_FLAG_RXNE) {
		c = uart->DR;
		if(pfifo->rxCnt < pfifo->rxSize) {
			pfifo->rxBuf[pfifo->rxPush] = c;
			pfifo->rxPush += 1;
			if(pfifo->rxPush >= pfifo->rxSize)
				pfifo->rxPush = 0;
			pfifo->rxCnt += 1;
			if(pfifo->rxCnt == pfifo->rxSize) {
				event |= UART_RX_FULL;
				UartDevice[COM_PORT_0].stat.count_rxfull++;
			}
			else if(pfifo->rxCnt > (pfifo->rxSize >> 1)) {
				event |= UART_RX_OVF;
				UartDevice[COM_PORT_0].stat.count_rxovf++;
			}
		}
	}

	if(iflag & USART_FLAG_TXE) {
		uart->CR1 &= (uint16_t)~0x0080;
		if(pfifo->txCnt) {
			c = pfifo->txBuf[pfifo->txPop];
			uart->DR = c;	
			uart->CR1 |= (uint16_t)0x0080;
			pfifo->txPop += 1;
			if(pfifo->txPop >= pfifo->txSize)
				pfifo->txPop = 0;
			pfifo->txCnt -= 1;
			if(pfifo->txCnt == 0) {
				USART_ITConfig(uart, USART_IT_TXE, DISABLE);	
				event |=  UART_TX_DONE;
				UartDevice[COM_PORT_0].stat.count_txdone++;
				#if 1
				__disable_interrupt();
				while (USART_GetFlagStatus(uart, USART_FLAG_TC) == RESET) {}
				__enable_interrupt();
				RS485RxEnable(COM_PORT_0);
				#endif
			}		
		}
	}

	if(USART_GetFlagStatus(UartTable[COM_PORT_0], USART_FLAG_ORE) == SET) {
		UartDevice[COM_PORT_0].stat.count_overrun++;
		USART_ClearFlag(UartTable[COM_PORT_0],USART_FLAG_ORE);
		USART_ReceiveData(UartTable[COM_PORT_0]);	
	}

	if((event & UART_RX_OVF) || (event & UART_RX_FULL) || (event & UART_RX_IDLE)) {
		xSemaphoreGiveFromISR(xSemUartRx[COM_PORT_0], &xHigherPriorityTaskWoken );
	}

	if(event & UART_TX_DONE) {
		xSemaphoreGiveFromISR(xSemUartTx[COM_PORT_0], &xHigherPriorityTaskWoken );
	}

	if( xHigherPriorityTaskWoken != pdFALSE ) {
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}
}

void USART2_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	USART_TypeDef *uart = UartDevice[COM_PORT_1].uart;
	uart_fifo_t *pfifo = &UartDevice[COM_PORT_1].fifo;
	u32 iflag,ie;
	u8 c,event = 0;
#if 1
	__disable_interrupt();
	iflag = (uart->SR & (USART_FLAG_TXE | USART_FLAG_RXNE | USART_FLAG_IDLE));
	__enable_interrupt();
#else
	ie = __get_PRIMASK();
	__set_PRIMASK(1);
	iflag = (uart->CR1 & (USART_FLAG_TXE|USART_FLAG_TC|USART_FLAG_RXNE|USART_FLAG_IDLE) & uart->SR);
	__set_PRIMASK(ie);
#endif

	//if(iflag & USART_FLAG_IDLE) {
	if(USART_GetITStatus(uart, USART_IT_IDLE) == SET) {
		c = uart->DR;
		if(pfifo->rxCnt <= (pfifo->rxSize >> 1)) {
			event |=  UART_RX_IDLE;
			UartDevice[COM_PORT_1].stat.count_rxidle++;
		}
	}
	
	//if(iflag & USART_FLAG_RXNE) {
	if(USART_GetITStatus(uart, USART_IT_RXNE) == SET) {
		c = uart->DR;
		if(pfifo->rxCnt < pfifo->rxSize) {
			pfifo->rxBuf[pfifo->rxPush] = c;
			pfifo->rxPush += 1;
			if(pfifo->rxPush >= pfifo->rxSize)
				pfifo->rxPush = 0;
			pfifo->rxCnt += 1;
			if(pfifo->rxCnt == pfifo->rxSize) {
				event |= UART_RX_FULL;
				UartDevice[COM_PORT_1].stat.count_rxfull++;
			}
			else if(pfifo->rxCnt > (pfifo->rxSize >> 1)) {
				event |= UART_RX_OVF;
				UartDevice[COM_PORT_1].stat.count_rxovf++;
			}
		}
	}

	//if(iflag & USART_FLAG_TXE) {
	if( USART_GetITStatus(uart, USART_IT_TXE) == SET) {
		uart->CR1 &= (uint16_t)~0x0080;
		if(pfifo->txCnt) {
			c = pfifo->txBuf[pfifo->txPop];
			uart->DR = c;
			uart->CR1 |= (uint16_t)0x0080;
			pfifo->txPop += 1;
			if(pfifo->txPop >= pfifo->txSize)
				pfifo->txPop = 0;
			pfifo->txCnt -= 1;
			if(pfifo->txCnt == 0) {
				USART_ITConfig(uart, USART_IT_TXE, DISABLE);
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
	
	if((event & UART_RX_OVF) || (event & UART_RX_FULL) || (event & UART_RX_IDLE)) {
		xSemaphoreGiveFromISR(xSemUartRx[COM_PORT_1], &xHigherPriorityTaskWoken );
	}

	if(event & UART_TX_DONE) {
		xSemaphoreGiveFromISR(xSemUartTx[COM_PORT_1], &xHigherPriorityTaskWoken );
	}

	if( xHigherPriorityTaskWoken != pdFALSE ) {
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}
}


#endif

