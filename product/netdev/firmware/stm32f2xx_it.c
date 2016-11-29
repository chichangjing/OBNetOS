/**
  ******************************************************************************
  * @file    stm32f2xx_it.c 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

#include "mconfig.h"

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_it.h"
#include "main.h"
#include "stm32f2x7_eth.h"
#include "console.h"
#include "rs_uart.h"

/* Scheduler includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#if MODULE_LWIP
/* lwip includes */
#include "lwip/sys.h"
#endif

#if MODULE_RING
#include "ob_ring.h"
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
unsigned int systick;

#if MODULE_LWIP
extern xSemaphoreHandle s_RxSemaphore;
#endif

extern tConsoleDev *pConsoleDev;
extern __IO uint32_t PeriodValue;
extern __IO uint32_t CaptureNumber;
uint16_t tmpCC4[2] = {0, 0};
/* Private function prototypes -----------------------------------------------*/
extern void xPortSysTickHandler(void); 

#if MODULE_RING
extern TimerGroup_t Tim;
#endif
/* Private functions ---------------------------------------------------------*/


/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    systick++;
#if MODULE_RING
	if(Tim.TimTxPortA > 0) Tim.TimTxPortA--;
	if(Tim.TimTxPortB > 0) Tim.TimTxPortB--;	
#endif
	xPortSysTickHandler(); 
		
}

#if !BOARD_GV3S_HONUE_QM && !MODULE_UART_SERVER
void USART1_IRQHandler(void)
{
	if(pConsoleDev->ComPort == COM_USART1)
		ConsoleIRQHandler();
	else
		Uart0_IRQHandler();
}
#endif

#if !MODULE_UART_SERVER
void USART2_IRQHandler(void)
{
	if(pConsoleDev->ComPort == COM_USART2)
		ConsoleIRQHandler();
	else
		Uart1_IRQHandler();
}
#endif /* !MODULE_UART_SERVER */

void USART3_IRQHandler(void)
{
	if(pConsoleDev->ComPort == COM_USART3)
		ConsoleIRQHandler();
#if MODULE_RS485
	else
		Uart2_IRQHandler();
#endif /* MODULE_RS485 */
}

void UART4_IRQHandler(void)
{
	if(pConsoleDev->ComPort == COM_UART4)
		ConsoleIRQHandler();
}

void UART5_IRQHandler(void)
{
	if(pConsoleDev->ComPort == COM_UART5)
		ConsoleIRQHandler();
#if MODULE_RS485
	else
		Uart4_IRQHandler();
#endif /* MODULE_RS485 */
}

void USART6_IRQHandler(void)
{
	if(pConsoleDev->ComPort == COM_USART6)
		ConsoleIRQHandler();
}

/**
  * @brief  This function handles ethernet DMA interrupt request.
  * @param  None
  * @retval None
  */
void ETH_IRQHandler(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  /* Frame received */
  if ( ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET) 
  {
#if MODULE_LWIP
    /* Give the semaphore to wakeup LwIP task */
	if(s_RxSemaphore != NULL)
    	xSemaphoreGiveFromISR(s_RxSemaphore, &xHigherPriorityTaskWoken); 
#endif    
  }
	
  /* Clear the interrupt flags. */
  /* Clear the Eth DMA Rx IT pending bits */
  ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
  ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);

  /* Switch tasks if necessary. */	
  if( xHigherPriorityTaskWoken != pdFALSE )
  {
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  }
}

void EXTI9_5_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	extern xSemaphoreHandle EXTI9_5_Semaphore;

#if 0// BOARD_GE22103MA
	if(EXTI_GetITStatus(EXTI_Line8) != RESET) {
		/* Clear interrupt pending bit */
		EXTI_ClearITPendingBit(EXTI_Line8);
		
	    /* Give the semaphore to wakeup LwIP task */
		if(EXTI9_5_Semaphore != NULL)
	    	xSemaphoreGiveFromISR(EXTI9_5_Semaphore, &xHigherPriorityTaskWoken); 
	}
#else
	if(EXTI_GetITStatus(EXTI_Line9) != RESET) {
		/* Clear interrupt pending bit */
		EXTI_ClearITPendingBit(EXTI_Line9);
		
	    /* Give the semaphore to wakeup LwIP task */
		if(EXTI9_5_Semaphore != NULL)
	    	xSemaphoreGiveFromISR(EXTI9_5_Semaphore, &xHigherPriorityTaskWoken); 
	}
#endif
	
	if(xHigherPriorityTaskWoken != pdFALSE) {
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}	
}

#if 0
void Switch_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	u16 port9linkChg=0;
	u16 port10linkChg=0;
	
	switch_getRegField(PHYADDR_PHY(9), 0x13, 10, 1, &port9linkChg);
	switch_getRegField(PHYADDR_PHY(10), 0x13, 10, 1, &port10linkChg);

	if((port9linkChg >> 10 == 1) || (port10linkChg >> 10 == 1)) {
		/* Give the semaphore to wakeup LwIP task */
		xSemaphoreGiveFromISR( s_PhySemaphore, &xHigherPriorityTaskWoken );   
	}
	
	/* Switch tasks if necessary. */	
	if( xHigherPriorityTaskWoken != pdFALSE ) {
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}
  
}
#endif


#if 0
/**
  * @brief  This function handles External line 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  if(EXTI_GetITStatus(ETH_LINK_EXTI_LINE) != RESET)
  {
    Eth_Link_ITHandler(DP83848_PHY_ADDRESS);
    /* Clear interrupt pending bit */
    EXTI_ClearITPendingBit(ETH_LINK_EXTI_LINE);
  }
}

/**
  * @brief  This function handles ethernet DMA interrupt request.
  * @param  None
  * @retval None
  */
void ETH_IRQHandler(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  /* Frame received */
  if ( ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET) 
  {
    /* Give the semaphore to wakeup LwIP task */
    xSemaphoreGiveFromISR( s_RxSemaphore, &xHigherPriorityTaskWoken );   
  }
	
  /* Clear the interrupt flags. */
  /* Clear the Eth DMA Rx IT pending bits */
  ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
  ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
	
  /* Switch tasks if necessary. */	
  if( xHigherPriorityTaskWoken != pdFALSE )
  {
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  }
}
#endif /* 0 */

/******************************************************************************/
/*                 STM32F2xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f2xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @brief  This function handles TIM5 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM5_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM5, TIM_IT_CC4) != RESET)
  {    
    /* Get the Input Capture value */
    tmpCC4[CaptureNumber++] = TIM_GetCapture4(TIM5);
   
    /* Clear CC4 Interrupt pending bit */
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC4);

    if (CaptureNumber >= 2)
    {
      /* Compute the period length */
      PeriodValue = (uint16_t)(0xFFFF - tmpCC4[0] + tmpCC4[1] + 1);
    }
  }
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
