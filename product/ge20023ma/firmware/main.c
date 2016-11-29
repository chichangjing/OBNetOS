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
#include "m88e6095_if.h"
#endif

/* Other includes */
#include "ob_version.h"
#include "ob_debug.h"
#include "netconf.h"
#include "conf_alarm.h"
#include "conf_port.h"

#if MODULE_OBNMS
#include "nms_if.h"
#endif
#if MODULE_RING
#include "ob_ring.h"
#endif

#if MODULE_UART_SERVER
#include "uart_server.h"
#endif

#include "sig_srv.h"

#define VectTab_Offset 0x10000
char FirmareVersion[] = OB_FIRMWARE_VERSION;
unsigned char DevMac[6];
unsigned char gHwMinorVersion=0;
static unsigned char DefaultDevMac[6] = {0x0c, 0xa4, 0x2a, 0x00, 0x00, 0x01};


/*  GE20023MA hw version setting
    +==========================================================================================+
    +  Major version   |   Minor version   |              Board Description                    +
    +   ( HW[8:6] )    |    ( HW[5:1] )    |                                                   +
    +==========================================================================================+
    +       110        |   HW[5:4] = 00    |   2 SFP + 3 RJ45                                  +
    +                  |-----------------------------------------------------------------------+
    +                  |   HW[5:4] = 01    |   1 SFP + 4 RJ45                                  +
    +                  |-----------------------------------------------------------------------+
    +                  |   HW[5:4] = 10    |   1 SFP + 2 RJ45                                  +
    +                  |-----------------------------------------------------------------------+
    +                  |   HW[5:4] = 11    |   1 SFP + 1 RJ45                                  +
    +                  |-----------------------------------------------------------------------+
    +                  |        HW3        |   RJ45(1#2#   fuction), 0: RS485; 1: K input      +
    +                  |-----------------------------------------------------------------------+
    +                  |        HW2        |   RJ45(3#4#5# fuction), 0: RS485; 1: RS232        +
    +                  |-----------------------------------------------------------------------+
    +                  |        HW1        |   RJ45(6#7#8# fuction), 0: RS232; 1: K output     +   
    +==========================================================================================+
*/

void MinorVerGpioInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* HW1 PC13 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	GPIO_Init(GPIOC, &GPIO_InitStructure);  

	/* HW2 PE6 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	GPIO_Init(GPIOE, &GPIO_InitStructure);  

	/* HW3 PE5 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	GPIO_Init(GPIOE, &GPIO_InitStructure);  

	/* HW4 PE4 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	GPIO_Init(GPIOE, &GPIO_InitStructure);  	


	/* HW5 PE3 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	GPIO_Init(GPIOE, &GPIO_InitStructure);  		
}


unsigned char ReadMinorVersion(void)
{
	u8 MinVer=0;
	
	if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_SET) {
		MinVer |= 1<<0;
	}
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_6) == Bit_SET) {
		MinVer |= 1<<1;
	}		
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5) == Bit_SET) {
		MinVer |= 1<<2;
	}
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == Bit_SET) {
		MinVer |= 1<<3;
	}
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) == Bit_SET) {
		MinVer |= 1<<4;
	}

	return MinVer;
}


void AppBanner(void)
{
	printf("\r\n");
	printf("  **********************************************\r\n");
	printf("  **           Welcome to OBNetOS             **\r\n");
	printf("  **  -- -- -- -- -- -- -- -- -- -- -- -- --  **\r\n");
	printf("  **   Firmware version release: v%s     **\r\n", &FirmareVersion[0]);
	printf("  **   Copyright (c) OB Telecom Electronics   **\r\n");
	printf("  **********************************************\r\n");	
	printf("\r\n");
}

void main(void) 
{  
	const u8 temp_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	
	/* Set Vector Table */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, VectTab_Offset);

	/* Configure ethernet (GPIOs, clocks, MAC, DMA) */
	ETH_BSP_Config();
	
	/* Configure console, and display the welcome banner */
#if BOARD_GE20023MA
	ConsoleInit(COM_UART5, 115200);
#endif
	AppBanner();  

	MinorVerGpioInit();
	gHwMinorVersion = ReadMinorVersion();
 	printf("Hardware minor version is 0x%02x\r\n", gHwMinorVersion);

	/* Configure Timer  */
	TimerInit();

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
	RoboSwitch_Init(gHwMinorVersion);
#elif SWITCH_CHIP_88E6095
	swif_Initialize();
#endif

	conf_port_base_init();

	/* Module debug init */
	ob_debug_init();

#if MODULE_RING		
	/* Ring Initialize */
	Ring_Initialize();
#endif

	/* Initilaize the LwIP stack */
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
	
#if MODULE_OBNMS
	xTaskCreate(NMS_Task,	"tNMS", 	configMINIMAL_STACK_SIZE*2, NULL,	tskIDLE_PRIORITY + 2, NULL);
#endif

#if MODULE_UART_SERVER
	UartServerStart();
#endif	
#if MODULE_SIGNALE    
    xTaskCreate(Signal_Task,	"tSIGNAL", 	configMINIMAL_STACK_SIZE*2, NULL,	tskIDLE_PRIORITY + 2, NULL);
#endif	
	/* Start the scheduler. */
	vTaskStartScheduler();

	for( ;; );
}


void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
	printf( "Assert failed: file %s, line %u\r\n", pcFile, ulLine );
}

