/*************************************************************
 * Filename     :  fpga_task.c
 * Description  :  
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/
/* include -------------------------------------------------------------------*/
#include "mconfig.h"
#include "fpga_task.h"
#include "common.h"
#include "fpga_debug.h"
#include "timer.h"
/* stadard library. */
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
/* LwIP includes */
/* BSP includes */
#include "halsw_i2c.h"
#include "si5327.h"
#include "i2c_ee.h"
#include "hal_uart.h"
#include "delay.h"
#include "hal_io.h"
#include "halcmd_msg.h"
#include "fpga_api.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if ((defined HAL_UART1) && HAL_UART1)
uartxinfo uart1conf =
{
	HAL_UART1,
	115200,
	USART_WordLength_8b,
	USART_StopBits_2,
	USART_Parity_Odd,
	USART_HardwareFlowControl_None
};
#endif

/* Private function prototypes -----------------------------------------------*/
void BoardPeriphInit(void);
void SYSHalInit(void);

/* Private functions ---------------------------------------------------------*/

void fpga_task(void *arg)
{
    SYSHalInit();
    BoardPeriphInit(); 
    
    for(;;){
        
        NetCmdExe();
//		Com1Exe();
		ComCmdExe();
//		PhyDet(&pdettime);
		UartR1TimeoutSet();
//		UartR2TimeoutSet();
//		An_test();
        vTaskDelay(1);
    }
}

void fpga_task_init(void)
{
    /* create fpga upgrade task */
    xTaskCreate(fpga_task,	"tFPGA", 	configMINIMAL_STACK_SIZE*3, NULL,	tskIDLE_PRIORITY + 3, NULL);
}

void SYSHalInit(void)
{
	//SysTick_Config(SystemCoreClock / (1000)); //8M
	/* uart Init */
#if	((defined HAL_UART1) && HAL_UART1)
	UartxInit(&uart1conf, TXDERXEN);
	Uart1RxCallBack((void *)Ut1Recv);
#endif
    
#if ((defined HAL_UART2) && HAL_UART2)
	UartxInit(&uart2conf, TXDERXEN);
	Uart2RxCallBack((void *)Ut2Recv);
#endif

#if ((defined HAL_UART4) && HAL_UART4)
	UartxInit(&uart4conf, TXDERXEN);
	Uart4RxCallBack((void *)Ut4Recv);
#endif

	//SPI1Init(SPISLAMODE);

	/* software I2C Init */
#if  (defined HAL_SWI2C0 && HAL_SWI2C0)
	SW_I2CInit(I2CDEV_0, 30);
	while (SW_CheakI2CState(I2CDEV_0) != I2C_IDLE_OK)
	{
		SW_I2CBusIdle(I2CDEV_0);
	}
#endif
    
#if ( defined HAL_SWI2C1 && HAL_SWI2C1)
	SW_I2CInit(I2CDEV_1, 30);
#endif
    
	/* Gpio Init */
#if (defined DEBUG_ONLYSWD)
	DebugPortAFSet(DJTAGESW);
#endif

	/* chip dev info get */
	DeviceInfoInit();
    
//	HalswMdioInit();
	GpioConfInit();    

	/* at24c64 wp */
	At24WpInit();
}


void BoardPeriphInit(void)
{
    si5324confreg  temp1 = {{0}},temp2 = {{0}};
	si5324confregp temp1_p = &temp1,temp2_p = &temp2;
    
    SW_I2CInit(I2CDEV_0, 30);
	while (SW_CheakI2CState(I2CDEV_0) != I2C_IDLE_OK)
	{
		SW_I2CBusIdle(I2CDEV_0);
	}

    /* configure si5327 clock chip */
    Si5324Conf(I2CDEV_0, SI5324ADD1, (si5324confregp)&cfg_7425_1485_1485, "148.5MHz");
    
   	FImageinfoCheck();
    
    Si5324AllGetReg(I2CDEV_0, SI5324ADD1, temp2_p);
    
#if BOARD_GV3S_HONUE_QM
    vTaskDelay(1000);
#else
    Delay_Ms(1000);
#endif

    BoardInfoGet();
 }

void dev_reset(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    /* Clock chip PD11 reset pin initialize */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
    
    /* FPAG PD12 reset pin initialize*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
    
	/* 88e6095,88E1112-64QFN PE7 reset pin initialize*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);  
    
    /* reset clock chip */
    GPIO_ResetBits(GPIOD, GPIO_Pin_11);    
    GPIO_SetBits(GPIOD, GPIO_Pin_11);
    
    /* reset fpga ,88e6095 and 88E1112-64QFN */
    GPIO_ResetBits(GPIOD, GPIO_Pin_12);
    TimerDelayMs(10);
    GPIO_ResetBits(GPIOE, GPIO_Pin_7); 
    GPIO_SetBits(GPIOE, GPIO_Pin_7);
    TimerDelayMs(10);
    GPIO_SetBits(GPIOD, GPIO_Pin_12); 
}