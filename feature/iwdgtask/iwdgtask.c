/**
  ******************************************************************************
  * @file    iwdgtask.h
  * @author  chichangjing
  * @version V1.0.0
  * @date    2014-4-14 
  * @brief   独立看门狗任务 
  *          
****************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

//#include "iwdgtask.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tcpip.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define TCPECHO_THREAD_PRIO  ( tskIDLE_PRIORITY + 1 )

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
 
__IO uint32_t LsiFreq = 0;
__IO uint32_t CaptureNumber = 0, PeriodValue = 0;

extern struct netif xnetif;
static __IO uint32_t test;

/* Private function prototypes -----------------------------------------------*/
uint32_t GetLSIFrequency(void);

/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Toggle Led4 task
  * @param  pvParameters not used
  * @retval None
  */
void iwdg_task(void * pvParameters)
{
  
  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(SystemCoreClock / 1000))
  { 
    /* Capture error */ 
    while (1);
  }

  /* Check if the system has resumed from IWDG reset */
  if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
  {
    /* IWDGRST flag set */
    /* Turn on LED1 */
    //STM_EVAL_LEDOn(LED1);

    /* Clear reset flags */
    RCC_ClearFlag();
  }
  else
  {
    /* IWDGRST flag is not set */
    /* Turn off LED1 */
    //STM_EVAL_LEDOff(LED1);
  }
 
  /* Get the LSI frequency:  TIM5 is used to measure the LSI frequency */
  LsiFreq = GetLSIFrequency();
   
  /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
#if 0
  /* IWDG counter clock: LSI/32 */
  IWDG_SetPrescaler(IWDG_Prescaler_32);

  /* Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 250ms/IWDG counter clock period
                          = 250ms / (LSI/32)
                          = 0.25s / (LsiFreq/32)
                          = LsiFreq/(32 * 4)
                          = LsiFreq/128
   */
  IWDG_SetReload(LsiFreq/128);
#endif
  /* IWDG counter clock: LSI/256 */
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  /* Set counter reload value to obtain 8000ms IWDG TimeOut.
     Counter Reload Value = 8000ms/IWDG counter clock period
                          = 8000ms / (LSI/256)
                          = 8s / (LsiFreq/256)
                          = LsiFreq/(256 / 8)
                          = LsiFreq/128
   */
  /* 8s IWDG TimeOut */
  IWDG_SetReload(LsiFreq/32);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
  
  while(1)
  {   
    test = xnetif.ip_addr.addr;
    /*check if IP address assigned*/
    if (test !=0)
    {
      for( ;; )
      {
        /* toggle LED2 each 240ms */
        
        //STM_EVAL_LEDToggle(LED2); 
        /* Reload IWDG counter */
        IWDG_ReloadCounter(); 
        /* Insert 1000 ms delay */
        vTaskDelay(1000);
      }
    }
  }
}


/**
  * @brief  Configures TIM5 to measure the LSI oscillator frequency. 
  * @param  None
  * @retval LSI Frequency
  */
uint32_t GetLSIFrequency(void)
{
  NVIC_InitTypeDef   NVIC_InitStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;
  RCC_ClocksTypeDef  RCC_ClockFreq;

  /* Enable the LSI oscillator ************************************************/
  RCC_LSICmd(ENABLE);
  
  /* Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {}

  /* TIM5 configuration *******************************************************/ 
  /* Enable TIM5 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  
  /* Connect internally the TIM5_CH4 Input Capture to the LSI clock output */
  TIM_RemapConfig(TIM5, TIM5_LSI);

  /* Configure TIM5 presclaer */
  TIM_PrescalerConfig(TIM5, 0, TIM_PSCReloadMode_Immediate);
  
  /* TIM5 configuration: Input Capture mode ---------------------
     The LSI oscillator is connected to TIM5 CH4
     The Rising edge is used as active edge,
     The TIM5 CCR4 is used to compute the frequency value 
  ------------------------------------------------------------ */
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV8;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInit(TIM5, &TIM_ICInitStructure);
  
  /* Enable TIM5 Interrupt channel */
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable TIM5 counter */
  TIM_Cmd(TIM5, ENABLE);

  /* Reset the flags */
  TIM5->SR = 0;
    
  /* Enable the CC4 Interrupt Request */  
  TIM_ITConfig(TIM5, TIM_IT_CC4, ENABLE);


  /* Wait until the TIM5 get 2 LSI edges (refer to TIM5_IRQHandler() in 
    stm32f2xx_it.c file) ******************************************************/
  while(CaptureNumber != 2)
  {
  }
  /* Deinitialize the TIM5 peripheral registers to their default reset values */
  TIM_DeInit(TIM5);


  /* Compute the LSI frequency, depending on TIM5 input clock frequency (PCLK1)*/
  /* Get SYSCLK, HCLK and PCLKx frequency */
  RCC_GetClocksFreq(&RCC_ClockFreq);

  /* Get PCLK1 prescaler */
  if ((RCC->CFGR & RCC_CFGR_PPRE1) == 0)
  { 
    /* PCLK1 prescaler equal to 1 => TIMCLK = PCLK1 */
    return ((RCC_ClockFreq.PCLK1_Frequency / PeriodValue) * 8);
  }
  else
  { /* PCLK1 prescaler different from 1 => TIMCLK = 2 * PCLK1 */
    return (((2 * RCC_ClockFreq.PCLK1_Frequency) / PeriodValue) * 8) ;
  }
}

/*-----------------------------------------------------------------------------------*/

void iwdg_task_init(void)
{
  xTaskCreate(iwdg_task, "iwdg_task", configMINIMAL_STACK_SIZE, NULL, TCPECHO_THREAD_PRIO, NULL);
}
/*-----------------------------------------------------------------------------------*/