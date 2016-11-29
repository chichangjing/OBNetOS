
/*******************************************************************
 * Filename     : hal_swif_port.c
 * Description  : Hardware Abstraction Layer for L2 Switch Port API
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *******************************************************************/
#include "mconfig.h"

/* Standard includes */
#include <stdio.h>

/* LwIP includes */
#include "lwip/netif.h"
#include "lwip/stats.h"

/* BSP includes */
#include "stm32f2xx.h"
#include "stm32f2x7_smi.h"
#include "soft_i2c.h"
#include "misc_drv.h"
#if ROBO_SWITCH
#include "robo_drv.h"
#elif MARVELL_SWITCH
#include "msApi.h"
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>
#endif

/* Configuration includes */
#include "conf_comm.h"
#include "conf_map.h"

#if MODULE_OBNMS
/* NMS includes */
#include "nms_comm.h"
#include "nms_if.h"
#endif

/* HAL for L2 includes */
#include "hal_swif_error.h"
#include "hal_swif_types.h"
#include "hal_swif_comm.h"
#include "hal_swif_port.h"
#include "hal_swif_message.h"

/* Other includes */
#include "cli_util.h"

#if BOARD_GV3S_HONUE_QM
#include "halsw_i2c.h"
#include "common.h"
#include "fpga_api.h"
#endif

#include "obring.h"

#if MARVELL_SWITCH
extern GT_QD_DEV *dev;
#endif
#include "os_mutex.h"


/**********************************************************************************************************************************

@@@
	The Device (GE-22103MA-213) front panel interface : (CPU port: hport2) 2光4电
     ----------------------------------------------------------------------------------------------------
    | Physical port:    |  hport10   |  hport9     |  hport8     |  hport7     | hport6     |  hport5    |       
    | Logic port:       |  lport1    |  lport2     |  lport3     |  lport4     | lport5     |  lport6    |	      
    | Panel interface:  |  1000M SFP |  1000M SFP  |  1000M RJ45 |  100M RJ45  | 100M RJ45  |  100M RJ45 |
     ----------------------------------------------------------------------------------------------------	 

	The Device (GE-22103MA-204) front panel interface : (CPU port: hport2) 2光4电
     ----------------------------------------------------------------------------------------------------
    | Physical port:    |  hport10   |  hport9     |  hport4     |  hport7     | hport6     |  hport5    |       
    | Logic port:       |  lport1    |  lport2     |  lport3     |  lport4     | lport5     |  lport6    |	      
    | Panel interface:  |  1000M SFP |  1000M SFP  |  100M RJ45  |  100M RJ45  | 100M RJ45  |  100M RJ45 |
     ----------------------------------------------------------------------------------------------------	     

	The Device (GE-22103MA-117) front panel interface : (CPU port: hport2) 1光8电
     ----------------------------------------------------------------------------------------------------
    | Physical port:    |                          |  hport4     |  hport3     | hport1     |  hport0    |       
    | Logic port:       |                          |  lport6     |  lport7     | lport8     |  lport9    |	      
    | Panel interface:  |                          |  100M RJ45  |  100M RJ45  | 100M RJ45  |  100M RJ45 |	
     ----------------------------------------------------------------------------------------------------
    | Physical port:    |  hport10   |             |  hport8     |  hport7     | hport6     |  hport5    |       
    | Logic port:       |  lport1    |             |  lport2     |  lport3     | lport4     |  lport5    |	      
    | Panel interface:  |  1000M SFP |             |  1000M RJ45 |  100M RJ45  | 100M RJ45  |  100M RJ45 |
     ----------------------------------------------------------------------------------------------------

	The Device (GE-22103MA-217) front panel interface : (CPU port: hport2) 2光8电
     ----------------------------------------------------------------------------------------------------
    | Physical port:    |                          |  hport4     |  hport3     | hport1     |  hport0    |       
    | Logic port:       |                          |  lport7     |  lport8     | lport9     |  lport10   |	      
    | Panel interface:  |                          |  100M RJ45  |  100M RJ45  | 100M RJ45  |  100M RJ45 |	
     ----------------------------------------------------------------------------------------------------
    | Physical port:    |  hport10   |  hport9     |  hport8     |  hport7     | hport6     |  hport5    |       
    | Logic port:       |  lport1    |  lport2     |  lport3     |  lport4     | lport5     |  lport6    |	      
    | Panel interface:  |  1000M SFP |  1000M SFP  |  100M RJ45  |  100M RJ45  | 100M RJ45  |  100M RJ45 |
     ----------------------------------------------------------------------------------------------------       
     
	The Device (GE-22103MA-110) front panel interface : (CPU port: hport2) 1光1电
     ----------------------------------------------------------------------------------------------------
    | Physical port:    |  hport10   |             |  hport8     |                                       |
    | Logic port:       |  lport1    |             |  lport2     |                                       |
    | Panel interface:  |  1000M SFP |             |  1000M RJ45 |                                       |
     ----------------------------------------------------------------------------------------------------
     
@@@
	The Device (GE-20023MA) front panel interface : (IMP port: hport8)
     -----------------------------------------------------------------------------------------------------
    | physical port:    |  hport0    |  hport1     |  hport2     |  hport3     | hport4     |             | 
    | Logic port:       |  lport1    |  lport2     |  lport3     |  lport4     | lport5     |             |	
    | panel interface:  |  100M SFP  |  100M SFP   |  100M RJ45  |  100M RJ45  | 100M RJ45  |  RS232/485  |
     -----------------------------------------------------------------------------------------------------


@@@
	The Device (GE-1040PU) front panel interface : (IMP port: hport24)
     --------------------------------------------------------------------------------------------------------------------
    | physical port:    |  hp00 hp02 hp04 hp06  |  hp08 hp10 hp12 hp14   |  hp16 hp18 hp20 hp22  |  hp26 hp26 hp28 hp28  | 
    |                   |  hp01 hp03 hp05 hp07  |  hp09 hp11 hp13 hp15   |  hp17 hp19 hp21 hp23  |  hp25 hp25 hp27 hp27  |
     --------------------------------------------------------------------------------------------------------------------
    | Logic port:       |  lp01 lp03 lp05 lp07  |  lp09 lp11 lp13 lp15   |  lp17 lp19 lp21 lp23  |  lp25 lp29 lp27 lp31  |	
    |                   |  lp02 lp04 lp06 lp08  |  lp10 lp12 lp14 lp16   |  lp18 lp20 lp22 lp24  |  lp26 lp30 lp28 lp32  |
     --------------------------------------------------------------------------------------------------------------------
    | panel interface:  |                               100M RJ45                                |  SFP  RJ45 SFP  RJ45  |
    |                   |                                                                        |  SFP  RJ45 SFP  RJ45  |
     --------------------------------------------------------------------------------------------------------------------

@@@
	The Device (GE-2C400U) front panel interface : (IMP port: hport24)
     --------------------------------------------------------------------------------------------------------------------
    | physical port:    |  hp03 hp02 hp01 hp00  |  hp07 hp06 hp05 hp04   |  hp11 hp10 hp09 hp08  |  hp12 hp13 hp14 hp15  | 
     --------------------------------------------------------------------------------------------------------------------
    | Logic port:       |  lp01 lp02 lp03 lp04  |  lp05 lp06 lp07 lp08   |  lp09 lp10 lp11 lp12  |  lp13 lp14 lp15 lp16  |	
     --------------------------------------------------------------------------------------------------------------------
    | panel interface:  |                               1000M SFP                                |  RJ45 RJ45 RJ45 RJ45  |
     --------------------------------------------------------------------------------------------------------------------
    
    
***********************************************************************************************************************************/


#if BOARD_GE22103MA
hal_port_map_t gHalPortMap[MAX_PORT_NUM] = {{1,10,S1000M_OPTICAL}, {2,9,S1000M_OPTICAL}, {3,8,S100M_CABLE}, {4,7,S100M_CABLE}, {5,6,S100M_CABLE}, {6,5,S100M_CABLE},
											{7,0,S100M_CABLE}, {8,1,S100M_CABLE}, {9,3,S100M_CABLE}, {10,4,S100M_CABLE}};
#elif BOARD_GE20023MA
hal_port_map_t gHalPortMap[MAX_PORT_NUM] = {{1,0,S100M_OPTICAL}, {2,1,S100M_OPTICAL}, {3,2,S100M_CABLE}, {4,3,S100M_CABLE}, {5,4,S100M_CABLE}};
#elif BOARD_GE11014MA
hal_port_map_t gHalPortMap[MAX_PORT_NUM] = {{1,0,S100M_OPTICAL}, {2,1,S100M_OPTICAL}, {3,2,S100M_CABLE}, {4,3,S100M_CABLE}, {5,4,S100M_CABLE}, {6,5,S100M_CABLE}};
#elif BOARD_GE1040PU
hal_port_map_t gHalPortMap[MAX_PORT_NUM] = {{1,0,S100M_CABLE}, {2,1,S100M_CABLE}, {3,2,S100M_CABLE}, {4,3,S100M_CABLE}, 
											{5,4,S100M_CABLE}, {6,5,S100M_CABLE}, {7,6,S100M_CABLE}, {8,7,S100M_CABLE}, 
											
											{9,8,S100M_CABLE}, {10,9,S100M_CABLE}, {11,10,S100M_CABLE}, {12,11,S100M_CABLE}, 
											{13,12,S100M_CABLE}, {14,13,S100M_CABLE}, {15,14,S100M_CABLE}, {16,15,S100M_CABLE}, 
											
											{17,16,S100M_CABLE}, {18,17,S100M_CABLE}, {19,18,S100M_CABLE}, {20,19,S100M_CABLE}, 
											{21,20,S100M_CABLE}, {22,21,S100M_CABLE}, {23,22,S100M_CABLE}, {24,23,S100M_CABLE}, 
											
											{25,26,S1000M_OPTICAL}, {26,25,S1000M_OPTICAL}, {27,28,S1000M_OPTICAL}, {28,27,S1000M_OPTICAL},
											{29,26,S1000M_CABLE}, {30,25,S1000M_CABLE}, {31,28,S1000M_CABLE}, {32,27,S1000M_CABLE}};


#elif BOARD_GE204P0U
hal_port_map_t gHalPortMap[MAX_PORT_NUM] = {{1,0,S100M_OPTICAL}, {2,1,S100M_OPTICAL}, {3,2,S100M_OPTICAL}, {4,3,S100M_OPTICAL}, 
											{5,4,S100M_OPTICAL}, {6,5,S100M_OPTICAL}, {7,6,S100M_OPTICAL}, {8,7,S100M_OPTICAL}, 
											
											{9,8,S100M_OPTICAL}, {10,9,S100M_OPTICAL}, {11,10,S100M_OPTICAL}, {12,11,S100M_OPTICAL}, 
											{13,12,S100M_OPTICAL}, {14,13,S100M_OPTICAL}, {15,14,S100M_OPTICAL}, {16,15,S100M_OPTICAL}, 
											
											{17,16,S100M_OPTICAL}, {18,17,S100M_OPTICAL}, {19,18,S100M_OPTICAL}, {20,19,S100M_OPTICAL}, 
											{21,20,S100M_OPTICAL}, {22,21,S100M_OPTICAL}, {23,22,S100M_OPTICAL}, {24,23,S100M_OPTICAL}, 
											
											{25,26,S1000M_OPTICAL}, {26,25,S1000M_OPTICAL}, {27,28,S1000M_OPTICAL}, {28,27,S1000M_OPTICAL},
											{29,26,S1000M_CABLE}, {30,25,S1000M_CABLE}, {31,28,S1000M_CABLE}, {32,27,S1000M_CABLE}};

#elif BOARD_GE2C400U
hal_port_map_t gHalPortMap[MAX_PORT_NUM] = {{1,3,S1000M_OPTICAL}, {2,2,S1000M_OPTICAL}, {3,1,S1000M_OPTICAL}, {4,0,S1000M_OPTICAL}, 
											{5,7,S1000M_OPTICAL}, {6,6,S1000M_OPTICAL}, {7,5,S1000M_OPTICAL}, {8,4,S1000M_OPTICAL}, 
											{9,11,S1000M_OPTICAL}, {10,10,S1000M_OPTICAL}, {11,9,S1000M_OPTICAL}, {12,8,S1000M_OPTICAL}, 
											{13,12,S1000M_CABLE}, {14,13,S1000M_CABLE}, {15,14,S1000M_CABLE}, {16,15,S1000M_CABLE}};
#elif BOARD_GV3S_HONUE_QM
hal_port_map_t gHalPortMap[MAX_PORT_NUM] = {{1,8,S1000M_CABLE}, {2,6,S100M_CABLE}, {3,5,S100M_CABLE}, {4,4,S100M_CABLE}, {5,0,S100M_CABLE}, 
                                            {6,1,S100M_CABLE},{7,2,S100M_CABLE}, {8,3,S100M_CABLE},{9,7,PORT_UNKOWN}, {10,10,S1000M_OPTICAL}};
#elif BOARD_GE11500MD
hal_port_map_t gHalPortMap[MAX_PORT_NUM] = {{1,5,S1000M_OPTICAL}, {2,4,S1000M_CABLE}, {3,3,S1000M_CABLE}, {4,2,S1000M_CABLE}, {5,1,S1000M_CABLE}, 
                                            {6,0,S1000M_CABLE}};
#elif BOARD_GE_EXT_22002EA
hal_port_map_t gHalPortMap[MAX_PORT_NUM] = {{1,8,S1000M_OPTICAL}, {2,10,S1000M_OPTICAL}, {3,7,S100M_CABLE}, {4,6,S100M_CABLE}};

#elif BOARD_GE220044MD
hal_port_map_t gHalPortMap[MAX_PORT_NUM] = {{1,10,S1000M_OPTICAL}, {2,8,S1000M_OPTICAL}, {3,5,S100M_CABLE}, {4,6,S100M_CABLE}, {5,0,S100M_CABLE}, {6,1,S100M_CABLE},
											{7,3,S100M_CABLE}, {8,4,S100M_CABLE}};

#endif

hal_port_config_info_t gPortConfigInfo[MAX_PORT_NUM];
hal_neighbor_record_t gNeighborInformation[MAX_PORT_NUM];
uint32 gTrafficEnableBitMap = 0;
HAL_BOOL gPortConfigInitialized = HAL_FALSE;
extern dev_base_info_t	DeviceBaseInfo;
OS_MUTEX_T cnt_clr_mutex;
uint32     clear_flag[MAX_PORT_NUM] = {0};
uint8	   disable_flag = 0;

/**************************************************************************
  * @brief  convert between logic port and hardware port
  * @param  lport
  * @retval hport
  *************************************************************************/
uint8 hal_swif_lport_2_hport(uint8 lport)
{
	int i;
	
	for(i=0; i<MAX_PORT_NUM; i++) {
		if(gHalPortMap[i].lport == lport) {
			break;
		}
	}
	if(i == MAX_PORT_NUM)
		return 0xFF;
	
	return gHalPortMap[i].hport;
}

#if (BOARD_GE204P0U || BOARD_GE1040PU)
/**************************************************************************
  * @brief  convert between hardware port and logic port GE204P0U 0nly
  * @param  hport
  * @retval lport
  *************************************************************************/
uint8 hal_swif_hport_2_lport(uint8 hport)
{
	int i;
    uint8   lport_plus_4;
	uint16	u16PhyRegVal;
		
	for(i=0; i<MAX_PORT_NUM; i++) {
		if(gHalPortMap[i].hport == hport) {
			break;
		}
	}

	if(i == MAX_PORT_NUM)
		return 0xFF;

	if(hport >= 25) {
		u16PhyRegVal = 0x7C00;
		robo_write(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2); 	
        
		robo_read(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2);

		/* Copper lport ,loprt + 4 */
		if((u16PhyRegVal & 0x0080) && ((u16PhyRegVal & 0x0002) == 0x0)) {
			lport_plus_4 = gHalPortMap[i].lport + 4;
		}

		/* Fiber lport ,do nothing */
		if((u16PhyRegVal & 0x0040) && ((u16PhyRegVal & 0x0002) == 0x2)) {
			lport_plus_4 = gHalPortMap[i].lport;
		}
	}
    else {
        lport_plus_4 = gHalPortMap[i].lport;
    }

	return lport_plus_4;
}
#else

/**************************************************************************
  * @brief  convert between hardware port and logic port
  * @param  hport
  * @retval lport
  *************************************************************************/
uint8 hal_swif_hport_2_lport(uint8 hport)
{
	int i;
		
	for(i=0; i<MAX_PORT_NUM; i++) {
		if(gHalPortMap[i].hport == hport) {
			break;
		}
	}
	if(i == MAX_PORT_NUM)
		return 0xFF;

	return gHalPortMap[i].lport;
}
#endif

/**************************************************************************
  * @brief  get logic port link state
  * @param  lport
  * @retval link_state
  *************************************************************************/
int hal_swif_port_get_link_state(uint8 lport, HAL_PORT_LINK_STATE *link_state)
{
#if SWITCH_CHIP_88E6095								/* SWITCH_CHIP_88E6095 */
	GT_STATUS status;
	GT_BOOL state;
	GT_U32 hport;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	if((status = gprtGetLinkState(dev,hport,&state)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}
	
	if(state == GT_TRUE)
		*link_state = LINK_UP;
	else
		*link_state = LINK_DOWN;

	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM53101							/* SWITCH_CHIP_BCM53101 */
	uint16	u16Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page160 */
	if(robo_read(0x01, 0x00, (u8 *)&u16Data, 2) != 0) 
		return HAL_SWIF_ERR_SPI_RW;
	*link_state = (HAL_PORT_LINK_STATE)((u16Data & (1 << hport)) >> hport);
	
	return HAL_SWIF_SUCCESS;
#elif SWITCH_CHIP_BCM53115							/* SWITCH_CHIP_BCM53115 */
	uint16	u16Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);//返回gHalPortMap[i].hport,返回值从5到0
	
	/* refer page192 */
	if(robo_read(0x01, 0x00, (u8 *)&u16Data, 2) != 0) 
		return HAL_SWIF_ERR_SPI_RW;
	*link_state = (HAL_PORT_LINK_STATE)((u16Data & (1 << hport)) >> hport);
	
	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53286							/* SWITCH_CHIP_BCM53286 */
	uint32	u32Data;
	uint16	u16PhyRegVal;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page287 */
	if(robo_read(0x02, 0x10, (u8 *)&u32Data, 4) != 0) 
		return HAL_SWIF_ERR_SPI_RW;

	*link_state = (HAL_PORT_LINK_STATE)((u32Data & (1 << hport)) >> hport);
	if(hport >= 25) {
		u16PhyRegVal = 0x7C00;
		if(robo_write(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2) != 0) 
			return HAL_SWIF_ERR_SPI_RW;	
		
		if(robo_read(0xD9+(hport-25), 0x38, (u8 *)&u16PhyRegVal, 2) != 0) 
			return HAL_SWIF_ERR_SPI_RW;

		if((lport >= 29) && (lport <=32)) {
			if((*link_state == LINK_UP) && (u16PhyRegVal & 0x0080) && ((u16PhyRegVal & 0x0002) == 0x0))
				*link_state = LINK_UP;
			else
				*link_state = LINK_DOWN;
		} else {
			if((*link_state == LINK_UP) && (u16PhyRegVal & 0x0040) && ((u16PhyRegVal & 0x0002) == 0x2))
				*link_state = LINK_UP;
			else
				*link_state = LINK_DOWN;
		}
	}
	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM5396							/* SWITCH_CHIP_BCM5396 */
	uint32	u32Data;
	uint16	u16PhyRegVal;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);

	if(robo_read(0x01, 0x00, (u8 *)&u32Data, 4) != 0) 
		return HAL_SWIF_ERR_SPI_RW;

	*link_state = (HAL_PORT_LINK_STATE)((u32Data & (1 << hport)) >> hport);

	return HAL_SWIF_SUCCESS;
#else
#error Get port link state unkonw board type
#endif

	return HAL_SWIF_FAILURE;
}

/**************************************************************************
  * @brief  set logic port stp state
  * @param  lport
  * @retval 
  *************************************************************************/
int hal_swif_port_set_stp_state(uint8 lport, HAL_PORT_STP_STATE stp_state)
{
#if SWITCH_CHIP_88E6095								/* SWITCH_CHIP_88E6095 */
	GT_STATUS status;
	GT_U32 hport;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	if(smi_setregfield(PHYADDR_PORT(hport), QD_REG_PORT_CONTROL,0,2,stp_state) != SMI_DRV_SUCCESS) {
		return HAL_SWIF_FAILURE;
	}

	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM53101							/* SWITCH_CHIP_BCM53101 */
	uint8	u8Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page137 */
	if(robo_read(0x00, hport, &u8Data, 1) != 0)
		return HAL_SWIF_ERR_SPI_RW;
	switch(stp_state) {
		case DISABLED:
		u8Data = 0x20 | (u8Data & 0x1F);
		break;

		case BLOCKING:
		u8Data = 0x40 | (u8Data & 0x1F);
		break;

		case LEARNING:
		u8Data = 0x80 | (u8Data & 0x1F);	
		break;

		case FORWARDING:
		u8Data = 0xA0 | (u8Data & 0x1F);	
		break;

		default:
		break;
	}
	if(robo_write(0x00, hport, &u8Data, 1) != 0)
		return HAL_SWIF_ERR_SPI_RW;

	return HAL_SWIF_SUCCESS;
    
#elif SWITCH_CHIP_BCM53115                          /* SWITCH_CHIP_BCM53115 */
	uint8	u8Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page137 */
	if(robo_read(0x00, hport, &u8Data, 1) != 0)
		return HAL_SWIF_ERR_SPI_RW;
	switch(stp_state) {
		case DISABLED:
		u8Data = 0x20 | (u8Data & 0x1F);
		break;

		case BLOCKING:
		u8Data = 0x40 | (u8Data & 0x1F);
		break;

		case LEARNING:
		u8Data = 0x80 | (u8Data & 0x1F);	
		break;

		case FORWARDING:
		u8Data = 0xA0 | (u8Data & 0x1F);	
		break;

		default:
		break;
	}
	if(robo_write(0x00, hport, &u8Data, 1) != 0)
		return HAL_SWIF_ERR_SPI_RW;

	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM53286							/* SWITCH_CHIP_BCM53286 */
	uint8	u8Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page282 */
	if(robo_read(0x01, 0x90+hport, &u8Data, 1) != 0)
		return HAL_SWIF_ERR_SPI_RW;
	u8Data = ((uint8)stp_state << 6) | (u8Data & 0x3F);

	/* If one of lport 25-28 stp_state set DISABLED,corresponding lport 29-32 do nothing */
	if((lport >= 25) && (lport <= 28) && (stp_state == DISABLED))
		disable_flag |= 1 << (lport-25);
	
	if((lport >= 29) && (disable_flag & (1<<(lport-29))))
		return HAL_SWIF_SUCCESS;
	
	if(robo_write(0x01, 0x90+hport, (u8 *)&u8Data, 1) != 0) 
		return HAL_SWIF_ERR_SPI_RW;
	
	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM5396							/* SWITCH_CHIP_BCM5396 */
	uint8	u8Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page93 */
	if(robo_read(0x00, hport, &u8Data, 1) != 0)
		return HAL_SWIF_ERR_SPI_RW;
	switch(stp_state) {
		case DISABLED:
		u8Data = 0x20 | (u8Data & 0x1F);
		break;

		case BLOCKING:
		u8Data = 0x40 | (u8Data & 0x1F);
		break;

		case LEARNING:
		u8Data = 0x80 | (u8Data & 0x1F);	
		break;

		case FORWARDING:
		u8Data = 0xA0 | (u8Data & 0x1F);	
		break;

		default:
		break;
	}

	if(robo_write(0x00, hport, &u8Data, 1) != 0)
		return HAL_SWIF_ERR_SPI_RW;

	return HAL_SWIF_SUCCESS;
#endif

	return HAL_SWIF_FAILURE;
}

/**************************************************************************
  * @brief  get logic port stp state
  * @param  lport
  * @retval stp_state
  *************************************************************************/
int hal_swif_port_get_stp_state(uint8 lport, HAL_PORT_STP_STATE *stp_state)
{
#if SWITCH_CHIP_88E6095								/* SWITCH_CHIP_88E6095 */
	GT_STATUS status;
	GT_U32 hport;
	GT_U16 data;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	if(smi_getregfield(PHYADDR_PORT(hport), SW_REG_PORT_CONTROL,0,2,&data) != SMI_DRV_SUCCESS) {
		return HAL_SWIF_FAILURE;
	}
	*stp_state = data & 0x3;

	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM53101							/* SWITCH_CHIP_BCM53101 */
	uint8	u8Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page137 */
	if(robo_read(0x00, hport, &u8Data, 1) != 0)
		return HAL_SWIF_ERR_SPI_RW;
	
	switch(u8Data & 0xE0) {
		case 0x20:
		*stp_state = DISABLED;
		break;

		case 0x40:
		*stp_state = BLOCKING;
		break;

		case 0x80:
		*stp_state = LEARNING;	
		break;

		case 0xA0:
		*stp_state = FORWARDING;	
		break;

		default:
		*stp_state = STP_UNKOWN;
		break;
	}

	return HAL_SWIF_SUCCESS;
#elif SWITCH_CHIP_BCM53115							/* SWITCH_CHIP_BCM53115 */
	uint8	u8Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page137 */
	if(robo_read(0x00, hport, &u8Data, 1) != 0)
		return HAL_SWIF_ERR_SPI_RW;
	
	switch(u8Data & 0xE0) {
		case 0x20:
		*stp_state = DISABLED;
		break;

		case 0x40:
		*stp_state = BLOCKING;
		break;

		case 0x80:
		*stp_state = LEARNING;	
		break;

		case 0xA0:
		*stp_state = FORWARDING;	
		break;

		default:
		*stp_state = STP_UNKOWN;
		break;
	}

	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM53286							/* SWITCH_CHIP_BCM53286 */
	uint8	u8Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page282 */
	if(robo_read(0x01, 0x90+hport, &u8Data, 1) != 0)
		return HAL_SWIF_ERR_SPI_RW;
	*stp_state = (HAL_PORT_STP_STATE)(u8Data >> 6);
	
	return HAL_SWIF_SUCCESS;	

#elif SWITCH_CHIP_BCM5396							/* SWITCH_CHIP_BCM5396 */
	uint8	u8Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page93 */
	if(robo_read(0x00, hport, &u8Data, 1) != 0)
		return HAL_SWIF_ERR_SPI_RW;

	switch(u8Data & 0xE0) {
		case 0x20:
		*stp_state = DISABLED;
		break;

		case 0x40:
		*stp_state = BLOCKING;
		break;

		case 0x80:
		*stp_state = LEARNING;	
		break;

		case 0xA0:
		*stp_state = FORWARDING;	
		break;

		default:
		*stp_state = STP_UNKOWN;
		break;
	}

	return HAL_SWIF_SUCCESS;
#endif

	return HAL_SWIF_FAILURE;
}

/**************************************************************************
  * @brief  get logic port duplex state
  * @param  lport
  * @retval stp_state
  *************************************************************************/
int hal_swif_port_get_duplex(uint8 lport, HAL_PORT_DUPLEX_STATE *duplex_state)
{
#if SWITCH_CHIP_88E6095								/* SWITCH_CHIP_88E6095 */
	GT_STATUS status;
	GT_BOOL	mode;
	GT_U32 hport;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	if((status = gprtGetDuplex(dev,hport,&mode)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	} 

	if(mode == GT_TRUE)
		*duplex_state = FULL_DUPLEX;
	else
		*duplex_state = HALF_DUPLEX;
	
	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM53101							/* SWITCH_CHIP_BCM53101 */
	uint16	u16Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page161 */
	if(robo_read(0x01, 0x08, (u8 *)&u16Data, 2) != 0)
		return HAL_SWIF_ERR_SPI_RW;
	*duplex_state = (HAL_PORT_DUPLEX_STATE)((u16Data & (1<<hport)) >> hport);

	return HAL_SWIF_SUCCESS;
#elif SWITCH_CHIP_BCM53115							/* SWITCH_CHIP_BCM53115 */
	uint16	u16Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page161 */
	if(robo_read(0x01, 0x08, (u8 *)&u16Data, 2) != 0)
		return HAL_SWIF_ERR_SPI_RW;
	*duplex_state = (HAL_PORT_DUPLEX_STATE)((u16Data & (1<<hport)) >> hport);

	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM53286							/* SWITCH_CHIP_BCM53286 */
	uint32	u32Data;
	uint8	hport,u8Data;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page290 */
	if(robo_read(0x02, 0x30, (u8 *)&u32Data, 4) != 0) 
		return HAL_SWIF_ERR_SPI_RW;
/*
	if(hport < 23){
		if(robo_read(0x01, 0x10+hport, (u8 *)&u8Data, 1) != 0)
			return HAL_SWIF_ERR_SPI_RW;
		*duplex_state = (HAL_PORT_DUPLEX_STATE)((u8Data & 0x02) >> 1);
		}
	else*/
		*duplex_state = (HAL_PORT_DUPLEX_STATE)((u32Data & (1 << hport)) >> hport);
	
	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM5396							/* SWITCH_CHIP_BCM5396 */
	uint32	u32Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page104 */
	if(robo_read(0x01, 0x10, (u8 *)&u32Data, 4) != 0) 
		return HAL_SWIF_ERR_SPI_RW;

	*duplex_state = (HAL_PORT_DUPLEX_STATE)((u32Data & (1 << hport)) >> hport);	

	return HAL_SWIF_SUCCESS;
#endif

	return HAL_SWIF_FAILURE;
}

/**************************************************************************
  * @brief  get logic port speed state
  * @param  lport
  * @retval speed_state
  *************************************************************************/
int hal_swif_port_get_speed(uint8 lport, HAL_PORT_SPEED_STATE *speed_state)
{
#if SWITCH_CHIP_88E6095								/* SWITCH_CHIP_88E6095 */
	GT_STATUS status;
	GT_PORT_SPEED_MODE	mode;
	GT_U32 hport;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	if((status = gprtGetSpeedMode(dev,hport,&mode)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	} 

	if(mode == PORT_SPEED_10_MBPS)
		*speed_state = SPEED_10M;
	else if(mode == PORT_SPEED_100_MBPS)
		*speed_state = SPEED_100M;
	else if(mode == PORT_SPEED_1000_MBPS)
		*speed_state = SPEED_1000M;
	else
		*speed_state = SPEED_UNKOWN;
	
	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53101							/* SWITCH_CHIP_BCM53101 */
	uint32	u32Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page161 */
	if(robo_read(0x01, 0x04, (uint8 *)&u32Data, 4) != 0)
		return HAL_SWIF_ERR_SPI_RW;
	*speed_state = (HAL_PORT_SPEED_STATE)((u32Data & (0x3<<hport*2)) >> hport*2);

	return HAL_SWIF_SUCCESS;
    
#elif SWITCH_CHIP_BCM53115							/* SWITCH_CHIP_BCM53115 */
	uint32	u32Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page193 */
	if(robo_read(0x01, 0x04, (uint8 *)&u32Data, 4) != 0)
		return HAL_SWIF_ERR_SPI_RW;
	*speed_state = (HAL_PORT_SPEED_STATE)((u32Data & (0x3<<hport*2)) >> hport*2);

	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM53286							/* SWITCH_CHIP_BCM53286 */
	uint64	u64Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page289 */
	if(robo_read(0x02, 0x20, (uint8 *)&u64Data, 8) != 0) 
		return HAL_SWIF_ERR_SPI_RW;

	if(hport > 15)
		*speed_state = (HAL_PORT_SPEED_STATE)((u64_H(u64Data) & (3 << (hport-16)*2)) >> (hport-16)*2);
	else
		*speed_state = (HAL_PORT_SPEED_STATE)((u64_L(u64Data) & (3 << hport*2)) >> hport*2);
	
	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM5396							/* SWITCH_CHIP_BCM5396 */
	uint64	u64Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	/* refer page103 */
	if(robo_read(0x01, 0x08, (uint8 *)&u64Data, 8) != 0) 
		return HAL_SWIF_ERR_SPI_RW;

	if(hport > 15)
		*speed_state = (HAL_PORT_SPEED_STATE)((u64_H(u64Data) & (3 << (hport-16)*2)) >> (hport-16)*2);
	else
		*speed_state = (HAL_PORT_SPEED_STATE)((u64_L(u64Data) & (3 << hport*2)) >> hport*2);
	
	return HAL_SWIF_SUCCESS;
#endif

	return HAL_SWIF_FAILURE;
}

/**************************************************************************
  * @brief  set logic port speed and duplex
  * @param  lport
  * @retval 
  *************************************************************************/
int hal_swif_port_set_speed_duplex(uint8 lport, HAL_PORT_SPEED_DUPLEX speed_duplex)
{
#if SWITCH_CHIP_88E6095								/* SWITCH_CHIP_88E6095 */
	GT_STATUS ret;
	GT_U32 hport;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
#if (BOARD_GE22103MA) || (BOARD_GV3S_HONUE_QM) || (BOARD_GE_EXT_22002EA) || (BOARD_GE220044MD)
    
	switch(speed_duplex) {
		case S10M_HALF:
		if(hport == 8) {
			/* Disable auto-negotiation, 10M half-duplex */
			if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0x8000)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }						
		} else if ((hport != 10) && (hport != 9)) {
			/* Disable auto-negotiation */
			if((ret = gprtPortAutoNegEnable(dev, hport, GT_FALSE)) != GT_OK) {
	            printf("Error: gprtPortAutoNegEnable, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* LED0 OFF */
			if((ret=hwSetPhyRegField(dev,hport, 0x19, 0, 2, 2)) != GT_OK) 	{
	            printf("Error: hwSetPhyRegField, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set speed to 10M bps */
			if((ret = gprtSetPortSpeed(dev, hport, PHY_SPEED_10_MBPS)) != GT_OK) {
				printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }		
			/* Set duplex to half-duplex */
			if((ret = gprtSetPortDuplexMode(dev, hport, GT_FALSE)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }						
		}
		break;

		case S10M_FULL:
		if(hport == 8) {
			/* Disable auto-negotiation, 10M full-duplex */
			if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0x8100)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }	
		} else if ((hport != 10) && (hport != 9)) {
			/* Disable auto-negotiation */
			if((ret = gprtPortAutoNegEnable(dev, hport, GT_FALSE)) != GT_OK) {
	            printf("Error: gprtPortAutoNegEnable, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }					
			/* LED0 OFF */
			if((ret=hwSetPhyRegField(dev,hport, 0x19, 0, 2, 2)) != GT_OK) 	{
	            printf("Error: hwSetPhyRegField, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set speed to 10M bps */
			if((ret = gprtSetPortSpeed(dev, hport, PHY_SPEED_10_MBPS)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set duplex to full-duplex */
			if((ret = gprtSetPortDuplexMode(dev, hport, GT_TRUE)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
		}						
		break;

		case S100M_HALF:
		if(hport == 8) {
			/* Disable auto-negotiation, 100M half-duplex */
			if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0xa000)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
		} else if ((hport != 10) && (hport != 9)) {
			/* Disable auto-negotiation */
			if((ret = gprtPortAutoNegEnable(dev, hport, GT_FALSE)) != GT_OK) {
	            printf("Error: gprtPortAutoNegEnable, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set speed to 100M bps */
			if((ret = gprtSetPortSpeed(dev, hport, PHY_SPEED_100_MBPS)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set duplex to half-duplex */
			if((ret = gprtSetPortDuplexMode(dev, hport, GT_FALSE)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }					
		}
		break;

		case S100M_FULL:
		if(hport == 8) {
			/* Disable auto-negotiation, 100M full-duplex */
			if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0xa100)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }		
		} else if ((hport != 10) && (hport != 9)) {
			/* Disable auto-negotiation */
			if((ret = gprtPortAutoNegEnable(dev, hport, GT_FALSE)) != GT_OK) {
	            printf("Error: gprtPortAutoNegEnable, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set speed to 100M bps */
			if((ret = gprtSetPortSpeed(dev, hport, PHY_SPEED_100_MBPS)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set duplex to full-duplex */
			if((ret = gprtSetPortDuplexMode(dev, hport, GT_TRUE)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }					
		}
		break;
		
		case S1000M_HALF:
		if(hport == 8) {
			/* Disable auto-negotiation, 1000M half-duplex */
			if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0x8140)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set speed to 1000M bps */
			if((ret = gpcsSetForceSpeed(dev, hport, PHY_SPEED_1000_MBPS)) != GT_OK) {
	            printf("Error: gpcsSetForceSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set duplex to half-duplex */
			if((ret = gpcsSetDpxValue(dev, hport, GT_TRUE)) != GT_OK) {
	            printf("Error: gpcsSetDpxValue, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }						
			if((ret = gpcsSetForcedDpx(dev, hport, GT_TRUE)) != GT_OK) {
	            printf("Error: gpcsSetForcedDpx, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }	
		} else {
            /* Do nothing */
		}

		break;	
		
		case S1000M_FULL:
		if(hport == 8) {
			/* Disable auto-negotiation, 1000M full-duplex */
			if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0x8140)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set speed to 1000M bps */
			if((ret = gpcsSetForceSpeed(dev, hport, PHY_SPEED_1000_MBPS)) != GT_OK) {
	            printf("Error: gpcsSetForceSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set duplex to full-duplex */
			if((ret = gpcsSetDpxValue(dev, hport, GT_TRUE)) != GT_OK) {
	            printf("Error: gpcsSetDpxValue, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }						
			if((ret = gpcsSetForcedDpx(dev, hport, GT_TRUE)) != GT_OK) {
	            printf("Error: gpcsSetForcedDpx, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
		} else if ((hport == 9) || (hport == 10)) {
			/* Disable auto-negotiation */
			if((ret = gpcsSetPCSAnEn(dev, hport, GT_FALSE)) != GT_OK) {
	            printf("Error: gpcsSetPCSAnEn, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set speed to 1000M bps */
			if((ret = gpcsSetForceSpeed(dev, hport, PHY_SPEED_1000_MBPS)) != GT_OK) {
	            printf("Error: gpcsSetForceSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Set duplex to full-duplex */
			if((ret = gpcsSetDpxValue(dev, hport, GT_TRUE)) != GT_OK) {
	            printf("Error: gpcsSetDpxValue, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }						
			if((ret = gpcsSetForcedDpx(dev, hport, GT_TRUE)) != GT_OK) {
	            printf("Error: gpcsSetForcedDpx, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }

			#if 0
			/* Auto-Nego Bypass */
			if((ret = gpcsSetAnBypassMode(dev, hport, GT_TRUE)) != GT_OK) {
	            printf("Error: gpcsSetAnBypassMode, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }		
			#endif			
		} else {
		    /* Do nothing */
		}
		break;
	
		default:
		break;
	}		

	return HAL_SWIF_SUCCESS;
	
#endif	/* BOARD_GE22103MA */

#elif SWITCH_CHIP_BCM53101							/* SWITCH_CHIP_BCM53101 */
	int ret;
	u8	u8Data;
	u16 u16Data;
	uint8 hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
    
    /* Do nothing for optical port */
    if(hport == 0 || hport == 1)
        return HAL_SWIF_SUCCESS;
    
    switch(speed_duplex){
        case S10M_HALF:
            /* Setup MII control register, duplex half, and speed 10M */
            u16Data = 0x0000;
            if(robo_write(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            break;
        case S10M_FULL:
            /* Setup MII control register, duplex full, and speed 10M */
            u16Data = 0x0100;
            if(robo_write(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            break;
        case S100M_HALF:
            /* MII control register */
	        if(robo_read(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* Setup MII control register, duplex half, and speed 100M */
            /* duplex half */
            u16Data &= ~(1<<8);
            /* speed 100M */
            u16Data &= ~( (1<<13) | (1<<6) );
            u16Data |= (1<<13);
            /* disable loopback */
            u16Data &= ~(1<<14);
            /* disable auto-negotiation */
            u16Data &= ~(1<<12);
            /* Write MII control register */
            if(robo_write(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            break;
        case S100M_FULL:
            /* Read MII control register */
	        if(robo_read(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* Setup MII control register, duplex full, and speed 100M */
            /* duplex full */
            u16Data |= (1<<8);
            /* speed 100M */
            u16Data &= ~( (1<<13) | (1<<6) );
            u16Data |= (1<<13);
            /* disable loopback */
            u16Data &= ~(1<<14);
            /* disable auto-negotiation */
            u16Data &= ~(1<<12);
            /* Write MII control register */
            if(robo_write(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            break;
        case S1000M_HALF:
            break;
        case S1000M_FULL:
            break;
        default:
            break;
    }
    
    return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53115						/* SWITCH_CHIP_BCM53115 */
	int ret;
	u8	u8Data;
	u16 u16Data;
	uint8 hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
    
    /* Do nothing for optical port */
    if(hport == 5)
        return HAL_SWIF_SUCCESS;
    
    switch(speed_duplex){
        case S10M_HALF:
            /* Setup MII control register, duplex half, and speed 10M */
            u16Data = 0x0000;
            if(robo_write(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            break;
        case S10M_FULL:
            /* Setup MII control register, duplex full, and speed 10M */
            u16Data = 0x0100;
            if(robo_write(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            break;
        case S100M_HALF:
            /* MII control register */
	        if(robo_read(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* Setup MII control register, duplex half, and speed 100M */
            /* duplex half */
            u16Data &= ~(1<<8);
            /* speed 100M */
            u16Data &= ~( (1<<13) | (1<<6) );
            u16Data |= (1<<13);
            /* disable loopback */
            u16Data &= ~(1<<14);
            /* disable auto-negotiation */
            u16Data &= ~(1<<12);
            /* Write MII control register */
            if(robo_write(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            /* Read Miscelaneous Control Register */
            /* Shadow register slector 111 Miscelaneous Control Register */
            u16Data = 0;
            u16Data |= (1<<12)|(1<<13)|(1<<14);
            if(robo_write(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
	        if(robo_read(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* enable Auto-MDIX */
            u16Data |= (1<<9);
            /* Write enable */
            u16Data |= (1<<15);
            /* Write shadow register slector 111 Miscelaneous Control Register */ 
            u16Data |= (1<<0)|(1<<1)|(1<<2);
            /* Writer Miscelaneous Control Register */
            if(robo_write(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            break;
        case S100M_FULL:
            /* Read MII control register */
	        if(robo_read(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* Setup MII control register, duplex full, and speed 100M */
            /* duplex full */
            u16Data |= (1<<8);
            /* speed 100M */
            u16Data &= ~( (1<<13) | (1<<6) );
            u16Data |= (1<<13);
            /* disable loopback */
            u16Data &= ~(1<<14);
            /* disable auto-negotiation */
            u16Data &= ~(1<<12);
            /* Write MII control register */
            if(robo_write(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            /* Read Miscelaneous Control Register */
            /* Read shadow register slector 111 Miscelaneous Control Register */
            u16Data = 0;
            u16Data |= (1<<12)|(1<<13)|(1<<14)|(1<<0)|(1<<1)|(1<<2);
            if(robo_write(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
	        if(robo_read(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* disable Auto-MDIX */
            u16Data |= (1<<9);
            /* Write enable */
            u16Data |= (1<<15);
            /* Write shadow register slector 111 Miscelaneous Control Register */ 
            u16Data |= (1<<0)|(1<<1)|(1<<2);
            /* Writer Miscelaneous Control Register */            
            if(robo_write(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            /* Read shadow register slector 111 Miscelaneous Control Register */
            u16Data = 0;
            u16Data |= (1<<12)|(1<<13)|(1<<14);
            if(robo_write(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
	        if(robo_read(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            break;
        case S1000M_HALF:
            /* MII control register */
	        if(robo_read(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* Setup MII control register, duplex half, and speed 1000M */
            /* duplex half */
            u16Data &= ~(1<<8);
            /* speed 1000M */
            u16Data &= ~( (1<<13) | (1<<6) );
            u16Data |= (1<<6);
            /* disable loopback */
            u16Data &= ~(1<<14);
            /* disable auto-negotiation */
            u16Data &= ~(1<<12);
            /* Write MII control register */
            if(robo_write(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            /* Read Miscelaneous Control Register */
            /* Shadow register slector 111 Miscelaneous Control Register */
            u16Data = 0;
            u16Data |= (1<<12)|(1<<13)|(1<<14);
            if(robo_write(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
	        if(robo_read(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* enable Auto-MDIX */
            u16Data |= (1<<9);
            /* Write enable */
            u16Data |= (1<<15);
            /* Write shadow register slector 111 Miscelaneous Control Register */ 
            u16Data |= (1<<0)|(1<<1)|(1<<2);
            /* Writer Miscelaneous Control Register */            
            if(robo_write(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
			break;
        case S1000M_FULL:
            /* MII control register */
	        if(robo_read(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* Setup MII control register, duplex full, and speed 1000M */
            /* duplex full */
            u16Data |= (1<<8);
            /* speed 1000M */
            u16Data &= ~( (1<<13) | (1<<6) );
            u16Data |= (1<<6);
            /* disable loopback */
            u16Data &= ~(1<<14);
            /* disable auto-negotiation */
            u16Data &= ~(1<<12);
            /* Write MII control register */            
            if(robo_write(0x10+hport, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            /* Read Miscelaneous Control Register */
            /* Shadow register slector 111 Miscelaneous Control Register */
            u16Data = 0;
            u16Data |= (1<<12)|(1<<13)|(1<<14);
            if(robo_write(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
	        if(robo_read(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* enable Auto-MDIX */
            u16Data |= (1<<9);
            /* Write enable */
            u16Data |= (1<<15);
            /* Write shadow register slector 111 Miscelaneous Control Register */ 
            u16Data |= (1<<0)|(1<<1)|(1<<2);
            /* Writer Miscelaneous Control Register */            
            if(robo_write(0x10+hport, 0x30, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
			break;
        default:
            break;
    }
    
    return HAL_SWIF_SUCCESS;	
	
#elif SWITCH_CHIP_BCM53286							/* SWITCH_CHIP_BCM53286 */
	int ret;
	u8	u8Data;
	u16 u16Data;
	uint8 hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
    
    /* Do nothing for optical port 0-23 */
    if(hport <= 23)
        return HAL_SWIF_SUCCESS;
    
    switch(speed_duplex){
        case S10M_HALF:
            /* Setup MII control register, duplex half, and speed 10M ,on Page 387*/
            u16Data = 0x0000;
            if(robo_write(0xD9+hport-25, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            break;
        case S10M_FULL:
            /* Setup MII control register, duplex full, and speed 10M ,on Page 387*/
            u16Data = 0x0100;
            if(robo_write(0xD9+hport-25, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            break;
        case S100M_HALF:
            /* MII control register */
	        if(robo_read(0xD9+hport-25, 0x00, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* Setup MII control register, duplex half, and speed 100M ,on Page 387 */
            /* duplex half */
            u16Data &= ~(1<<8);
            /* speed 100M */
            u16Data &= ~( (1<<13) | (1<<6) );
            u16Data |= (1<<13);
            /* disable loopback */
            u16Data &= ~(1<<14);
            /* disable auto-negotiation */
            u16Data &= ~(1<<12);
            /* Write MII control register */
            if(robo_write(0xD9+hport-25, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            break;
        case S100M_FULL:
            /* Read MII control register */
	        if(robo_read(0xD9+hport-25, 0x00, (u8 *)&u16Data, 2) != 0)
		        return HAL_SWIF_ERR_MSAPI;
            /* Setup MII control register, duplex full, and speed 100M */
            /* duplex full */
            u16Data |= (1<<8);
            /* speed 100M */
            u16Data &= ~( (1<<13) | (1<<6) );
            u16Data |= (1<<13);
            /* disable loopback */
            u16Data &= ~(1<<14);
            /* disable auto-negotiation */
            u16Data &= ~(1<<12);
            /* Write MII control register */
            if(robo_write(0xD9+hport-25, 0x00, (u8 *)&u16Data, 2) != 0)
                return HAL_SWIF_ERR_MSAPI;
            break;
		/*force to 1000M can lead to link-configuration mismatches an no-link situations */
        case S1000M_HALF:
			break;
        case S1000M_FULL:
 			break;
        default:
            break;
    }
    
    return HAL_SWIF_SUCCESS;	

#elif SWITCH_CHIP_BCM5396							/* SWITCH_CHIP_BCM5396 */

	return HAL_SWIF_SUCCESS;

#endif

	return HAL_SWIF_FAILURE;
}

/**************************************************************************
  * @brief  set flow control
  * @param  lport, flow_ctrl_en
  * @retval 
  *************************************************************************/
int hal_swif_port_set_flow_control(uint8 lport, HAL_BOOL flow_ctrl_en)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS ret;
	GT_U32 hport;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);

	#if 0
	if((hport != 9) && (hport != 10)) {	
		/* Program Phy's Pause bit in AutoNegotiation Advertisement Register. */
		if((ret = gprtSetPause(dev,hport,GT_PHY_PAUSE)) != GT_OK) {
            printf("Error: gprtSetPause, hwport=%d, ret=%d\r\n", hport,ret);
			return CONF_ERR_MSAPI;
        }
		if((single_port_cfg & PC_MASK_AUTO_NEG) != 0) {
			/* Restart AutoNegotiation of the given Port's phy */
			if((ret = gprtPortRestartAutoNeg(dev,hport)) != GT_OK) {
	            printf("Error: gprtPortRestartAutoNeg, hwport=%d, ret=%d\r\n", hport,ret);
				return CONF_ERR_MSAPI;
	        }
		}
	}
	#endif

	if(flow_ctrl_en) {
		/* Program Phy's Pause bit in AutoNegotiation Advertisement Register. */
		if((ret = gprtSetPause(dev,hport,GT_PHY_PAUSE)) != GT_OK) {
	        printf("Error: gprtPortRestartAutoNeg, hwport=%d, ret=%d\r\n", hport,ret);
			return CONF_ERR_MSAPI;
	    }
		#if 0
		/* Restart AutoNegotiation of the given Port's phy. */
		if((ret = gprtPortRestartAutoNeg(dev,hport)) != GT_OK) {
	        printf("Error: gprtPortRestartAutoNeg, hwport=%d, ret=%d\r\n", hport,ret);
			return CONF_ERR_MSAPI;
	    }
		#endif
		/* Program Port's Flow Control. */
		if((ret = gprtSetForceFc(dev,hport,GT_TRUE)) != GT_OK) {
	        printf("Error: gprtSetForceFc, hwport=%d, ret=%d\r\n", hport,ret);
			return CONF_ERR_MSAPI;
	    }
	} else {
		/* Program Phy's Pause bit in AutoNegotiation Advertisement Register. */
		if((ret = gprtSetPause(dev,hport,GT_PHY_NO_PAUSE)) != GT_OK) {
	        printf("Error: gprtPortRestartAutoNeg, hwport=%d, ret=%d\r\n", hport,ret);
			return CONF_ERR_MSAPI;
	    }
		#if 0
		/* Restart AutoNegotiation of the given Port's phy. */
		if((ret = gprtPortRestartAutoNeg(dev,hport)) != GT_OK) {
	        printf("Error: gprtPortRestartAutoNeg, hwport=%d, ret=%d\r\n", hport,ret);
			return CONF_ERR_MSAPI;
	    }
		#endif
		/* Program Port's Flow Control. */
		if((ret = gprtSetForceFc(dev,hport,GT_FALSE)) != GT_OK) {
	        printf("Error: gprtSetForceFc, hwport=%d, ret=%d\r\n", hport,ret);
			return CONF_ERR_MSAPI;
	    }
	}
	
	return HAL_SWIF_SUCCESS;
    
#elif SWITCH_CHIP_BCM53115									/* SWITCH_CHIP_BCM53115 */
	
	return HAL_SWIF_SUCCESS;
		
#elif SWITCH_CHIP_BCM53101
    return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53286							/* SWITCH_CHIP_BCM53286 */

	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM5396							/* SWITCH_CHIP_BCM5396 */

	return HAL_SWIF_SUCCESS;    

#endif
	return HAL_SWIF_FAILURE;
}

/**************************************************************************
  * @brief  set logic port mdi/mdix
  * @param  lport
  * @retval 
  *************************************************************************/
int hal_swif_port_set_mdi_mdix(uint8 lport, HAL_MDI_MDIX_STATE mdi_mdix)
{
#if SWITCH_CHIP_88E6095								/* SWITCH_CHIP_88E6095 */
	GT_STATUS ret;
	GT_U8	hport;
	GT_U16	u16Data;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);

#if (BOARD_GE22103MA) || (BOARD_GV3S_HONUE_QM) || (BOARD_GE_EXT_22002EA) || (BOARD_GE220044MD)

	switch(mdi_mdix) {
		case MODE_MDIX:	/* MDIX */
		if(hport <= 7) {
			if((ret = hwSetPhyRegField(dev, hport, QD_PHY_SPEC_CONTROL_REG,4,2, 0x0)) != GT_OK) {
	            printf("Error: hwSetPhyRegField, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
			}
			if((ret = hwPhyReset(dev,hport,0xFF)) != GT_OK) {
	            printf("Error: hwPhyReset, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
			}
		}

		if(hport == 8) {
			if((ret = gprtGetPagedPhyReg(dev, hport, 16, 0, &u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			u16Data = (u16Data & 0xFF9F) | 0x0020;
			if((ret = gprtSetPagedPhyReg(dev, hport, 16, 0, u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Phy soft reset */
			if((ret = gprtGetPagedPhyReg(dev, hport, 0, 0, &u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			u16Data = u16Data | 0x8000;
			if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
		}			
		break;

		case MODE_MDI:	/* MDI */
		if(hport <= 7) {
			if((ret = hwSetPhyRegField(dev, hport, QD_PHY_SPEC_CONTROL_REG,4,2, 0x1)) != GT_OK) {
	            printf("Error: hwSetPhyRegField, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
			}
			if((ret = hwPhyReset(dev,hport,0xFF)) != GT_OK) {
	            printf("Error: hwPhyReset, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
			}
		}
		
		if(hport == 8) {
			if((ret = gprtGetPagedPhyReg(dev, hport, 16, 0, &u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			u16Data = (u16Data & 0xFF9F) | 0x0000;
			if((ret = gprtSetPagedPhyReg(dev, hport, 16, 0, u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Phy soft reset */
			if((ret = gprtGetPagedPhyReg(dev, hport, 0, 0, &u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			u16Data = u16Data | 0x8000;
			if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }				
		}							
		break;

		case MODE_AutoMDIX:	/* AutoMDIX */
		if(hport <= 7) {
			if((ret = hwSetPhyRegField(dev, hport, QD_PHY_SPEC_CONTROL_REG,4,2, 0x3)) != GT_OK) {
	            printf("Error: hwSetPhyRegField, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
			}
			if((ret = hwPhyReset(dev,hport,0xFF)) != GT_OK) {
	            printf("Error: hwPhyReset, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
			}
		}
		if(hport == 8) {
			if((ret = gprtGetPagedPhyReg(dev, hport, 16, 0, &u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			u16Data = (u16Data & 0xFF9F) | 0x0060;
			if((ret = gprtSetPagedPhyReg(dev, hport, 16, 0, u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			/* Phy soft reset */
			if((ret = gprtGetPagedPhyReg(dev, hport, 0, 0, &u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }
			u16Data = u16Data | 0x8000;
			if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, u16Data)) != GT_OK) {
	            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
				return HAL_SWIF_ERR_MSAPI;
	        }				
		}
		break;

		default:
		break;
	}	
	
#endif /* BOARD_GE22103MA */
	
	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53101							/* SWITCH_CHIP_BCM53101 */
	int ret;
	u8	u8Data;
	u16 u16Data;
	u32 u32Data;
	uint8 hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
#elif SWITCH_CHIP_BCM53286							/* SWITCH_CHIP_BCM53286 */

	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM5396							/* SWITCH_CHIP_BCM5396 */

	return HAL_SWIF_SUCCESS;
#endif

	return HAL_SWIF_FAILURE;
}


/**************************************************************************
  * @brief  get logic port mdi/mdix state
  * @param  lport
  * @retval mdi_mdix
  *************************************************************************/
int hal_swif_port_get_mdi_mdix(uint8 lport, HAL_MDI_MDIX_STATE *mdi_mdix)
{
#if SWITCH_CHIP_88E6095								/* SWITCH_CHIP_88E6095 */
	GT_STATUS status;
	GT_U32 hport;
	GT_U16	u16Data;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);

#if (BOARD_GE22103MA) || (BOARD_GV3S_HONUE_QM) || (BOARD_GE_EXT_22002EA) ||(BOARD_GE220044MD)
	if(hport == 8) {
		gprtGetPagedPhyReg(dev, hport, 17, 0, &u16Data);
		if(u16Data & 0x0040)
			*mdi_mdix = MODE_MDIX;
		else
			*mdi_mdix = MODE_MDI;
	} else {
		gprtGetPhyReg(dev, hport, SW_PHY_SPEC_STATUS_REG, &u16Data);
		if(u16Data & 0x0040)
			*mdi_mdix = MODE_MDI;
		else
			*mdi_mdix = MODE_MDIX;
	}
#endif	/* BOARD_GE22103MA */

	return HAL_SWIF_SUCCESS;


#elif SWITCH_CHIP_BCM53101							/* SWITCH_CHIP_BCM53101 */
	int ret;
	u8	u8Data;
	u16 u16Data;
	u32 u32Data;
	uint8 hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
#elif SWITCH_CHIP_BCM53286							/* SWITCH_CHIP_BCM53286 */

	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM5396							/* SWITCH_CHIP_BCM5396 */

	return HAL_SWIF_SUCCESS;

#endif

	return HAL_SWIF_FAILURE;

}

/*
 *  Description : This routine initializes mutex. 
 *  Parameters  : m - mutex pointer 
 *  Return value:
 *    OS_MUTEX_SUCCESS
 *	  OS_MUTEX_ERROR
 */
int cnt_clr_mutex_init(void)
{
    
    if(os_mutex_init(&cnt_clr_mutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return OS_MUTEX_ERROR;
	}
    return OS_MUTEX_SUCCESS;
}

/**************************************************************************
  * @brief  check port counters clear flage
  * @param  lport
  * @retval 1 if clear ,0 if none clear
  *************************************************************************/
int hal_swif_port_get_clear_counters_flag(uint8 lport)
{
    os_mutex_lock(&cnt_clr_mutex, OS_MUTEX_WAIT_FOREVER);
    if(clear_flag[lport-1] == 1){
        clear_flag[lport-1] = 0;
        os_mutex_unlock(&cnt_clr_mutex);
        return 1;
    }
    os_mutex_unlock(&cnt_clr_mutex);
	return 0;
}

/**************************************************************************
  * @brief  check port counters clear flage
  * @param  lport
  * @retval 1 if clear ,0 if none clear
  *************************************************************************/
int hal_swif_port_clear_counters_flag(uint8 lport)
{
    int status;
    
    os_mutex_lock(&cnt_clr_mutex, OS_MUTEX_WAIT_FOREVER);
    clear_flag[lport-1] = 1;
    status = hal_swif_port_clear_counters(lport);
    os_mutex_unlock(&cnt_clr_mutex);
    
    return status;
}
/**************************************************************************
  * @brief  get logic port mib counters
  * @param  lport
  * @retval port_counters
  *************************************************************************/
//int hal_swif_port_get_counters(uint8 lport, hal_port_counters_t *port_counters, HAL_HISTOGRAM_MODE HistogramMode)
int hal_swif_port_get_counters(uint8 lport, hal_port_counters_t *port_counters, uint16 *valid_bit_mask)
{
#if SWITCH_CHIP_88E6095								/* SWITCH_CHIP_88E6095 */
	GT_STATUS status;
	GT_U32 hport;
	GT_STATS_COUNTER_SET3 CounterStats;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	if(lport == 0)
		hport = CONFIG_SWITCH_CPU_PORT;	/* CPU port */
	else
		hport = hal_swif_lport_2_hport(lport);
	
	if((status = gstatsGetPortAllCounters3(dev, hport, &CounterStats)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	port_counters->RxGoodOctetsLo	= CounterStats.InGoodOctetsLo;
	port_counters->RxGoodOctetsHi	= CounterStats.InGoodOctetsHi;
	port_counters->RxUnicastPkts	= CounterStats.InUnicasts;
	port_counters->RxBroadcastPkts	= CounterStats.InBroadcasts;
	port_counters->RxMulticastPkts	= CounterStats.InMulticasts;
	port_counters->RxPausePkts		= CounterStats.InPause;
	
	port_counters->TxOctetsLo		= CounterStats.OutOctetsLo;
	port_counters->TxOctetsHi		= CounterStats.OutOctetsHi;
	port_counters->TxUnicastPkts	= CounterStats.OutUnicasts;
	port_counters->TxBroadcastPkts	= CounterStats.OutBroadcasts;
	port_counters->TxMulticastPkts	= CounterStats.OutMulticasts;
	port_counters->TxPausePkts		= CounterStats.OutPause;

	*valid_bit_mask = 0xFFF0;
	
	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53101							/* SWITCH_CHIP_BCM53101 */
	uint8	u8Data;
	uint64	u64Data;
	uint8	hport;
    uint32  u32Data,timeout = 0;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	if(lport == 0)
		hport = 8;	/* IMP port */
	else
		hport = hal_swif_lport_2_hport(lport);
		
	/* Rx Statistics */
	if(robo_read(0x20+hport, 0x88, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	port_counters->RxGoodOctetsHi = u64_H(u64Data);
	port_counters->RxGoodOctetsLo = u64_L(u64Data);
	if(robo_read(0x20+hport, 0x94, (uint8 *)(&port_counters->RxUnicastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x9C, (uint8 *)(&port_counters->RxBroadcastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x98, (uint8 *)(&port_counters->RxMulticastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x5C, (uint8 *)(&port_counters->RxPausePkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
   
	/* Tx Statistics */
	if(robo_read(0x20+hport, 0x00, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	port_counters->TxOctetsHi = u64_H(u64Data);
	port_counters->TxOctetsLo = u64_L(u64Data);
	if(robo_read(0x20+hport, 0x18, (uint8 *)(&port_counters->TxUnicastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x10, (uint8 *)(&port_counters->TxBroadcastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x14, (uint8 *)(&port_counters->TxMulticastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x38, (uint8 *)(&port_counters->TxPausePkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;

	*valid_bit_mask = 0xFFF0;
	
	return HAL_SWIF_SUCCESS;

//*********************************************************************//    

#elif SWITCH_CHIP_BCM53115							/* SWITCH_CHIP_BCM53115 */
	uint8	u8Data;
	uint64	u64Data;
	uint8	hport;
    uint32  u32Data,timeout = 0;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	if(lport == 0)
		hport = 8;	/* IMP port */
	else
		hport = hal_swif_lport_2_hport(lport);
		
	/* Rx Statistics */
	if(robo_read(0x20+hport, 0x88, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	port_counters->RxGoodOctetsHi = u64_H(u64Data);
	port_counters->RxGoodOctetsLo = u64_L(u64Data);
	if(robo_read(0x20+hport, 0x94, (uint8 *)(&port_counters->RxUnicastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x9C, (uint8 *)(&port_counters->RxBroadcastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x98, (uint8 *)(&port_counters->RxMulticastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x5C, (uint8 *)(&port_counters->RxPausePkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
   
	/* Tx Statistics */
	if(robo_read(0x20+hport, 0x00, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	port_counters->TxOctetsHi = u64_H(u64Data);
	port_counters->TxOctetsLo = u64_L(u64Data);
	if(robo_read(0x20+hport, 0x18, (uint8 *)(&port_counters->TxUnicastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x10, (uint8 *)(&port_counters->TxBroadcastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x14, (uint8 *)(&port_counters->TxMulticastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x20+hport, 0x38, (uint8 *)(&port_counters->TxPausePkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;

	*valid_bit_mask = 0xFFF0;

	return HAL_SWIF_SUCCESS;
	
////////////////////////////////////////////////////////////////////////////////////
	
#elif SWITCH_CHIP_BCM53286							/* SWITCH_CHIP_BCM53286 */
	uint8	u8Data;
    uint32  u32Data;
	uint64	u64Data;
	uint8	hport;
	uint16	u16PhyRegVal;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	if(lport == 0)
		hport = 24;	/* IMP port */
	else
		hport = hal_swif_lport_2_hport(lport);

	/* MIB Port Select Register, refer page366 */
	u8Data = hport;
	if(robo_write(0x50, 0x00, &u8Data, 1) != 0) 
		return HAL_SWIF_ERR_SPI_RW;

	/* Rx Statistics */
	if(robo_read(0x51, 0x3C, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	port_counters->RxGoodOctetsHi = u64_H(u64Data);
	port_counters->RxGoodOctetsLo = u64_L(u64Data);
	if(robo_read(0x51, 0x48, (uint8 *)(&port_counters->RxUnicastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x51, 0x50, (uint8 *)(&port_counters->RxBroadcastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x51, 0x4C, (uint8 *)(&port_counters->RxMulticastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x51, 0x28, (uint8 *)(&port_counters->RxPausePkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;

	/* Tx Statistics */
	if(robo_read(0x52, 0x1C, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	port_counters->TxOctetsHi = u64_H(u64Data);
	port_counters->TxOctetsLo = u64_L(u64Data);
	if(robo_read(0x52, 0x34, (uint8 *)(&port_counters->TxUnicastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x52, 0x2C, (uint8 *)(&port_counters->TxBroadcastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x52, 0x30, (uint8 *)(&port_counters->TxMulticastPkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_read(0x52, 0x28, (uint8 *)(&port_counters->TxPausePkts), 4) != 0) return HAL_SWIF_ERR_SPI_RW;

	*valid_bit_mask = 0xFFF0;
#if 0
	/* Histogram Statistics */
	if(HistogramMode == MODE_COUNT_RX_ONLY) {
		if(robo_read(0x51, 0x00, (uint8 *)(&port_counters->Octets64), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_read(0x51, 0x04, (uint8 *)(&port_counters->Octets127), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_read(0x51, 0x08, (uint8 *)(&port_counters->Octets255), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_read(0x51, 0x0C, (uint8 *)(&port_counters->Octets511), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_read(0x51, 0x10, (uint8 *)(&port_counters->Octets1023), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_read(0x51, 0x14, (uint8 *)(&port_counters->OctetsMax), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	} else if(HistogramMode == MODE_COUNT_TX_ONLY) {
		if(robo_read(0x52, 0x00, (uint8 *)(&port_counters->Octets64), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_read(0x52, 0x04, (uint8 *)(&port_counters->Octets127), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_read(0x52, 0x08, (uint8 *)(&port_counters->Octets255), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_read(0x52, 0x0C, (uint8 *)(&port_counters->Octets511), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_read(0x52, 0x10, (uint8 *)(&port_counters->Octets1023), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_read(0x52, 0x14, (uint8 *)(&port_counters->OctetsMax), 4) != 0) return HAL_SWIF_ERR_SPI_RW;		
	} else {
		port_counters->Octets64 = 0;
		port_counters->Octets127 = 0;
		port_counters->Octets255 = 0;
		port_counters->Octets511 = 0;
		port_counters->Octets1023 = 0;
		port_counters->OctetsMax = 0;
	}
#endif 

	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM5396							/* SWITCH_CHIP_BCM5396 */
	uint8	u8Data;
	uint64	u64Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	if(lport == 0)
		hport = 16;	/* IMP port */
	else
		hport = hal_swif_lport_2_hport(lport);

	/* Rx Statistics */
	if(robo_read(0x50+hport, 0x0c, (uint8 *)(&port_counters->RxGoodOctetsLo), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	port_counters->RxGoodOctetsHi	= 0;
	port_counters->RxUnicastPkts	= 0;
	port_counters->RxBroadcastPkts	= 0;
	port_counters->RxMulticastPkts	= 0;
	port_counters->RxPausePkts		= 0;
		
	/* Tx Statistics */
	if(robo_read(0x50+hport, 0x04, (uint8 *)(&port_counters->TxOctetsLo), 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	port_counters->TxOctetsHi	= 0;
	port_counters->TxUnicastPkts	= 0;
	port_counters->TxBroadcastPkts	= 0;
	port_counters->TxMulticastPkts	= 0;
	port_counters->TxPausePkts		= 0;

	*valid_bit_mask = 0x8200;
	
	return HAL_SWIF_SUCCESS;

#endif
	
    return HAL_SWIF_FAILURE;
}

/**************************************************************************
  * @brief  clear logic port mib counters
  * @param  lport
  * @retval 
  *************************************************************************/

//int hal_swif_port_clear_counters(uint8 lport, HAL_HISTOGRAM_MODE HistogramMode)
int hal_swif_port_clear_counters(uint8 lport)
{
#if SWITCH_CHIP_88E6095								/* SWITCH_CHIP_88E6095 */
	GT_STATUS status;
	GT_U32 hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	if(lport == 0)
		hport = 2;	/* CPU port */
	else
		hport = hal_swif_lport_2_hport(lport);
	
	if((status = gstatsFlushPort(dev, hport)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53101							/* SWITCH_CHIP_BCM53101 */
	uint8	hport;       
	uint32	u32Data;
	uint64	u64Data;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	if(lport == 0)
		hport = 8;	/* IMP port */
	else
		hport = hal_swif_lport_2_hport(lport);
        
    /* Rx Statistics */
	u64_H(u64Data) = u64_L(u64Data) = u32Data = 0;
	if(robo_write(0x20+hport, 0x88, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x94, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x9c, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x98, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x5C, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;

	/* Tx Statistics */
	if(robo_write(0x20+hport, 0x00, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x18, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x10, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x14, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x38, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	
	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53115							/* SWITCH_CHIP_BCM53115 */
//	uint8	u8Data1, u8Data2;  //reset MIB ports
	uint8	hport;       
	uint32	u32Data;           //clear hport port
	uint64	u64Data;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	if(lport == 0)
		hport = 8;	/* IMP port */
	else
		hport = hal_swif_lport_2_hport(lport);
        
    /* Rx Statistics */
	u64_H(u64Data) = u64_L(u64Data) = u32Data = 0;
	if(robo_write(0x20+hport, 0x88, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x94, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x9c, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x98, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x5C, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;

	/* Tx Statistics */
	if(robo_write(0x20+hport, 0x00, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x18, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x10, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x14, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x20+hport, 0x38, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
    
	/* Reset MIB Counters, refer to spec document for bcm53115    
	if(robo_read(0x02, 0x00, &u8Data1, 1) != 0) return HAL_SWIF_ERR_SPI_RW;
	u8Data2 = u8Data1;
	u8Data1 |= 0x01; 
	if(robo_write(0x02, 0x00, &u8Data1, 1) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x02, 0x00, &u8Data2, 1) != 0) return HAL_SWIF_ERR_SPI_RW;
	*/
	return HAL_SWIF_SUCCESS;

	
#elif SWITCH_CHIP_BCM53286							/* SWITCH_CHIP_BCM53286 */
	uint8	u8Data;
	uint32	u32Data;
	uint64	u64Data;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	if(lport == 0)
		hport = 24;	/* IMP port */
	else
		hport = hal_swif_lport_2_hport(lport);
	
	/* MIB Port Select Register, refer page366 */
	u8Data = hport;
	if(robo_write(0x50, 0x00, &u8Data, 1) != 0) 
		return HAL_SWIF_ERR_SPI_RW;

	/* Rx Statistics */
	u64_H(u64Data) = u64_L(u64Data) = u32Data = 0;
	if(robo_write(0x51, 0x3C, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x51, 0x48, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x51, 0x50, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x51, 0x4C, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x51, 0x28, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;

	/* Tx Statistics */
	if(robo_write(0x52, 0x1C, (uint8 *)&u64Data, 8) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x52, 0x34, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x52, 0x2C, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x52, 0x30, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x52, 0x28, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;

#if 0
	/* Histogram Statistics */ 
	if(HistogramMode == MODE_COUNT_RX_ONLY) {
		if(robo_write(0x51, 0x00, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_write(0x51, 0x04, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_write(0x51, 0x08, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_write(0x51, 0x0C, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_write(0x51, 0x10, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_write(0x51, 0x14, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
	} else if(HistogramMode == MODE_COUNT_TX_ONLY) {
		if(robo_write(0x52, 0x00, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_write(0x52, 0x04, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_write(0x52, 0x08, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_write(0x52, 0x0C, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_write(0x52, 0x10, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;
		if(robo_write(0x52, 0x14, (uint8 *)&u32Data, 4) != 0) return HAL_SWIF_ERR_SPI_RW;		
	} else {
		/* do nothing */
	}
#endif		
	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM5396							/* SWITCH_CHIP_BCM5396 */
	uint8	u8Data1, u8Data2;
	uint8	hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	if(lport == 0)
		hport = 16;	/* IMP port */
	else
		hport = hal_swif_lport_2_hport(lport);

	/* Reset MIB Counters, refer to spec document for bcm5396 */
	if(robo_read(0x02, 0x00, &u8Data1, 1) != 0) return HAL_SWIF_ERR_SPI_RW;
	u8Data2 = u8Data1;
	u8Data1 |= 0x01; 
	if(robo_write(0x02, 0x00, &u8Data1, 1) != 0) return HAL_SWIF_ERR_SPI_RW;
	if(robo_write(0x02, 0x00, &u8Data2, 1) != 0) return HAL_SWIF_ERR_SPI_RW;
	
	return HAL_SWIF_SUCCESS;
#endif
	return HAL_SWIF_FAILURE;
}

//int hal_swif_port_set_enable(uint8 lport, HAL_BOOL bEnable)
//int hal_port_set_autonego(unsigned int lport, HAL_PHY_AUTO_MODE mode)


/**************************************************************************
  * @brief  show logic port status
  * @param  
  * @retval 
  *************************************************************************/
int hal_swif_port_show_config(void *pCliEnv)
{
	uint8	lport;
	uint32 PortConfig;

	if(gPortConfigInitialized) {
		cli_printf(pCliEnv, "\r\n");
		cli_printf(pCliEnv, "  Port Configuration Information Display :\r\n");
		cli_printf(pCliEnv, "  =====================================================================================\r\n");
		cli_printf(pCliEnv, "  Port   Type    Enable  SpeedDuplex  FlowCtrl  NeigborSearch  TxThreshold  RxThreshold\r\n");
		cli_printf(pCliEnv, "  =====================================================================================\r\n");

		for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) 
		{

			cli_printf(pCliEnv, "   %02d   %7s    %3s    %10s     %3s        %3s  %14s%13s\r\n", 
				gPortConfigInfo[lport-1].PortNo, 
				
				(gPortConfigInfo[lport-1].PortType == S100M_CABLE) ?	"FE-RJ45" : \
				(gPortConfigInfo[lport-1].PortType == S100M_OPTICAL)?	"FE-SFP " : \
				(gPortConfigInfo[lport-1].PortType == S1000M_CABLE)?	"GE-RJ45" : \
				(gPortConfigInfo[lport-1].PortType == S1000M_OPTICAL)?	"GE-SFP " : "Unkown",
				
				(gPortConfigInfo[lport-1].PortEnable == HAL_TRUE)? "Yes" : "No",
				
				(gPortConfigInfo[lport-1].SpeedDuplex == S10M_HALF)?	"  10M-Half" : \
				(gPortConfigInfo[lport-1].SpeedDuplex == S10M_FULL)?	"  10M-Full" : \
				(gPortConfigInfo[lport-1].SpeedDuplex == S100M_HALF)?	" 100M-Half" : \
				(gPortConfigInfo[lport-1].SpeedDuplex == S100M_FULL)?	" 100M-Full" : \
				(gPortConfigInfo[lport-1].SpeedDuplex == S1000M_HALF)?	"1000M-Half" : \
				(gPortConfigInfo[lport-1].SpeedDuplex == S1000M_FULL)?	"1000M-Full" : \
				(gPortConfigInfo[lport-1].SpeedDuplex == AUTO_NEGO)?	" Auto-Nego" : "Unkown",

				(gPortConfigInfo[lport-1].FlowCtrl == HAL_TRUE)? "Yes" : "No",

				(gPortConfigInfo[lport-1].NeigborSearch == HAL_TRUE)? "Yes" : "No",


				(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_10) ? " 10 %" : \
				(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_20) ? " 20 %" : \
				(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_30) ? " 30 %" : \
				(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_40) ? " 40 %" : \
				(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_50) ? " 50 %" : \
				(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_60) ? " 60 %" : \
				(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_70) ? " 70 %" : \
				(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_80) ? " 80 %" : \
				(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_90) ? " 90 %" : \
				(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_100)? "100 %" : "Unkown",
				
				(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_10) ? " 10 %" : \
				(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_20) ? " 20 %" : \
				(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_30) ? " 30 %" : \
				(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_40) ? " 40 %" : \
				(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_50) ? " 50 %" : \
				(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_60) ? " 60 %" : \
				(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_70) ? " 70 %" : \
				(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_80) ? " 80 %" : \
				(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_90) ? " 90 %" : \
				(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_100) ?"100 %" : "Unkown");	
		}
		
		cli_printf(pCliEnv, "\r\n");
	} else {
		cli_printf(pCliEnv, "\r\n");
		cli_printf(pCliEnv, "  No port configuration information!\r\n");
		cli_printf(pCliEnv, "\r\n");
	}

	return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  show logic port status
  * @param  
  * @retval 
  *************************************************************************/
int hal_swif_port_show_status(void *pCliEnv)
{
	uint8	lport;
	uint8	portType;
    u16     data;
	HAL_PORT_LINK_STATE linkState;
	HAL_PORT_DUPLEX_STATE duplexState;
	HAL_PORT_SPEED_STATE speedState;
	HAL_PORT_STP_STATE stpState;
	
	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  Port Status Information Display :\r\n");
	cli_printf(pCliEnv, "  ====================================================\r\n");
	cli_printf(pCliEnv, "  Port   Type     Link   Duplex    Speed     STP-State \r\n");
	cli_printf(pCliEnv, "  ====================================================\r\n");

#if BOARD_GV3S_HONUE_QM
    /* display cpu port state */
    lport = CONFIG_SWITCH_CPU_PORT;
    gprtGetLinkState(dev,lport,(OUT GT_BOOL *)&linkState);
    gprtGetDuplex(dev,lport,(OUT GT_BOOL *)&duplexState);
    gprtGetSpeedMode(dev,lport,(OUT GT_PORT_SPEED_MODE *)&speedState);
	smi_getregfield(lport, SW_REG_PORT_CONTROL,0,2,(u16 *)&data); 
	stpState = (data & 0x3);

    cli_printf(pCliEnv, "   %02d   %7s   %4s    %4s    %6s    %10s\r\n", 0, 
        
        ("CpuPort"),
        
        (linkState == LINK_UP)? "Up" : "Down",
        
        (linkState == LINK_DOWN)? "--" : \
        (duplexState == FULL_DUPLEX)? "Full" : "Half",

        (linkState == LINK_DOWN)? "--" : \
        (speedState == SPEED_10M)? "10 M" : \
        (speedState == SPEED_100M)? "100 M" : \
        (speedState == SPEED_1000M)? "1000 M" : "Unkown",
        
        (stpState == DISABLED)? "Disabled" : \
        (stpState == BLOCKING)? "Blocking" : \
        (stpState == LEARNING)? "Learning" : \
        (stpState == FORWARDING)? "Forwarding" : "Unkown");	  
     /* display port state */
	for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
		portType = gHalPortMap[lport-1].port_type;

        if(lport == 9){
            fpga_get_sdi_status(1, (FPGA_BOOL *)&linkState);
        }else if(lport == 10){
            fpga_get_opt_lock(1, (FPGA_BOOL *)&linkState);
        }
        else{
            hal_swif_port_get_link_state(lport, &linkState);
        }
        
        if(lport == 9){
            duplexState = UNKOWN_DUPLEX;
            speedState = SPEED_UNKOWN;
            stpState = STP_UNKOWN;
        }else{
            hal_swif_port_get_duplex(lport, &duplexState);
            hal_swif_port_get_speed(lport, &speedState);
            hal_swif_port_get_stp_state(lport, &stpState);
        }

		cli_printf(pCliEnv, "   %02d   %7s   %4s    %4s    %6s    %10s\r\n", lport, 
			
			(gPortConfigInfo[lport-1].PortType == S100M_CABLE) ?	"FE-RJ45" : \
			(gPortConfigInfo[lport-1].PortType == S100M_OPTICAL)?	"FE-SFP" : \
			(gPortConfigInfo[lport-1].PortType == S1000M_CABLE)?	"GE-RJ45" : \
			(gPortConfigInfo[lport-1].PortType == S1000M_OPTICAL)?	"GE-SFP " : "Unkown",
			
			(linkState == LINK_UP)? "Up" : "Down",
			
			(linkState == LINK_DOWN)? "--" : \
			(duplexState == FULL_DUPLEX)? "Full" : "Half",

			(linkState == LINK_DOWN)? "--" : \
			(speedState == SPEED_10M)? "10 M" : \
			(speedState == SPEED_100M)? "100 M" : \
			(speedState == SPEED_1000M)? "1000 M" : "Unkown",
			
			(stpState == DISABLED)? "Disabled" : \
			(stpState == BLOCKING)? "Blocking" : \
			(stpState == LEARNING)? "Learning" : \
			(stpState == FORWARDING)? "Forwarding" : "Unkown");	
	}
        
#else /* NOT BOARD_GV3S_HONUE_QM */

#if SWITCH_CHIP_88E6095   
    /* display cpu port state */
    lport = CONFIG_SWITCH_CPU_PORT;
    gprtGetLinkState(dev,lport,(OUT GT_BOOL *)&linkState);
    gprtGetDuplex(dev,lport,(OUT GT_BOOL *)&duplexState);
    gprtGetSpeedMode(dev,lport,(OUT GT_PORT_SPEED_MODE *)&speedState);
	smi_getregfield(PHYADDR_PORT(lport), SW_REG_PORT_CONTROL,0,2,(u16 *)&data); 
	stpState = (data & 0x3);

    cli_printf(pCliEnv, "   %02d   %7s   %4s    %4s    %6s    %10s\r\n", 0, 
        
        ("CpuPort"),
        
        (linkState == LINK_UP)? "Up" : "Down",
        
        (linkState == LINK_DOWN)? "--" : \
        (duplexState == FULL_DUPLEX)? "Full" : "Half",

        (linkState == LINK_DOWN)? "--" : \
        (speedState == SPEED_10M)? "10 M" : \
        (speedState == SPEED_100M)? "100 M" : \
        (speedState == SPEED_1000M)? "1000 M" : "Unkown",
        
        (stpState == DISABLED)? "Disabled" : \
        (stpState == BLOCKING)? "Blocking" : \
        (stpState == LEARNING)? "Learning" : \
        (stpState == FORWARDING)? "Forwarding" : "Unkown");	 
#endif		
     /* display port state */
	for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
		portType = gHalPortMap[lport-1].port_type;
        
        hal_swif_port_get_link_state(lport, &linkState);
		hal_swif_port_get_duplex(lport, &duplexState);
		hal_swif_port_get_speed(lport, &speedState);
		hal_swif_port_get_stp_state(lport, &stpState);

		cli_printf(pCliEnv, "   %02d   %7s   %4s    %4s    %6s    %10s\r\n", lport, 
			
			(gPortConfigInfo[lport-1].PortType == S100M_CABLE) ?	"FE-RJ45" : \
			(gPortConfigInfo[lport-1].PortType == S100M_OPTICAL)?	"FE-SFP " : \
			(gPortConfigInfo[lport-1].PortType == S1000M_CABLE)?	"GE-RJ45" : \
			(gPortConfigInfo[lport-1].PortType == S1000M_OPTICAL)?	"GE-SFP " : "Unkown",
			
			(linkState == LINK_UP)? "Up" : "Down",
			
			(linkState == LINK_DOWN)? "--" : \
			(duplexState == FULL_DUPLEX)? "Full" : "Half",

			(linkState == LINK_DOWN)? "--" : \
			(speedState == SPEED_10M)? "10 M" : \
			(speedState == SPEED_100M)? "100 M" : \
			(speedState == SPEED_1000M)? "1000 M" : "Unkown",
			
			(stpState == DISABLED)? "Disabled" : \
			(stpState == BLOCKING)? "Blocking" : \
			(stpState == LEARNING)? "Learning" : \
			(stpState == FORWARDING)? "Forwarding" : "Unkown");	
	}

#if BOARD_GE220044MD
	/* display debug port state */
    lport = CONFIG_SWITCH_DEBUG_PORT;
    gprtGetLinkState(dev,lport,(OUT GT_BOOL *)&linkState);
    gprtGetDuplex(dev,lport,(OUT GT_BOOL *)&duplexState);
    gprtGetSpeedMode(dev,lport,(OUT GT_PORT_SPEED_MODE *)&speedState);
	smi_getregfield(PHYADDR_PORT(lport), SW_REG_PORT_CONTROL,0,2,(u16 *)&data); 
	stpState = (data & 0x3);

    cli_printf(pCliEnv, "   %02d   %-7s   %4s    %4s    %6s    %10s\r\n", 9, 
        
        ("Debug"),
        
        (linkState == LINK_UP)? "Up" : "Down",
        
        (linkState == LINK_DOWN)? "--" : \
        (duplexState == FULL_DUPLEX)? "Full" : "Half",

        (linkState == LINK_DOWN)? "--" : \
        (speedState == SPEED_10M)? "10 M" : \
        (speedState == SPEED_100M)? "100 M" : \
        (speedState == SPEED_1000M)? "1000 M" : "Unkown",
        
        (stpState == DISABLED)? "Disabled" : \
        (stpState == BLOCKING)? "Blocking" : \
        (stpState == LEARNING)? "Learning" : \
        (stpState == FORWARDING)? "Forwarding" : "Unkown");	 
#endif

#endif /* BOARD_GV3S_HONUE_QM */
	cli_printf(pCliEnv, "\r\n");
	
	return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  show logic port status
  * @param  
  * @retval 
  *************************************************************************/
int hal_swif_port_show_neigbor(void *pCliEnv)
{
	uint8	i, lport;
	uint8	portType;
	struct ip_addr ipaddr;
	char	switch_type[20];
	char	mac_str[20];
	char	neighbour_port_str[4];
	
	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  Port Neighbor Information Display :\r\n");
	cli_printf(pCliEnv, "  ==================================================================================================\r\n");
	cli_printf(pCliEnv, "  Port   PortStatus   ProbeEnable     NeighborMac     NeighborPort      NeighborIP    NeighborDevice\r\n");
	cli_printf(pCliEnv, "  ==================================================================================================\r\n");

	for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {

		for(i=0; i<8; i++) {
			if(gNeighborInformation[lport-1].NeighborSwitchType[i] != 0xFF) {
				sprintf(&switch_type[i*2],"%02X", gNeighborInformation[lport-1].NeighborSwitchType[i]);
			} else {
				switch_type[i*2] = 0;
				break;
			}
		}

		IP4_ADDR(&ipaddr, gNeighborInformation[lport-1].NeighborIP[0], gNeighborInformation[lport-1].NeighborIP[1], 
							gNeighborInformation[lport-1].NeighborIP[2], gNeighborInformation[lport-1].NeighborIP[3]);
//IP4_ADDR 高16位与低16位互换（最高~最低互换，中间互换）
		sprintf(mac_str,"%02x:%02x:%02x:%02x:%02x:%02x",	
			gNeighborInformation[lport-1].NeighborMac[0], gNeighborInformation[lport-1].NeighborMac[1],
			gNeighborInformation[lport-1].NeighborMac[2], gNeighborInformation[lport-1].NeighborMac[3],
			gNeighborInformation[lport-1].NeighborMac[4], gNeighborInformation[lport-1].NeighborMac[5]);

		sprintf(neighbour_port_str,"%02d", gNeighborInformation[lport-1].NeighborPort);  
		
		cli_printf(pCliEnv, "   %02d    %10s      %3s       %17s      %2s        %15s    %s\r\n", lport, 
			
			(gNeighborInformation[lport-1].PortStatus == 0x03)? "Forwarding" : \
			(gNeighborInformation[lport-1].PortStatus == 0x02)? "Blocking" : \
			(gNeighborInformation[lport-1].PortStatus == 0x01)? "Disconnect" : "Unkown",

			(gNeighborInformation[lport-1].PortSearchEnable == HAL_TRUE)? "Yes" : "No",

			
			((gNeighborInformation[lport-1].PortSearchEnable == HAL_TRUE) && \
			 ((gNeighborInformation[lport-1].PortStatus == 0x03) || (gNeighborInformation[lport-1].PortStatus == 0x02)) )? mac_str : "       --        ",

			((gNeighborInformation[lport-1].PortSearchEnable == HAL_TRUE) && \
			 ((gNeighborInformation[lport-1].PortStatus == 0x03) || (gNeighborInformation[lport-1].PortStatus == 0x02)) )? neighbour_port_str : "--",

			((gNeighborInformation[lport-1].PortSearchEnable == HAL_TRUE) && \
			 ((gNeighborInformation[lport-1].PortStatus == 0x03) || (gNeighborInformation[lport-1].PortStatus == 0x02)) )? ip_ntoa(&ipaddr) : "        --     ", 

			((gNeighborInformation[lport-1].PortSearchEnable == HAL_TRUE) && \
			 ((gNeighborInformation[lport-1].PortStatus == 0x03) || (gNeighborInformation[lport-1].PortStatus == 0x02)) )? switch_type : "   --");
	
	}
	cli_printf(pCliEnv, "\r\n");
	
	return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  show logic port mib counters
  * @param  lport
  * @retval 
  *************************************************************************/
//int hal_swif_port_show_counters(uint8 lport, HAL_HISTOGRAM_MODE HistogramMode)
int hal_swif_port_show_counters(void *pCliEnv, uint8 lport)
{
	hal_port_counters_t counters;
	uint16 valid_bit_mask;
	
	if(hal_swif_port_get_counters(lport, &counters, &valid_bit_mask) != HAL_SWIF_SUCCESS)	
		return HAL_SWIF_FAILURE;
	
	if(lport == 0)
		cli_printf(pCliEnv, "\r\n  IMP port statistics display :\r\n");
	else
		cli_printf(pCliEnv, "\r\n  Port%02d statistics display :\r\n", lport);
	cli_printf(pCliEnv, "  ------------------------------------------------------------\r\n", lport);
#if SWITCH_CHIP_BCM5396
	cli_printf(pCliEnv, "  RxGoodOctetsLo   : %-10u  TxOctetsLo       : %-10u\r\n", counters.RxGoodOctetsLo, counters.TxOctetsLo);	
#else
	cli_printf(pCliEnv, "  RxGoodOctetsLo   : %-10u  TxOctetsLo       : %-10u\r\n", counters.RxGoodOctetsLo, counters.TxOctetsLo);	
	cli_printf(pCliEnv, "  RxGoodOctetsHi   : %-10u  TxOctetsHi       : %-10u\r\n", counters.RxGoodOctetsHi, counters.TxOctetsHi);
	cli_printf(pCliEnv, "  RxUnicastPkts    : %-10u  TxUnicastPkts    : %-10u\r\n", counters.RxUnicastPkts, counters.TxUnicastPkts);
	cli_printf(pCliEnv, "  RxBroadcastPkts  : %-10u  TxBroadcastPkts  : %-10u\r\n", counters.RxBroadcastPkts, counters.TxBroadcastPkts);
	cli_printf(pCliEnv, "  RxMulticastPkts  : %-10u  TxMulticastPkts  : %-10u\r\n", counters.RxMulticastPkts, counters.TxMulticastPkts);
	cli_printf(pCliEnv, "  RxPausePkts      : %-10u  TxPausePkts      : %-10u\r\n\r\n", counters.RxPausePkts, counters.TxPausePkts);
#endif	

    return HAL_SWIF_SUCCESS;
}


int hal_swif_port_show_traffic(void *pCliEnv)
{
	hal_port_counters_t counters;
	uint8	lport;
	char TxRateStr[8], RxRateStr[8], TxThresholdStr[8], RxThresholdStr[8];
	extern hal_trap_info_t gTrapInfo;
	extern hal_port_traffic_info_t	gPortTrafficInfo[];
	
	if((gTrapInfo.GateMask & TRAP_MASK_TRAFFIC_OVER) == 0) {
		cli_printf(pCliEnv, "Warning: traffic over trap is not enable!\r\n");
		return 0;
	}
	
	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  Port traffic Information Display :\r\n");
	cli_printf(pCliEnv, "  ==========================================================\r\n");
	cli_printf(pCliEnv, "       |           ( Tx )         |           ( Rx )        |\r\n");
	cli_printf(pCliEnv, "  Port |  Rate(Mbps/s) Threshold  |  Rate(Mbps/s) Threshold |\r\n");
	cli_printf(pCliEnv, "  ==========================================================\r\n");
	
#if 1	
	for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
		if(gPortConfigInfo[lport-1].TxThreshold < PERCENTAGE_100) {
			sprintf(TxRateStr,"%2d.%02d", ((gPortTrafficInfo[lport-1].Interval_TxOctets/HAL_DEFAULT_TRAFFIC_INTERVAL)<<3)/1000000, 
											(((gPortTrafficInfo[lport-1].Interval_TxOctets/HAL_DEFAULT_TRAFFIC_INTERVAL)<<3)%1000000)/10000);
		} else {
			sprintf(TxRateStr,"%s"," --- ");
		}
			
		if(gPortConfigInfo[lport-1].RxThreshold < PERCENTAGE_100) {
			sprintf(RxRateStr,"%2d.%02d", ((gPortTrafficInfo[lport-1].Interval_RxOctets/HAL_DEFAULT_TRAFFIC_INTERVAL)<<3)/1000000, 
											(((gPortTrafficInfo[lport-1].Interval_RxOctets/HAL_DEFAULT_TRAFFIC_INTERVAL)<<3)%1000000)/10000);			
		} else {
			sprintf(RxRateStr,"%s"," --- ");
		}

		sprintf(TxThresholdStr, "%s",	(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_10) ? " 10 %" : \
										(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_20) ? " 20 %" : \
										(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_30) ? " 30 %" : \
										(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_40) ? " 40 %" : \
										(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_50) ? " 50 %" : \
										(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_60) ? " 60 %" : \
										(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_70) ? " 70 %" : \
										(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_80) ? " 80 %" : \
										(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_90) ? " 90 %" : \
										(gPortConfigInfo[lport-1].TxThreshold == PERCENTAGE_100)? "100 %" : "Unkown");
		

		sprintf(RxThresholdStr, "%s",	(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_10) ? " 10 %" : \
										(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_20) ? " 20 %" : \
										(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_30) ? " 30 %" : \
										(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_40) ? " 40 %" : \
										(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_50) ? " 50 %" : \
										(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_60) ? " 60 %" : \
										(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_70) ? " 70 %" : \
										(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_80) ? " 80 %" : \
										(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_90) ? " 90 %" : \
										(gPortConfigInfo[lport-1].RxThreshold == PERCENTAGE_100)? "100 %" : "Unkown");
				
		cli_printf(pCliEnv, "   %02d  |  %9s    %7s    |  %9s    %7s   |\r\n", lport, TxRateStr, TxThresholdStr, RxRateStr, RxThresholdStr);		
	}
#endif
    return HAL_SWIF_SUCCESS;
}

int hal_swif_port_show_frames(void *pCliEnv)
{
	hal_port_counters_t counters;
	uint8	lport;
	
	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  Frame types Information Display :\r\n");
	cli_printf(pCliEnv, "  ================================================================================================== \r\n");
	cli_printf(pCliEnv, "       |                    ( Tx )                   |                    ( Rx )                    |\r\n");
	cli_printf(pCliEnv, "  Port |    Unicast  Broadcast  Multicast      Pause |    Unicast  Broadcast  Multicast      Pause  |\r\n");
	cli_printf(pCliEnv, "  ================================================================================================== \r\n");
	cli_printf(pCliEnv, "   11    0123456789 0123456789 0123456789 0123456789   0123456789 0123456789 0123456789 0123456789   \r\n");

	for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
		cli_printf(pCliEnv, "   %02d    %10u %10u %10u %10u  %10u %10u %10u %10u\r\n", lport, 
			counters.TxUnicastPkts, counters.TxBroadcastPkts, counters.TxMulticastPkts, counters.TxPausePkts,
			counters.RxUnicastPkts, counters.RxBroadcastPkts, counters.RxMulticastPkts, counters.RxPausePkts);
	}
    
    return HAL_SWIF_SUCCESS;
}

int hal_swif_port_show_reglist(void *pCliEnv, uint8 lport)
{
#if SWITCH_CHIP_88E6095	
	GT_STATUS status;
	GT_U32 hport;
	GT_U16 reg_addr, reg_val;

	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);

	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  Port %d Registers :\r\n", lport);

	for(reg_addr=0; reg_addr<=25; reg_addr++) {
		if((reg_addr != 2) && (reg_addr != 5) && \
			(reg_addr != 12) && (reg_addr != 13) && (reg_addr != 14) && (reg_addr != 15) && \
			(reg_addr != 20) && (reg_addr != 21) && (reg_addr != 22) && (reg_addr != 23)) {
			
			if(smi_getreg(PHYADDR_PORT(hport), reg_addr, &reg_val) != SMI_DRV_SUCCESS) {
				return HAL_SWIF_FAILURE;
			}
			cli_printf(pCliEnv, "    Reg_addr = %02d, Reg_value = 0x%04x\r\n", reg_addr, reg_val);
		}
	}
	cli_printf(pCliEnv, "\r\n");

	cli_printf(pCliEnv, "  Switch Global Registers :\r\n");
	for(reg_addr=0; reg_addr<=31; reg_addr++) {
		if (reg_addr != 27) {
			if(smi_getreg(PHYADDR_GLOBAL, reg_addr, &reg_val) != SMI_DRV_SUCCESS) {
				return HAL_SWIF_FAILURE;
			}
			cli_printf(pCliEnv, "    Reg_addr = %02d, Reg_value = 0x%04x\r\n", reg_addr, reg_val);
		}
	}
	cli_printf(pCliEnv, "\r\n");

	
	cli_printf(pCliEnv, "  Switch Global2 Registers :\r\n");
	for(reg_addr=3; reg_addr<=8; reg_addr++) {
		if(smi_getreg(PHYADDR_GLOBAL2, reg_addr, &reg_val) != SMI_DRV_SUCCESS) {
			return HAL_SWIF_FAILURE;
		}
		cli_printf(pCliEnv, "    Reg_addr = %02d, Reg_value = 0x%04x\r\n", reg_addr, reg_val);
	}
	cli_printf(pCliEnv, "\r\n");
	
#endif	
    return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  port initialize use configuration
  * @param  none
  * @retval none
  *************************************************************************/
int hal_swif_port_conf_initialize(void)
{
	uint8 lport;
	hal_port_conf_t PortConfig;
	uint32 single_port_cfg;
	HAL_PORT_SPEED_DUPLEX speed_duplex;
	HAL_PORT_STP_STATE stp_state;
	HAL_PORT_TYPE port_type;
	uint8 tx_threshold, rx_threshold;
	int ret;
	extern tRingInfo RingInfo;
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	unsigned char RingIndex;
	unsigned char PortEnableCfgValid = 0;
	
	/* Initialize the Global infomation */
	memset(&gPortConfigInfo[0], 0, MAX_PORT_NUM*sizeof(hal_port_config_info_t));
	for(lport=1; lport<=MAX_PORT_NUM; lport++) {
		gPortConfigInfo[lport-1].PortNo = lport;
		gPortConfigInfo[lport-1].PortType = gHalPortMap[lport-1].port_type;
		gPortConfigInfo[lport-1].PortEnable = HAL_TRUE;
		gPortConfigInfo[lport-1].SpeedDuplex = AUTO_NEGO;
		gPortConfigInfo[lport-1].FlowCtrl = HAL_FALSE;
		gPortConfigInfo[lport-1].NeigborSearch = HAL_FALSE;
		gPortConfigInfo[lport-1].TxThreshold = PERCENTAGE_100;
		gPortConfigInfo[lport-1].RxThreshold = PERCENTAGE_100;
	}
	
	memset(&gNeighborInformation[0], 0, MAX_PORT_NUM*sizeof(hal_neighbor_record_t));
	for(lport=1; lport<=MAX_PORT_NUM; lport++) {
			gNeighborInformation[lport-1].PortSearchEnable = HAL_FALSE;
			gNeighborInformation[lport-1].PortNo = lport;
			gNeighborInformation[lport-1].PortStatus = 0x01;	
	}

	/* Read Configuration from EEPROM */
	if(eeprom_read(NVRAM_PORT_CFG_BASE, (uint8 *)&PortConfig, sizeof(hal_port_conf_t)) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	if((PortConfig.PortNum == 0) || (PortConfig.PortNum > MAX_PORT_NUM)) {
		for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
			if(hal_swif_port_set_stp_state(lport, FORWARDING) != HAL_SWIF_SUCCESS)
				return CONF_ERR_SWITCH_HAL;
			gPortConfigInfo[lport-1].PortEnable = HAL_TRUE;
		}
		return CONF_ERR_NO_CFG;
	}

	if(PortConfig.PortNum < DeviceBaseInfo.PortNum)
		DeviceBaseInfo.PortNum = PortConfig.PortNum;
		
	for(lport=1; lport<=PortConfig.PortNum; lport++) {
		single_port_cfg = *(uint32 *)&(PortConfig.PortConfig[(lport-1)*4]);
		single_port_cfg = cli_ntohl(single_port_cfg); 

		/**************************************************************************************/
		if((RingIndex = obring_get_ring_idx_by_port(lport)) != 0xFF) {
			pRingConfig = &(pRingInfo->RingConfig[RingIndex]);
			pRingState = &(pRingInfo->DevState[RingIndex]);
			if(pRingConfig->ucEnable == 0x01) {
				PortEnableCfgValid = 0;
			} else {
				PortEnableCfgValid = 1;
			}
		} else {
			PortEnableCfgValid = 1;
		}

		if(PortEnableCfgValid == 1) {
			if((single_port_cfg & PORT_CFG_MASK_ENABLE) == 0) {
				if(hal_swif_port_set_stp_state(lport, DISABLED) != HAL_SWIF_SUCCESS)
					return CONF_ERR_SWITCH_HAL;
				gPortConfigInfo[lport-1].PortEnable = HAL_FALSE;
			} else {
				if(hal_swif_port_set_stp_state(lport, FORWARDING) != HAL_SWIF_SUCCESS)
					return CONF_ERR_SWITCH_HAL;
				gPortConfigInfo[lport-1].PortEnable = HAL_TRUE;
			}
		}
		
		/**************************************************************************************/
		if((single_port_cfg & PORT_CFG_MASK_AUTO_NEG) == 0) {
			speed_duplex = (HAL_PORT_SPEED_DUPLEX)(((single_port_cfg & PORT_CFG_MASK_SPEED_DUPLEX) >> 3) & 0xFF);
			if(hal_swif_port_set_speed_duplex(lport, speed_duplex) != HAL_SWIF_SUCCESS)
				return CONF_ERR_SWITCH_HAL;
			gPortConfigInfo[lport-1].SpeedDuplex = speed_duplex;
		} else {
			gPortConfigInfo[lport-1].SpeedDuplex = AUTO_NEGO;
		}
		
		/**************************************************************************************/
		if(single_port_cfg & PORT_CFG_MASK_FLOW_CTRL) {
			gPortConfigInfo[lport-1].FlowCtrl = HAL_TRUE;
			if((gPortConfigInfo[lport-1].PortType == S100M_CABLE) || (gPortConfigInfo[lport-1].PortType == S1000M_CABLE)) {
				if(hal_swif_port_set_flow_control(lport, HAL_TRUE) != HAL_SWIF_SUCCESS)
					return CONF_ERR_SWITCH_HAL;	
			}
		} else {
			gPortConfigInfo[lport-1].FlowCtrl = HAL_FALSE;
		}
		
		/**************************************************************************************/
		if(single_port_cfg & PORT_CFG_MASK_NEIGHBOR_SEARCH) {
			if(PortEnableCfgValid == 1) {
				gPortConfigInfo[lport-1].NeigborSearch = HAL_TRUE;
				gNeighborInformation[lport-1].PortSearchEnable = HAL_TRUE;
				gNeighborInformation[lport-1].PortNo = lport;
			} else {
				gPortConfigInfo[lport-1].NeigborSearch = HAL_FALSE;
				gNeighborInformation[lport-1].PortSearchEnable = HAL_FALSE;
				gNeighborInformation[lport-1].PortNo = lport;
			}
		} else {
			gPortConfigInfo[lport-1].NeigborSearch = HAL_FALSE;
			gNeighborInformation[lport-1].PortSearchEnable = HAL_FALSE;
			gNeighborInformation[lport-1].PortNo = lport;
		}

		/**************************************************************************************/
#if (OB_NMS_PROTOCOL_VERSION == 1)
		switch((single_port_cfg & PORT_CFG_MASK_MDIX) >> 1) {
			case MODE_MDIX:		/* MDIX */
			break;

			case MODE_MDI:		/* MDI */
			break;

			case MODE_AutoMDIX:	/* AutoMDIX */
			break;

			default:
			break;
		}
#elif (OB_NMS_PROTOCOL_VERSION > 1)

		tx_threshold = ((single_port_cfg & PORT_CFG_MASK_TX_RATE_THRESHOLD) >> 20) & 0x0F;
		rx_threshold = ((single_port_cfg & PORT_CFG_MASK_RX_RATE_THRESHOLD) >> 16) & 0x0F;

		port_type = (HAL_PORT_TYPE)((single_port_cfg & PORT_CFG_MASK_TYPE) >> 30);
		
		gPortConfigInfo[lport-1].TxThreshold = (tx_threshold > PERCENTAGE_100)? PERCENTAGE_100 : tx_threshold;
		gPortConfigInfo[lport-1].RxThreshold = (rx_threshold > PERCENTAGE_100)? PERCENTAGE_100 : rx_threshold;			
#endif
		
	}

	gPortConfigInitialized = HAL_TRUE;
	
	return CONF_ERR_NONE;
}

#if MODULE_OBNMS

static u8 NMS_SetPortConfigIndex = 0;
static obnet_record_get_stat_t NMS_GetPortConfigState;
static obnet_record_get_stat_t NMS_GetNeighborState;
extern u8 NMS_TxBuffer[];

void nms_rsp_get_port_status(u8 *DMA, u8 *RequestID)//获取端口设备状态
{ 
	obnet_rsp_port_status RspPortStatus;	
	HAL_PORT_LINK_STATE link_state;
	HAL_PORT_SPEED_STATE speed;
	HAL_PORT_DUPLEX_STATE duplex;
	HAL_MDI_MDIX_STATE mdi_mdix;
	HAL_PORT_STP_STATE stp_state;
	u16	RspLength;
	u8	Lport;	
#if (RURAL_CREDIT_PROJECT && BOARD_GE1040PU)	
	extern u8 HonuPortStatus[];
	extern u8 HonuMasterFlag;
#endif

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	memset(&RspPortStatus, 0, sizeof(obnet_rsp_port_status));
	RspPortStatus.GetCode = CODE_PORT_STATUS;
	RspPortStatus.RetCode = 0x00;
/*#if (RURAL_CREDIT_PROJECT && BOARD_GE1040PU)
	if(HonuMasterFlag)
		RspPortStatus.PortNum = 26;
	else
		RspPortStatus.PortNum = 24;
#else*/
	RspPortStatus.PortNum = DeviceBaseInfo.PortNum;
//#endif

#if BOARD_GV3S_HONUE_QM  
        /* 添加一个SDI逻辑端口,从8个rj45、opt到sdi按次序数是1口到10口 */
    	for(Lport=1; Lport<=DeviceBaseInfo.PortNum; Lport++) {
		RspPortStatus.PortStatus[Lport-1] = 0;

        if(Lport>=1 && Lport<=8){
            hal_swif_port_get_link_state(Lport, &link_state);
            hal_swif_port_get_speed(Lport, &speed);
		    hal_swif_port_get_duplex(Lport, &duplex);
            hal_swif_port_get_stp_state(Lport, &stp_state);
        }
        
        if(Lport == 9){
            fpga_get_sdi_status(1, (FPGA_BOOL *)&link_state);
            speed = SPEED_UNKOWN;
            duplex = FULL_DUPLEX;
            stp_state = FORWARDING;
        }
        
        if(Lport == 10){
            fpga_get_opt_lock(1, (FPGA_BOOL *)&link_state);
            hal_swif_port_get_speed(Lport, &speed);
		    hal_swif_port_get_duplex(Lport, &duplex);
            hal_swif_port_get_stp_state(Lport, &stp_state);
        }

#if (OB_NMS_PROTOCOL_VERSION == 1)		
		hal_swif_port_get_mdi_mdix(Lport, &mdi_mdix);
		
		RspPortStatus.PortStatus[Lport-1] = ((link_state == LINK_UP)? 0x80 : 0x00) | \
											((speed == SPEED_10M)? 0x00 : (speed == SPEED_100M)? 0x10 : (speed == SPEED_1000M)? 0x20 : 0x00) | \
											((duplex == FULL_DUPLEX)? 0x08 : 0x00) | \
											((mdi_mdix == MODE_MDI)? 0x02 : 0x00);
#elif (OB_NMS_PROTOCOL_VERSION > 1)
		RspPortStatus.PortStatus[Lport-1] = ((link_state == LINK_UP)? 0x80 : 0x00) | \
											((speed == SPEED_10M)? 0x00 : (speed == SPEED_100M)? 0x10 : (speed == SPEED_1000M)? 0x20 : 0x00) | \
											((duplex == FULL_DUPLEX)? 0x08 : 0x00) | \
											((stp_state != DISABLED)? 0x04 : 0x00) | \
                                            ((Lport == 9 || Lport == 10)? 0x40 : 0x00); /* 非压缩 */
#endif /* OB_NMS_PROTOCOL_VERSION */
	}
		
#else /* Not BOARD_GV3S_HONUE_QM */
	for(Lport=1; Lport<=DeviceBaseInfo.PortNum; Lport++) {
		RspPortStatus.PortStatus[Lport-1] = 0;
#if (RURAL_CREDIT_PROJECT && BOARD_GE1040PU)
		if((Lport == 25) || (Lport == 26)) {		/* Port25: SDI, Port26: OPT */
			RspPortStatus.PortStatus[Lport-1] = HonuPortStatus[Lport-25];
		} else {
#endif						
			hal_swif_port_get_link_state(Lport, &link_state);
			hal_swif_port_get_speed(Lport, &speed);
			hal_swif_port_get_duplex(Lport, &duplex);
#if (OB_NMS_PROTOCOL_VERSION == 1)		
			hal_swif_port_get_mdi_mdix(Lport, &mdi_mdix);
			
			RspPortStatus.PortStatus[Lport-1] = ((link_state == LINK_UP)? 0x80 : 0x00) | \
												((speed == SPEED_10M)? 0x00 : (speed == SPEED_100M)? 0x10 : (speed == SPEED_1000M)? 0x20 : 0x00) | \
												((duplex == FULL_DUPLEX)? 0x08 : 0x00) | \
												((mdi_mdix == MODE_MDI)? 0x02 : 0x00);
#elif (OB_NMS_PROTOCOL_VERSION > 1)
			hal_swif_port_get_stp_state(Lport, &stp_state);
			RspPortStatus.PortStatus[Lport-1] = ((link_state == LINK_UP)? 0x80 : 0x00) | \
												((speed == SPEED_10M)? 0x00 : (speed == SPEED_100M)? 0x10 : (speed == SPEED_1000M)? 0x20 : 0x00) | \
												((duplex == FULL_DUPLEX)? 0x08 : 0x00) | \
												((stp_state != DISABLED)? 0x04 : 0x00);
#endif
#if (RURAL_CREDIT_PROJECT && BOARD_GE1040PU)
		}
#endif
	}    
#endif /* BOARD_GV3S_HONUE_QM */

	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspPortStatus, sizeof(obnet_rsp_port_status));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_port_status);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}


void nms_rsp_set_port_config(u8 *DMA, u8 *RequestID, obnet_set_port_config *pSetPortConfig)
{
	OBNET_SET_RSP RspSet;
	obnet_set_port_config2 *pSetPortConfig2 = (obnet_set_port_config2 *)pSetPortConfig;
	u16 RspLength;
	u8	retVal;
	int Ret;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_PORT_CFG;

	if(NMS_SetPortConfigIndex == 0) {
		if((pSetPortConfig->PortNum > MAX_PORT_NUM) || (pSetPortConfig->PortNum == 0)) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
		} else {
			if(pSetPortConfig->PortNum > FRAME_MAX_PORT_CONFIG_REC) {
				if(eeprom_page_write(NVRAM_PORT_CFG_BASE, (u8 *)&(pSetPortConfig->PortNum), FRAME_MAX_PORT_CONFIG_REC*4 + 1) != I2C_SUCCESS) {
					RspSet.RetCode = 0x01;
					RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
				} else {
					RspSet.RetCode = 0x00;
					RspSet.Res = 0x00;
					NMS_SetPortConfigIndex = 1;
				}			
			} else {
				if(eeprom_page_write(NVRAM_PORT_CFG_BASE, (u8 *)&(pSetPortConfig->PortNum), pSetPortConfig->PortNum*4 + 1) != I2C_SUCCESS) {
					RspSet.RetCode = 0x01;
					RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
				} else {
					RspSet.RetCode = 0x00;
					RspSet.Res = 0x00;
				}			
			}
		}
	} else {
		if(eeprom_page_write(NVRAM_PORT_CFG_BASE + FRAME_MAX_PORT_CONFIG_REC*4 + 1, (u8 *)&(pSetPortConfig2->PortConfig[0]), pSetPortConfig2->ItemsInData * 4) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
			NMS_SetPortConfigIndex = 0;
		}
	}

	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);//38
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}

void nms_rsp_get_port_config(u8 *DMA, u8 *RequestID, obnet_get_port_config *pGetPortConfig)
{
	obnet_rsp_port_config 	RspPortConfig;
	obnet_rsp_port_config2	RspPortConfig2;
	u8	PortNum;
	u16 RspLength;
	u8	bFirstRecFlag;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	memset(&RspPortConfig, 0, sizeof(obnet_rsp_port_config));
	memset(&RspPortConfig2, 0, sizeof(obnet_rsp_port_config2));
	RspPortConfig.GetCode = CODE_PORT_CONFIG;
	RspPortConfig2.GetCode = CODE_PORT_CONFIG;
	
	if(eeprom_read(NVRAM_PORT_CFG_BASE, &PortNum, 1) != I2C_SUCCESS) {
		goto ErrorPortConfig;
	} else {
		if((PortNum == 0) || (PortNum > MAX_PORT_NUM)) {
			goto ErrorPortConfig;
		} else {
			if(pGetPortConfig->OpCode == 0x00) {
				NMS_GetPortConfigState.PacketIndex = 1;
				NMS_GetPortConfigState.RemainCount = PortNum;
				NMS_GetPortConfigState.OffsetAddress = 0;
				bFirstRecFlag = 1;
			} else {
				bFirstRecFlag = 0;
			}

			if(bFirstRecFlag) {
				if(PortNum > FRAME_MAX_PORT_CONFIG_REC) {
					if(eeprom_read(NVRAM_PORT_CFG_DATA, (u8 *)&(RspPortConfig.PortConfig[0]), FRAME_MAX_PORT_CONFIG_REC * 4) != I2C_SUCCESS) {
						goto ErrorPortConfig;
					} else {
						RspPortConfig.RetCode = 0x00;
						RspPortConfig.PortNum = PortNum;
						
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspPortConfig, 3 + FRAME_MAX_PORT_CONFIG_REC * 4);
						RspLength = PAYLOAD_OFFSET + 3 + FRAME_MAX_PORT_CONFIG_REC * 4;
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);

						NMS_GetPortConfigState.PacketIndex++;
						NMS_GetPortConfigState.RemainCount -= FRAME_MAX_PORT_CONFIG_REC;
						NMS_GetPortConfigState.OffsetAddress += FRAME_MAX_PORT_CONFIG_REC * 4;
					}					
				} else {
					if(eeprom_read(NVRAM_PORT_CFG_DATA, (u8 *)&(RspPortConfig.PortConfig[0]), PortNum * 4) != I2C_SUCCESS) {
						goto ErrorPortConfig;
					} else {
						RspPortConfig.RetCode = 0x00;
						RspPortConfig.PortNum = PortNum;
						
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspPortConfig, 3 + PortNum * 4);
						RspLength = PAYLOAD_OFFSET + 3 + PortNum * 4;
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
					}
				}
			} else {
				if(NMS_GetPortConfigState.RemainCount > FRAME_MAX_PORT_CONFIG_REC) {
					if(eeprom_read(NVRAM_PORT_CFG_DATA + NMS_GetPortConfigState.OffsetAddress, (u8 *)&(RspPortConfig2.PortConfig[0]), FRAME_MAX_PORT_CONFIG_REC * 4) != I2C_SUCCESS) {
						goto ErrorPortConfig;
					} else {
						RspPortConfig2.RetCode = 0x00;
						RspPortConfig2.Res = 0x00;
						RspPortConfig2.OpCode = (0xc0 | (NMS_GetPortConfigState.PacketIndex - 1));
						RspPortConfig2.ItemsInData = FRAME_MAX_PORT_CONFIG_REC;
						
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspPortConfig2, 5 + FRAME_MAX_PORT_CONFIG_REC * 4);
						RspLength = PAYLOAD_OFFSET + 5 + FRAME_MAX_PORT_CONFIG_REC * 4;
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);

						NMS_GetPortConfigState.PacketIndex++;
						NMS_GetPortConfigState.RemainCount -= FRAME_MAX_PORT_CONFIG_REC;
						NMS_GetPortConfigState.OffsetAddress += FRAME_MAX_PORT_CONFIG_REC * 4;
					}
				} else {
					if(eeprom_read(NVRAM_PORT_CFG_DATA + NMS_GetPortConfigState.OffsetAddress, (u8 *)&(RspPortConfig2.PortConfig[0]), NMS_GetPortConfigState.RemainCount * 4) != I2C_SUCCESS) {
						goto ErrorPortConfig;
					} else {
						RspPortConfig2.RetCode = 0x00;
						RspPortConfig2.Res = 0x00;
						RspPortConfig2.OpCode = (0x80 | (NMS_GetPortConfigState.PacketIndex - 1));
						RspPortConfig2.ItemsInData = NMS_GetPortConfigState.RemainCount;
						
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspPortConfig2, 5 + NMS_GetPortConfigState.RemainCount * 4);
						RspLength = PAYLOAD_OFFSET + 5 + NMS_GetPortConfigState.RemainCount * 4;
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);

						NMS_GetPortConfigState.PacketIndex++;
						NMS_GetPortConfigState.RemainCount = 0;
						NMS_GetPortConfigState.OffsetAddress += NMS_GetPortConfigState.RemainCount;
					}
				}
			}
		}
	}

	return;
	
ErrorPortConfig:
	memset(&RspPortConfig, 0, sizeof(obnet_rsp_port_config));
	RspPortConfig.GetCode = CODE_PORT_CONFIG;
	RspPortConfig.RetCode = 0x01;
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspPortConfig, sizeof(obnet_rsp_port_config));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_port_config);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}


void nms_rsp_get_port_neighbor(u8 *DMA, u8 *RequestID, obnet_get_port_neighbor *pGetNeighbor)
{
	obnet_rsp_get_port_neighbor RspGetNeighbor;	
	u16 RspLength;
	u8	bFirstRecFlag;
	u8	TotalRecordCount;
	u8	Lport;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	memset(&RspGetNeighbor, 0, sizeof(obnet_rsp_get_port_neighbor));
	RspGetNeighbor.GetCode = CODE_GET_NEIGHBOR;
	RspGetNeighbor.RetCode = 0x00;
	RspGetNeighbor.PortNum = DeviceBaseInfo.PortNum;
	TotalRecordCount = DeviceBaseInfo.PortNum;
	
	if(pGetNeighbor->OpCode == 0x00) {
		NMS_GetNeighborState.PacketIndex = 1;
		NMS_GetNeighborState.RemainCount = TotalRecordCount;
		NMS_GetNeighborState.OffsetAddress = 0;
		bFirstRecFlag = 1;
	} else {
		bFirstRecFlag = 0;
	}

	if(TotalRecordCount == 0) {
		RspGetNeighbor.OpCode = 0x00;
		RspGetNeighbor.RecordCount = 0x00;
		memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetNeighbor, sizeof(obnet_rsp_get_port_neighbor));
		RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_neighbor);
		if (RspLength < MSG_MINSIZE)
			RspLength = MSG_MINSIZE;
		PrepareEtherHead(DMA);
		PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
		if(RspLength == MSG_MINSIZE)
			RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
		else
			RspSend(NMS_TxBuffer, RspLength);
		
	} else if((TotalRecordCount > 0) && (TotalRecordCount < 3)) {
		RspGetNeighbor.OpCode = 0x00;
		RspGetNeighbor.RecordCount = TotalRecordCount;
		memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetNeighbor, sizeof(obnet_rsp_get_port_neighbor));
		memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_neighbor)], (u8 *)&(gNeighborInformation[0]), TotalRecordCount * sizeof(hal_neighbor_record_t));
		RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_neighbor) + TotalRecordCount * sizeof(hal_neighbor_record_t);
		if (RspLength < MSG_MINSIZE)
			RspLength = MSG_MINSIZE;
		PrepareEtherHead(DMA);
		PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
		if(RspLength == MSG_MINSIZE)
			RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
		else
			RspSend(NMS_TxBuffer, RspLength);
				
	} else {
		if(NMS_GetNeighborState.RemainCount >= 2) {
			if(bFirstRecFlag) {
				bFirstRecFlag = 0;
				RspGetNeighbor.OpCode = (0x40 | NMS_GetNeighborState.PacketIndex);
			} else {
				if(NMS_GetNeighborState.RemainCount == 2)
					RspGetNeighbor.OpCode = (0x80 | NMS_GetNeighborState.PacketIndex);
				else
					RspGetNeighbor.OpCode = (0xc0 | NMS_GetNeighborState.PacketIndex);
			}
			RspGetNeighbor.RecordCount = 0x02;

			memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetNeighbor, sizeof(obnet_rsp_get_port_neighbor));
			memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_neighbor)], (u8 *)&(gNeighborInformation[TotalRecordCount - NMS_GetNeighborState.RemainCount]), 2 * sizeof(hal_neighbor_record_t));
			RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_neighbor) + 2 * sizeof(hal_neighbor_record_t);
			if (RspLength < MSG_MINSIZE)
				RspLength = MSG_MINSIZE;
			PrepareEtherHead(DMA);
			PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
			if(RspLength == MSG_MINSIZE)
				RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
			else
				RspSend(NMS_TxBuffer, RspLength);
			
			NMS_GetNeighborState.RemainCount -= 2;
			NMS_GetNeighborState.OffsetAddress += 2 * sizeof(hal_neighbor_record_t);
			NMS_GetNeighborState.PacketIndex++;
			
		} else {
			RspGetNeighbor.OpCode = (0x80 | NMS_GetNeighborState.PacketIndex);
			RspGetNeighbor.RecordCount = NMS_GetNeighborState.RemainCount;

			memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetNeighbor, sizeof(obnet_rsp_get_port_neighbor));
			memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_neighbor)], (u8 *)&(gNeighborInformation[TotalRecordCount - NMS_GetNeighborState.RemainCount]), NMS_GetNeighborState.RemainCount * sizeof(hal_neighbor_record_t));
			RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_neighbor) + NMS_GetNeighborState.RemainCount * sizeof(hal_neighbor_record_t);
			if (RspLength < MSG_MINSIZE)
				RspLength = MSG_MINSIZE;
			PrepareEtherHead(DMA);
			PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
			if(RspLength == MSG_MINSIZE)
				RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
			else
				RspSend(NMS_TxBuffer, RspLength);
			
			NMS_GetNeighborState.RemainCount = 0;
			NMS_GetNeighborState.OffsetAddress = 0;	
		}
	}
}

void nms_rsp_port_statistics(u8 *DMA, u8 *RequestID, obnet_port_statistic *pSetPortStatistics)
{
	obnet_rsp_port_statistic RspPortStatistic;
	hal_port_counters_t Counters;
	u16 RspLength;
	u8	LportMap[MAX_STATISTIC_PORT_NUM];
	u32	LportBitMap;
	int	Lport,k,statistic_port_num;
	uint16	valid_bit_mask;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	memset(&RspPortStatistic, 0, sizeof(obnet_rsp_port_statistic));
	RspPortStatistic.GetCode = CODE_PORT_STATISTICS;
	RspPortStatistic.RetCode = 0;
	RspPortStatistic.PortNum = pSetPortStatistics->PortNum;
	RspPortStatistic.OpCode = pSetPortStatistics->OpCode;
	memcpy(RspPortStatistic.PortMap, pSetPortStatistics->PortMap, 4);
	
	LportBitMap = *(u32 *)&(pSetPortStatistics->PortMap[0]);
	LportBitMap = cli_ntohl(LportBitMap);
	for(Lport=1, k=0; Lport<=32 && k<MAX_STATISTIC_PORT_NUM; Lport++) {
		if((LportBitMap >> (Lport-1)) & 0x01) {
			LportMap[k] = Lport;
			k++;
		}
	}
	statistic_port_num = k;
	
	if(statistic_port_num > 0) {
		switch(pSetPortStatistics->OpCode) {
			case COUNTERS_READ:
			for(k=0; k<statistic_port_num; k++) {
				if(hal_swif_port_get_counters(LportMap[k], &Counters, &valid_bit_mask) != HAL_SWIF_SUCCESS)
					continue;
				RspPortStatistic.ValidItemsMask[0] = (uint8)((valid_bit_mask & 0xFF00) >> 8);
				RspPortStatistic.ValidItemsMask[1] = valid_bit_mask & 0x00FF;

				RspPortStatistic.CountersInfo[k].RxGoodOctetsLo = cli_htonl(Counters.RxGoodOctetsLo);
				RspPortStatistic.CountersInfo[k].RxGoodOctetsHi = cli_htonl(Counters.RxGoodOctetsHi);
				RspPortStatistic.CountersInfo[k].RxUnicastPkts = cli_htonl(Counters.RxUnicastPkts);
				RspPortStatistic.CountersInfo[k].RxBroadcastPkts = cli_htonl(Counters.RxBroadcastPkts);
				RspPortStatistic.CountersInfo[k].RxMulticastPkts = cli_htonl(Counters.RxMulticastPkts);
				RspPortStatistic.CountersInfo[k].RxPausePkts = cli_htonl(Counters.RxPausePkts);
				
				RspPortStatistic.CountersInfo[k].TxOctetsLo = cli_htonl(Counters.TxOctetsLo);
				RspPortStatistic.CountersInfo[k].TxOctetsHi = cli_htonl(Counters.TxOctetsHi);
				RspPortStatistic.CountersInfo[k].TxUnicastPkts = cli_htonl(Counters.TxUnicastPkts);
				RspPortStatistic.CountersInfo[k].TxBroadcastPkts = cli_htonl(Counters.TxBroadcastPkts);
				RspPortStatistic.CountersInfo[k].TxMulticastPkts = cli_htonl(Counters.TxMulticastPkts);
				RspPortStatistic.CountersInfo[k].TxPausePkts = cli_htonl(Counters.TxPausePkts);
			
			}
			break;

			case COUNTERS_CLEAR:
			for(k=0; k<statistic_port_num; k++) {
				if(hal_swif_port_clear_counters_flag(LportMap[k]) != HAL_SWIF_SUCCESS)
					continue;
#if SWITCH_CHIP_BCM5396
				break;
#endif
			}
			break;

			default:
			break;
		}
	}
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspPortStatistic, sizeof(obnet_rsp_port_statistic));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_port_statistic);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}

#endif

