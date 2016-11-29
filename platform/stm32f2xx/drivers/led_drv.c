
#include "mconfig.h"

/* Standard includes. */
#include "stdio.h"

#if defined(OS_FREERTOS)
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif

/* BSP include */
#include "stm32f2xx.h"
#include "led_drv.h"

#include "obring.h"

u8 portA_blink_flag;
u8 portB_blink_flag;
eBlinkRate RingLED_BlinkMode;

void LED_GPIO_config(void)
{
#if BOARD_GE22103MA
	GPIO_InitTypeDef  GPIO_InitStructure;
	extern u8 gHardwareVer87;
    extern u16 gHardwareAddr;

    if(gHardwareAddr == 0x007F){
        if(gHardwareVer87 != 0x03){
            /* RUN_LED, PB9 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOB, &GPIO_InitStructure);
    
            /* D_LED1, D_LED2 : PE0,PE1 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOE, &GPIO_InitStructure);
    
            /* D_LED3, D_LED4 : PC10,PC11 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
    
            /* Initialize the LED state, LED off */
            GPIOE->BSRRL = GPIO_Pin_0;
            GPIOE->BSRRL = GPIO_Pin_1;
            GPIOC->BSRRL = GPIO_Pin_10;
            GPIOC->BSRRL = GPIO_Pin_11;
        } else {
            /* TEST_LED, PB8 for LED D3 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOB, &GPIO_InitStructure);
    
            /* RUN_LED, PE9 for LED D15 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOE, &GPIO_InitStructure);  
        }
    }else{
        /* RUN_LED, PB8 for LED10 */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        
        /* Ring_LED, PB9 for LED9 */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    
        /* Initialize the LED state, LED off */
        GPIOB->BSRRL = GPIO_Pin_8;
        GPIOB->BSRRL = GPIO_Pin_9;
    }

#elif BOARD_GE20023MA
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* PB15 for test LED2 D6 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* PB14 for test LED1 D7 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* PB9 for RUN LED */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* PE0 for Port6(RJ45) D_LED1 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	/* PE1 for Port6(RJ45) D_LED2 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	/* Initialize the LED state, LED off */
	GPIOB->BSRRL = GPIO_Pin_9;
	GPIOB->BSRRL = GPIO_Pin_14;
	GPIOB->BSRRL = GPIO_Pin_15;

	GPIOE->BSRRL = GPIO_Pin_0;
	GPIOE->BSRRL = GPIO_Pin_1;
#elif BOARD_GE11014MA
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* RUN_LED, PB8 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#elif BOARD_GE2C400U
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* RUN_LED, PB8 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	/* PD4 for M_LED1 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* PA11 for M_LED2 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	/* Initialize the LED state, LED off */
	GPIOD->BSRRL = GPIO_Pin_4;
	GPIOA->BSRRL = GPIO_Pin_11;	

#elif BOARD_GE1040PU
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* RUN_LED, PB8 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	/* NET_LED, PD12 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIOD->BSRRH = GPIO_Pin_12;	/* NET LED On */
	
#elif BOARD_GE204P0U
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* RUN_LED, PB8 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
    
#elif BOARD_GV3S_HONUE_QM
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    /* PD14 for system LED */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
    
#elif BOARD_GE11500MD
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    /* PB9 for system LED */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#elif BOARD_GE_EXT_22002EA
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* RUN_LED, PB8 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Ring_LED, PB9 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Initialize the LED state, LED off */
    GPIOB->BSRRL = GPIO_Pin_8;
    GPIOB->BSRRL = GPIO_Pin_9;

#elif BOARD_GE220044MD
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* RUN_LED, PB8 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Ring_LED, PB9 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Initialize the LED state, LED off */
    GPIOB->BSRRL = GPIO_Pin_8;
    GPIOB->BSRRL = GPIO_Pin_9;

#else
#error system running led config unkown
#endif

	portA_blink_flag = 0;
	portB_blink_flag = 0;
	RingLED_BlinkMode = BLINK_1HZ;
}

void Test1_LED_On(void)
{
#if BOARD_GE2C400U
	GPIOD->BSRRH = GPIO_Pin_4;
#endif	
}

void Test1_LED_Off(void)
{
#if BOARD_GE2C400U
	GPIOD->BSRRL = GPIO_Pin_4;
#endif	
}

void Test2_LED_On(void)
{
#if BOARD_GE2C400U
	GPIOA->BSRRH = GPIO_Pin_11;
#endif	
}

void Test2_LED_Off(void)
{
#if BOARD_GE2C400U
	GPIOA->BSRRL = GPIO_Pin_11;
#endif	
}

void PortA_LED_On(void)
{
#if BOARD_GE20023MA
	/* if portA linkup and forwarding, LED D6 on */
	portA_blink_flag = 0;
	GPIOB->BSRRH = GPIO_Pin_15;
#endif	
}

void PortA_LED_Off(void)
{
#if BOARD_GE20023MA
	/* if portA linkdown, LED D6 off */
	portA_blink_flag = 0;
	GPIOB->BSRRL = GPIO_Pin_15;
#endif	
}

void PortA_LED_Blink(void)
{
#if BOARD_GE20023MA
	/* if portA linkup and blocking */
	portA_blink_flag = 1;
#endif
}

void PortB_LED_On(void)
{
#if BOARD_GE20023MA
	/* if portB linkup and forwarding, LED D7 on */
	portB_blink_flag = 0;
	GPIOB->BSRRH = GPIO_Pin_14;
#endif	
}

void PortB_LED_Off(void)
{
#if BOARD_GE20023MA
	/* if portB linkdown, LED D7 off */
	portB_blink_flag = 0;
	GPIOB->BSRRL = GPIO_Pin_14;
#endif	
}

void PortB_LED_Blink(void)
{
#if BOARD_GE20023MA
	/* if portB linkup and blocking */
	portB_blink_flag = 1;
#endif
}

void Set_RingLED_BlinkMode(eBlinkRate rate)
{
	RingLED_BlinkMode = rate;
}

#if defined(OS_FREERTOS)
#if BOARD_GE220044MD
void RUN_LED_Task(void *arg)
{

	for( ;; ) {
		GPIO_ToggleBits(GPIOB, GPIO_Pin_8);
		
		vTaskDelay(1000);
	}
}

void RING_LED_Task(void *arg)
{

	for( ;; ) {
		
		//GPIO_ToggleBits(GPIOB, GPIO_Pin_8);
		if(1 == obring_check_enable(0x00)){
			/* Ring_LED */
            GPIO_ToggleBits(GPIOB, GPIO_Pin_9);
		}

		if(RingLED_BlinkMode == BLINK_1HZ)
			vTaskDelay(1000);
		else if(RingLED_BlinkMode == BLINK_2HZ)
			vTaskDelay(250);
		else if(RingLED_BlinkMode == BLINK_5HZ)
			vTaskDelay(100);
		else if(RingLED_BlinkMode == BLINK_10HZ)
			vTaskDelay(50);
		else
			vTaskDelay(1000);
	}
}
#else
void LED_Task(void *arg)
{
#if BOARD_GE22103MA
	extern u8 gHardwareVer87;
    extern u16 gHardwareAddr;
#endif

	for( ;; ) {
#if BOARD_GE20023MA	
		if(portA_blink_flag)
			GPIO_ToggleBits(GPIOB, GPIO_Pin_15);
		if(portB_blink_flag)
			GPIO_ToggleBits(GPIOB, GPIO_Pin_14);	
#endif

#if BOARD_GE22103MA
        if(gHardwareAddr == 0x007F){
            		
            if(gHardwareVer87 != 0x03)
                GPIO_ToggleBits(GPIOB, GPIO_Pin_9);	
            else
                GPIO_ToggleBits(GPIOE, GPIO_Pin_9);	
        }else{
            /* RUN_LED */
            GPIO_ToggleBits(GPIOB, GPIO_Pin_8);
            if(1 == obring_check_enable(0x00))
            {            
                /* Ring_LED */
                GPIO_ToggleBits(GPIOB, GPIO_Pin_9);
            }
        }
		/* K_Sig_In test */ 
		if((u8)((GPIOD->IDR & GPIO_Pin_11) >> 11) == 0)
			GPIOE->BSRRH = GPIO_Pin_1;	/* LED on */
		else
			GPIOE->BSRRL = GPIO_Pin_1;	/* LED off */
#elif BOARD_GE11014MA
        GPIO_ToggleBits(GPIOB, GPIO_Pin_8);
#elif BOARD_GE2C400U
		GPIO_ToggleBits(GPIOB, GPIO_Pin_8);
#elif BOARD_GE1040PU
		GPIO_ToggleBits(GPIOB, GPIO_Pin_8);	
#elif BOARD_GE204P0U
		GPIO_ToggleBits(GPIOB, GPIO_Pin_8);	
#elif BOARD_GE20023MA
		GPIO_ToggleBits(GPIOB, GPIO_Pin_9);	
#elif BOARD_GV3S_HONUE_QM
		GPIO_ToggleBits(GPIOD, GPIO_Pin_14);	
#elif BOARD_GE11500MD
	    GPIO_ToggleBits(GPIOB, GPIO_Pin_9);		
#elif BOARD_GE_EXT_22002EA
        GPIO_ToggleBits(GPIOB, GPIO_Pin_8);
        if(1 == obring_check_enable(0x00)){            
            /* Ring_LED */
            GPIO_ToggleBits(GPIOB, GPIO_Pin_9);
        }
#elif BOARD_GE220044MD
		GPIO_ToggleBits(GPIOB, GPIO_Pin_8);
		if(1 == obring_check_enable(0x00)){
			/* Ring_LED */
            GPIO_ToggleBits(GPIOB, GPIO_Pin_9);
		}
#endif

		if(RingLED_BlinkMode == BLINK_1HZ)
			vTaskDelay(1000);
		else if(RingLED_BlinkMode == BLINK_2HZ)
			vTaskDelay(250);
		else if(RingLED_BlinkMode == BLINK_5HZ)
			vTaskDelay(100);
		else if(RingLED_BlinkMode == BLINK_10HZ)
			vTaskDelay(50);
		else
			vTaskDelay(1000);
	}
}
#endif /*END IF BOARD_GE220044MD*/
#endif


