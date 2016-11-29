/*******************************************************************************
  * @file    main.c
  * @author  OB T&C Development Team, HeJianguo
  * @brief   Main program body
  ******************************************************************************/

#include "mconfig.h"

#include "svn_revision.h"
/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* BSP includes */
#include "stm32f2xx.h"
#include "misc_drv.h"
#include "console.h"
#include "timer.h"
#include "soft_i2c.h"
#include "stm32f2x7_eth_bsp.h"
#include "led_drv.h"
#include "rs_uart.h"
#include "drvtest.h"

/* Other includes */
#include "netconf.h"
#include "conf_global.h"

#if MODULE_OBNMS
#include "nms_if.h"
#endif
#if MODULE_RING
#include "ob_ring.h"
#endif

#if MODULE_UART_SERVER
#include "uart_server.h"
#endif	/* MODULE_UART_SERVER */

#if MODULE_SIGNAL
#include "sig_srv.h"
#endif /* MODULE_SIGNAL */

#if MODULE_SNMP_TRAP  
#include "private_trap.h"
#endif /* MODULE_SNMP_TRAP */

#if BOARD_GV3S_HONUE_QM
#include "fpga_task.h"
#include "halcmd_msg.h"
#include "tsensor.h"
#endif /* BOARD_GV3S_HONUE_QM */

#include "hal_swif.h"
#include "obring.h"

#define VectTab_Offset 0x10000
char FirmareVersion[16] = {0};
unsigned char DevMac[6] = {0};
static unsigned char DefaultDevMac[6] = {0x0c, 0xa4, 0x2a, 0x00, 0x00, 0x01};

void AppBanner(void)
{
#if RELEASE_TRUNK_VERSION
	sprintf(FirmareVersion, "%d.%d.%d.%d", 
		(FIRMWARE_VERSION & 0xF000)>>12, 
		(FIRMWARE_VERSION & 0x0F00)>>8, 
		(FIRMWARE_VERSION & 0x00F0)>>4, 
		 FIRMWARE_VERSION & 0x000F);
#endif	
	printf("\r\n");
	printf("  ******************************************\r\n");
	printf("  **          Welcome to OBNetOS          **\r\n");
	printf("  **   -- -- -- -- -- -- -- -- -- -- --   **\r\n");
#if RELEASE_TRUNK_VERSION
	printf("  **   Version release : v%s r%-5d  **\r\n", &FirmareVersion[0], SVN_REVISION);
#else
	printf("  **   Revision release: r%-5d           **\r\n", SVN_REVISION);
#endif
	printf("  **   Build time: %s    **\r\n", BUILD_TIME);
	printf("  ******************************************\r\n");
	printf("\r\n");
}

void main(void) 
{
	const u8 temp_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	extern tConsoleDev *pConsoleDev;
	extern void cli_main(void);

	/* Set Vector Table */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, VectTab_Offset);

	/* Configure Timer  */
	TimerInit();
#if BOARD_GV3S_HONUE_QM
    dev_reset();
    HalQueueInit();
    T_Adc_Init();
#endif /* BOARD_GV3S_HONUE_QM */
    
	/* Early initialize */
	board_early_initialize();

	/* Configure ethernet (GPIOs, clocks, MAC, DMA) */
	ETH_BSP_Config();

	/* Configure console, and display the welcome banner */
	ConsoleInit(pConsoleDev->ComPort, 115200);
	AppBanner();
	
	/* Configure I2C GPIOs, and load device MAC address  */
	I2C_GPIO_Config();
	if(I2C_Read(DevMac, 6, EPROM_ADDR_MAC, EEPROM_SLAVE_ADDR) != I2C_SUCCESS) {
		memcpy(DevMac, DefaultDevMac, 6);
	} else {
		if(memcmp(DevMac, temp_mac, 6) == 0)
			memcpy(DevMac, DefaultDevMac, 6);
	}

    /* GPIO configuration */
	misc_initialize();
	
	/* Configure LED GPIOs */
	LED_GPIO_config();

	/* Configure Switch chip */
	hal_swif_init();

#if (BOARD_FEATURE & L2_OBRING)		
	/* Ring Initialize */
	obring_initialize();
#endif

	/* Initialize configuration */
	hal_swif_conf_initialize();

	/* Initialize the LwIP stack */
	LwIP_Init();

	/* Start the CLI */
	cli_main();
#if BOARD_GE220044MD
	xTaskCreate(RUN_LED_Task, 			"Run_tLED",		configMINIMAL_STACK_SIZE*1, NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(RING_LED_Task, 			"Ring_tLED",	configMINIMAL_STACK_SIZE*1, NULL, tskIDLE_PRIORITY + 1, NULL);
#else/* NOT BOARD_GE220044MD */
	xTaskCreate(LED_Task, 				"tLED",		configMINIMAL_STACK_SIZE*1, NULL, tskIDLE_PRIORITY + 1, NULL);
#endif
	xTaskCreate(hal_swif_poll_task, 	"tSwPoll",	configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 2, NULL);	

#if (BOARD_GE1040PU || BOARD_GE204P0U)
	xTaskCreate(combo_led_control_task,	"tCombo",	configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 2, NULL);
#endif
	hal_swif_traffic_entry();
	
#if (BOARD_FEATURE & L2_OBRING)	
	//Ring_Start();
#endif
	
#if MODULE_OBNMS
	xTaskCreate(NMS_Task,	"tNMS", 	configMINIMAL_STACK_SIZE*3, NULL,	tskIDLE_PRIORITY + 6, NULL);
#endif

#if MODULE_UART_SERVER && MODULE_RS485
#error USART function both enable
#elif MODULE_UART_SERVER
	UartServerStart();
#elif MODULE_RS485
	UartStart();
#endif /* MODULE_UART_SERVER && MODULE_RS485 */
    
#if BOARD_GV3S_HONUE_QM
    fpga_task_init();	
#endif
    
#if MODULE_SIGNAL
    SignalTaskInit();
#endif
    
#if MODULE_IWDG
    iwdg_task_init();
#endif

#if MODULE_SNMP_TRAP    
    SendTrapTaskInit();
#endif
    
#if MODULE_UDP_TCP_ECHO
    udpecho_init();
    tcpecho_init();
#endif
    
    //drv_test_init();

	/* Start the scheduler. */
	vTaskStartScheduler();

	for( ;; );
}

void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
	printf( "Assert failed: file %s, line %u\r\n", pcFile, ulLine );
}

