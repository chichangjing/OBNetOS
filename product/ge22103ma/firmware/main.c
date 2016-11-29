/*******************************************************************************
  * @file    main.c
  * @author  OB T&C Development Team, HeJianguo
  * @brief   Main program body
  ******************************************************************************/

#include "mconfig.h"

/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* BSP includes */
#include "stm32f2xx.h"
#include "console.h"
#include "timer.h"
#include "soft_i2c.h"
#include "stm32f2x7_eth_bsp.h"
#include "led_drv.h"

#if SWITCH_CHIP_BCM53101
#include "robo_drv.h"
#elif SWITCH_CHIP_88E6095
#include "m88e6095_drv.h"
#include "m88e6095_if.h"
#endif

/* Other includes */
#include "ob_version.h"
#include "netconf.h"
#include "conf_alarm.h"
#include "conf_port.h"

#if MODULE_OBNMS
#include "nms_if.h"
#endif
#if MODULE_RING
#include "ob_ring.h"
#endif
#if MODULE_SIGNAL
#include "sig_srv.h"
#endif

#if MODULE_UART_SERVER
#include "uart_server.h"
#endif	

#if BOARD_GV3S_HONUE_QM_V1_00
#include "fpga_task.h"
#endif	

#define VectTab_Offset 0x10000
char FirmareVersion[] = OB_FIRMWARE_VERSION;
unsigned char DevMac[6] = {0};
static unsigned char DefaultDevMac[6] = {0x0c, 0xa4, 0x2a, 0x00, 0x00, 0x01};

void AppBanner(void)
{
	printf("\r\n");
	printf("  ***************************************\r\n");
	printf("  **        Welcome to OBNetOS         **\r\n");
	printf("  **   -- -- -- -- -- -- -- -- -- --   **\r\n");
	printf("  **   Version release: v%s       **\r\n", &FirmareVersion[0]);
	printf("  **   OB Telecom Electronics Co.,Ltd  **\r\n");
	printf("  ***************************************\r\n");	
	printf("\r\n");
}

void Uart485_test(void *arg);
void dev_reset(void);

void main(void) 
{
	const u8 temp_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	/* Set Vector Table */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, VectTab_Offset);

	/* Configure Timer  */
	TimerInit();
#if BOARD_GV3S_HONUE_QM_V1_00        
    dev_reset();
#endif /* BOARD_GV3S_HONUE_QM_V1_00 */

#if SERIAL_DEBUG
	/* Configure console, and display the welcome banner */
#if CONSOLE_USE_UART5
	ConsoleInit(COM_UART5, 115200);
#elif CONSOLE_USE_USART3
	ConsoleInit(COM_USART3, 115200);
#endif
    
#endif /* SERIAL_DEBUG */
    
	AppBanner();  

	/* Configure ethernet (GPIOs, clocks, MAC, DMA) */
	ETH_BSP_Config();

	/* Configure I2C GPIOs, and load device MAC address  */
	I2C_GPIO_Config();
	if(I2C_Read(DevMac, 6, EPROM_ADDR_MAC, EEPROM_SLAVE_ADDR) != I2C_SUCCESS) {
		memcpy(DevMac, DefaultDevMac, 6);
	} else {
		if(memcmp(DevMac, temp_mac, 6) == 0)
			memcpy(DevMac, DefaultDevMac, 6);
	}

	/* Configure LED GPIOs */
	LED_GPIO_config();

	/* Configure Switch chip */
#if SWITCH_CHIP_BCM53101
	RoboSwitch_Init();
#elif SWITCH_CHIP_88E6095
{
	extern int obSwitch_start(void);
	if(smi_probe() != SW_DRV_SUCCESS) {
		printf("Error: can't probe switch chip\r\n");
	} 

	obSwitch_start();
	swif_Initialize();
}
#endif

	conf_initialize();

#if MODULE_RING		
	/* Ring Initialize */
	Ring_Initialize();
#endif

	/* Initialize the LwIP stack */
	LwIP_Init();

	/* Start the CLI */
	{
		extern void cli_main(void);
		cli_main();
	}

	xTaskCreate(LED_Task, 	"tLED",		configMINIMAL_STACK_SIZE*1, NULL,	tskIDLE_PRIORITY + 1, NULL);
	
#if MODULE_RING	
	Ring_Start();
#endif

#if BOARD_GV3S_HONUE_QM_V1_00    
    fpga_task_init();
#endif /* BOARD_GV3S_HONUE_QM_V1_00 */
       
#if MODULE_SIGNAL
	xTaskCreate(Signal_Task,	"tSIGNAL", 	configMINIMAL_STACK_SIZE*1, NULL,	tskIDLE_PRIORITY + 2, NULL);
#endif /* MODULE_SIGNAL */
	
#if MODULE_OBNMS
	xTaskCreate(NMS_Task,	"tNMS", 	configMINIMAL_STACK_SIZE*3, NULL,	tskIDLE_PRIORITY + 2, NULL);
#endif

#if MODULE_UART_SERVER
	UartServerStart();
#endif	
	
	/* Start the scheduler. */
	vTaskStartScheduler();

	for( ;; );
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

void Uart485_test(void *arg)
{
    
}

void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
	printf( "Assert failed: file %s, line %u\r\n", pcFile, ulLine );
}

