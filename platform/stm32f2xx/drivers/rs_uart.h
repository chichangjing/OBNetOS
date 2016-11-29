
#ifndef __RS_UART_H
#define __RS_UART_H

#include "mconfig.h"
#include "stm32f2xx.h"

#define  COM_PORT_MAX					6
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

#define UART_MSG_BUF_SIZE				128

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
 * Definition for COM port1, connected to USART3                     
 * TX pin: PD8, RX pin: PD9 
 *************************************************************************/
#define COM_PORT2                       USART3
#define COM_PORT2_CLK                   RCC_APB1Periph_USART3
#define COM_PORT2_TX_PIN                GPIO_Pin_8
#define COM_PORT2_TX_GPIO_PORT          GPIOD
#define COM_PORT2_TX_GPIO_CLK           RCC_AHB1Periph_GPIOD
#define COM_PORT2_TX_SOURCE             GPIO_PinSource8
#define COM_PORT2_TX_AF                 GPIO_AF_USART3
#define COM_PORT2_RX_PIN                GPIO_Pin_9
#define COM_PORT2_RX_GPIO_PORT          GPIOD
#define COM_PORT2_RX_GPIO_CLK           RCC_AHB1Periph_GPIOD
#define COM_PORT2_RX_SOURCE             GPIO_PinSource9
#define COM_PORT2_RX_AF                 GPIO_AF_USART3
#define COM_PORT2_IRQn                  USART3_IRQn

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

typedef enum { 
	UART_RS232	= 0,
	UART_RS485	= 1,
	UART_RS422	= 2
} uart_type_e;

typedef enum { 
	UART_ISR_STAT_IDLE	= 0,
	UART_ISR_STAT_RXNE	= 1,
	UART_ISR_STAT_TXE	= 2
} uart_isr_stat_e;

typedef struct _uart_stat {
	u32	tx_bytes;
	u32	rx_bytes;
	u32 prv_isr_stat;
	u32	count_rxidle;
	u32	count_rxovf;
	u32	count_rxfull;
	u32	count_txdone;
	u32	count_overrun;
	u32	count_tim_ethtx;
	u32	count_ovf_ethtx;
} uart_stat_t;

typedef __packed struct _uart_dev {
	u8				lport;
	u8				hport;
	USART_TypeDef*	uart;	
	uart_fifo_t		fifo;
	uart_stat_t		stat;
	uart_property_t property;
} uart_dev_t;


typedef __packed struct _uart_msg
{
	u16	msgLen;
	u8	msgBuf[UART_MSG_BUF_SIZE];	
} uart_msg_t;


#if RURAL_CREDIT_PROJECT

typedef struct  {
	u8	StartFlag;
	u8	ShelfID;
	u8	SlotID;
	u8	Res1;
	u8	LenH;
	u8	LenL;
	u8	CmdType;
	u8	CmdDir;
	u8	PeerID[4];
	u8	SndLportID;
	u8	SndHportID;
	u8	ChannelID;
	u8	CmdID;
} RsHeader_t;

#define RS_SWITCH_NAME_LEN	16
typedef struct  {
	u8	StartFlag;
	u8	SwitchType[8];
	u8	SwitchMAC[6];
	u8	SwitchNameLen;
} RsSwitchStatus_t;

typedef struct  {
	u8	StartFlag;
	u8	MasterSlave;
	u8	SwitchMAC[6];
} RsMacStatus_t;

typedef struct  {
	u8	StartFlag;
	u8	Res;
	u8	RetCode;
} RsSfpSdiRsp_t;

#endif

/* Exported functions  */
void UartLowInit(u8 port, USART_InitTypeDef *pUSART_InitStructure);
void Uart0_IRQHandler(void);
void Uart1_IRQHandler(void);
void Uart2_IRQHandler(void);
void Uart4_IRQHandler(void);
void UartStart(void);

#endif /* __USART_IF_H */