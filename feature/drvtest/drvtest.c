/*************************************************************
 * Filename     : drvtest.c
 * Description  : 
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/
/* include -------------------------------------------------------------------*/
#include "mconfig.h"
#include "drvtest.h"
#include "led_drv.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
/* BSP include */
#include "stm32f2xx.h"
#include "robo_drv.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

int robo_stress_test(u32 loop_count)
{
#if BOARD_GE2C400U
	u8	hport;
	u32 i, phyid;
	u16 phyid1;

	Test1_LED_Off();
	Test2_LED_Off();
	
	if(loop_count > 0) {
		for(i=0; i<loop_count; i++) {
			for(hport=0; hport<16; hport++) {
				if(robo_read(0x80+hport, 0x04, (u8 *)&phyid1, 2) != 0) {
					Test1_LED_On();
					return -1;
				}
				if(phyid1 != 0x0362){
					return -2;
					Test2_LED_On();
				}
			}
		}
	} else {
		while(1) {
			for(hport=0; hport<16; hport++) {
				if(robo_read(0x80+hport, 0x04, (u8 *)&phyid1, 2) != 0) {
					Test1_LED_On();
					return -1;
				}
				if(phyid1 != 0x0362){
					Test2_LED_On();
                    return -2;
				}
                phyid1 = 0;
			}
		}
	}

	return 0;
#endif
	return 0;
}

void drv_test(void * pvParameters)
{
    robo_stress_test(0);
    vTaskDelete( NULL );
    for(;;);
}

void drv_test_init(void)
{    
    xTaskCreate(drv_test, "drv_test", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
}
