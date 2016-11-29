




#include "config.h"





/*******************************************************************************
* Name: RCC_Configuration
* Deion:
* Input: None
* Output: None
* Return: None
*******************************************************************************/

void RCC_Configuration(void)
{
	SystemInit();
#if  ( (defined HAL_SWI2C0) || ( defined HAL_SWI2C1 ))
	RCC_APB2PeriphClockCmd(PHERIH_I2C0 | PHERIH_I2C1, ENABLE);
#endif
#if  ( defined HAL_UART1 )
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#endif
#if  ( defined HAL_UART2 )
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#endif
}

/*******************************************************************************
* Name: Gpio_Configuration
* Deion:
* Input: None
* Output: None
* Return: None
*******************************************************************************/
void Gpio_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* general gpio */
#if  ( defined HAL_SWI2C0 )
	/* software_i2c gpio */
	/* i2c0_CLK(PB6),i2c0_DAT(PB7) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
#if  ( defined HAL_SWI2C1 )
	/* i2c1_CLK(PB10),i2c1_DAT(PB11) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
#if  ( defined HAL_EEPROM )
	/* i2c1_CLK(PB10),i2c1_DAT(PB11) */
	GPIO_InitStructure.GPIO_Pin = EEWPPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(EEWPPORT, &GPIO_InitStructure);
#endif
	/* ADC gpio */
#if  ( defined HAL_UART1 )
	/* uart gpio */
	/* uart0_Tx(PA9), uart1_Tx(PA2)*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_9 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
#if  ( defined HAL_UART2 )
	/* uart0_Rx(PA10),uart1_Rx(PA3) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_9 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
}

/*******************************************************************************
* Name   : NVIC_Configuration
* Deion        : Configures NVIC and Vector Table base location.
* Input                    : None
* Output                 : None
* Return                 : None
*******************************************************************************/
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
#ifdef   VECT_TAB_RAM
	/* Set the Vector Table base location at 0x20000000 */
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else   /* VECT_TAB_FLASH   */
	/* Set the Vector Table base location at 0x08000000 */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
#if ( defined HAL_UART1 )
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#if	( deifned HAL_UART2 )
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1
	        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
}

void Uartx_Configuration(uartxconfig_t *uartconf)
{
#if  ( (defined HAL_UART1) || (defined HAL_UART2) )
	USART_Init(uartconf->uart_dev, &uartconf->uart_conf);
	// ¨º1?¨¹ USART1
	if (uartconf->itflag.rxne)
	{
		USART_ITConfig(uartconf->uart_dev, USART_IT_RXNE, ENABLE);
	}
	if (uartconf->itflag.tc)
	{
		USART_ITConfig(uartconf->uart_dev, USART_IT_TC, ENABLE);
	}
	// USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	USART_Cmd(uartconf->uart_dev, ENABLE);
#endif
}

