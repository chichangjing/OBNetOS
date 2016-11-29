/* Standard includes */
#include <stdio.h>

/* BSP includes */
#include "misc.h"
#include "console.h"

/* Other includes */
#include "common.h"

/* Private variables */
USART_TypeDef *ConsolePort = NULL;
eComMode ConsoleMode;

static void USART2_Init(unsigned int BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

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

	USART_Cmd(COM_PORT1, ENABLE);
}

static void USART3_Init(unsigned int BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	USART_Cmd(COM_PORT2, ENABLE);
}

static void UART5_Init(unsigned int BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
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
	USART_Init( COM_PORT4, &USART_InitStructure );

	/* Enable the UART interrupt in the NVIC. */
	NVIC_InitStructure.NVIC_IRQChannel = COM_PORT4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );
	
	USART_Cmd(COM_PORT4, ENABLE);
}

static void USART6_Init(unsigned int BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

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
	USART_Init( COM_PORT5, &USART_InitStructure );	
	
	/* Enable the USART interrupt in the NVIC. */
	NVIC_InitStructure.NVIC_IRQChannel = COM_PORT5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	USART_Cmd(COM_PORT5, ENABLE);
}

static void RS485_GpioInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
#if BOARD_GE22103MA
	extern u8 gPcbVer87;
    extern u16 gPcbAddr;
#endif

#if BOARD_GE22103MA
    if(gPcbAddr == 0x007F){
        if(gPcbVer87 != 0x03) {
            /* RS485-EN, USART3, PD10 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
        } else {
            /* RS485-EN, UART5, PD1 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
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
#elif BOARD_GE20023MA
    /* RS485-EN, UART5, PD0 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
    
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
}

static void RS485_TxEnable(void)
{
#if BOARD_GE22103MA
	extern u8 gPcbVer87;
    extern u16 gPcbAddr;
#endif

#if BOARD_GE22103MA
    if(gPcbAddr == 0x007F){
        if(gPcbVer87 != 0x03) {
            GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_SET);
        } else {
            GPIO_WriteBit(GPIOD, GPIO_Pin_1, Bit_SET);
        } 
    }else{
        GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_SET);
    }
#elif BOARD_GE20023MA
    GPIO_WriteBit(GPIOD, GPIO_Pin_0, Bit_SET);
#elif BOARD_GV3S_HONUE_QM
    GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_SET);
#endif
}

static void RS485_RxEnable(void)
{
#if BOARD_GE22103MA
	extern u8 gPcbVer87;
    extern u16 gPcbAddr;
#endif

#if BOARD_GE22103MA
    if(gPcbAddr == 0x007F)
    {
        if(gPcbVer87 != 0x03) {
            GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_RESET);
        } else {
            GPIO_WriteBit(GPIOD, GPIO_Pin_1, Bit_RESET);
        }
    }else{
        GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_RESET);
    }
#elif BOARD_GE20023MA
    GPIO_WriteBit(GPIOD, GPIO_Pin_0, Bit_SET);
#elif BOARD_GV3S_HONUE_QM
    GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_RESET);
#endif
}


void ConsoleInit(int Port, unsigned int BaudRate)
{
#if BOARD_GE22103MA || BOARD_GV3S_HONUE_QM
	ConsoleMode = RS485;
#elif BOARD_GE20023MA
    ConsoleMode = RS485;
#else
	ConsoleMode = RS232;
#endif
	if(ConsoleMode == RS485) {
		RS485_GpioInit();
		RS485_TxEnable();
	}
			
	switch(Port){
		case COM_USART2:
			USART2_Init(BaudRate);
			ConsolePort = USART2;
			break;
		case COM_USART3:
			USART3_Init(BaudRate);
			ConsolePort = USART3;
			break;			
		case COM_UART5:
			UART5_Init(BaudRate); 
			ConsolePort = UART5;
			break;
		case COM_USART6:
			USART6_Init(BaudRate);
			ConsolePort = USART6;
			break;			
		default:
			break;
	}
}

void ConsolePutChar(char c)
{
	if(ConsoleMode == RS485) {
		RS485_TxEnable();
	}
	
	while((~ConsolePort->SR) & USART_SR_TXE);
	ConsolePort->DR = c;
	while(!(ConsolePort->SR & USART_SR_TC));

	if(ConsoleMode == RS485) {
		RS485_RxEnable();
	}
}

void ConsolePutString(const char * s)
{
	if(ConsoleMode == RS485) {
		RS485_TxEnable();
	}
	
	while(*s != '\0') {
		while(!(ConsolePort->SR & USART_SR_TXE));
		ConsolePort->DR = *s++;
		while(!(ConsolePort->SR & USART_SR_TC));
	}
	
	if(ConsoleMode == RS485) {
		RS485_RxEnable();
	}	
}

char ConsoleGetChar(void)
{
	while(!(ConsolePort->SR & USART_SR_RXNE));
	if(ConsoleMode == RS485)
		while(!(ConsolePort->SR & USART_SR_IDLE));
		
	return (char)(ConsolePort->DR);
}

int ConsoleCheckRxChar(char c)
{	
	if(ConsolePort->SR & USART_SR_RXNE) {
		if((char)(ConsolePort->DR) == c)
			return 1;
		else
			return 0;
	} else
		return 0;
}


void ConsoleDisable(void)
{
	USART_Cmd(ConsolePort, DISABLE);	
}

void ConsoleEnable(void)
{
	USART_Cmd(ConsolePort, ENABLE);	
}		


#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

/* Retargets the C library printf function. */
PUTCHAR_PROTOTYPE {
	ConsolePutChar((int8_t)ch);
	return ch;
}

