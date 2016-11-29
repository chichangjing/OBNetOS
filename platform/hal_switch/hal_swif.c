
/*******************************************************************
 * Filename     : hal_swif.c
 * Description  : Hardware Abstraction Layer for L2 Switch initialize
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *******************************************************************/
#include "mconfig.h"

/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Kernel includes */
#include "FreeRTOS.h"
#include "task.h"
#include "os_mutex.h"

/* BSP includes */
#include "stm32f2xx.h"
#include "stm32f2x7_smi.h"
#include "misc_drv.h"
#if ROBO_SWITCH
#include "robo_drv.h"
#endif

/* Configuration includes */
#include "conf_comm.h"
#include "conf_global.h"

/* HAL for L2 includes */
#include "hal_swif_error.h"
#include "hal_swif.h"
#include "hal_swif_port.h"
#include "hal_swif_rate_ctrl.h"
#include "hal_swif_mirror.h"
#include "hal_swif_qos.h"
#include "hal_swif_message.h"
#include "hal_swif_mac.h"
#include "hal_swif_vlan.h"
#include "hal_swif_aggregation.h"
#include "hal_swif_multicast.h"


#if BOARD_GV3S_HONUE_QM
#include "fpga_api.h"
#endif

#if MARVELL_SWITCH
extern int obSwitch_start(void);
#endif

extern OS_MUTEX_T robo_mutex;
extern OS_MUTEX_T OBMsgTxMutex;
extern OS_MUTEX_T NeighborInfoMutex;
extern OS_MUTEX_T mutex_smi;
extern dev_base_info_t	DeviceBaseInfo;
extern hal_trap_info_t gTrapInfo;
extern unsigned char NeigSearchMultiAddr[];
//OS_MUTEX_T swif_poll_mutex;

#if (BOARD_GE1040PU || BOARD_GE204P0U)

#define MCU_IO1_CK_H			GPIOE->BSRRL = GPIO_Pin_3
#define MCU_IO1_CK_L			GPIOE->BSRRH = GPIO_Pin_3
#define MCU_IO2_DA_H			GPIOE->BSRRL = GPIO_Pin_4
#define MCU_IO2_DA_L			GPIOE->BSRRH = GPIO_Pin_4
#define MCU_IO3_CS_H			GPIOE->BSRRL = GPIO_Pin_14
#define MCU_IO3_CS_L			GPIOE->BSRRH = GPIO_Pin_14


void IOxx_Delay(void)
{
	uint8 i=100;
 	while(i--) ;
}

void IO_Send(uint8 data)
{
	uint8 loop=8;

	MCU_IO3_CS_L;

	/* Start */
	MCU_IO2_DA_L;

	MCU_IO1_CK_H;
	IOxx_Delay();
	MCU_IO1_CK_L;
	IOxx_Delay();

	/* Send Data */
	while(loop > 0) {
		if(data & 0x80)
			MCU_IO2_DA_H;
		else
			MCU_IO2_DA_L;

		MCU_IO1_CK_H;
		IOxx_Delay();
		MCU_IO1_CK_L;
		IOxx_Delay();

		data <<= 1;
		loop--;
	}

	/* End */
	MCU_IO2_DA_H;

	MCU_IO1_CK_H;
	IOxx_Delay();
	MCU_IO1_CK_L;
	IOxx_Delay();

	MCU_IO1_CK_H;

	MCU_IO3_CS_H;
}

static void _GE25_LedCtrl_IOLevel(int level)
{
	if(level == 1)
		GPIOE->BSRRL = GPIO_Pin_3;
	else
		GPIOE->BSRRH = GPIO_Pin_3;
}

static void _GE26_LedCtrl_IOLevel(int level)
{
	if(level == 1)
		GPIOE->BSRRL = GPIO_Pin_4;
	else
		GPIOE->BSRRH = GPIO_Pin_4;
}

static void _GE27_LedCtrl_IOLevel(int level)
{
	if(level == 1)
		GPIOE->BSRRL = GPIO_Pin_14;
	else
		GPIOE->BSRRH = GPIO_Pin_14;
}

static void _GE28_LedCtrl_IOLevel(int level)
{
	if(level == 1)
		GPIOE->BSRRL = GPIO_Pin_15;
	else
		GPIOE->BSRRH = GPIO_Pin_15;
}

static void _GE_LedCtrl_IOLevel(int ge_port, int level)
{
	switch(ge_port) {
		case BCM53286_GE25_PORT:
		if(level == 1)
			GPIOE->BSRRL = GPIO_Pin_3;
		else
			GPIOE->BSRRH = GPIO_Pin_3;
		break;

		case BCM53286_GE26_PORT:
		if(level == 1)
			GPIOE->BSRRL = GPIO_Pin_4;
		else
			GPIOE->BSRRH = GPIO_Pin_4;
		break;

		case BCM53286_GE27_PORT:
		if(level == 1)
			GPIOE->BSRRL = GPIO_Pin_14;
		else
			GPIOE->BSRRH = GPIO_Pin_14;
		break;

		case BCM53286_GE28_PORT:
		if(level == 1)
			GPIOE->BSRRL = GPIO_Pin_15;
		else
			GPIOE->BSRRH = GPIO_Pin_15;
		break;

		default:
		break;
	}
}

#endif

int hal_swif_init(void)
{
	if(os_mutex_init(&OBMsgTxMutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return HAL_SWIF_FAILURE;
	}

	if(os_mutex_init(&NeighborInfoMutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return HAL_SWIF_FAILURE;
	}
	
/***********************************************************************************************************/// For Broadcom Switch
#if BOARD_GE20023MA
    u16	phyid_high, phyid_low;
	u8	ge_port;
	u8	u8Data;
    u16 u16Data;
	u32	u32Data;
    extern u8 gHardwareVer;

	if(os_mutex_init(&robo_mutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return HAL_SWIF_FAILURE;
	}
	robo_SpiInit(); 

	if(robo_read(0x10, 0x04, (u8 *)&phyid_high, 2) != 0)
		return HAL_SWIF_FAILURE;
	if(robo_read(0x10, 0x06, (u8 *)&phyid_low, 2) != 0)
		return HAL_SWIF_FAILURE;

	if((phyid_high != 0x0362) && (phyid_low != 0x5ED4))
		return HAL_SWIF_FAILURE;

    /* Receive unicast/multicast/broadcast enable */
	if(robo_read(0x00, 0x08, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
    u8Data = u8Data | 0x1C;
	robo_write(0x00, 0x08, &u8Data, 1);
    
    /* Page178, Switch Mode Register */
	/*  Set the Swtich Mode to managed mode, and enable Frame forwarding  */
	if(robo_read(0x00, 0x0B, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x01 | 0x02;
	robo_write(0x00, 0x0B, &u8Data, 1);
    
    /* Port forward control register */
	if(robo_read(0x00, 0x21, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
    /* Forward unicast lookup failed frames according */
	u8Data = u8Data | 0x40;
	robo_write(0x00, 0x21, &u8Data, 1);
    /* Unicast lookup failed */
    if(robo_read(0x00, 0x32, (u8 *)&u16Data, 2) != 0)
		return HAL_SWIF_FAILURE;
    /* Do not forward a unicast lookup failure to cpu port */
	u16Data = (u16Data | 0x00FF) & ~(0x0001 << 8);
	robo_write(0x00, 0x32, (u8 *)&u16Data, 2);
	
	/* Page179, IMP Port State Override Register */
	/* Set IMP port 100M full-duplex, link up */
	if(robo_read(0x00, 0x0E, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = (u8Data & 0xF0) | 0x07 | 0xB0;
	robo_write(0x00, 0x0E, &u8Data, 1);
    
    /* Enable IMP0 only, receive BPDU enable, reset MIB */
    if(robo_read(0x02, 0x00, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x83 & 0xFE;    //0xFE 释放最低位，MIB counters 
	robo_write(0x02, 0x00, &u8Data, 1);

	/* Broadcom Tag enable for IMP0 only */
    if(robo_read(0x02, 0x03, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x01;
	robo_write(0x02, 0x03, &u8Data, 1);	

    /*1st SFP Port 0 */
	u16Data = 0x2100;
	robo_write(0x10, 0x00, (u8 *)&u16Data, 2);
	u16Data = 0x0220;
	robo_write(0x10, 0x20, (u8 *)&u16Data, 2);
	u16Data = 0x0020;
	robo_write(0x10, 0x2E, (u8 *)&u16Data, 2);	
	u16Data = 0x008B;
	robo_write(0x10, 0x3E, (u8 *)&u16Data, 2);
	u16Data = 0x0200;
	robo_write(0x10, 0x32, (u8 *)&u16Data, 2);
	u16Data = 0x0084;
	robo_write(0x10, 0x3A, (u8 *)&u16Data, 2);
	u16Data = 0x000B;
	robo_write(0x10, 0x3E, (u8 *)&u16Data, 2);

    switch((gHardwareVer & 0x18) >> 3){ 
    case 0x00:
        /* 2nd SFP Port 1  */
        u16Data = 0x2100;
        robo_write(0x11, 0x00, (u8 *)&u16Data, 2);
        u16Data = 0x0220;
        robo_write(0x11, 0x20, (u8 *)&u16Data, 2);
        u16Data = 0x0020;
        robo_write(0x11, 0x2E, (u8 *)&u16Data, 2);	
        u16Data = 0x008B;
        robo_write(0x11, 0x3E, (u8 *)&u16Data, 2);
        u16Data = 0x0200;
        robo_write(0x11, 0x32, (u8 *)&u16Data, 2);
        u16Data = 0x0084;
        robo_write(0x11, 0x3A, (u8 *)&u16Data, 2);
        u16Data = 0x000B;
        robo_write(0x11, 0x3E, (u8 *)&u16Data, 2);
        break;
    case 0x01: 
    case 0x02:
    case 0x03: 
    default:
        break;
    }
        
    /* Initialize the ports STP states */
    hal_swif_port_set_stp_state(1, FORWARDING);
    hal_swif_port_set_stp_state(2, FORWARDING);
    hal_swif_port_set_stp_state(3, FORWARDING);
    hal_swif_port_set_stp_state(4, FORWARDING);
    hal_swif_port_set_stp_state(5, FORWARDING);

    return HAL_SWIF_SUCCESS;

#elif BOARD_GE11014MA
    u16	phyid_high, phyid_low;
	u8	ge_port;
	u8	u8Data;
    u16 u16Data;
	u32	u32Data;

	if(os_mutex_init(&robo_mutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return HAL_SWIF_FAILURE;
	}
	robo_SpiInit(); 

	if(robo_read(0x10, 0x04, (u8 *)&phyid_high, 2) != 0)
		return HAL_SWIF_FAILURE;
	if(robo_read(0x10, 0x06, (u8 *)&phyid_low, 2) != 0)
		return HAL_SWIF_FAILURE;

	if((phyid_high != 0x0362) && (phyid_low != 0x5ED4))
		return HAL_SWIF_FAILURE;

    /* Receive unicast/multicast/broadcast enable */
	if(robo_read(0x00, 0x08, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
    u8Data = u8Data | 0x1C;
	robo_write(0x00, 0x08, &u8Data, 1);
    
    /* Page178, Switch Mode Register */
	/*  Set the Swtich Mode to managed mode, and enable Frame forwarding  */
	if(robo_read(0x00, 0x0B, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x01 | 0x02;
	robo_write(0x00, 0x0B, &u8Data, 1);
	
	/* Page179, IMP Port State Override Register */
	/* Set IMP port 100M full-duplex, link up */
	if(robo_read(0x00, 0x0E, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = (u8Data & 0xF0) | 0x07 | 0xB0;
	robo_write(0x00, 0x0E, &u8Data, 1);
    
    /* Enable IMP0 and IMP1, receive BPDU enable, reset MIB */
    if(robo_read(0x02, 0x00, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0xC3 & 0xFE;    //0xFE 释放最低位，MIB counters 
	robo_write(0x02, 0x00, &u8Data, 1);

	/* Broadcom Tag enable for IMP1 only */
    if(robo_read(0x02, 0x03, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x02;
	robo_write(0x02, 0x03, &u8Data, 1);	
    
    /* Port 0 */
	u16Data = 0x2100;
	robo_write(0x10, 0x00, (u8 *)&u16Data, 2);
	u16Data = 0x0220;
	robo_write(0x10, 0x20, (u8 *)&u16Data, 2);
	u16Data = 0x0020;
	robo_write(0x10, 0x2E, (u8 *)&u16Data, 2);	
	u16Data = 0x008B;
	robo_write(0x10, 0x3E, (u8 *)&u16Data, 2);
	u16Data = 0x0200;
	robo_write(0x10, 0x32, (u8 *)&u16Data, 2);
	u16Data = 0x0084;
	robo_write(0x10, 0x3A, (u8 *)&u16Data, 2);
	u16Data = 0x000B;
	robo_write(0x10, 0x3E, (u8 *)&u16Data, 2);	

    return HAL_SWIF_SUCCESS;
    
#elif BOARD_GE11500MD
	u16	phyid_high, phyid_low;
	u8	ge_port;
	u8	u8Data;
    u16 u16Data;
	u32	u32Data;

	if(os_mutex_init(&robo_mutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return HAL_SWIF_FAILURE;
	}
	robo_SpiInit(); 

	if(robo_read(0x10, 0x04, (u8 *)&phyid_high, 2) != 0)
		return HAL_SWIF_FAILURE;
	if(robo_read(0x10, 0x06, (u8 *)&phyid_low, 2) != 0)
		return HAL_SWIF_FAILURE;

	if((phyid_high != 0x0143) && (phyid_low != 0xBF88))
		return HAL_SWIF_FAILURE;

    /* Receive unicast/multicast/broadcast enable */
	if(robo_read(0x00, 0x08, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
    u8Data = u8Data | 0x1C;
	robo_write(0x00, 0x08, &u8Data, 1);
    
    /* Page178, Switch Mode Register */
	/*  Set the Swtich Mode to managed mode, and enable Frame forwarding  */
	if(robo_read(0x00, 0x0B, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x01 | 0x02;
	robo_write(0x00, 0x0B, &u8Data, 1);
	
	/* Page179, IMP Port State Override Register */
	/* Set IMP port 100M full-duplex, link up */
	if(robo_read(0x00, 0x0E, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = (u8Data & 0xF0) | 0x07 | 0x80;
	robo_write(0x00, 0x0E, &u8Data, 1);
    
    /* Enable IMP0 only, receive BPDU enable, reset MIB */
    if(robo_read(0x02, 0x00, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x83 & 0xFE;    //0xFE 释放最低位，MIB counters 
	robo_write(0x02, 0x00, &u8Data, 1);

	/* Broadcom Tag enable for IMP0 only */
    if(robo_read(0x02, 0x03, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x01;
	robo_write(0x02, 0x03, &u8Data, 1);	
    
    /* Internal SerDes Port Register for Port5 only */
    if(robo_read(0x15, 0x20, (u8 *)&u16Data, 2) != 0)
		return HAL_SWIF_FAILURE;
    /* Set Port5 fiber mode,disable auto detection(fiber or SGMII mode) */
	u16Data = (u16Data | 0x0001)& ~0x0010;
	if(robo_write(0x15, 0x20, (u8 *)&u16Data, 2) != 0)
        return HAL_SWIF_FAILURE;
    
    /* Set LED function 1 control regsiter */
    u16Data = 0x0324;
    if(robo_write(0x00, 0x12, (u8 *)&u16Data, 2) != 0)
        return HAL_SWIF_FAILURE;
    /* LED function map register */
    u16Data = 0x01FF;
    if(robo_write(0x00, 0x14, (u8 *)&u16Data, 2) != 0)
        return HAL_SWIF_FAILURE;
    /* LED enable map register */
    u16Data = 0x003F;
    if(robo_write(0x00, 0x16, (u8 *)&u16Data, 2) != 0)
        return HAL_SWIF_FAILURE;
	
	for(ge_port=0; ge_port<6; ge_port++) {     //STP forwording
		u8Data = 0xA0;
		robo_write(0x00, ge_port, (u8 *)&u8Data, 1);			
	}

    return HAL_SWIF_SUCCESS;

#elif BOARD_GE1040PU
	u16	phyid_high, phyid_low;
	u8	chipid[8];
	u8	ge_port;
	u8	u8Data;
    u16 u16Data;
	u32	u32Data;

	if(os_mutex_init(&robo_mutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return HAL_SWIF_FAILURE;
	}
	robo_SpiInit();

	if(robo_read(0x10, 0x04, (u8 *)&phyid_high, 2) != 0)
		return HAL_SWIF_FAILURE;
	if(robo_read(0x10, 0x06, (u8 *)&phyid_low, 2) != 0)
		return HAL_SWIF_FAILURE;
	if(robo_read(0x00, 0xe8, chipid, 8) != 0)
		return HAL_SWIF_FAILURE;

	if((phyid_high != 0x0362) && (phyid_low != 0x5e96))
		return HAL_SWIF_FAILURE;
	if(chipid[0] != 0x04)
		return HAL_SWIF_FAILURE;

	/* Page263, Switch Mode Register */
	/* Set the Swtich Mode to managed mode, and enable Frame forwarding */
	if(robo_read(0x00, 0x00, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x01 | 0x02;
	robo_write(0x00, 0x00, &u8Data, 1);


	/* Page292, Global Management Configuration Register */
	/* Define Frame Management Port as MII port, and receive BPDU enable */
	if(robo_read(0x03, 0x00, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x80 | 0x02;
	robo_write(0x03, 0x00, &u8Data, 1);


	/* Page278, IMP Port State Override Register */
	/* Set IMP port 100M full-duplex, link up  */
	if(robo_read(0x01, 0x28, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = (u8Data & 0xF0) | 0x07 | 0x40;
	robo_write(0x01, 0x28, &u8Data, 1);

	/* Page284, IMP Port Control Register */
	/* Receive Unknown Unicast Enable. */
	/* Receive Unknown Multicast Enable. */
	/* Receive Broadcast Enable */
	if(robo_read(0x01, 0xa8, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = 0xc0 | u8Data | 0x18 | 0x04 ;
	robo_write(0x01, 0xa8, &u8Data, 1);

	/* Disable External Phy Scan for Gibt Port */
	u8Data = 0x00;
	for(ge_port=0; ge_port<4; ge_port++) {
		if(robo_write(0x01, 0x29+ge_port, &u8Data, 1) != 0)
			return HAL_SWIF_FAILURE;
	}

	/* Enable Interrupt Mask for Link Status Change */
	for(ge_port=0; ge_port<4; ge_port++) {
		/* 1000BASE-T/100BASE-TX/10BASE-T Interrupt Mask (Address 1Bh), Page158 for BCM54640 */
		/* Enable Interrupt for Link Status Change */
		u16Data = 0xFFFD;
		if(robo_write(0xD9+ge_port, 0x36, (u8 *)&u16Data, 2) != 0)
			return HAL_SWIF_FAILURE;

		/* Expansion Register Access (Address 17h), Page142 for BCM54640 */
		/* Expansion register selected, Address 02h */
		u16Data = 0x0F02;
		if(robo_write(0xD9+ge_port, 0x2E, (u8 *)&u16Data, 2) != 0)
			return HAL_SWIF_FAILURE;
		/* Fiber Interrupt Mask Register (Expansion register Address 02h), Page223 for BCM54640 */
		/* Enable Interrupt for Fiber Link Status Change */
		u16Data = 0xFFBF;
		if(robo_write(0xD9+ge_port, 0x2A, (u8 *)&u16Data, 2) != 0)
			return HAL_SWIF_FAILURE;
		/* Expansion Register Access (Address 17h), Page142 for BCM54640 */
		/* Expansion register not selected, Address 02h */
		u16Data = 0x0000;
		if(robo_write(0xD9+ge_port, 0x2E, (u8 *)&u16Data, 2) != 0)
			return HAL_SWIF_FAILURE;
	}


	/* MCU GPIO initialize */
	
		GPIO_InitTypeDef  GPIO_InitStructure;

		/* PE3/MCU_F_IO1, for GE25 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		/* PE4/MCU_F_IO2, for GE26 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		/* PE14/MCU_F_IO3, for GE27 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		/* PE15/MCU_F_IO4, for GE28 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		_GE25_LedCtrl_IOLevel(1);
		_GE26_LedCtrl_IOLevel(1);
		_GE27_LedCtrl_IOLevel(1);
		_GE28_LedCtrl_IOLevel(1);

	//IO_Send(0x88);
	return HAL_SWIF_SUCCESS;

#elif BOARD_GE204P0U
	u16	phyid_high, phyid_low;
	u8	chipid[8];
	u8	ge_port;
	u8	u8Data;
    u16 u16Data;
	u32	u32Data;

	if(os_mutex_init(&robo_mutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return HAL_SWIF_FAILURE;
	}
	robo_SpiInit();

	if(robo_read(0x10, 0x04, (u8 *)&phyid_high, 2) != 0)
		return HAL_SWIF_FAILURE;
	if(robo_read(0x10, 0x06, (u8 *)&phyid_low, 2) != 0)
		return HAL_SWIF_FAILURE;
	if(robo_read(0x00, 0xe8, chipid, 8) != 0)
		return HAL_SWIF_FAILURE;

	if((phyid_high != 0x0362) && (phyid_low != 0x5e96))
		return HAL_SWIF_FAILURE;
	if(chipid[0] != 0x04)
		return HAL_SWIF_FAILURE;

	/* Page263, Switch Mode Register */
	/* Set the Swtich Mode to managed mode, and enable Frame forwarding */
	if(robo_read(0x00, 0x00, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x01 | 0x02;
	robo_write(0x00, 0x00, &u8Data, 1);


	/* Page292, Global Management Configuration Register */
	/* Define Frame Management Port as MII port, and receive BPDU enable */
	if(robo_read(0x03, 0x00, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = u8Data | 0x80 | 0x02;
	robo_write(0x03, 0x00, &u8Data, 1);


	/* Page278, IMP Port State Override Register */
	/* Set IMP port 100M full-duplex, link up  */
	if(robo_read(0x01, 0x28, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = (u8Data & 0xF0) | 0x07 | 0x40;
	robo_write(0x01, 0x28, &u8Data, 1);

	/* Page284, IMP Port Control Register */
	/* Receive Unknown Unicast Enable. */
	/* Receive Unknown Multicast Enable. */
	/* Receive Broadcast Enable */
	if(robo_read(0x01, 0xa8, (u8 *)&u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;
	u8Data = 0xc0 | u8Data | 0x18 | 0x04 ;
	robo_write(0x01, 0xa8, &u8Data, 1);

	/*Set port 0-23 fiber Mode,see 5328X-AN10x-RDS Page 24*/
	for(ge_port=0; ge_port<24; ge_port++){
		/*to put the PHY in 100M, full-duplex mode with auto-negotiation disabled*/
		if(robo_read(0xa0+ge_port, 0x00, (u8 *)&u16Data, 2) != 0)
			return HAL_SWIF_FAILURE;
        u16Data = u16Data & ~(1<<12) | (1<<8);
        if(robo_write(0xa0+ge_port, 0x00, (u8 *)&u16Data, 2) != 0)
            return HAL_SWIF_FAILURE;

		/*to bypass the scrambler/descrambler block.*/
        u16Data = 0x0220;
        if(robo_write(0xa0+ge_port, 0x20, (u8 *)&u16Data, 2) != 0)
            return HAL_SWIF_FAILURE;

		/*to change MLT-3 code to two-level binary*/
        u16Data = 0x0020;
        if(robo_write(0xa0+ge_port, 0x2E, (u8 *)&u16Data, 2) != 0)
            return HAL_SWIF_FAILURE;

		/*to set bit 7 to enable the shadow register*/
	    u16Data = 0x008B;
	    if(robo_write(0xa0+ge_port, 0x3E, (u8 *)&u16Data, 2) != 0)
	        return HAL_SWIF_FAILURE;

		/*to enable the special signal detection block.*/
	    u16Data = 0x0200;
	    if(robo_write(0xa0+ge_port, 0x32, (u8 *)&u16Data, 2) != 0)
	        return HAL_SWIF_FAILURE;

		/*to configure the transmit amplitude to 1v pk-pk*/
	    u16Data = 0x0084;
	    if(robo_write(0xa0+ge_port, 0x3A, (u8 *)&u16Data, 2) != 0)
	        return HAL_SWIF_FAILURE;

		/*to exit the shadow register mode*/
	    u16Data = 0x000B;
	    if(robo_write(0xa0+ge_port, 0x3E, (u8 *)&u16Data, 2) != 0)
	        return HAL_SWIF_FAILURE;
	}
	

	/* Disable External Phy Scan for Gibt Port */
	u8Data = 0x00;
	for(ge_port=0; ge_port<4; ge_port++) {
		if(robo_write(0x01, 0x29+ge_port, &u8Data, 1) != 0)
			return HAL_SWIF_FAILURE;
	}

	/* Enable Interrupt Mask for Link Status Change */
	for(ge_port=0; ge_port<4; ge_port++) {
		/* 1000BASE-T/100BASE-TX/10BASE-T Interrupt Mask (Address 1Bh), Page158 for BCM54640 */
		/* Enable Interrupt for Link Status Change */
		u16Data = 0xFFFD;
		if(robo_write(0xD9+ge_port, 0x36, (u8 *)&u16Data, 2) != 0)
			return HAL_SWIF_FAILURE;

		/* Expansion Register Access (Address 17h), Page142 for BCM54640 */
		/* Expansion register selected, Address 02h */
		u16Data = 0x0F02;
		if(robo_write(0xD9+ge_port, 0x2E, (u8 *)&u16Data, 2) != 0)
			return HAL_SWIF_FAILURE;
		/* Fiber Interrupt Mask Register (Expansion register Address 02h), Page223 for BCM54640 */
		/* Enable Interrupt for Fiber Link Status Change */
		u16Data = 0xFFBF;
		if(robo_write(0xD9+ge_port, 0x2A, (u8 *)&u16Data, 2) != 0)
			return HAL_SWIF_FAILURE;
		/* Expansion Register Access (Address 17h), Page142 for BCM54640 */
		/* Expansion register not selected, Address 02h */
		u16Data = 0x0000;
		if(robo_write(0xD9+ge_port, 0x2E, (u8 *)&u16Data, 2) != 0)
			return HAL_SWIF_FAILURE;
	}

	/* MCU GPIO initialize */
	{
		GPIO_InitTypeDef  GPIO_InitStructure;

		/* PE3/MCU_F_IO1, for GE25 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		/* PE4/MCU_F_IO2, for GE26 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		/* PE14/MCU_F_IO3, for GE27 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		/* PE15/MCU_F_IO4, for GE28 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		_GE25_LedCtrl_IOLevel(1);
		_GE26_LedCtrl_IOLevel(1);
		_GE27_LedCtrl_IOLevel(1);
		_GE28_LedCtrl_IOLevel(1);
	}
	//IO_Send(0x88);
	return HAL_SWIF_SUCCESS;

#elif BOARD_GE2C400U
	u8	u8Data,hport;
	u16	u16Data;
	u32	u32Data;
	uint64 mib_group_select_reg;

	if(os_mutex_init(&robo_mutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return HAL_SWIF_FAILURE;
	}
	robo_SpiInit();

	/* Check Chip Model ID */
	robo_read(0x02, 0x30, &u8Data, 1);
	if(u8Data != 0x96)
		return HAL_SWIF_FAILURE;

	/* Page93, IMP Port(Port16) Control register */
	/* Set IMP port stp-status to forwarding, and enable receive unicast/multicast/broadcast enable */
	u8Data = 0xBC;
	robo_write(0x00, 0x10, &u8Data, 1);

	/* Page95, Switch Mode Register */
	/* Set the Swtich Mode to Managed mode and Forwarding enable */
	robo_read(0x00, 0x20, &u8Data, 1);
	u8Data |= 0x03;
	robo_write(0x00, 0x20, &u8Data, 1);

	/* Page98, (Port16)IMP State Override Register */
	/* Set IMP port 100M full-duplex, link up, and tx/rx flow control enable */
	u8Data = 0xB7;
	robo_write(0x00, 0x70, &u8Data, 1);

	/* Page107, Global Management Configuration Register */
	/* Enable IMP Port, Receive BPDU Enable*/
	u8Data = 0x82;
	robo_write(0x02, 0x00, &u8Data, 1);

	/* Initialize the ports STP states */
	u8Data = 0xA0;
	for(hport=0; hport<16; hport++) {
		if(robo_write(0x00, hport, &u8Data, 1) != 0) {
			printf("Error: robo write failed\r\n");
			return HAL_SWIF_ERR_SPI_RW;
		}
	}

	/* Page95, LED Control Register */
	u16Data = 0x0120;
	robo_write(0x00, 0x24, (u8 *)&u16Data, 2);
	robo_read(0x00, 0x24, (u8 *)&u16Data, 2);
	if(u16Data != 0x0120)
		NVIC_SystemReset();

	/* Select group of MIB counters, see (BCM5396_MIB_reg_spec.pdf) */
	/* Select group-1 of MIB counters */
	u64_H(mib_group_select_reg) = 0x00000001;
	u64_L(mib_group_select_reg) = 0x55555555;
	if(robo_write(0x02, 04, (u8 *)&mib_group_select_reg, 8) != 0) {
		printf("Error: robo write mib_group_select_reg failed\r\n");
		return HAL_SWIF_ERR_SPI_RW;
	}

#if 0
	/* Page192, Auto-Detect Medium Register for BCM54640E */
	u16Data = 0xF8C3;
	for(hport=0; hport<12; hport++) {
		robo_write(0x80+hport, 0x38, (u8 *)&u16Data, 2);
	}
#else
	for(hport=0; hport<12; hport++) {
		/* Page180, Serdes 100BASE-FX Status Register (Address 1Ch, Shadow 13h) */
		/* Enable 1000BASE-X mode */
		u16Data = 0xCC0A;
		robo_write(0x80+hport, 0x38, (u8 *)&u16Data, 2);

		/* Page192, Auto-Detect Medium Register (Address 1Ch, Shadow 1Eh) */
		/* Fiber signal detect from bin, Disable auto-detect medium and Fiber select */
		u16Data = 0xF8C6;
		robo_write(0x80+hport, 0x38, (u8 *)&u16Data, 2);

		/* Page194, Mode Control Register (Address 1Ch, Shadow 1Fh) */
		/* Mode used SGMII to Fiber, Enable SGMII Registers */
		u16Data = 0xFC0B;
		robo_write(0x80+hport, 0x38, (u8 *)&u16Data, 2);
	}


#endif
	return HAL_SWIF_SUCCESS;
/***********************************************************************************************************/// For Marvell Switch
#elif (BOARD_GE22103MA) || (BOARD_GV3S_HONUE_QM) || (BOARD_GE_EXT_22002EA) || (BOARD_GE220044MD)
	GT_STATUS status;
	u16 reg_val=0;
	int port;
	u16 regData;

	if(os_mutex_init(&mutex_smi) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return HAL_SWIF_FAILURE;
	}

	smi_getreg(PHYADDR_PORT(0), 0x03, &reg_val);
	printf("Switch ChipType            : %s\r\n", 
		((reg_val&0xff0)==0x0950)?"88E6095": 
		((reg_val&0xff0)==0x0990)?"88E6097(F)":"Unkown");
		
	obSwitch_start();

#if 0
	/* Use reserved multicast DA addresses: 01-80-C2-00-00-0B for OB-Ring protocol */
	smi_setreg(PHYADDR_GLOBAL2, SW_REG_MGMT_ENABLE, 0x0800);

	/* Reserved multicast frames to CPU, MGMT Priority: 7 */
	smi_setreg(PHYADDR_GLOBAL2, SW_REG_MANAGEMENT, 0x007f);
#else
    #if 0
	/* Use reserved multicast DA addresses: 01-80-C2-00-00-0E for Neighbor Search Protocol */
	smi_setreg(PHYADDR_GLOBAL2, SW_REG_MGMT_ENABLE, 0x4000);
    #endif /* 0 */
    
    if((NeigSearchMultiAddr[0] == 0x01) && (NeigSearchMultiAddr[1] == 0x80) &&
       (NeigSearchMultiAddr[2] == 0xC2) && (NeigSearchMultiAddr[3] == 0x00) &&
       (NeigSearchMultiAddr[4] == 0x00) && (NeigSearchMultiAddr[5] > 0x00)  && 
       (NeigSearchMultiAddr[5] <= 0x0F)){
        /* Use reserved multicast DA addresses: 01-80-C2-00-00-0x for Neighbor Search Protocol */
        smi_setreg(PHYADDR_GLOBAL2, SW_REG_MGMT_ENABLE, 0x0001<<NeigSearchMultiAddr[5]);
    }else if(NeigSearchMultiAddr[5] == 0x00){
        /* Use reserved multicast DA addresses: 01-80-C2-00-00-01 for Neighbor Search Protocol */
        smi_setreg(PHYADDR_GLOBAL2, SW_REG_MGMT_ENABLE, 0x0001);
    }else{
        /* Use reserved multicast DA addresses: None for Neighbor Search Protocol */
        smi_setreg(PHYADDR_GLOBAL2, SW_REG_MGMT_ENABLE, 0x0000);
        printf("warning: are you sure multicast DA addresses for Neighbor Search Protocol\r\n");
    }
	/* Rsvd2CPU = 1, MGMT Priority: 7 */
	smi_setreg(PHYADDR_GLOBAL2, SW_REG_MANAGEMENT, 0x007f);		
#endif

	/* Set DSA_tag bit */
	smi_setregfield(PHYADDR_PORT(CONFIG_SWITCH_CPU_PORT), SW_REG_PORT_CONTROL,8,1,0x1);

	/* Set the CPU Port. */
	for (port=0; port<11; port++) {
		smi_setregfield(PHYADDR_PORT(port), SW_REG_PORT_CONTROL2,0,4,CONFIG_SWITCH_CPU_PORT);
	}
	
	/* Initialize the ports STP states */
	smi_getregfield(PHYADDR_GLOBAL, SW_REG_GLOBAL_STATUS,12,2,&regData);
	printf("Switch Mode                : %s\r\n", (regData == 0x00)? "CPU attached mode":"Stand alone mode");
	printf("\r\n");

#if BOARD_GE220044MD
	if(regData == 0x00) {
		smi_setregfield(PHYADDR_PORT(0), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(1), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(2), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(3), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(4), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(5), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(6), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(7), SW_REG_PORT_CONTROL,0,2,FORWARDING);
	} 
#else
	if(regData == 0x00) {
		smi_setregfield(PHYADDR_PORT(0), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(1), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(2), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(3), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(4), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(5), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(6), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(7), SW_REG_PORT_CONTROL,0,2,FORWARDING);
		smi_setregfield(PHYADDR_PORT(8), SW_REG_PORT_CONTROL,0,2,FORWARDING);
	} 
#endif

#if 0	
	smi_setregfield(PHYADDR_PORT(9), SW_REG_PORT_CONTROL,0,2,FORWARDING);
	smi_setregfield(PHYADDR_PORT(10), SW_REG_PORT_CONTROL,0,2,FORWARDING);
#endif

	return HAL_SWIF_SUCCESS;
#endif

    return HAL_SWIF_FAILURE;
}


void hal_swif_conf_initialize(void)
{
	int ret;

	printf("Loading configuration ...\r\n");

#if (BOARD_FEATURE & L2_PORT_CONFIG)
	printf("   Port base config        : ");
	ret = hal_swif_port_conf_initialize();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
#endif

#if (BOARD_FEATURE & L2_PORT_RATE_CTRL)
	printf("   Port rate limit config  : ");
	ret = hal_swif_rate_ctrl_conf_initialize();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
#endif

#if (BOARD_FEATURE & L2_PORT_MIRROR)
	printf("   Port mirror config      : ");
	ret = hal_swif_mirror_conf_initialize();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
#endif

#if (BOARD_FEATURE & L2_QOS)
	printf("   Port QOS config         : ");
    ret = hal_swif_qos_conf_initialize();
    if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
#endif

#if (BOARD_FEATURE & L2_PORT_VLAN)
	printf("   Port isolation config   : ");
    ret = hal_swif_port_isolation_conf_initialize();
    if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
#endif

#if (BOARD_FEATURE & L2_8021Q_VLAN)
	printf("   Port pvid config        : ");
    ret = hal_swif_pvid_conf_initialize();
    if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
	
	printf("   802.1q vlan config      : ");
    ret = hal_swif_8021q_vlan_conf_initialize();
    if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}	
#endif

#if (BOARD_FEATURE & L2_MAC_FILTER)
	printf("   Port security config    : ");
	ret = hal_swif_mac_security_conf_initialize();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else {
			printf("Done\r\n");
#if ((BOARD_GE22103MA || BOARD_GE_EXT_22002EA || BOARD_GE220044MD) && (BOARD_FEATURE & L2_MAC_FILTER))
	hal_interrupt_proc_entry();
#endif
		}
	}
#endif

#if (BOARD_FEATURE & L2_LINK_AGGREGATION)
	printf("   Port aggregation config : ");
	ret = hal_swif_aggr_conf_initialize();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
#endif

#if (BOARD_FEATURE & LOCAL_TRAP)
	printf("   Local trap config       : ");
	ret = conf_trap_init();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
#endif

#if (BOARD_FEATURE & L2_STATIC_MULTICAST)
		printf("   Static Multicast config : ");
		ret = hal_swif_mcast_conf_initialize();
		if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
			printf("Failed\r\n");
		} else {
			if(ret == CONF_ERR_NO_CFG)
				printf("Ignored\r\n");
			else if(ret == CONF_ERR_NOT_SUPPORT)
				printf("Not support\r\n");
			else
				printf("Done\r\n");
		}
#endif

	printf("\r\n");
}

uint32 gNeighborReqEnBitMap = 0;
extern hal_port_config_info_t gPortConfigInfo[];

void hal_swif_poll_task(void *arg)
{
#if BOARD_GE2C400U
	uint8	lport, hport;
	uint16	u16Data, OpModeStatus;
	uint32	u32Data;
	uint32	SDStatusMap = 0, CurrLinkMap = 0, PrevLinkMap = 0, Link_DownUp_Map, Link_UpDown_Map;
	uint32	NeighborClearEnBitMap=0;
	uint16	ReqID = 0;
	HAL_PORT_LINK_STATE CurrLinkStatus;
	uint8	LoopCount[MAX_PORT_NUM] = {0};
    uint8	LoopLinkStatusMax = 20000/LINK_STATUS_POLLING_DELAY; /* 20s */
    uint8   LoopCountLinkStatus[MAX_PORT_NUM] = {0};
	uint8	LoopCnt = 0;
	uint8	LoopMaxCount = 1000/LINK_STATUS_POLLING_DELAY;
	uint8	PortStatus[MAX_PORT_NUM] = {0};
	HAL_PORT_DUPLEX_STATE Duplex;
	HAL_PORT_SPEED_STATE Speed;
	hal_trap_port_status TrapPortStatus;


	while(1) {

		for(lport=1; lport<=MAX_PORT_NUM; lport++) {
         //   LoopCountLinkStatus[lport-1]++;
			hport = hal_swif_lport_2_hport(lport);

			if(lport > 12) {
				if(hal_swif_port_get_link_state(lport, &CurrLinkStatus) == HAL_SWIF_SUCCESS) {
        
                    /* If timeout neighbor_req_send 20S */
                    if(LoopCountLinkStatus[lport-1] >= LoopLinkStatusMax){
                        LoopCountLinkStatus[lport-1] = 0;
                    }
                    
					if(CurrLinkStatus == LINK_UP) {
						if((PrevLinkMap & (1<<(lport-1))) == 0) {	/* Previous LinkStatus is down */
							gNeighborReqEnBitMap |= (1<<(lport-1));
							NeighborClearEnBitMap |= (1<<(lport-1));
							LoopCount[lport-1] = 0;
							LoopCountLinkStatus[lport-1] = 0;
							ReqID = 1;
						}
						if(LoopCount[lport-1] >= LoopMaxCount)
							LoopCount[lport-1] = 0;
						/* If not received Neighbor_response,Neighbor_req_send per 1s,Else force Neighbor_req_send per 20s */
						if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && (gNeighborReqEnBitMap & (1<<(lport-1))) && (LoopCount[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
						else if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && ((gNeighborReqEnBitMap & (1<<(lport-1))) == 0) && (LoopCount[lport-1] == 0) \
																		&& (LoopCountLinkStatus[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
						LoopCount[lport-1]++;
						LoopCountLinkStatus[lport-1]++;
						CurrLinkMap |= (1<<(lport-1));
					} else {
						if(PrevLinkMap & (1<<(lport-1))) {			/* Previous LinkStatus is up */
							CurrLinkMap &= ~(uint32)(1<<(lport-1));
							gNeighborReqEnBitMap &= ~(1<<(lport-1));
							if(NeighborClearEnBitMap & (1<<(lport-1))) {
								hal_swif_neighbor_info_clear(lport);
								NeighborClearEnBitMap &= ~(uint32)(1<<(lport-1));
							}
						}
					}
				}
			} else {
				if(SDStatusMap & (1<<(lport-1))) {	/* SD signal is detected */
					/* Page183, Read SGMII Slave Register(Address 1Ch, Shadow 15h) */
					u16Data = 0x5400;
					robo_write(0x80+hport, 0x38, (u8 *)&u16Data, 2);
					robo_read(0x80+hport, 0x38, (u8 *)&u16Data, 2);
                    
                    /* If timeout neighbor_req_send 20S */
                    if(LoopCountLinkStatus[lport-1] >= LoopLinkStatusMax){
                        LoopCountLinkStatus[lport-1] = 0;
                        //u16Data &= 0xfdff; /* Set port link down */
                    }

					if(u16Data&0x0200) {							/* SGMII link up */
						if((PrevLinkMap & (1<<(lport-1))) == 0) {	/* Previous LinkStatus is down */
							gNeighborReqEnBitMap |= (1<<(lport-1));
							NeighborClearEnBitMap |= (1<<(lport-1));
							LoopCount[lport-1] = 0;
							LoopCountLinkStatus[lport-1] = 0;
							ReqID = 1;
						}
						if(LoopCount[lport-1] >= LoopMaxCount)
							LoopCount[lport-1] = 0;
						/* If not received Neighbor_response,Neighbor_req_send per 1s,Else force Neighbor_req_send per 20s */
						if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && (gNeighborReqEnBitMap & (1<<(lport-1))) && (LoopCount[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
						else if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && ((gNeighborReqEnBitMap & (1<<(lport-1))) == 0) && (LoopCount[lport-1] == 0) \
																		&& (LoopCountLinkStatus[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
						LoopCount[lport-1]++;
						LoopCountLinkStatus[lport-1]++;
						CurrLinkMap |= (1<<(lport-1));
						//continue;
					} else {										/* SGMII link down */
						if(u16Data&0x0080) {						/* Speed 1000M */
							/* Page180, Serdes 100BASE-FX Status Register (Address 1Ch, Shadow 13h) */
							/* Enable 100BASE-FX mode */
							u16Data = 0xCC0B;
							robo_write(0x80+hport, 0x38, (u8 *)&u16Data, 2);
						}

						if(u16Data&0x0040) {	/* Speed 100M */
							/* Page180, Serdes 100BASE-FX Status Register (Address 1Ch, Shadow 13h) */
							/* Enable 1000BASE-X mode */
							u16Data = 0xCC0A;
							robo_write(0x80+hport, 0x38, (u8 *)&u16Data, 2);
						}
					}
				}

				/* Page233, Expansion Register 42h: Operating Mode Status */
				u16Data = 0x0F42;
				robo_write(0x80+hport, 0x2E, (u8 *)&u16Data, 2);
				robo_read(0x80+hport, 0x2A, (u8 *)&OpModeStatus, 2);
				u16Data = 0x0000;
				robo_write(0x80+hport, 0x2E, (u8 *)&u16Data, 2);
				if(OpModeStatus & 0x0040) {
					SDStatusMap |= (1<<(lport-1));
					//printf("port%02d SD signal is detected\r\n", hport);
				} else {
					if(SDStatusMap & (1<<(lport-1))) {
						SDStatusMap &= ~(uint32)(1<<(lport-1));
						CurrLinkMap &= ~(uint32)(1<<(lport-1));
						gNeighborReqEnBitMap &= ~(uint32)(1<<(lport-1));
						if(NeighborClearEnBitMap & (1<<(lport-1))) {
							hal_swif_neighbor_info_clear(lport);
							NeighborClearEnBitMap &= ~(uint32)(1<<(lport-1));
						}
						/* Re-enable 1000BASE-X mode */
						u16Data = 0xCC0A;
						robo_write(0x80+hport, 0x38, (u8 *)&u16Data, 2);
					}
				}
			}
		}

		/* Process for Trap */
		if((gTrapInfo.FeatureEnable == HAL_TRUE) && (gTrapInfo.GateMask & TRAP_MASK_PORT_STATUS)) {
			if(CurrLinkMap ^ PrevLinkMap) {
				Link_DownUp_Map = CurrLinkMap & (CurrLinkMap ^ PrevLinkMap);
				Link_UpDown_Map = PrevLinkMap & (CurrLinkMap ^ PrevLinkMap);
				for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
					if(Link_DownUp_Map & (1<<(lport-1))) {
						hal_swif_port_get_duplex(lport, &Duplex);
						hal_swif_port_get_speed(lport, &Speed);
						PortStatus[lport-1] = 0x80 | ((uint8)Duplex << 3) | ((uint8)Speed << 4);
					}
					if(Link_UpDown_Map & (1<<(lport-1))) {
						PortStatus[lport-1] = 0;
					}
				}
				if(CurrLinkMap > 0) {
					gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] = HAL_TRUE;
					gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]++;
					LoopCnt = 0;
				}
			}

			if(LoopCnt >= LoopMaxCount)
				LoopCnt = 0;

			if((gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] == HAL_TRUE) && (LoopCnt == 0)) {
				memset(&TrapPortStatus, 0, sizeof(hal_trap_port_status));
				TrapPortStatus.TrapIndex = TRAP_INDEX_PORT_STATUS;
				TrapPortStatus.PortNum = DeviceBaseInfo.PortNum;
				memcpy(&TrapPortStatus.PortStatus[0], PortStatus, DeviceBaseInfo.PortNum);
				hal_swif_trap_send(gTrapInfo.ServerMac, (uint8 *)&TrapPortStatus, sizeof(hal_trap_port_status), gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]);
			}

			LoopCnt++;
		}

		PrevLinkMap = CurrLinkMap;
		
		vTaskDelay(LINK_STATUS_POLLING_DELAY);
	}

/******************************************************************************************************/          
#elif BOARD_GE20023MA 
	uint8	lport, hport;
	uint16	u16Data, OpModeStatus;
	uint32	SDStatusMap = 0;
	uint32	CurrLinkMap = 0, PrevLinkMap = 0, Link_DownUp_Map, Link_UpDown_Map;
	uint32	NeighborClearEnBitMap=0;
	uint16	ReqID = 0;
	HAL_PORT_LINK_STATE CurrLinkStatus;
	uint8	LoopCount[MAX_PORT_NUM] = {0};
    uint8	LoopLinkStatusMax = 20000/LINK_STATUS_POLLING_DELAY; /* 20s */
    uint8   LoopCountLinkStatus[MAX_PORT_NUM] = {0};
	uint8	LoopCnt = 0;
	uint8	LoopMaxCount = 1000/LINK_STATUS_POLLING_DELAY;
	uint8	PortStatus[MAX_PORT_NUM] = {0};
	HAL_PORT_DUPLEX_STATE Duplex;
	HAL_PORT_SPEED_STATE Speed;
	hal_trap_port_status TrapPortStatus;

	while(1) {
		for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
			if(hal_swif_port_get_link_state(lport, &CurrLinkStatus) == HAL_SWIF_SUCCESS) {
               
              //  LoopCountLinkStatus[lport-1]++; 
                /* If timeout neighbor_req_send 20s*/
                if(LoopCountLinkStatus[lport-1] >= LoopLinkStatusMax){
                    LoopCountLinkStatus[lport-1] = 0;
                }
                
				if(CurrLinkStatus == LINK_UP) {
					if((CurrLinkMap & (1<<(lport-1))) == 0) {	/* Previous LinkStatus is down */
						gNeighborReqEnBitMap |= (1<<(lport-1));
						NeighborClearEnBitMap |= (1<<(lport-1));
						LoopCount[lport-1] = 0;
						LoopCountLinkStatus[lport-1] = 0;
						ReqID = 1;
					}
					if(LoopCount[lport-1] >= LoopMaxCount)
						LoopCount[lport-1] = 0;
					/* If not received Neighbor_response,Neighbor_req_send per 1s,Else force Neighbor_req_send per 20s */
					if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && (gNeighborReqEnBitMap & (1<<(lport-1))) && (LoopCount[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
					else if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && ((gNeighborReqEnBitMap & (1<<(lport-1))) == 0) && (LoopCount[lport-1] == 0) \
																		&& (LoopCountLinkStatus[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
					LoopCount[lport-1]++;
					LoopCountLinkStatus[lport-1]++;
					CurrLinkMap |= (1<<(lport-1));
				} else {
					CurrLinkMap &= ~(uint32)(1<<(lport-1));
					gNeighborReqEnBitMap &= ~(1<<(lport-1));
					if(NeighborClearEnBitMap & (1<<(lport-1))) {
						hal_swif_neighbor_info_clear(lport);
						NeighborClearEnBitMap &= ~(uint32)(1<<(lport-1));
					}
				}
			}
		}

		/* Process for Trap */
		if((gTrapInfo.FeatureEnable == HAL_TRUE) && (gTrapInfo.GateMask & TRAP_MASK_PORT_STATUS)) {
			if(CurrLinkMap ^ PrevLinkMap) {
				Link_DownUp_Map = CurrLinkMap & (CurrLinkMap ^ PrevLinkMap);
				Link_UpDown_Map = PrevLinkMap & (CurrLinkMap ^ PrevLinkMap);
				for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
					if(Link_DownUp_Map & (1<<(lport-1))) {
						hal_swif_port_get_duplex(lport, &Duplex);
						hal_swif_port_get_speed(lport, &Speed);
						PortStatus[lport-1] = 0x80 | ((uint8)Duplex << 3) | ((uint8)Speed << 4);
					}
					if(Link_UpDown_Map & (1<<(lport-1))) {
						PortStatus[lport-1] = 0;
					}
				}
				if(CurrLinkMap > 0) {
					gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] = HAL_TRUE;
					gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]++;
					LoopCnt = 0;
				}
			}

			if(LoopCnt >= LoopMaxCount)
				LoopCnt = 0;

			if((gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] == HAL_TRUE) && (LoopCnt == 0)) {
				memset(&TrapPortStatus, 0, sizeof(hal_trap_port_status));
				TrapPortStatus.TrapIndex = TRAP_INDEX_PORT_STATUS;
				TrapPortStatus.PortNum = DeviceBaseInfo.PortNum;
				memcpy(&TrapPortStatus.PortStatus[0], PortStatus, DeviceBaseInfo.PortNum);
				hal_swif_trap_send(gTrapInfo.ServerMac, (uint8 *)&TrapPortStatus, sizeof(hal_trap_port_status), gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]);
			}

			LoopCnt++;

			PrevLinkMap = CurrLinkMap;
		}

		vTaskDelay(LINK_STATUS_POLLING_DELAY);
	}
    
/******************************************************************************************************/      
#elif BOARD_GE11500MD 
	uint8	lport, hport;
	uint16	u16Data, OpModeStatus;
	uint32	SDStatusMap = 0;
	uint32	CurrLinkMap = 0, PrevLinkMap = 0, Link_DownUp_Map, Link_UpDown_Map;
	uint32	NeighborClearEnBitMap=0;
	uint16	ReqID = 0;
	HAL_PORT_LINK_STATE CurrLinkStatus;
	uint8	LoopCount[MAX_PORT_NUM] = {0};
    uint8	LoopLinkStatusMax = 20000/LINK_STATUS_POLLING_DELAY; /* 20s */
    uint8   LoopCountLinkStatus[MAX_PORT_NUM] = {0};
	uint8	LoopCnt = 0;
	uint8	LoopMaxCount = 1000/LINK_STATUS_POLLING_DELAY;
	uint8	PortStatus[MAX_PORT_NUM] = {0};
	HAL_PORT_DUPLEX_STATE Duplex;
	HAL_PORT_SPEED_STATE Speed;
	hal_trap_port_status TrapPortStatus;

	while(1) {
		for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
			if(hal_swif_port_get_link_state(lport, &CurrLinkStatus) == HAL_SWIF_SUCCESS) {
                
               // LoopCountLinkStatus[lport-1]++; 
                /* If timeout neighbor_req_send 20S */
                if(LoopCountLinkStatus[lport-1] >= LoopLinkStatusMax){
                    LoopCountLinkStatus[lport-1] = 0;
                }
                
				if(CurrLinkStatus == LINK_UP) {
					if((CurrLinkMap & (1<<(lport-1))) == 0) {	/* Previous LinkStatus is down */
						gNeighborReqEnBitMap |= (1<<(lport-1));
						NeighborClearEnBitMap |= (1<<(lport-1));
						LoopCount[lport-1] = 0;
						LoopCountLinkStatus[lport-1] = 0;
						ReqID = 1;
					}
					if(LoopCount[lport-1] >= LoopMaxCount)
						LoopCount[lport-1] = 0;
					/* If not received Neighbor_response,Neighbor_req_send per 1s,Else force Neighbor_req_send per 20s */
					if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && (gNeighborReqEnBitMap & (1<<(lport-1))) && (LoopCount[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
					else if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && ((gNeighborReqEnBitMap & (1<<(lport-1))) == 0) && (LoopCount[lport-1] == 0) \
																		&& (LoopCountLinkStatus[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
					LoopCount[lport-1]++;
					LoopCountLinkStatus[lport-1]++;
					CurrLinkMap |= (1<<(lport-1));
				} else {
					CurrLinkMap &= ~(uint32)(1<<(lport-1));
					gNeighborReqEnBitMap &= ~(1<<(lport-1));
					if(NeighborClearEnBitMap & (1<<(lport-1))) {
						hal_swif_neighbor_info_clear(lport);
						NeighborClearEnBitMap &= ~(uint32)(1<<(lport-1));
					}
				}
			}
		}

		/* Process for Trap */
		if((gTrapInfo.FeatureEnable == HAL_TRUE) && (gTrapInfo.GateMask & TRAP_MASK_PORT_STATUS)) {
			if(CurrLinkMap ^ PrevLinkMap) {
				Link_DownUp_Map = CurrLinkMap & (CurrLinkMap ^ PrevLinkMap);
				Link_UpDown_Map = PrevLinkMap & (CurrLinkMap ^ PrevLinkMap);
				for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
					if(Link_DownUp_Map & (1<<(lport-1))) {
						hal_swif_port_get_duplex(lport, &Duplex);
						hal_swif_port_get_speed(lport, &Speed);
						PortStatus[lport-1] = 0x80 | ((uint8)Duplex << 3) | ((uint8)Speed << 4);
					}
					if(Link_UpDown_Map & (1<<(lport-1))) {
						PortStatus[lport-1] = 0;
					}
				}
				if(CurrLinkMap > 0) {
					gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] = HAL_TRUE;
					gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]++;
					LoopCnt = 0;
				}
			}

			if(LoopCnt >= LoopMaxCount)
				LoopCnt = 0;

			if((gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] == HAL_TRUE) && (LoopCnt == 0)) {
				memset(&TrapPortStatus, 0, sizeof(hal_trap_port_status));
				TrapPortStatus.TrapIndex = TRAP_INDEX_PORT_STATUS;
				TrapPortStatus.PortNum = DeviceBaseInfo.PortNum;
				memcpy(&TrapPortStatus.PortStatus[0], PortStatus, DeviceBaseInfo.PortNum);
				hal_swif_trap_send(gTrapInfo.ServerMac, (uint8 *)&TrapPortStatus, sizeof(hal_trap_port_status), gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]);
			}

			LoopCnt++;

			PrevLinkMap = CurrLinkMap;
		}

		vTaskDelay(LINK_STATUS_POLLING_DELAY);
	}
    
/******************************************************************************************************/
#elif BOARD_GE1040PU
	uint8	lport, hport;
	uint32	CurrLinkMap = 0, PrevLinkMap = 0, Link_DownUp_Map, Link_UpDown_Map;
	uint32	NeighborClearEnBitMap=0;
	uint16	ReqID = 0;
	HAL_PORT_LINK_STATE CurrLinkStatus;
	uint8	LoopCount[MAX_PORT_NUM] = {0};
    uint8	LoopLinkStatusMax = 20000/LINK_STATUS_POLLING_DELAY; /* 20s */
    uint8   LoopCountLinkStatus[MAX_PORT_NUM] = {0};
	uint8	LoopCnt = 0;
	uint8	LoopMaxCount = 1000/LINK_STATUS_POLLING_DELAY;
	uint8	PortStatus[MAX_PORT_NUM] = {0};
	HAL_PORT_DUPLEX_STATE Duplex;
	HAL_PORT_SPEED_STATE Speed;
	hal_trap_port_status TrapPortStatus;
#if RURAL_CREDIT_PROJECT
	extern u8 HonuKinStatus;
	extern u8 HonuPortStatus[];
    extern u8 HonuMasterFlag;
#endif

	while(1) {
		for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
#if RURAL_CREDIT_PROJECT			
			if((lport == 25) || (lport == 26)) {		/* Port25: SDI, Port26: OPT */
				if(HonuPortStatus[lport-25] == 0xC0) {	/* Port is locked */
					if((CurrLinkMap & (1<<(lport-1))) == 0) {	/* Previous Status is not locked */
						LoopCount[lport-1] = 0;
						ReqID = 1;
					}
					if(LoopCount[lport-1] >= LoopMaxCount)
						LoopCount[lport-1] = 0;
					LoopCount[lport-1]++;
					CurrLinkMap |= (1<<(lport-1));
				} else {
					CurrLinkMap &= ~(uint32)(1<<(lport-1));
				}
			} else {
#endif
				if(hal_swif_port_get_link_state(lport, &CurrLinkStatus) == HAL_SWIF_SUCCESS) {
                    
                   // LoopCountLinkStatus[lport-1]++; 
                    /* If timeout neighbor_req_send 20S */
                    if(LoopCountLinkStatus[lport-1] >= LoopLinkStatusMax){
                        LoopCountLinkStatus[lport-1] = 0;
                    }
                    
					if(CurrLinkStatus == LINK_UP) {
						if((CurrLinkMap & (1<<(lport-1))) == 0) {	/* Previous LinkStatus is down */
							gNeighborReqEnBitMap |= (1<<(lport-1));
							NeighborClearEnBitMap |= (1<<(lport-1));
							LoopCount[lport-1] = 0;
							LoopCountLinkStatus[lport-1] = 0;
							ReqID = 1;
						}
						if(LoopCount[lport-1] >= LoopMaxCount)
							LoopCount[lport-1] = 0;
						/* If not received Neighbor_response,Neighbor_req_send per 1s,Else force Neighbor_req_send per 20s */
						if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && (gNeighborReqEnBitMap & (1<<(lport-1))) && (LoopCount[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
						else if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && ((gNeighborReqEnBitMap & (1<<(lport-1))) == 0) && (LoopCount[lport-1] == 0) \
																		&& (LoopCountLinkStatus[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
						LoopCount[lport-1]++;
						LoopCountLinkStatus[lport-1]++;
						CurrLinkMap |= (1<<(lport-1));
					} else {
						CurrLinkMap &= ~(uint32)(1<<(lport-1));
						gNeighborReqEnBitMap &= ~(1<<(lport-1));
						if(NeighborClearEnBitMap & (1<<(lport-1))) {
							hal_swif_neighbor_info_clear(lport);
							NeighborClearEnBitMap &= ~(uint32)(1<<(lport-1));
						}
					}
				}
#if RURAL_CREDIT_PROJECT				
			}
#endif			
		}

		/* Process for Trap */
		if((gTrapInfo.FeatureEnable == HAL_TRUE) && (gTrapInfo.GateMask & TRAP_MASK_PORT_STATUS)) {
			if(CurrLinkMap ^ PrevLinkMap) {
				Link_DownUp_Map = CurrLinkMap & (CurrLinkMap ^ PrevLinkMap);
				Link_UpDown_Map = PrevLinkMap & (CurrLinkMap ^ PrevLinkMap);
				for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {	
					if(Link_DownUp_Map & (1<<(lport-1))) {
#if RURAL_CREDIT_PROJECT
						if((lport == 25) || (lport == 26)) {		/* Port25: SDI, Port26: OPT */
							PortStatus[lport-1] = 0xC0;
						} else {
#endif
							hal_swif_port_get_duplex(lport, &Duplex);
							hal_swif_port_get_speed(lport, &Speed);
							PortStatus[lport-1] = 0x80 | ((uint8)Duplex << 3) | ((uint8)Speed << 4);
#if RURAL_CREDIT_PROJECT
						}
#endif
					}
					if(Link_UpDown_Map & (1<<(lport-1))) {	
#if RURAL_CREDIT_PROJECT
						if((lport == 25) || (lport == 26)) {		/* Port25: SDI, Port26: OPT */
							PortStatus[lport-1] = 0x40;
						} else {
#endif						
							PortStatus[lport-1] = 0;
#if RURAL_CREDIT_PROJECT
						}
#endif
					}				
				}

#if RURAL_CREDIT_PROJECT
				if((CurrLinkMap & 0x03FFFFFF) > 0) 
#else
				if(CurrLinkMap > 0) 
#endif
				{
					gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] = HAL_TRUE;
					gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]++;
					LoopCnt = 0;
				}
			}					

			if(LoopCnt >= LoopMaxCount)
				LoopCnt = 0;
			
			if((gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] == HAL_TRUE) && (LoopCnt == 0)) {
				memset(&TrapPortStatus, 0, sizeof(hal_trap_port_status));
				TrapPortStatus.TrapIndex = TRAP_INDEX_PORT_STATUS;
/*#if RURAL_CREDIT_PROJECT
				if(HonuMasterFlag)
					TrapPortStatus.PortNum = 26;
				else
					TrapPortStatus.PortNum = 24;
#else*/
				TrapPortStatus.PortNum = DeviceBaseInfo.PortNum;
//#endif
				memcpy(&TrapPortStatus.PortStatus[0], PortStatus, DeviceBaseInfo.PortNum);
				hal_swif_trap_send(gTrapInfo.ServerMac, (uint8 *)&TrapPortStatus, sizeof(hal_trap_port_status), gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]);
			}

			LoopCnt++;
			
			PrevLinkMap = CurrLinkMap;
		}
		
		vTaskDelay(LINK_STATUS_POLLING_DELAY);
	}

/************************************************************************************************************/

#elif BOARD_GE204P0U
	uint8	lport, hport;
	uint32	CurrLinkMap = 0, PrevLinkMap = 0, Link_DownUp_Map, Link_UpDown_Map;
	uint32	NeighborClearEnBitMap=0;
	uint16	ReqID = 0;
	HAL_PORT_LINK_STATE CurrLinkStatus;
	uint8	LoopCount[MAX_PORT_NUM] = {0};
    uint8	LoopLinkStatusMax = 20000/LINK_STATUS_POLLING_DELAY; /* 20s */
    uint8   LoopCountLinkStatus[MAX_PORT_NUM] = {0};
	uint8	LoopCnt = 0;
	uint8	LoopMaxCount = 1000/LINK_STATUS_POLLING_DELAY;
	uint8	PortStatus[MAX_PORT_NUM] = {0};
	HAL_PORT_DUPLEX_STATE Duplex;
	HAL_PORT_SPEED_STATE Speed;
	hal_trap_port_status TrapPortStatus;
	
	while(1) {
		for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
			if(hal_swif_port_get_link_state(lport, &CurrLinkStatus) == HAL_SWIF_SUCCESS) {
                
             //   LoopCountLinkStatus[lport-1]++;
                /* If timeout neighbor_req_send 20S */
                if(LoopCountLinkStatus[lport-1] >= LoopLinkStatusMax){
                    LoopCountLinkStatus[lport-1] = 0;
                }
                
				if(CurrLinkStatus == LINK_UP) {
					if((CurrLinkMap & (1<<(lport-1))) == 0) {	/* Previous LinkStatus is down */
						gNeighborReqEnBitMap |= (1<<(lport-1));
						NeighborClearEnBitMap |= (1<<(lport-1));
						LoopCount[lport-1] = 0;
						LoopCountLinkStatus[lport-1] = 0;
						ReqID = 1;
					}
					if(LoopCount[lport-1] >= LoopMaxCount)
						LoopCount[lport-1] = 0;
					/* If not received Neighbor_response,Neighbor_req_send per 1s,Else force Neighbor_req_send per 20s */
					if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && (gNeighborReqEnBitMap & (1<<(lport-1))) && (LoopCount[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
					else if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && ((gNeighborReqEnBitMap & (1<<(lport-1))) == 0) && (LoopCount[lport-1] == 0) \
																		&& (LoopCountLinkStatus[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
					LoopCount[lport-1]++;
					LoopCountLinkStatus[lport-1]++;
					CurrLinkMap |= (1<<(lport-1));
				} else {
					CurrLinkMap &= ~(uint32)(1<<(lport-1));
					gNeighborReqEnBitMap &= ~(1<<(lport-1));
					if(NeighborClearEnBitMap & (1<<(lport-1))) {
						hal_swif_neighbor_info_clear(lport);
						NeighborClearEnBitMap &= ~(uint32)(1<<(lport-1));
					}
				}
			}		
		}

		/* Process for Trap */
	//	if((gTrapInfo.FeatureEnable == HAL_TRUE) && (gTrapInfo.GateMask & TRAP_MASK_PORT_STATUS)) {
		if(CurrLinkMap ^ PrevLinkMap) {
			Link_DownUp_Map = CurrLinkMap & (CurrLinkMap ^ PrevLinkMap);
			Link_UpDown_Map = PrevLinkMap & (CurrLinkMap ^ PrevLinkMap);
			for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {	
				if(Link_DownUp_Map & (1<<(lport-1))) {
						hal_swif_port_get_duplex(lport, &Duplex);
						hal_swif_port_get_speed(lport, &Speed);
						PortStatus[lport-1] = 0x80 | ((uint8)Duplex << 3) | ((uint8)Speed << 4);
						hal_swif_aggr_link_changed(lport, LINK_UP);
				}
				if(Link_UpDown_Map & (1<<(lport-1))) {					
						PortStatus[lport-1] = 0;
						hal_swif_aggr_link_changed(lport, LINK_DOWN);
				}				
			}
		}
		if((gTrapInfo.FeatureEnable == HAL_TRUE) && (gTrapInfo.GateMask & TRAP_MASK_PORT_STATUS)) {
			if(CurrLinkMap ^ PrevLinkMap) {
				if(CurrLinkMap > 0) 
				{
					gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] = HAL_TRUE;
					gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]++;
					LoopCnt = 0;
				}
			}
			if(LoopCnt >= LoopMaxCount)
				LoopCnt = 0;
			
			if((gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] == HAL_TRUE) && (LoopCnt == 0)) {
				memset(&TrapPortStatus, 0, sizeof(hal_trap_port_status));
				TrapPortStatus.TrapIndex = TRAP_INDEX_PORT_STATUS;
				TrapPortStatus.PortNum = DeviceBaseInfo.PortNum;
				memcpy(&TrapPortStatus.PortStatus[0], PortStatus, DeviceBaseInfo.PortNum);
				hal_swif_trap_send(gTrapInfo.ServerMac, (uint8 *)&TrapPortStatus, sizeof(hal_trap_port_status), gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]);
			}
			LoopCnt++;					
		}
		PrevLinkMap = CurrLinkMap;
		
		vTaskDelay(LINK_STATUS_POLLING_DELAY);
	}

#elif (BOARD_GE22103MA) || (BOARD_GE_EXT_22002EA) || (BOARD_GE220044MD)
	uint8	lport, hport;
	uint16	u16Data, OpModeStatus;
	uint32	SDStatusMap = 0;
	uint32	CurrLinkMap = 0, PrevLinkMap = 0, Link_DownUp_Map, Link_UpDown_Map;
	uint32	NeighborClearEnBitMap=0;
	uint16	ReqID = 0;
	HAL_PORT_LINK_STATE CurrLinkStatus;
	uint8	LoopCount[MAX_PORT_NUM] = {0};
    uint8	LoopLinkStatusMax = 20000/LINK_STATUS_POLLING_DELAY; /* 20s */
    uint8   LoopCountLinkStatus[MAX_PORT_NUM] = {0};
	uint8	LoopCnt = 0;
	uint8	LoopMaxCount = 1000/LINK_STATUS_POLLING_DELAY;
	uint8	PortStatus[MAX_PORT_NUM] = {0};
	HAL_PORT_DUPLEX_STATE Duplex;
	HAL_PORT_SPEED_STATE Speed;
	hal_trap_port_status TrapPortStatus;

	while(1) {
		for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
			if(hal_swif_port_get_link_state(lport, &CurrLinkStatus) == HAL_SWIF_SUCCESS) {
				
                /* If timeout neighbor_req_send 20S */
                if(LoopCountLinkStatus[lport-1] >= LoopLinkStatusMax){
                    LoopCountLinkStatus[lport-1] = 0;
                }
                
				if(CurrLinkStatus == LINK_UP) {
					if((CurrLinkMap & (1<<(lport-1))) == 0) {	/* Previous LinkStatus is down */
						gNeighborReqEnBitMap |= (1<<(lport-1));
						NeighborClearEnBitMap |= (1<<(lport-1));
						LoopCount[lport-1] = 0;
						LoopCountLinkStatus[lport-1] = 0;
						ReqID = 1;
					}
					if(LoopCount[lport-1] >= LoopMaxCount)
						LoopCount[lport-1] = 0;
					/* If not received Neighbor_response,Neighbor_req_send per 1s,Else force Neighbor_req_send per 20s */
					if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && (gNeighborReqEnBitMap & (1<<(lport-1))) && (LoopCount[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
					else if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && ((gNeighborReqEnBitMap & (1<<(lport-1))) == 0) && (LoopCount[lport-1] == 0) \
																		&& (LoopCountLinkStatus[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
					LoopCount[lport-1]++;
					LoopCountLinkStatus[lport-1]++;
					CurrLinkMap |= (1<<(lport-1));
				} else {
					CurrLinkMap &= ~(uint32)(1<<(lport-1));
					gNeighborReqEnBitMap &= ~(1<<(lport-1));
					if(NeighborClearEnBitMap & (1<<(lport-1))) {
						hal_swif_neighbor_info_clear(lport);
						NeighborClearEnBitMap &= ~(uint32)(1<<(lport-1));
					}
				}
			}
		}
		/* Process for Trap */
		if(CurrLinkMap ^ PrevLinkMap) {
			Link_DownUp_Map = CurrLinkMap & (CurrLinkMap ^ PrevLinkMap);
			Link_UpDown_Map = PrevLinkMap & (CurrLinkMap ^ PrevLinkMap);
			for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
				if(Link_DownUp_Map & (1<<(lport-1))) {
					hal_swif_port_get_duplex(lport, &Duplex);
					hal_swif_port_get_speed(lport, &Speed);
					PortStatus[lport-1] = 0x80 | ((uint8)Duplex << 3) | ((uint8)Speed << 4);
					hal_swif_aggr_link_changed(lport, LINK_UP);
				}
				if(Link_UpDown_Map & (1<<(lport-1))) {
					PortStatus[lport-1] = 0;
					hal_swif_aggr_link_changed(lport, LINK_DOWN);
				}
			}
		}
		
		if((gTrapInfo.FeatureEnable == HAL_TRUE) && (gTrapInfo.GateMask & TRAP_MASK_PORT_STATUS)) {
            if(CurrLinkMap ^ PrevLinkMap) {
                if(CurrLinkMap > 0) {
                    gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] = HAL_TRUE;
                    gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]++;
                    LoopCnt = 0;
                }
            }

			if(LoopCnt >= LoopMaxCount)
				LoopCnt = 0;
		
			if((gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] == HAL_TRUE) && (LoopCnt == 0)) {
				memset(&TrapPortStatus, 0, sizeof(hal_trap_port_status));
				TrapPortStatus.TrapIndex = TRAP_INDEX_PORT_STATUS;
				TrapPortStatus.PortNum = DeviceBaseInfo.PortNum;
				memcpy(&TrapPortStatus.PortStatus[0], PortStatus, DeviceBaseInfo.PortNum);
				hal_swif_trap_send(gTrapInfo.ServerMac, (uint8 *)&TrapPortStatus, sizeof(hal_trap_port_status), gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]);
			}           
			LoopCnt++;
		}
        PrevLinkMap = CurrLinkMap;
        
		vTaskDelay(LINK_STATUS_POLLING_DELAY);
	}

/******************************************************************************************************/
#elif BOARD_GV3S_HONUE_QM
	uint8	lport, hport, Gv3slport;
	uint16	u16Data, OpModeStatus;
	uint32	SDStatusMap = 0;
	uint32	CurrLinkMap = 0, PrevLinkMap = 0, Link_DownUp_Map, Link_UpDown_Map;
	uint32	NeighborClearEnBitMap=0;
	uint16	ReqID = 0;
	HAL_PORT_LINK_STATE CurrLinkStatus;
	uint8	LoopCount[MAX_PORT_NUM] = {0};
    uint8	LoopLinkStatusMax = 20000/LINK_STATUS_POLLING_DELAY; /* 20s */
    uint8   LoopCountLinkStatus[MAX_PORT_NUM] = {0};
	uint8	LoopCnt = 0;
	uint8	LoopMaxCount = 1000/LINK_STATUS_POLLING_DELAY;   //10
	uint8	PortStatus[MAX_PORT_NUM] = {0};
	HAL_PORT_DUPLEX_STATE Duplex;
	HAL_PORT_SPEED_STATE Speed;
	hal_trap_port_status TrapPortStatus;

    PortStatus[8] = 0x40; /* port9  SDI非网络类口bit6 */
    PortStatus[9] = 0x40; /* port10 OPT非网络类口bit6 */
	while(1) {
		/* 添加一个SDI逻辑端口,从8个rj45、opt到sdi按次序数是1口到10口 */
		for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
			if(lport>=1 && lport<=8 ){
				if(hal_swif_port_get_link_state(lport, &CurrLinkStatus) == HAL_SWIF_SUCCESS) {
                    
                 //   LoopCountLinkStatus[lport-1]++; 
                    /* If timeout neighbor_req_send 20S */
                    if(LoopCountLinkStatus[lport-1] >= LoopLinkStatusMax){
                        LoopCountLinkStatus[lport-1] = 0;
                    }
                    
					if(CurrLinkStatus == LINK_UP) {
						if((CurrLinkMap & (1<<(lport-1))) == 0) {	/* Previous LinkStatus is down */
							gNeighborReqEnBitMap |= (1<<(lport-1));
							NeighborClearEnBitMap |= (1<<(lport-1));
							LoopCount[lport-1] = 0;
							LoopCountLinkStatus[lport-1] = 0;
							ReqID = 1;
						}
						if(LoopCount[lport-1] >= LoopMaxCount)
							LoopCount[lport-1] = 0;
						/* If not received Neighbor_response,Neighbor_req_send per 1s,Else force Neighbor_req_send per 20s */
						if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && (gNeighborReqEnBitMap & (1<<(lport-1))) && (LoopCount[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
						else if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && ((gNeighborReqEnBitMap & (1<<(lport-1))) == 0) && (LoopCount[lport-1] == 0) \
																		&& (LoopCountLinkStatus[lport-1] == 0))
							hal_swif_neighbor_req_send(lport, ReqID++);
						LoopCount[lport-1]++;
						LoopCountLinkStatus[lport-1]++;
						CurrLinkMap |= (1<<(lport-1));
					} else {
						CurrLinkMap &= ~(uint32)(1<<(lport-1));
						gNeighborReqEnBitMap &= ~(1<<(lport-1));
						if(NeighborClearEnBitMap & (1<<(lport-1))) {
							hal_swif_neighbor_info_clear(lport);
							NeighborClearEnBitMap &= ~(uint32)(1<<(lport-1));
						}
					}
				}
			}

			if(lport == 10){
				if(fpga_get_opt_lock(1, (FPGA_BOOL *)&CurrLinkStatus) == HAL_SWIF_SUCCESS) {
					if(CurrLinkStatus == LINK_UP) {
						if((CurrLinkMap & (1<<(lport-1))) == 0) {	/* Previous LinkStatus is down */
							gNeighborReqEnBitMap |= (1<<(lport-1));
							NeighborClearEnBitMap |= (1<<(lport-1));
							LoopCount[lport-1] = 0;
							ReqID = 1;
						}
						if(LoopCount[lport-1] >= LoopMaxCount)
							LoopCount[lport-1] = 0;
						if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && (gNeighborReqEnBitMap & (1<<(lport-1))) && (LoopCount[lport-1] == 0))
							//hal_swif_neighbor_req_send(lport, ReqID++);
						LoopCount[lport-1]++;
						CurrLinkMap |= (1<<(lport-1));
					} else {
						CurrLinkMap &= ~(uint32)(1<<(lport-1));
						gNeighborReqEnBitMap &= ~(1<<(lport-1));
						if(NeighborClearEnBitMap & (1<<(lport-1))) {
							//hal_swif_neighbor_info_clear(lport);
							NeighborClearEnBitMap &= ~(uint32)(1<<(lport-1));
						}
					}
				}


			}

			if(lport == 9){
				if(fpga_get_sdi_status(1, (FPGA_BOOL *)&CurrLinkStatus) == HAL_SWIF_SUCCESS) {
					if(CurrLinkStatus == LINK_UP) {
						if((CurrLinkMap & (1<<(lport-1))) == 0) {	/* Previous LinkStatus is down */
							gNeighborReqEnBitMap |= (1<<(lport-1));
							NeighborClearEnBitMap |= (1<<(lport-1));
							LoopCount[lport-1] = 0;
							ReqID = 1;
						}
						if(LoopCount[lport-1] >= LoopMaxCount)
							LoopCount[lport-1] = 0;
						if((gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE) && (gNeighborReqEnBitMap & (1<<(lport-1))) && (LoopCount[lport-1] == 0))
							//hal_swif_neighbor_req_send(lport, ReqID++);
						LoopCount[lport-1]++;
						CurrLinkMap |= (1<<(lport-1));
					} else {
						CurrLinkMap &= ~(uint32)(1<<(lport-1));
						gNeighborReqEnBitMap &= ~(1<<(lport-1));
						if(NeighborClearEnBitMap & (1<<(lport-1))) {
							//hal_swif_neighbor_info_clear(lport);
							NeighborClearEnBitMap &= ~(uint32)(1<<(lport-1));
						}
					}
				}

			}


		}

		/* Process for Trap */
		if((gTrapInfo.FeatureEnable == HAL_TRUE) && (gTrapInfo.GateMask & TRAP_MASK_PORT_STATUS)) {
			if(CurrLinkMap ^ PrevLinkMap) {
				Link_DownUp_Map = CurrLinkMap & (CurrLinkMap ^ PrevLinkMap);
				Link_UpDown_Map = PrevLinkMap & (CurrLinkMap ^ PrevLinkMap);
				/* 添加一个SDI逻辑端口,从8个rj45、opt到sdi按次序数是1口到10口 */
				for(lport=1; lport<=DeviceBaseInfo.PortNum + 1; lport++) {

					if(lport>=1 && lport<=8){
						if(Link_DownUp_Map & (1<<(lport-1))) {
							hal_swif_port_get_duplex(lport, &Duplex);
							hal_swif_port_get_speed(lport, &Speed);
							PortStatus[lport-1] = 0x80 | ((uint8)Duplex << 3) | ((uint8)Speed << 4);
						}
						if(Link_UpDown_Map & (1<<(lport-1))) {
							PortStatus[lport-1] = 0;
						}
					}

					if(lport == 10){
                        
						if(Link_DownUp_Map & (1<<(lport-1))) {
							hal_swif_port_get_duplex(lport, &Duplex);
							hal_swif_port_get_speed(lport, &Speed);
							PortStatus[lport-1] = 0xC0 | ((uint8)Duplex << 3) | ((uint8)Speed << 4);
						}
						if(Link_UpDown_Map & (1<<(lport-1))) {
							PortStatus[lport-1] = 0x40; /* 非网络类口bit6 */
						}

					}

					if(lport == 9){
                        
						if(Link_DownUp_Map & (1<<(lport-1))) {
							PortStatus[lport-1] = 0xC0;
						}
						if(Link_UpDown_Map & (1<<(lport-1))) {
							PortStatus[lport-1] = 0x40; /* 非网络类口bit6 */
						}

					}


				}
				if(CurrLinkMap > 0) {
					gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] = HAL_TRUE;
					gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]++;
					LoopCnt = 0;
				}
			}

			if(LoopCnt >= LoopMaxCount)
				LoopCnt = 0;

			if((gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] == HAL_TRUE) && (LoopCnt == 0)) {
				memset(&TrapPortStatus, 0, sizeof(hal_trap_port_status));
				TrapPortStatus.TrapIndex = TRAP_INDEX_PORT_STATUS;
				TrapPortStatus.PortNum = DeviceBaseInfo.PortNum;
				memcpy(&TrapPortStatus.PortStatus[0], PortStatus, DeviceBaseInfo.PortNum);
				hal_swif_trap_send(gTrapInfo.ServerMac, (uint8 *)&TrapPortStatus, sizeof(hal_trap_port_status), gTrapInfo.RequestID[TRAP_INDEX_PORT_STATUS]);
			}

			LoopCnt++;

			PrevLinkMap = CurrLinkMap;
		}

		vTaskDelay(LINK_STATUS_POLLING_DELAY);
	}

/******************************************************************************************************/
#else
	while(1) {
		vTaskDelay(500);
	}
#endif

}

#if (BOARD_GE1040PU || BOARD_GE204P0U)

void combo_led_control_task(void *arg)
{
	uint8	ge_port, medium_vec, link_vec, io_data_curr = 0, io_data_prev = 0;
	uint16	u16Data;
	uint32	GExLinkMaskCurr = 0;
	uint16	Bcm54640ModeCtrlRegVal;
	extern xSemaphoreHandle EXTI9_5_Semaphore;

	//os_mutex_init(&swif_poll_mutex);

	while(1) {
		if(EXTI9_5_Semaphore != NULL) {
			if(xSemaphoreTake(EXTI9_5_Semaphore, portMAX_DELAY) == pdTRUE) {

				//os_mutex_lock(&swif_poll_mutex, OS_MUTEX_WAIT_FOREVER);

				/* Read 1000BASE-T/100BASE-TX/10BASE-T Interrupt Status */
				for(ge_port=0; ge_port<4; ge_port++) {
					robo_read(0xD9+ge_port, 0x34, (u8 *)&u16Data, 2);
				}

				/* Read Fiber Interrupt Status */
				for(ge_port=0; ge_port<4; ge_port++) {
					u16Data = 0x0F01;
					robo_write(0xD9+ge_port, 0x2E, (u8 *)&u16Data, 2);
					robo_read(0xD9+ge_port, 0x2A, (u8 *)&u16Data, 2);
					u16Data = 0x0000;
					robo_write(0xD9+ge_port, 0x2E, (u8 *)&u16Data, 2);
				}

				vTaskDelay(10);

				robo_read(0x02, 0x10, (u8 *)&GExLinkMaskCurr, 4);
				link_vec = (uint8)((GExLinkMaskCurr & 0x1E000000) >> 25);

				medium_vec = 0;
				for(ge_port=0; ge_port<4; ge_port++) {
					Bcm54640ModeCtrlRegVal = 0x7C00;
					robo_write(0xD9+ge_port, 0x38, (u8 *)&Bcm54640ModeCtrlRegVal, 2);
					robo_read(0xD9+ge_port, 0x38, (u8 *)&Bcm54640ModeCtrlRegVal, 2);
					if(Bcm54640ModeCtrlRegVal & 0x0002) {
						//_GE_LedCtrl_IOLevel(1<<ge_port, 1);
						medium_vec |= 1<<ge_port;
					} else {
						//_GE_LedCtrl_IOLevel(1<<ge_port, 0);
					}
				}

				io_data_curr = (link_vec << 4) | medium_vec;
				if(io_data_curr ^ io_data_prev) {
					#if 0
					printf("GE28~25 link-medium = (%d-%d-%d-%d)-(%d-%d-%d-%d), send_data = 0x%02x\r\n",
						(link_vec & BCM53286_GE28_PORT) >> 3, (link_vec & BCM53286_GE27_PORT) >> 2,
						(link_vec & BCM53286_GE26_PORT) >> 1, link_vec & BCM53286_GE25_PORT,
						(medium_vec & BCM53286_GE28_PORT) >> 3, (medium_vec & BCM53286_GE27_PORT) >> 2,
						(medium_vec & BCM53286_GE26_PORT) >> 1, medium_vec & BCM53286_GE25_PORT, io_data_curr);
					#endif
					IO_Send(io_data_curr);
				}

				io_data_prev = io_data_curr;

				//os_mutex_unlock(&swif_poll_mutex);
			}
		} else {
			vTaskDelay(200);
		}
	}
}

#endif

xSemaphoreHandle xSemTraffic = NULL;
hal_port_traffic_info_t	gPortTrafficInfo[MAX_PORT_NUM];

void hal_swif_traffic_task(void *arg)
{
	uint8 lport;
	uint8 FirstTrafficFlag[MAX_PORT_NUM] = {0};
	hal_port_counters_t counter;
	uint16 valid_bit_mask, traffic_status;
	hal_trap_traffic_status TrapTrafficStatus;
	uint8 	TrapFlag = 0;
	uint8	hport;
	uint16	u16PhyRegVal;
	
    /* initializes counters clear flag mutex. */
    cnt_clr_mutex_init();
    
	for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
		gPortTrafficInfo[lport-1].PortNo = lport;
		gPortTrafficInfo[lport-1].Interval = HAL_DEFAULT_TRAFFIC_INTERVAL;
		if((gPortConfigInfo[lport-1].PortType == S1000M_CABLE) || (gPortConfigInfo[lport-1].PortType == S1000M_OPTICAL)) {
			gPortTrafficInfo[lport-1].Threshold_TxOctets = 12500000 * (gPortConfigInfo[lport-1].TxThreshold + 1) * HAL_DEFAULT_TRAFFIC_INTERVAL;
			gPortTrafficInfo[lport-1].Threshold_RxOctets = 12500000 * (gPortConfigInfo[lport-1].RxThreshold + 1) * HAL_DEFAULT_TRAFFIC_INTERVAL;
		} else {
			gPortTrafficInfo[lport-1].Threshold_TxOctets = 1250000 * (gPortConfigInfo[lport-1].TxThreshold + 1) * HAL_DEFAULT_TRAFFIC_INTERVAL;
			gPortTrafficInfo[lport-1].Threshold_RxOctets = 1250000 * (gPortConfigInfo[lport-1].RxThreshold + 1) * HAL_DEFAULT_TRAFFIC_INTERVAL;
		}
		gPortTrafficInfo[lport-1].PrevTxOctets = 0;
		gPortTrafficInfo[lport-1].PrevRxOctets = 0;
		gPortTrafficInfo[lport-1].Interval_TxOctets = 0;
		gPortTrafficInfo[lport-1].Interval_RxOctets = 0;
	}

	for(;;) {
		if(xSemaphoreTake(xSemTraffic, portMAX_DELAY) == pdTRUE) {
			memset(&TrapTrafficStatus, 0, sizeof(hal_trap_traffic_status));
			TrapTrafficStatus.TrapIndex = TRAP_INDEX_TRAFFIC_OVER;
			TrapTrafficStatus.PortNum = DeviceBaseInfo.PortNum;
			TrapFlag = 0;
			
			for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
                if(hal_swif_port_get_clear_counters_flag(lport) == 1){
                    gPortTrafficInfo[lport-1].PrevTxOctets = 0;
					gPortTrafficInfo[lport-1].PrevRxOctets = 0;
					gPortTrafficInfo[lport-1].Interval_TxOctets = 0;
					gPortTrafficInfo[lport-1].Interval_RxOctets = 0;
					FirstTrafficFlag[lport-1] = 0;
                    continue;
                }
                    
				if((gPortConfigInfo[lport-1].TxThreshold < PERCENTAGE_100) || (gPortConfigInfo[lport-1].RxThreshold < PERCENTAGE_100)) {
					if(hal_swif_port_get_counters(lport, &counter, &valid_bit_mask) == HAL_SWIF_SUCCESS) {
						if(FirstTrafficFlag[lport-1] == 0) {
							gPortTrafficInfo[lport-1].PrevTxOctets = counter.TxOctetsLo;
							gPortTrafficInfo[lport-1].PrevRxOctets = counter.RxGoodOctetsLo;
							gPortTrafficInfo[lport-1].Interval_TxOctets = 0;
							gPortTrafficInfo[lport-1].Interval_RxOctets = 0;
							FirstTrafficFlag[lport-1] = 1;
							continue;
						}

						traffic_status = 0;
						if(gPortConfigInfo[lport-1].TxThreshold < PERCENTAGE_100) {
							if(counter.TxOctetsLo < gPortTrafficInfo[lport-1].PrevTxOctets) {
								gPortTrafficInfo[lport-1].Interval_TxOctets = (0xFFFFFFFF - gPortTrafficInfo[lport-1].PrevTxOctets + 1 + counter.TxOctetsLo);
							} else {
								gPortTrafficInfo[lport-1].Interval_TxOctets = (counter.TxOctetsLo - gPortTrafficInfo[lport-1].PrevTxOctets);
							}

							if(gPortTrafficInfo[lport-1].Interval_TxOctets >= gPortTrafficInfo[lport-1].Threshold_TxOctets) {
#if BOARD_GE204P0U				
								hport = hal_swif_lport_2_hport(lport);
								if((lport >= 25) && (lport <= 28)){
									u16PhyRegVal = 0x7C00;
									robo_write(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2); 	        
									robo_read(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2);
									/* Fiber select */
									if((u16PhyRegVal & 0x0040) && ((u16PhyRegVal & 0x0002) == 0x2)) {
										TrapFlag = 1;            /* traffick trap send */
									}else{
										TrapFlag = 0;
									}
								}else if(lport >= 29){
									u16PhyRegVal = 0x7C00;
									robo_write(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2); 	        
									robo_read(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2);
									/* Copper select */
									if((u16PhyRegVal & 0x0080) && ((u16PhyRegVal & 0x0002) == 0x0)) {
										TrapFlag = 1;			 /* traffick trap send */
									}else{
										TrapFlag = 0;
									}
								}else{
									TrapFlag = 1;                 /* lport 1-24 */
								}																
								traffic_status |= (0x8000 | ((uint16)(gPortConfigInfo[lport-1].TxThreshold) << 4));
								TrapTrafficStatus.TrafficStatus[2*(lport-1)] = (uint8)((traffic_status & 0xFF00) >> 8); 
								TrapTrafficStatus.TrafficStatus[2*(lport-1)+1] = (uint8)(traffic_status & 0x00FF); 								
#else
								traffic_status |= (0x8000 | ((uint16)(gPortConfigInfo[lport-1].TxThreshold) << 4));
								TrapTrafficStatus.TrafficStatus[2*(lport-1)] = (uint8)((traffic_status & 0xFF00) >> 8); 
								TrapTrafficStatus.TrafficStatus[2*(lport-1)+1] = (uint8)(traffic_status & 0x00FF); 
								TrapFlag = 1;
#endif
							}
						}

						if(gPortConfigInfo[lport-1].RxThreshold < PERCENTAGE_100) {
							if(counter.RxGoodOctetsLo < gPortTrafficInfo[lport-1].PrevRxOctets) {
								gPortTrafficInfo[lport-1].Interval_RxOctets = (0xFFFFFFFF - gPortTrafficInfo[lport-1].PrevRxOctets + 1 + counter.RxGoodOctetsLo);
							} else {
								gPortTrafficInfo[lport-1].Interval_RxOctets = (counter.RxGoodOctetsLo - gPortTrafficInfo[lport-1].PrevRxOctets);
							}

							if(gPortTrafficInfo[lport-1].Interval_RxOctets >= gPortTrafficInfo[lport-1].Threshold_RxOctets) {
#if BOARD_GE204P0U				
								hport = hal_swif_lport_2_hport(lport);
								if((lport >= 25) && (lport <= 28)){
									u16PhyRegVal = 0x7C00;
									robo_write(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2); 	        
									robo_read(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2);
									/* Fiber select */
									if((u16PhyRegVal & 0x0040) && ((u16PhyRegVal & 0x0002) == 0x2)) {
										TrapFlag = 1;            /* traffick trap send */
									}else{
										TrapFlag = 0;
									}
								}else if(lport >= 29){
									u16PhyRegVal = 0x7C00;
									robo_write(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2); 	        
									robo_read(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2);
									/* Copper select */
									if((u16PhyRegVal & 0x0080) && ((u16PhyRegVal & 0x0002) == 0x0)) {
										TrapFlag = 1;			 /* traffick trap send */
									}else{
										TrapFlag = 0;
									}
								}else{
									TrapFlag = 1;                 /* lport 1-24 */
								}																
								traffic_status |= (0x4000 | (uint16)(gPortConfigInfo[lport-1].RxThreshold));
								TrapTrafficStatus.TrafficStatus[2*(lport-1)] = (uint8)((traffic_status & 0xFF00) >> 8); 
								TrapTrafficStatus.TrafficStatus[2*(lport-1)+1] = (uint8)(traffic_status & 0x00FF); 								
#else								
								traffic_status |= (0x4000 | (uint16)(gPortConfigInfo[lport-1].RxThreshold));
								TrapTrafficStatus.TrafficStatus[2*(lport-1)] = (uint8)((traffic_status & 0xFF00) >> 8); 
								TrapTrafficStatus.TrafficStatus[2*(lport-1)+1] = (uint8)(traffic_status & 0x00FF); 
								TrapFlag = 1;
#endif
							}
						}
						
						gPortTrafficInfo[lport-1].PrevTxOctets = counter.TxOctetsLo;
						gPortTrafficInfo[lport-1].PrevRxOctets = counter.RxGoodOctetsLo;
					}
				}
			}

			if(TrapFlag) {
				gTrapInfo.RequestID[TRAP_INDEX_TRAFFIC_OVER]++;
				hal_swif_trap_send(gTrapInfo.ServerMac, (uint8 *)&TrapTrafficStatus, sizeof(hal_trap_traffic_status), gTrapInfo.RequestID[TRAP_INDEX_TRAFFIC_OVER]);
			}
		}
	}
}

void hal_swif_traffic_entry(void)
{
	if(gTrapInfo.GateMask & TRAP_MASK_TRAFFIC_OVER) {
		if(xSemTraffic == NULL)
			vSemaphoreCreateBinary(xSemTraffic);

		xTaskCreate(hal_swif_traffic_task, 	"tTraffic",	configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 2, NULL);

	}
}
