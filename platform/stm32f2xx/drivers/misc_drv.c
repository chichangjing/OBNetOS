

/********************************************************************************
  * @file    misc_drv.c
  * @author  OB networks
  * @brief   MISC driver
  *******************************************************************************/ 

#include "mconfig.h"

/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "os_mutex.h"

/* BSP include */
#include "stm32f2xx.h"
#include "stm32f2x7_smi.h"
#include "misc_drv.h"
#include "console.h"

/* HAL for L2 includes */
#include "hal_swif_port.h"

dev_base_info_t	DeviceBaseInfo;
xSemaphoreHandle EXTI9_5_Semaphore = NULL;

#if BOARD_GE22103MA
u8 gHardwareVer87 = 0;
u16 gHardwareAddr = 0;
#elif BOARD_GE20023MA
u8 gHardwareVer = 0;
#endif

int get_hardware_version(u16 *hardware_version)
{
	int ret = 0;
	
#if BOARD_GE22103MA
	GPIO_InitTypeDef  GPIO_InitStructure;
	u8 hwv_config_data;
	u16 first_version, second_version, third_version;
	
	extern hal_port_map_t gHalPortMap[];
	
    /*----------------------------------------------------------------------------------------------------
    | HWV[8:7]         |  HWV[6]    |  HWV[5:4]     |  HWV[3]       |  HWV[2]     | HWV[1:0]              | 
    | Hardware_Version |  SFP       |  RJ45_Ctrl    |  Ext-Board    |  Data       | Ext_Board_Version     |	
     -----------------------------------------------------------------------------------------------------
    | 00: v1.30        |  1: 2*SFP  |  00: 0+4      |  0: 4*RJ45    |  0: Data    | 00: v1.10             |
    | 01: Res          |  0: 1*SFP  |  01: Res      |  1: No Ext    |  1: No Data | 01: Res               |
    | 10: Res          |            |  10: 1+0      |               |             | 10: Res               |
    | 11: default      |            |  11: 1+3      |               |             | 11: Res               |
     ----------------------------------------------------------------------------------------------------*/

    /*----------------------------------------------------------------------------------------------------
    |     HWV[6:2]     |  MinorVer  |                 Board Name Description                              | 
     -----------------------------------------------------------------------------------------------------
    |      0x4C        |     0      | 2 * SFP + 4 *  100M-RJ45                                            |
    |      0x6C        |     1      | 2 * SFP + 1 * 1000M-RJ45                                            |
    |      0x7C        |     2      | 2 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45                            |
    |      0x74        |     3      | 2 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45 + 4 * 100M-ExtRJ45         |
    |      0x70        |     4      | 2 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45 + 4 * 100M-ExtRJ45 + Data  |
    |      0x0C        |     5      | 1 * SFP + 4 *  100M-RJ45                                            |
    |      0x2C        |     6      | 1 * SFP + 1 * 1000M-RJ45                                            |
    |      0x3C        |     7      | 1 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45                            |
    |      0x34        |     8      | 1 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45 + 4 * 100M-ExtRJ45         |
    |      0x30        |     9      | 1 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45 + 4 * 100M-ExtRJ45 + Data  |
     ----------------------------------------------------------------------------------------------------*/	 

	
	
	/*=======================================================================================================
						For GE22103MA PCB v1.20, v1.30 
	  =======================================================================================================*/
	printf("Board Type Description     : ");
	
    if(gHardwareAddr == 0x007F){

        if(gHardwareVer87 == 0x00) {
            first_version = 1;
            second_version = 3;
            /* HWV0 = PD1 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            
            /* HWV6 = PA8 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
            GPIO_Init(GPIOA, &GPIO_InitStructure);
            
            /* HWV5 = PC13 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
    
            /* HWV4, HWV3, HWV2, HWV1 = PE6, PE5, PE4, PE3 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_3;
            GPIO_Init(GPIOE, &GPIO_InitStructure);
    
            hwv_config_data = (u8)((GPIOD->IDR & GPIO_Pin_1) >> 1);				/* HWV0 */
            hwv_config_data |= (u8)(((GPIOE->IDR & GPIO_Pin_3) >> 3) << 1);		/* HWV1 */
            hwv_config_data |= (u8)(((GPIOE->IDR & GPIO_Pin_4) >> 4) << 2);		/* HWV2 */
            hwv_config_data |= (u8)(((GPIOE->IDR & GPIO_Pin_5) >> 5) << 3);		/* HWV3 */
            hwv_config_data |= (u8)(((GPIOE->IDR & GPIO_Pin_6) >> 6) << 4);		/* HWV4 */
            hwv_config_data |= (u8)(((GPIOC->IDR & GPIO_Pin_13) >> 13) << 5);	/* HWV5 */
            hwv_config_data |= (u8)(((GPIOA->IDR & GPIO_Pin_8) >> 8) << 6);		/* HWV6 */
    
            if(hwv_config_data & 0x40) {
                gHalPortMap[0].lport = 1;
                gHalPortMap[0].hport = 10;
                gHalPortMap[0].port_type = S1000M_OPTICAL;
                gHalPortMap[1].lport = 2;
                gHalPortMap[1].hport = 9;
                gHalPortMap[1].port_type = S1000M_OPTICAL;
                switch(hwv_config_data & 0x3C) {
                    case 0x0C:												/* cfg = 0x4C, 2 * SFP + 4 * 100M-RJ45 */	
                    gHalPortMap[2].lport = 3;
                    gHalPortMap[2].hport = 4;
                    gHalPortMap[2].port_type = S100M_CABLE;
                    gHalPortMap[3].lport = 4;
                    gHalPortMap[3].hport = 6;
                    gHalPortMap[3].port_type = S100M_CABLE;
                    gHalPortMap[4].lport = 5;
                    gHalPortMap[4].hport = 5;
                    gHalPortMap[4].port_type = S100M_CABLE;
                    gHalPortMap[5].lport = 6;
                    gHalPortMap[5].hport = 4;
                    gHalPortMap[5].port_type = S100M_CABLE;	
                    DeviceBaseInfo.PortNum = 6;
                    third_version = 0;
                    printf("2SFP(0+4)RJ45\r\n");
                    break;
    
                    case 0x2C:												/* cfg = 0x6C, 2 * SFP + 1 * 1000M-RJ45 */
                    gHalPortMap[2].lport = 3;
                    gHalPortMap[2].hport = 8;
                    gHalPortMap[2].port_type = S1000M_CABLE;
                    DeviceBaseInfo.PortNum = 3;
                    third_version = 1;
                    printf("2SFP(1+0)RJ45\r\n");
                    break;
    
                    case 0x3C:												/* cfg = 0x7C, 2 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45 */
                    gHalPortMap[2].lport = 3;
                    gHalPortMap[2].hport = 8;
                    gHalPortMap[2].port_type = S1000M_CABLE;
                    gHalPortMap[3].lport = 4;
                    gHalPortMap[3].hport = 7;
                    gHalPortMap[3].port_type = S100M_CABLE;
                    gHalPortMap[4].lport = 5;
                    gHalPortMap[4].hport = 6;
                    gHalPortMap[4].port_type = S100M_CABLE;
                    gHalPortMap[5].lport = 6;
                    gHalPortMap[5].hport = 5;
                    gHalPortMap[5].port_type = S100M_CABLE;
                    DeviceBaseInfo.PortNum = 6;
                    third_version = 2;
                    printf("2SFP(1+3)RJ45\r\n");
                    break;
    
                    case 0x34:												/* cfg = 0x74, 2 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45 + 4 * Ext-100M-RJ45 */
                    case 0x30:												/* cfg = 0x70, 2 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45 + 4 * Ext-100M-RJ45 + Data */
                    gHalPortMap[2].lport = 3;
                    gHalPortMap[2].hport = 8;
                    gHalPortMap[2].port_type = S1000M_CABLE;
                    gHalPortMap[3].lport = 4;
                    gHalPortMap[3].hport = 7;
                    gHalPortMap[3].port_type = S100M_CABLE;
                    gHalPortMap[4].lport = 5;
                    gHalPortMap[4].hport = 6;
                    gHalPortMap[4].port_type = S100M_CABLE;
                    gHalPortMap[5].lport = 6;
                    gHalPortMap[5].hport = 5;
                    gHalPortMap[5].port_type = S100M_CABLE;
                    
                    gHalPortMap[6].lport = 7;
                    gHalPortMap[6].hport = 0;
                    gHalPortMap[6].port_type = S100M_CABLE;
                    gHalPortMap[7].lport = 8;
                    gHalPortMap[7].hport = 1;
                    gHalPortMap[7].port_type = S100M_CABLE;
                    gHalPortMap[8].lport = 9;
                    gHalPortMap[8].hport = 3;
                    gHalPortMap[8].port_type = S100M_CABLE;
                    gHalPortMap[9].lport = 10;
                    gHalPortMap[9].hport = 4;
                    gHalPortMap[9].port_type = S100M_CABLE;	
                    DeviceBaseInfo.PortNum = 10;
                    if(hwv_config_data & 0x7C == 0x74)
                        third_version = 3;
                    else
                        third_version = 4;
                    printf("2SFP(1+7)RJ45\r\n");
                    break;
    
                    default:
                    break;
                }
            } else {
                gHalPortMap[0].lport = 1;
                gHalPortMap[0].hport = 10;
                gHalPortMap[0].port_type = S1000M_OPTICAL;
                switch(hwv_config_data & 0x3C) {
                    case 0x0C:												/* cfg = 0x0C, 1 * SFP + 4 * 100M-RJ45 */
                    gHalPortMap[1].lport = 2;
                    gHalPortMap[1].hport = 4;
                    gHalPortMap[1].port_type = S100M_CABLE;
                    gHalPortMap[2].lport = 3;
                    gHalPortMap[2].hport = 6;
                    gHalPortMap[2].port_type = S100M_CABLE;
                    gHalPortMap[3].lport = 4;
                    gHalPortMap[3].hport = 5;
                    gHalPortMap[3].port_type = S100M_CABLE;
                    gHalPortMap[4].lport = 5;
                    gHalPortMap[4].hport = 4;
                    gHalPortMap[4].port_type = S100M_CABLE;	
                    DeviceBaseInfo.PortNum = 5;
                    third_version = 5;
                    printf("1SFP(0+4)RJ45\r\n");
                    break;
    
                    case 0x2C:												/* cfg = 0x2C, 1 * SFP + 1 * 1000M-RJ45 */
                    gHalPortMap[1].lport = 2;
                    gHalPortMap[1].hport = 8;
                    gHalPortMap[1].port_type = S1000M_CABLE;
                    DeviceBaseInfo.PortNum = 2;
                    third_version = 6;
                    printf("1SFP(1+0)RJ45\r\n");
                    break;
    
                    case 0x3C:												/* cfg = 0x3C, 1 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45 */	
                    gHalPortMap[1].lport = 2;
                    gHalPortMap[1].hport = 8;
                    gHalPortMap[1].port_type = S1000M_CABLE;
                    gHalPortMap[2].lport = 3;
                    gHalPortMap[2].hport = 7;
                    gHalPortMap[2].port_type = S100M_CABLE;
                    gHalPortMap[3].lport = 4;
                    gHalPortMap[3].hport = 6;
                    gHalPortMap[3].port_type = S100M_CABLE;
                    gHalPortMap[4].lport = 5;
                    gHalPortMap[4].hport = 5;
                    gHalPortMap[4].port_type = S100M_CABLE;
                    DeviceBaseInfo.PortNum = 5;
                    third_version = 7;
                    printf("1SFP(1+3)RJ45\r\n");
                    break;
    
                    case 0x34:												/* cfg = 0x34, 1 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45 + 4 * Ext-100M-RJ45 */
                    case 0x30:												/* cfg = 0x30, 1 * SFP + 1 * 1000M-RJ45 + 3 * 100M-RJ45 + 4 * Ext-100M-RJ45 + Data */
                    gHalPortMap[1].lport = 2;
                    gHalPortMap[1].hport = 8;
                    gHalPortMap[1].port_type = S1000M_CABLE;
                    gHalPortMap[2].lport = 3;
                    gHalPortMap[2].hport = 7;
                    gHalPortMap[2].port_type = S100M_CABLE;
                    gHalPortMap[3].lport = 4;
                    gHalPortMap[3].hport = 6;
                    gHalPortMap[3].port_type = S100M_CABLE;
                    gHalPortMap[4].lport = 5;
                    gHalPortMap[4].hport = 5;
                    gHalPortMap[4].port_type = S100M_CABLE;
                    
                    gHalPortMap[5].lport = 6;
                    gHalPortMap[5].hport = 0;
                    gHalPortMap[5].port_type = S100M_CABLE;
                    gHalPortMap[6].lport = 7;
                    gHalPortMap[6].hport = 1;
                    gHalPortMap[6].port_type = S100M_CABLE;
                    gHalPortMap[7].lport = 8;
                    gHalPortMap[7].hport = 3;
                    gHalPortMap[7].port_type = S100M_CABLE;
                    gHalPortMap[8].lport = 9;
                    gHalPortMap[8].hport = 4;
                    gHalPortMap[8].port_type = S100M_CABLE;	
                    DeviceBaseInfo.PortNum = 9;
                    if(hwv_config_data & 0x7C == 0x34)
                        third_version = 8;
                    else
                        third_version = 9;
                    printf("1SFP(1+7)RJ45\r\n");
                    break;
    
                    default:
                    break;
                }
            }
        } else if(gHardwareVer87 == 0x03) {
        /*=======================================================================================================
                            For GE22103MA PCB v1.10, GE2A500U
          =======================================================================================================*/
            first_version = 1;
            second_version = 1;
            third_version = 0;
            
            gHalPortMap[0].lport = 1;
            gHalPortMap[0].hport = 10;
            gHalPortMap[0].port_type = S1000M_OPTICAL;
            gHalPortMap[1].lport = 2;
            gHalPortMap[1].hport = 9;
            gHalPortMap[1].port_type = S1000M_OPTICAL;
            gHalPortMap[2].lport = 3;
            gHalPortMap[2].hport = 8;
            gHalPortMap[2].port_type = S1000M_CABLE;
            gHalPortMap[3].lport = 4;
            gHalPortMap[3].hport = 7;
            gHalPortMap[3].port_type = S100M_CABLE;
            gHalPortMap[4].lport = 5;
            gHalPortMap[4].hport = 6;
            gHalPortMap[4].port_type = S100M_CABLE;
            gHalPortMap[5].lport = 6;
            gHalPortMap[5].hport = 5;
            gHalPortMap[5].port_type = S100M_CABLE;	
            
            gHalPortMap[6].lport = 7;
            gHalPortMap[6].hport = 0;
            gHalPortMap[6].port_type = S100M_CABLE;
            gHalPortMap[7].lport = 8;
            gHalPortMap[7].hport = 1;
            gHalPortMap[7].port_type = S100M_CABLE;
            gHalPortMap[8].lport = 9;
            gHalPortMap[8].hport = 3;
            gHalPortMap[8].port_type = S100M_CABLE;
            gHalPortMap[9].lport = 10;
            gHalPortMap[9].hport = 4;
            gHalPortMap[9].port_type = S100M_CABLE;	
            
            DeviceBaseInfo.PortNum = 10;
    
            printf("2SFP(1+7)RJ45\r\n");
        } else {
            printf("Error: Invalid PCB Version!\r\n");
            first_version = 0;
            second_version = 0;
            third_version = 0;
            ret = -1;
        }

    }else if(gHardwareAddr == 0x0000){
        /*=======================================================================================================
                            For GE2A500U PCB v1.11
          =======================================================================================================*/
 
        first_version = 1;
        second_version = 1;
        third_version = 1;
        
        gHalPortMap[0].lport = 1;
        gHalPortMap[0].hport = 9;
        gHalPortMap[0].port_type = S1000M_OPTICAL;
        gHalPortMap[1].lport = 2;
        gHalPortMap[1].hport = 10;
        gHalPortMap[1].port_type = S1000M_OPTICAL;
        gHalPortMap[2].lport = 3;
        gHalPortMap[2].hport = 8;
        gHalPortMap[2].port_type = S1000M_CABLE;
        
        DeviceBaseInfo.PortNum = 3;
        printf("2SFP(1+0)RJ45\r\n");
        
    }else{
        /*=======================================================================================================
                            For GE2A005U PCB v1.00
          =======================================================================================================*/
 
        first_version = 1;
        second_version = 0;
        third_version = 0;
        
        gHalPortMap[0].lport = 1;
        gHalPortMap[0].hport = 9;
        gHalPortMap[0].port_type = S1000M_OPTICAL;
        gHalPortMap[1].lport = 2;
        gHalPortMap[1].hport = 10;
        gHalPortMap[1].port_type = S1000M_OPTICAL;
        gHalPortMap[2].lport = 3;
        gHalPortMap[2].hport = 7;
        gHalPortMap[2].port_type = S100M_CABLE;
        gHalPortMap[3].lport = 4;
        gHalPortMap[3].hport = 8;
        gHalPortMap[3].port_type = S1000M_CABLE;
        
        DeviceBaseInfo.PortNum = 4;
        printf("2SFP(1+1)RJ45\r\n");
    }
    
	printf("Hardware Version           : v%d.%d.%d\r\n", first_version, second_version, third_version);
	*hardware_version = ((first_version & 0x000F) << 12) | ((second_version & 0x000F) << 8) | (third_version & 0x00FF);
	
    
#elif BOARD_GE20023MA
	GPIO_InitTypeDef  GPIO_InitStructure;
	u8 hwv_config_data;
	u16 first_version, second_version, third_version;
	
	extern hal_port_map_t gHalPortMap[];
	
/** GE20023MA hw version setting
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
    +                  |        HW[3]      |   RJ45(1#2#   fuction), 0: RS485; 1: K input      +
    +                  |-----------------------------------------------------------------------+
    +                  |        HW[2]      |   RJ45(3#4#5# fuction), 0: RS485; 1: RS232        +
    +                  |-----------------------------------------------------------------------+
    +                  |        HW[1]      |   RJ45(6#7#8# fuction), 0: RS232; 1: K output     +   
    +==========================================================================================+
  */
	
	
	/*=======================================================================================================
						For GE20023MA PCB v1.00 
	  =======================================================================================================*/
	printf("Board Type Description     : ");

	/* HW1 PC13 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	GPIO_Init(GPIOC, &GPIO_InitStructure);  

	/* HW2, HW3, HW4, HW5 = PE6, PE5, PE4, PE3 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    hwv_config_data = (u8)((GPIOC->IDR & GPIO_Pin_13) >> 1);			/* HWV1 */
    hwv_config_data |= (u8)(((GPIOE->IDR & GPIO_Pin_6) >> 6) << 1);		/* HWV2 */
    hwv_config_data |= (u8)(((GPIOE->IDR & GPIO_Pin_5) >> 5) << 2);		/* HWV3 */
    hwv_config_data |= (u8)(((GPIOE->IDR & GPIO_Pin_4) >> 4) << 3);		/* HWV4 */
    hwv_config_data |= (u8)(((GPIOE->IDR & GPIO_Pin_3) >> 3) << 4);		/* HWV5 */
    gHardwareVer = hwv_config_data;
    
    first_version = 1;
    second_version = 0;
    third_version = 0;
    switch((hwv_config_data & 0x18) >> 3){
        case 0x00:
            gHalPortMap[0].lport = 1;
            gHalPortMap[0].hport = 0;
            gHalPortMap[0].port_type = S100M_OPTICAL;
            gHalPortMap[1].lport = 2;
            gHalPortMap[1].hport = 1;
            gHalPortMap[1].port_type = S100M_OPTICAL;
            DeviceBaseInfo.PortNum = MAX_PORT_NUM;
            third_version = 0;
            printf("2SFP(0+3)RJ45\r\n");
            break;
        case 0x01:
            gHalPortMap[0].lport = 1;
            gHalPortMap[0].hport = 0;
            gHalPortMap[0].port_type = S100M_OPTICAL;
            gHalPortMap[1].lport = 2;
            gHalPortMap[1].hport = 1;
            gHalPortMap[1].port_type = S100M_CABLE;
            DeviceBaseInfo.PortNum = MAX_PORT_NUM;
            third_version = 1;
            printf("1SFP(0+4)RJ45\r\n");
            break;
        case 0x02:
            gHalPortMap[0].lport = 1;
            gHalPortMap[0].hport = 0;
            gHalPortMap[0].port_type = S100M_OPTICAL;
            gHalPortMap[1].lport = 2;
            gHalPortMap[1].hport = 2;
            gHalPortMap[1].port_type = S100M_CABLE;
            gHalPortMap[2].lport = 3;
            gHalPortMap[2].hport = 3;
            gHalPortMap[2].port_type = S100M_CABLE;
            DeviceBaseInfo.PortNum = 3;
            third_version = 2;
            printf("1SFP(0+2)RJ45\r\n");
            break;
        case 0x03:
            gHalPortMap[0].lport = 1;
            gHalPortMap[0].hport = 0;
            gHalPortMap[0].port_type = S100M_OPTICAL;
            gHalPortMap[1].lport = 2;
            gHalPortMap[1].hport = 2;
            gHalPortMap[1].port_type = S100M_CABLE;
            DeviceBaseInfo.PortNum = 2;
            third_version = 3;
            printf("1SFP(0+1)RJ45\r\n");
            break;
        default:     
            break;
    }

	printf("Hardware Version           : v%d.%d.%d\r\n", first_version, second_version, third_version);
	*hardware_version = ((first_version & 0x000F) << 12) | ((second_version & 0x000F) << 8) | (third_version & 0x00FF);

	printf("\r\n");
    
#else
	*hardware_version = 0x1100;
	DeviceBaseInfo.PortNum = MAX_PORT_NUM;
#endif

	return ret;
}

void get_shelf_slot_id(u8 *shelf_id, u8 slot_id)
{
	return;
}


void gpio_initialize(void)
{
#if BOARD_GE22103MA
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
    if(gHardwareAddr == 0x007F){
        if(gHardwareVer87 != 0x03) {
            /* KSigIn_1, KSigIn_2 : PD11,PD12 */
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
        }
    }else{
        /* KSigIn_1, KSigIn_2 : PD11,PD12 */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOD, &GPIO_InitStructure);
    }
	
#if 0
	if(EXTI9_5_Semaphore == NULL) {
		vSemaphoreCreateBinary(EXTI9_5_Semaphore);
	}

	/* Configure INTn pin as input */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Connect EXTI Line to INT Pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource8);

	/* Configure EXTI line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set the EXTI interrupt to the highest priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#elif BOARD_GE2C400U
	GPIO_InitTypeDef  GPIO_InitStructure; 
	u8 ShelfSlotID;
	
	/* BOX CS input */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 

	/* SW1-4 input, PD0-PD3 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 

	/* SW5-8 input, PE7-PE10 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	ShelfSlotID =  (u8)(((GPIOD->IDR & GPIO_Pin_0) >> 0) << 7);		/* SW1 */
	ShelfSlotID |= (u8)(((GPIOD->IDR & GPIO_Pin_1) >> 1) << 6);		/* SW2 */
	ShelfSlotID |= (u8)(((GPIOD->IDR & GPIO_Pin_2) >> 2) << 5);		/* SW3 */
	ShelfSlotID |= (u8)(((GPIOD->IDR & GPIO_Pin_3) >> 3) << 4);		/* SW4 */
	ShelfSlotID |= (u8)(((GPIOE->IDR & GPIO_Pin_7) >> 7) << 3);		/* SW5 */
	ShelfSlotID |= (u8)(((GPIOE->IDR & GPIO_Pin_8) >> 8) << 2);		/* SW6 */
	ShelfSlotID |= (u8)(((GPIOE->IDR & GPIO_Pin_9) >> 9) << 1);		/* SW7 */
	ShelfSlotID |= (u8)(((GPIOE->IDR & GPIO_Pin_10) >> 10) << 0);	/* SW8 */

	printf("Reading ShelfID/SlotID : %d/%d\r\n\r\n", (ShelfSlotID & 0xE0) >> 5, ShelfSlotID & 0x1F);

#elif BOARD_GE1040PU
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* BOX CS input, PE6 for GE1040PU on 5U BOX */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOE, &GPIO_InitStructure); 

	
	if(EXTI9_5_Semaphore == NULL) {
		vSemaphoreCreateBinary(EXTI9_5_Semaphore);
	}

	#if 0
	/* Enable the INT_PHY (PE9) Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	#endif
	
	/* Configure INT_PHY pin as input */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Connect EXTI Line to INT Pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource9);

	/* Configure EXTI line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set the EXTI interrupt to the highest priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#elif BOARD_GE204P0U
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* BOX CS input, PE6 for GE1040PU on 5U BOX */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOE, &GPIO_InitStructure); 

	
	if(EXTI9_5_Semaphore == NULL) {
		vSemaphoreCreateBinary(EXTI9_5_Semaphore);
	}

	#if 0
	/* Enable the INT_PHY (PE9) Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	#endif
	
	/* Configure INT_PHY pin as input */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Connect EXTI Line to INT Pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource9);

	/* Configure EXTI line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set the EXTI interrupt to the highest priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	
#endif
	return;
}

void board_early_initialize(void)
{
	extern tConsoleDev *pConsoleDev;
	
#if BOARD_GE22103MA
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* HWV8, HWV7  = PD4, PD3 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_3;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    /* U5_ADDR1, U5_ADDR2, U5_ADDR3, U5_ADDR4, U5_ADDR5, U5_ADDR6,U5_ADDR7 =
    PE9,     PE10,     PE11,     PE12,     PE13,     PE14,    PE15 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | 
        GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	gHardwareVer87  = (u8)((GPIOD->IDR & GPIO_Pin_3) >> 3);
	gHardwareVer87 |= (u8)((GPIOD->IDR & GPIO_Pin_4) >> 3);
    
    /* GE-2A005U PCB address, bit0...bit7 = ADDR1...ADDR8 */
    gHardwareAddr  = (u16)(((GPIOE->IDR & GPIO_Pin_9) >> 9) & 0x01) << 0;
	gHardwareAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_10) >> 10) & 0x01) << 1;
    gHardwareAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_11) >> 11) & 0x01) << 2;
    gHardwareAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_12) >> 12) & 0x01) << 3;
    gHardwareAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_13) >> 13) & 0x01) << 4;
    gHardwareAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_14) >> 14) & 0x01) << 5;
    gHardwareAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_15) >> 15) & 0x01) << 6;

    if(gHardwareAddr == 0x007F){
        if(gHardwareVer87 != 0x03) {
            pConsoleDev->UARTx = USART3;
            pConsoleDev->ComPort = COM_USART3;
            pConsoleDev->Mode = RS485;
	    } else {
            pConsoleDev->UARTx = UART5;
            pConsoleDev->ComPort = COM_UART5;
            pConsoleDev->Mode = RS485;
	    }
    }else{
        pConsoleDev->UARTx = USART3;
        pConsoleDev->ComPort = COM_USART3;
        pConsoleDev->Mode = RS485;
    }
#elif BOARD_GE20023MA
		pConsoleDev->UARTx = UART5;
		pConsoleDev->ComPort = COM_UART5;
		pConsoleDev->Mode = RS485;
#elif BOARD_GE11014MA
		pConsoleDev->UARTx = USART3;
		pConsoleDev->ComPort = COM_USART3;
		pConsoleDev->Mode = RS485;
#elif BOARD_GE2C400U
		pConsoleDev->UARTx = USART6;
		pConsoleDev->ComPort = COM_USART6;
		pConsoleDev->Mode = RS232;
#elif BOARD_GE1040PU
		pConsoleDev->UARTx = USART6;
		pConsoleDev->ComPort = COM_USART6;
		pConsoleDev->Mode = RS232;
#elif BOARD_GE204P0U
		pConsoleDev->UARTx = USART6;
		pConsoleDev->ComPort = COM_USART6;
		pConsoleDev->Mode = RS232;		
#elif BOARD_GV3S_HONUE_QM
		pConsoleDev->UARTx = USART3;
		pConsoleDev->ComPort = COM_USART3;
		pConsoleDev->Mode = RS485;
#elif BOARD_GE11500MD
        pConsoleDev->UARTx = USART6;
		pConsoleDev->ComPort = COM_USART6;
		pConsoleDev->Mode = RS232;
#elif BOARD_GE_EXT_22002EA
		pConsoleDev->UARTx = USART3;
		pConsoleDev->ComPort = COM_USART3;
		pConsoleDev->Mode = RS232;
#elif BOARD_GE220044MD
		pConsoleDev->UARTx = USART3;
		pConsoleDev->ComPort = COM_USART3;
		pConsoleDev->Mode = RS232;
#else
#error unknown board
#endif
}

void misc_initialize(void)
{
	u16 HardwareVer;
	u32	BoardType;
	
	/* Initialize board base information */
	memset(&DeviceBaseInfo, 0, sizeof(dev_base_info_t));
	get_hardware_version(&HardwareVer);
	DeviceBaseInfo.BoardType[0] = (u8)((BOARD_TYPE & 0xFF000000) >> 24);
	DeviceBaseInfo.BoardType[1] = (u8)((BOARD_TYPE & 0x00FF0000) >> 16);
	DeviceBaseInfo.BoardType[2] = (u8)((BOARD_TYPE & 0x0000FF00) >> 8);
	DeviceBaseInfo.BoardType[3] = (u8)(BOARD_TYPE & 0x000000FF);
	DeviceBaseInfo.BoardType[4] = 0xFF;
	DeviceBaseInfo.BoardType[5] = 0xFF;
	DeviceBaseInfo.BoardType[6] = 0xFF;
	DeviceBaseInfo.BoardType[7] = 0xFF;
#if (BOARD_TYPE == BT_GE22103MA)
    if(gHardwareAddr == 0x007F){
        if(gHardwareVer87 != 0x03)
           DeviceBaseInfo.BoardType[3] = (u8)(HardwareVer & 0x00FF);
    }else if(gHardwareAddr == 0x0000){
        DeviceBaseInfo.BoardType[3] = (u8)0x20;
    }else{
        DeviceBaseInfo.BoardType[3] = (u8)0x10;
    }
#elif (BOARD_TYPE == BT_GE20023MA)
    switch((gHardwareVer & 0x18) >> 3){
    case 0x00: 
        DeviceBaseInfo.BoardType[3] = (u8)0xFF;
        break;
    case 0x01: 
        DeviceBaseInfo.BoardType[3] = (u8)0x00;
        break;
    case 0x02: 
        DeviceBaseInfo.BoardType[3] = (u8)0x01;
        break;
    case 0x03: 
        DeviceBaseInfo.BoardType[3] = (u8)0x02;
        break;
    default:
        break;
    }
#endif

#if (BOARD_TYPE == BT_GE22103MA)
	strcpy((char *)DeviceBaseInfo.BoardName, "GE22103MA");
#elif (BOARD_TYPE == BT_GE20023MA)
	strcpy((char *)DeviceBaseInfo.BoardName, "GE20023MA");
#elif (BOARD_TYPE == BT_GE11014MA)
	strcpy((char *)DeviceBaseInfo.BoardName, "GE11014MA");
#elif (BOARD_TYPE == BT_GV3S_HONUE_QM)
	strcpy((char *)DeviceBaseInfo.BoardName, "GV3S_HONUE_QM");
#elif (BOARD_TYPE == BT_GE2C400U)
	strcpy((char *)DeviceBaseInfo.BoardName, "GE2C400U");
#elif (BOARD_TYPE == BT_GE1040PU)
	strcpy((char *)DeviceBaseInfo.BoardName, "GE1040PU");
#elif (BOARD_TYPE == BT_GE204P0U)
	strcpy((char *)DeviceBaseInfo.BoardName, "GE204P0U");
#elif (BOARD_TYPE == BT_GE11500MD)
	strcpy((char *)DeviceBaseInfo.BoardName, "GE11500MD");
#elif (BOARD_TYPE == BT_GE_EXT_22002EA)
	strcpy((char *)DeviceBaseInfo.BoardName, "GE_EXT_22002EA");
#elif (BOARD_TYPE == BT_GE220044MD)
	strcpy((char *)DeviceBaseInfo.BoardName, "GE220044MD"); 
#else
	strcpy((char *)DeviceBaseInfo.BoardName, "Unkown");
#endif
	
	DeviceBaseInfo.HardwareVer[0] = (u8)((HardwareVer & 0xFF00) >> 8);
	DeviceBaseInfo.HardwareVer[1] = (u8)(HardwareVer & 0x00FF);
	#if RELEASE_TRUNK_VERSION
	DeviceBaseInfo.FirmwareVer[0] = (u8)((FIRMWARE_VERSION & 0xFF00) >> 8);
	DeviceBaseInfo.FirmwareVer[1] = (u8)(FIRMWARE_VERSION & 0x00FF);
	#else
	DeviceBaseInfo.FirmwareVer[0] = 0x00;
	DeviceBaseInfo.FirmwareVer[1] = 0x00;
	#endif
	DeviceBaseInfo.ChipType = SWITCH_CHIP_TYPE;
	DeviceBaseInfo.FeatureMask[0] = (u8)((BOARD_FEATURE & 0xFF000000) >> 24);
	DeviceBaseInfo.FeatureMask[1] = (u8)((BOARD_FEATURE & 0x00FF0000) >> 16);
	DeviceBaseInfo.FeatureMask[2] = (u8)((BOARD_FEATURE & 0x0000FF00) >> 8);
	DeviceBaseInfo.FeatureMask[3] = (u8)(BOARD_FEATURE & 0x000000FF);

	/* Initialize external interrupt */
	gpio_initialize();

	return;
}



