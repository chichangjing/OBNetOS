

#ifndef __USART_IF_H
#define __USART_IF_H

#include "stm32f2xx.h"

#define  COM_PORT_MAX					2
#define  COM_PORT_0						0
#define  COM_PORT_1						1
#define  COM_PORT_2						2
#define  COM_PORT_3						3
#define  COM_PORT_4						4
#define  COM_PORT_5						5

#ifndef NULL
#define NULL ((void *)0)
#endif

#define UART_RX_IDLE  					0x01		/* rxCount is less than half of the buffer size when idle */
#define UART_RX_OVF   					0x02		/* rxCount is more than half of the buffer size */
#define UART_RX_FULL  					0x04		/* Rx buffer is full */
#define UART_TX_DONE  					0x08		/* Tx Buffer is empty */

/*************************************************************************
 * Definition for COM port0, connected to USART1                     
 * TX pin: PA9, RX pin: PA10 
 *************************************************************************/
#define COM_PORT0						USART1
#define COM_PORT0_CLK					RCC_APB2Periph_USART1
#define COM_PORT0_TX_PIN				GPIO_Pin_9
#define COM_PORT0_TX_GPIO_PORT			GPIOA
#define COM_PORT0_TX_GPIO_CLK			RCC_AHB1Periph_GPIOA
#define COM_PORT0_TX_SOURCE				GPIO_PinSource9
#define COM_PORT0_TX_AF					GPIO_AF_USART1
#define COM_PORT0_RX_PIN				GPIO_Pin_10
#define COM_PORT0_RX_GPIO_PORT			GPIOA
#define COM_PORT0_RX_GPIO_CLK			RCC_AHB1Periph_GPIOA
#define COM_PORT0_RX_SOURCE				GPIO_PinSource10
#define COM_PORT0_RX_AF					GPIO_AF_USART1
#define COM_PORT0_IRQn 					USART1_IRQn

/*************************************************************************
 * Definition for COM port1, connected to USART2                     
 * TX pin: PD5, RX pin: PD6 
 *************************************************************************/
#define COM_PORT1                       USART2
#define COM_PORT1_CLK                   RCC_APB1Periph_USART2
#define COM_PORT1_TX_PIN                GPIO_Pin_5
#define COM_PORT1_TX_GPIO_PORT          GPIOD
#define COM_PORT1_TX_GPIO_CLK           RCC_AHB1Periph_GPIOD
#define COM_PORT1_TX_SOURCE             GPIO_PinSource5
#define COM_PORT1_TX_AF                 GPIO_AF_USART2
#define COM_PORT1_RX_PIN                GPIO_Pin_6
#define COM_PORT1_RX_GPIO_PORT          GPIOD
#define COM_PORT1_RX_GPIO_CLK           RCC_AHB1Periph_GPIOD
#define COM_PORT1_RX_SOURCE             GPIO_PinSource6
#define COM_PORT1_RX_AF                 GPIO_AF_USART2
#define COM_PORT1_IRQn                  USART2_IRQn

/*************************************************************************
 * Definition for COM port1, connected to USART5                     
 * TX pin: PD0, RX pin: PD2 
 *************************************************************************/
#define COM_PORT4                       UART5
#define COM_PORT4_CLK                   RCC_APB1Periph_UART5
#define COM_PORT4_TX_PIN                GPIO_Pin_12
#define COM_PORT4_TX_GPIO_PORT          GPIOC
#define COM_PORT4_TX_GPIO_CLK           RCC_AHB1Periph_GPIOC
#define COM_PORT4_TX_SOURCE             GPIO_PinSource12
#define COM_PORT4_TX_AF                 GPIO_AF_UART5
#define COM_PORT4_RX_PIN                GPIO_Pin_2
#define COM_PORT4_RX_GPIO_PORT          GPIOD
#define COM_PORT4_RX_GPIO_CLK           RCC_AHB1Periph_GPIOD
#define COM_PORT4_RX_SOURCE             GPIO_PinSource2
#define COM_PORT4_RX_AF                 GPIO_AF_UART5
#define COM_PORT4_IRQn                  UART5_IRQn


/* The structure that defines uart buffer property */
typedef void (*isr_cb_t)(u8 UartPort, u8 Event);
typedef __packed struct {
	u16 rxSize;
	u16 txSize;        
	u8* rxBuf;
	u8* txBuf;
} uart_setup_t;

/* Private typedef */
typedef __packed struct _uart_fifo {
	u8*		rxBuf;
	u16		rxSize;
	vu16	rxPush;
	vu16	rxPop;
	vu16	rxCnt;
	
	u8*		txBuf;
	u16		txSize;
	vu16	txPush;
	vu16	txPop;
	vu16	txCnt;
} uart_fifo_t;

typedef __packed struct _uart_property {
	u32 baudrate;
	u16 databits;
	u16 stopbits;
	u16 parity;
	u16 flowctrl;
} uart_property_t;

typedef struct _uart_stat {
	u32	tx_bytes;
	u32	rx_bytes;
	u32	count_rxidle;
	u32	count_rxovf;
	u32	count_rxfull;
	u32	count_txdone;
	u32	count_overrun;
	u32	count_tim_ethtx;
	u32	count_ovf_ethtx;
} uart_stat_t;

typedef struct _uart_dev {
	USART_TypeDef*	uart;	
	uart_fifo_t		fifo;
	uart_stat_t		stat;
	uart_property_t property;
} uart_dev_t;

/* Exported functions ------------------------------------------------------- */
void UartLowInit(u8 port, USART_InitTypeDef *pUSART_InitStructure);
void UartDevSetup(u8 port, uart_setup_t setup);

u16 UartRead(u8 port,u8 *data,u16 len);
u16 UartWrite(u8 port,u8 *data,u16 len);


void UartLedOn(u8 port);
void UartLedOff(u8 port);
void UartLedFlash(u8 port);
	
void RS485GpioInit(u8 port);
void RS485RxEnable(u8 port);
void RS485TxEnable(u8 port);

uart_dev_t *GetUartDev(u8 port);

#endif /* __USART_IF_H */



