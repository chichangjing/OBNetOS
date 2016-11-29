
/*****************************************************************
 * Filename     : hal_swif_mac.c
 * Description  : Hardware Abstraction Layer for L2 MAC table API
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *****************************************************************/
#include "mconfig.h"

/* Standard includes */
#include <stdio.h>

/* Kernel includes */
#include "FreeRTOS.h"
#include "task.h"
#include "os_mutex.h"

/* LwIP includes */
#include "lwip/netif.h"
#include "lwip/stats.h"

/* BSP includes */
#include "stm32f2x7_smi.h"
#include "soft_i2c.h"
#include "misc_drv.h"
#if ROBO_SWITCH
#include "robo_drv.h"
#elif MARVELL_SWITCH
#include "msApi.h"
#include <gtSem.h>
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
#include "hal_swif_mac.h"

/* Other includes */
#include "cli_util.h"

#if MARVELL_SWITCH
extern GT_QD_DEV *dev;
#endif

HAL_BOOL PortSecurityEnable = HAL_FALSE;
port_security_info *pSecurityInfos = NULL;

int hal_swif_mac_flush_all(void)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	if((status = gfdbFlush(dev,GT_FLUSH_ALL_UNLOCKED)) != GT_OK) {
		return HAL_SWIF_FAILURE;
	}

	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53101
	int port;
	unsigned char reg_val;
	int i;

	for(port=0; port<MAX_PORT_NUM; port++) {
		/* Set fast-aging port control register */
		reg_val = port;
		robo_write(PAGE_CONTROL, REG_FAST_AGE_PORT, &reg_val, 1);	

		/* Start fast aging process */
		robo_read(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);	

		reg_val |= (MASK_EN_FAST_AGE_STATIC | MASK_EN_FAST_AGE_DYNAMIC | MASK_FAST_AGE_STR_DONE); 
		robo_write(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);

		/* Wait for complete */
		for(i=0; i<100; i++) {
			robo_read(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);	
			if((reg_val & 0x80) == 0)
				break;
		}

		/* Restore register value to 0x2, otherwise aging will fail	*/
		reg_val = MASK_EN_FAST_AGE_DYNAMIC;
		robo_write(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);
	}
	
	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM53286
	uint8 	hport;
	uint8 	fast_age_ctrl_val, tmp_val;
	uint32	age_out_ctrl_val;
	int i;

	if(robo_read(BCM53286_PAGE_ARL_CTRL, BCM53286_FAST_AGING_CTRL, &fast_age_ctrl_val, 1) != 0)
		return HAL_SWIF_FAILURE;
	
	fast_age_ctrl_val &= (MASK_EN_AGE_DYNAMIC_MC | MASK_EN_AGE_DYNAMIC_UC | MASK_EN_AGE_STATIC_MC | MASK_EN_AGE_STATIC_UC);
	fast_age_ctrl_val |= (MASK_EN_AGE_DYNAMIC_MC | MASK_EN_AGE_DYNAMIC_UC | MASK_FAST_AGE_STDN);
	
	for(hport=0; hport<29; hport++) {
		/* Set Age Out Control Register, Selects the Port ID to be aged out */
		age_out_ctrl_val = hport;
		if(robo_write(BCM53286_PAGE_ARL_CTRL, BCM53286_AGE_OUT_CTRL, (uint8 *)&age_out_ctrl_val, 4) != 0)
			return HAL_SWIF_FAILURE;
		
		/* Set fast-aging control register, AGE_MODE_CTRL = 3'b000 */
		if(robo_write(BCM53286_PAGE_ARL_CTRL, BCM53286_FAST_AGING_CTRL, &fast_age_ctrl_val, 1) != 0)
			return HAL_SWIF_FAILURE;	

		/* Wait for fast aging process complete */
		for(i=0; i<100; i++) {
			if(robo_read(BCM53286_PAGE_ARL_CTRL, BCM53286_FAST_AGING_CTRL, &tmp_val, 1) != 0)
				return HAL_SWIF_FAILURE;

			if((tmp_val & MASK_FAST_AGE_STDN) == 0)
				break;
		}

		if(i==100) {
			printf("Error: Fast aging timeout\r\n");
			return HAL_SWIF_FAILURE;
		}
	}
	
	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM5396
	u8 	hport, u8Data;
	u16	reg_age_vid, vid;
	u8	reg_age_port, tmp_val;
	int i;

#if 1
	/* All VIDs shall be aged out */
	reg_age_vid = 0x8000;
	if(robo_write(BCM5396_PAGE_CTRL, BCM5396_FAST_AGING_VID, (u8 *)&reg_age_vid, 2) != 0)
		return HAL_SWIF_FAILURE;
		
	for(hport=0; hport<=16; hport++) {
		
		/* Port ID shall be aged out */
		reg_age_port = hport;
		if(robo_write(BCM5396_PAGE_CTRL, BCM5396_FAST_AGING_PORT, (u8 *)&reg_age_port, 1) != 0)
			return HAL_SWIF_FAILURE;	
		
		/* Start fast aging process */
		u8Data = 0x80;
		if(robo_write(BCM5396_PAGE_CTRL, BCM5396_FAST_AGING_CTRL, (u8 *)&u8Data, 1) != 0)
			return HAL_SWIF_FAILURE;
	
		/* Wait for fast aging process complete */
		for(i=0; i<100; i++) {
			if(robo_read(BCM5396_PAGE_CTRL, BCM5396_FAST_AGING_CTRL, (u8 *)&tmp_val, 1) != 0)
				return HAL_SWIF_FAILURE;

			if((tmp_val & 0x80) == 0)
				break;
		}

		if(i==100) {
			printf("Error: Fast aging timeout\r\n");
			return HAL_SWIF_FAILURE;
		}
	}
#else
	/* All Ports shall be aged out */
	reg_age_port = 0x80;
	if(robo_write(BCM5396_PAGE_CTRL, BCM5396_FAST_AGING_PORT, (u8 *)&reg_age_port, 1) != 0)
		return HAL_SWIF_FAILURE;
		
	for(vid=0; vid<=1; vid++) {
		
		/* VID shall be aged out */
		reg_age_vid = vid;
		if(robo_write(BCM5396_PAGE_CTRL, BCM5396_FAST_AGING_VID, (u8 *)&reg_age_vid, 2) != 0)
			return HAL_SWIF_FAILURE;
		
		/* Start fast aging process */
		u8Data = 0x80;
		if(robo_write(BCM5396_PAGE_CTRL, BCM5396_FAST_AGING_CTRL, (u8 *)&u8Data, 1) != 0)
			return HAL_SWIF_FAILURE;
	
		/* Wait for fast aging process complete */
		for(i=0; i<100; i++) {
			if(robo_read(BCM5396_PAGE_CTRL, BCM5396_FAST_AGING_CTRL, (u8 *)&tmp_val, 1) != 0)
				return HAL_SWIF_FAILURE;

			if((tmp_val & 0x80) == 0)
				break;
		}

		if(i==100) {
			printf("Error: Fast aging timeout\r\n");
			return HAL_SWIF_FAILURE;
		}
	}
#endif
	
	return HAL_SWIF_SUCCESS;
#endif

	return HAL_SWIF_SUCCESS;
}

int hal_swif_mac_add(u8 *Mac, u8 Prio, u32 PortVec)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;

	memset(&macEntry,0,sizeof(GT_ATU_ENTRY));	
	macEntry.macAddr.arEther[0] = Mac[0];
	macEntry.macAddr.arEther[1] = Mac[1];
	macEntry.macAddr.arEther[2] = Mac[2];
	macEntry.macAddr.arEther[3] = Mac[3];
	macEntry.macAddr.arEther[4] = Mac[4];
	macEntry.macAddr.arEther[5] = Mac[5];
	macEntry.DBNum = 0;
	macEntry.portVec = PortVec;
	macEntry.prio = Prio;
	macEntry.entryState.ucEntryState = GT_UC_TO_CPU_STATIC;//GT_UC_STATIC;

	if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK) {
		return HAL_SWIF_FAILURE;
	}

	return HAL_SWIF_SUCCESS;
#endif	
	return HAL_SWIF_SUCCESS;
}

int hal_swif_mac_unicast_get(void)
{
#if SWITCH_CHIP_88E6095
	return HAL_SWIF_SUCCESS;
#elif SWITCH_CHIP_BCM53101
	
	
	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM53286
	
	return HAL_SWIF_SUCCESS;
#endif
	
	
    return HAL_SWIF_SUCCESS;
}

int hal_swif_mac_unicast_show(void *pCliEnv)
{
#if SWITCH_CHIP_88E6095
	return HAL_SWIF_SUCCESS;
#elif SWITCH_CHIP_BCM53101

	uint32	arl_idx;
	uint8	poll_cnt;
	uint8	u8Data;
	uint16	mem_addr;
	uint8	mac_addr[6];
	uint8	hport_id, lport_id;
	uint16	vlan_id;
	br_arl64_t	arl_macvid_result1, arl_macvid_result0;
	br_arl32_t	arl_result1, arl_result0;
	char	s[16], s1[3];

	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  =====================================================\r\n");
	cli_printf(pCliEnv, "   No.  Mac Address            EntryState    Port  VID  \r\n");
	cli_printf(pCliEnv, "  =====================================================\r\n");
	
	/* Start search ARL entries */
	u8Data = 0x80;
	if(robo_write(BCM53101M_PAGE_ARL_VLAN_TBL, BCM53101M_ARL_SEARCH_CTRL, &u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;

	/* Wait until MEM_CTRL.MEM_STDN = 0 */
	arl_idx = 0;
    while (1) {
	    poll_cnt = 0;
	    while (poll_cnt++ < 20) {
			if(robo_read(BCM53101M_PAGE_ARL_VLAN_TBL, BCM53101M_ARL_SEARCH_CTRL, &u8Data, 1) != 0)
				return HAL_SWIF_FAILURE;

			/* Arl is valid or search operation is done */
			if(((u8Data & 0x80) == 0) || ((u8Data & 0x01) == 0x01))
				break;
	    }

		if(poll_cnt == 20)
			return HAL_SWIF_FAILURE;

		/* Arl is valid */
		if((u8Data & 0x01) == 0x01) {

			if(robo_read(BCM53101M_PAGE_ARL_VLAN_TBL, BCM53101M_ARL_SEARCH_MACVID_RESULT0, (uint8 *)&arl_macvid_result0, 8) != 0)
				return HAL_SWIF_FAILURE;

			if(robo_read(BCM53101M_PAGE_ARL_VLAN_TBL, BCM53101M_ARL_SEARCH_RESULT0, (uint8 *)&arl_result0, 4) != 0)
				return HAL_SWIF_FAILURE;
			
			if(robo_read(BCM53101M_PAGE_ARL_VLAN_TBL, BCM53101M_ARL_SEARCH_MACVID_RESULT1, (uint8 *)&arl_macvid_result1, 8) != 0)
				return HAL_SWIF_FAILURE;

			if(robo_read(BCM53101M_PAGE_ARL_VLAN_TBL, BCM53101M_ARL_SEARCH_RESULT1, (uint8 *)&arl_result1, 4) != 0)
				return HAL_SWIF_FAILURE;
			
			if(arl_result1.arl_data & 0x00010000) {
				mac_addr[0] = (uint8)((arl_macvid_result1.arl_hi32.bcm5396_arl_macvid_hi._arl_mac_47_32 & 0x0000FF00) >> 8);
				mac_addr[1] = (uint8)(arl_macvid_result1.arl_hi32.bcm5396_arl_macvid_hi._arl_mac_47_32 & 0x000000FF);
				mac_addr[2] = (uint8)((arl_macvid_result1.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0xFF000000) >> 24);
				mac_addr[3] = (uint8)((arl_macvid_result1.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x00FF0000) >> 16);
				mac_addr[4] = (uint8)((arl_macvid_result1.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x0000FF00) >> 8);
				mac_addr[5] = (uint8)(arl_macvid_result1.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x000000FF);
				vlan_id = (uint16)(arl_macvid_result1.arl_hi32.bcm5396_arl_macvid_hi._arl_vid);

				if(mac_addr[0] & 0x01) {	/* Multicast */
//					cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)*\r\n",arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);	
				} else {					/* Unicast */
					hport_id = (uint8)(arl_result1.bcm5396_arl_unicast._arl_portid_n);
					lport_id = hal_swif_hport_2_lport(hport_id);
					
					if(arl_result1.bcm5396_arl_unicast._arl_sr_staric_n)
						sprintf(s,"%s","Uc-Static");
					else
						sprintf(s,"%s","Uc-Dynamic");

					if(mac_addr[0] & 0x80)
						sprintf(s1,"%s","m1");
					else
						sprintf(s1,"%s","  ");
					
					if(hport_id == 8)
						cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)%s  %-10s    CPU   %-04d\r\n",
							arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], s1, s, vlan_id);				
					else
						cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)%s  %-10s    P%-4d %-04d\r\n",
							arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], s1, s, lport_id, vlan_id);
				}
                  
			}

			if(arl_result0.arl_data & 0x00010000) {
				mac_addr[0] = (uint8)((arl_macvid_result0.arl_hi32.bcm5396_arl_macvid_hi._arl_mac_47_32 & 0x0000FF00) >> 8);
				mac_addr[1] = (uint8)(arl_macvid_result0.arl_hi32.bcm5396_arl_macvid_hi._arl_mac_47_32 & 0x000000FF);
				mac_addr[2] = (uint8)((arl_macvid_result0.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0xFF000000) >> 24);
				mac_addr[3] = (uint8)((arl_macvid_result0.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x00FF0000) >> 16);
				mac_addr[4] = (uint8)((arl_macvid_result0.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x0000FF00) >> 8);
				mac_addr[5] = (uint8)(arl_macvid_result0.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x000000FF);
				vlan_id = (uint16)(arl_macvid_result0.arl_hi32.bcm5396_arl_macvid_hi._arl_vid);

				if(mac_addr[0] & 0x01) {	/* Multicast */
//					cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)\r\n",arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);				
				} else {					/* Unicast */
					hport_id = (uint8)(arl_result0.bcm5396_arl_unicast._arl_portid_n);
					lport_id = hal_swif_hport_2_lport(hport_id);
					
					if(arl_result0.bcm5396_arl_unicast._arl_sr_staric_n)
						sprintf(s,"%s","Uc-Static");
					else
						sprintf(s,"%s","Uc-Dynamic");

					if(mac_addr[0] & 0x80)
						sprintf(s1,"%s","m ");
					else
						sprintf(s1,"%s","  ");
					
					if(hport_id == 8)
						cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)%s  %-10s    CPU   %-04d\r\n",
							arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], s1, s, vlan_id);				
					else
						cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)%s  %-10s    P%-4d %-04d\r\n",
							arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], s1, s, lport_id, vlan_id);
				}
			}

		}

		arl_idx++;
		
		/* Search operation is done */
		if((u8Data & 0x80) == 0)
			break;
    }

	cli_printf(pCliEnv, "\r\n");

	
	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM53286
	uint32	arl_idx;
	uint8	poll_cnt;
	uint8	u8Data;
	uint16	mem_addr;
	uint8	mac_addr[6];
	uint8	hport_id, lport_id;
	uint16	vlan_id;
	br_arl64_t	arl_data0, arl_data1, arl_key2;
	char	s[16];

	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  ====================================================\r\n");
	cli_printf(pCliEnv, "   No.  Mac Address          EntryState    Port  VID  \r\n");
	cli_printf(pCliEnv, "  ====================================================\r\n");
	
	/* Select ARL table */
	u8Data = 0x01;
	if(robo_write(BCM53286_PAGE_MEM_SEARCH, BCM53286_MEM_SEARCH_INDEX, &u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;

	/* Start search operation */
	u8Data = 0x85;
	if(robo_write(BCM53286_PAGE_MEM_SEARCH, BCM53286_MEM_SEARCH_CTRL, &u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;

	/* Wait until MEM_CTRL.MEM_STDN = 0 */
	arl_idx = 0;
    while (1) {
	    poll_cnt = 0;
	    while (poll_cnt++ < 20) {
			if(robo_read(BCM53286_PAGE_MEM_SEARCH, BCM53286_MEM_SEARCH_CTRL, &u8Data, 1) != 0)
				return HAL_SWIF_FAILURE;

			/* Arl is valid or search operation is done */
			if(((u8Data & 0x80) == 0) || ((u8Data & 0x40) == 0x40))
				break;
	    }

		if(poll_cnt == 20)
			return HAL_SWIF_FAILURE;

		/* Arl is valid */
		if((u8Data & 0x40) == 0x40) {
			if(robo_read(BCM53286_PAGE_MEM_SEARCH, BCM53286_MEM_SEARCH_ADDR, (uint8 *)&mem_addr, 2) != 0)
				return HAL_SWIF_FAILURE;
			
			if(robo_read(BCM53286_PAGE_MEM_SEARCH, BCM53286_MEM_SEARCH_DATA0, (uint8 *)&arl_data0, 8) != 0)
				return HAL_SWIF_FAILURE;
			
			mac_addr[0] = (uint8)((arl_data0.arl_hi32.bcm5328x_arl_hi_unicast._arl_mac_47_44 & 0x0000000F)<< 4) | (uint8)((arl_data0.arl_lo32.bcm5328x_arl_lo_mac._arl_mac_43_12 & 0xF0000000) >> 28);
			mac_addr[1] = (uint8)((arl_data0.arl_lo32.bcm5328x_arl_lo_mac._arl_mac_43_12 & 0x0FF00000) >> 20);
			mac_addr[2] = (uint8)((arl_data0.arl_lo32.bcm5328x_arl_lo_mac._arl_mac_43_12 & 0x000FF000) >> 12);
			mac_addr[3] = (uint8)((arl_data0.arl_lo32.bcm5328x_arl_lo_mac._arl_mac_43_12 & 0x00000FF0) >> 4);
			mac_addr[4] = (uint8)((arl_data0.arl_lo32.bcm5328x_arl_lo_mac._arl_mac_43_12 & 0x0000000F) << 4);
			hport_id = (uint8)(arl_data0.arl_hi32.bcm5328x_arl_hi_unicast._arl_pid);
			lport_id = hal_swif_hport_2_lport(hport_id);
			vlan_id = (uint16)(arl_data0.arl_hi32.bcm5328x_arl_hi_unicast._arl_vid);
			
			if(arl_data0.arl_hi32.bcm5328x_arl_hi_unicast._arl_static)
				sprintf(s,"%s","Uc-Static");
			else
				sprintf(s,"%s","Uc-Dynamic");
			
			if(robo_read(BCM53286_PAGE_MEM_SEARCH, BCM53286_MEM_SEARCH_DATA1, (uint8 *)&arl_data1, 8) != 0)
				return HAL_SWIF_FAILURE;
			
			if(robo_read(BCM53286_PAGE_MEM_SEARCH, BCM53286_MEM_SEARCH_KEY2, (uint8 *)&arl_key2, 8) != 0)
				return HAL_SWIF_FAILURE;

			mac_addr[4] |= (uint8)((arl_key2.arl_lo32.bcm5328x_arl_lo_mac._arl_mac_43_12 & 0x00000F00) >> 8);
			mac_addr[5] = (uint8)(arl_key2.arl_lo32.bcm5328x_arl_lo_mac._arl_mac_43_12 & 0x000000FF);

			if(hport_id == 24)
				cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)  %-10s    CPU   %-04d\r\n",
					arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], s, vlan_id);				
			else
				cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)  %-10s    P%-4d %-04d\r\n",
					arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], s, lport_id, vlan_id);
		}

		arl_idx++;
		
		/* Search operation is done */
		if((u8Data & 0x80) == 0)
			break;
    }

	cli_printf(pCliEnv, "\r\n");
	
	return HAL_SWIF_SUCCESS;
	
#elif SWITCH_CHIP_BCM5396
	uint32	arl_idx;
	uint8	poll_cnt;
	uint8	u8Data;
	uint16	mem_addr;
	uint8	mac_addr[6];
	uint8	hport_id, lport_id;
	uint16	vlan_id;
	br_arl64_t	arl_macvid_result1, arl_macvid_result0;
	br_arl32_t	arl_result1, arl_result0;
	char	s[16], s1[3];

	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  =====================================================\r\n");
	cli_printf(pCliEnv, "   No.  Mac Address            EntryState    Port  VID  \r\n");
	cli_printf(pCliEnv, "  =====================================================\r\n");
	
	/* Start search ARL entries */
	u8Data = 0x80;
	if(robo_write(BCM5396_PAGE_ARL_VLAN_TBL, BCM5396_ARL_SEARCH_CTRL, &u8Data, 1) != 0)
		return HAL_SWIF_FAILURE;

	/* Wait until MEM_CTRL.MEM_STDN = 0 */
	arl_idx = 0;
    while (1) {
	    poll_cnt = 0;
	    while (poll_cnt++ < 20) {
			if(robo_read(BCM5396_PAGE_ARL_VLAN_TBL, BCM5396_ARL_SEARCH_CTRL, &u8Data, 1) != 0)
				return HAL_SWIF_FAILURE;

			/* Arl is valid or search operation is done */
			if(((u8Data & 0x80) == 0) || ((u8Data & 0x01) == 0x01))
				break;
	    }

		if(poll_cnt == 20)
			return HAL_SWIF_FAILURE;

		/* Arl is valid */
		if((u8Data & 0x01) == 0x01) {
			if(robo_read(BCM5396_PAGE_ARL_VLAN_TBL, BCM5396_ARL_SEARCH_ADDR, (uint8 *)&mem_addr, 2) != 0)
				return HAL_SWIF_FAILURE;
			
			if(robo_read(BCM5396_PAGE_ARL_VLAN_TBL, BCM5396_ARL_SEARCH_MACVID_RESULT1, (uint8 *)&arl_macvid_result1, 8) != 0)
				return HAL_SWIF_FAILURE;

			if(robo_read(BCM5396_PAGE_ARL_VLAN_TBL, BCM5396_ARL_SEARCH_RESULT1, (uint8 *)&arl_result1, 4) != 0)
				return HAL_SWIF_FAILURE;
			
			if(robo_read(BCM5396_PAGE_ARL_VLAN_TBL, BCM5396_ARL_SEARCH_MACVID_RESULT0, (uint8 *)&arl_macvid_result0, 8) != 0)
				return HAL_SWIF_FAILURE;

			if(robo_read(BCM5396_PAGE_ARL_VLAN_TBL, BCM5396_ARL_SEARCH_RESULT0, (uint8 *)&arl_result0, 4) != 0)
				return HAL_SWIF_FAILURE;
			
			if(arl_result1.arl_data & 0x00000008) {
				mac_addr[0] = (uint8)((arl_macvid_result1.arl_hi32.bcm5396_arl_macvid_hi._arl_mac_47_32 & 0x0000FF00) >> 8);
				mac_addr[1] = (uint8)(arl_macvid_result1.arl_hi32.bcm5396_arl_macvid_hi._arl_mac_47_32 & 0x000000FF);
				mac_addr[2] = (uint8)((arl_macvid_result1.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0xFF000000) >> 24);
				mac_addr[3] = (uint8)((arl_macvid_result1.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x00FF0000) >> 16);
				mac_addr[4] = (uint8)((arl_macvid_result1.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x0000FF00) >> 8);
				mac_addr[5] = (uint8)(arl_macvid_result1.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x000000FF);
				vlan_id = (uint16)(arl_macvid_result1.arl_hi32.bcm5396_arl_macvid_hi._arl_vid);

				if(mac_addr[0] & 0x01) {	/* Multicast */
					//cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)*\r\n",arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);	
				} else {					/* Unicast */
					hport_id = (uint8)(arl_result1.bcm5396_arl_unicast._arl_portid);
					lport_id = hal_swif_hport_2_lport(hport_id);
					
					if(arl_result1.bcm5396_arl_unicast._arl_static)
						sprintf(s,"%s","Uc-Static");
					else
						sprintf(s,"%s","Uc-Dynamic");

					if(mac_addr[0] & 0x80)
						sprintf(s1,"%s","m1");
					else
						sprintf(s1,"%s","  ");
					
					if(hport_id == 16)
						cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)%s  %-10s    CPU   %-04d\r\n",
							arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], s1, s, vlan_id);				
					else
						cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)%s  %-10s    P%-4d %-04d\r\n",
							arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], s1, s, lport_id, vlan_id);
				}
			}

			if(arl_result0.arl_data & 0x00000008) {
				mac_addr[0] = (uint8)((arl_macvid_result0.arl_hi32.bcm5396_arl_macvid_hi._arl_mac_47_32 & 0x0000FF00) >> 8);
				mac_addr[1] = (uint8)(arl_macvid_result0.arl_hi32.bcm5396_arl_macvid_hi._arl_mac_47_32 & 0x000000FF);
				mac_addr[2] = (uint8)((arl_macvid_result0.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0xFF000000) >> 24);
				mac_addr[3] = (uint8)((arl_macvid_result0.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x00FF0000) >> 16);
				mac_addr[4] = (uint8)((arl_macvid_result0.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x0000FF00) >> 8);
				mac_addr[5] = (uint8)(arl_macvid_result0.arl_lo32.bcm5396_arl_macvid_lo._arl_mac_31_00 & 0x000000FF);
				vlan_id = (uint16)(arl_macvid_result0.arl_hi32.bcm5396_arl_macvid_hi._arl_vid);

				if(mac_addr[0] & 0x01) {	/* Multicast */
					//cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)\r\n",arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);				
				} else {					/* Unicast */
					hport_id = (uint8)(arl_result0.bcm5396_arl_unicast._arl_portid);
					lport_id = hal_swif_hport_2_lport(hport_id);
					
					if(arl_result0.bcm5396_arl_unicast._arl_static)
						sprintf(s,"%s","Uc-Static");
					else
						sprintf(s,"%s","Uc-Dynamic");

					if(mac_addr[0] & 0x80)
						sprintf(s1,"%s","m ");
					else
						sprintf(s1,"%s","  ");
					
					if(hport_id == 16)
						cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)%s  %-10s    CPU   %-04d\r\n",
							arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], s1, s, vlan_id);				
					else
						cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)%s  %-10s    P%-4d %-04d\r\n",
							arl_idx, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], s1, s, lport_id, vlan_id);
				}
			}			
		}

		arl_idx++;
		
		/* Search operation is done */
		if((u8Data & 0x80) == 0)
			break;
    }

	cli_printf(pCliEnv, "\r\n");
	
	return HAL_SWIF_SUCCESS;	
#endif
	
	
    return 0;
}

/**************************************************************************
  * @brief  port security initialize use configuration
  * @param  none
  * @retval none
  *************************************************************************/
int hal_swif_mac_security_conf_initialize(void)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;
	unsigned char i,j,hport, lport;
	port_security_conf_t cfg_security;
	port_security_record_conf_t port_security_rec;	
	unsigned short cfg_single_port;
	u32 port_list_vec, security_port_vec, lport_list, hwport_vec;
	int ret;
	 
	if(eeprom_read(NVRAM_PORT_SECURITY_CFG_BASE, (u8 *)&cfg_security, sizeof(port_security_conf_t)) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	if((cfg_security.PortNum > MAX_PORT_NUM) || (cfg_security.PortNum == 0x00))
		return CONF_ERR_NO_CFG;

	if((cfg_security.TotalRecordCount == 00) || (cfg_security.TotalRecordCount > MAX_PORT_SECURITY_RECORD_COUNT))
		return CONF_ERR_NO_CFG;	

	pSecurityInfos = pvPortMalloc(sizeof(port_security_info) * MAX_PORT_SECURITY_RECORD_COUNT);
	if(pSecurityInfos == NULL)
		return CONF_ERR_NO_CFG;	

	memset(pSecurityInfos, 0, sizeof(port_security_info) * MAX_PORT_SECURITY_RECORD_COUNT);
	
	security_port_vec = 0;
	for(i=0; i<cfg_security.PortNum; i++) {
		cfg_single_port = *(u16 *)&(cfg_security.SecurityConfig[i*2]);
		cfg_single_port = ntohs(cfg_single_port);
		hport = hal_swif_lport_2_hport(i+1);

		if(cfg_single_port & PSC_MASK_SECURITY_ENABLE) {
			if((ret = gprtSetForwardUnknown(dev,hport,GT_FALSE)) != GT_OK) {
        		printf("Error: gprtSetForwardUnknown, hport=%d, ret=%d\r\n", hport, ret);
				return CONF_ERR_MSAPI;
			}
			if((ret = gprtSetLockedPort(dev,hport,GT_TRUE)) != GT_OK) {
        		printf("Error: gprtSetLockedPort, hport=%d, ret=%d\r\n", hport, ret);
				return CONF_ERR_MSAPI;
			}
			if((ret = gprtSetDropOnLock(dev,hport,GT_TRUE)) != GT_OK) {
        		printf("Error: gprtSetDropOnLock, hport=%d, ret=%d\r\n", hport, ret);
				return CONF_ERR_MSAPI;
			}
			if((ret = gfdbRemovePort(dev,GT_FLUSH_ALL_UNBLK,hport)) != GT_OK) {
        		printf("Error: gfdbRemovePort, hport=%d, ret=%d\r\n", hport, ret);
				return CONF_ERR_MSAPI;
			}	
			security_port_vec |= 1<<i;
		}	
	}

	for(i=0; i<cfg_security.TotalRecordCount; i++) {
		if(eeprom_read(NVRAM_PORT_SECURITY_REC_CFG_BASE + i*sizeof(port_security_record_conf_t), (u8 *)&port_security_rec, sizeof(port_security_record_conf_t)) != I2C_SUCCESS)
			return CONF_ERR_I2C;

		port_list_vec = *(u32 *)&(port_security_rec.PortVec[0]);
		port_list_vec = ntohl(port_list_vec);
		hwport_vec = 0;
		for(j=0; j<cfg_security.PortNum; j++) {
			if(port_list_vec & (1<<j)) {
				lport = j+1;
				hport = hal_swif_lport_2_hport(j+1);
				hwport_vec |= 1<<hport;
			}
		}
		
		//pSecurityInfos[i].IsEmpty = HAL_TRUE;
		memcpy(pSecurityInfos[i].MacAddr, port_security_rec.MacAddr, 6);
		pSecurityInfos[i].PortVecCfg = hwport_vec;
		//pSecurityInfos[i].CurrPort = 0;		
	}

	PortSecurityEnable = HAL_TRUE;
	
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}


#if ((BOARD_GE22103MA || BOARD_GE_EXT_22002EA || BOARD_GE220044MD) && (BOARD_FEATURE & L2_MAC_FILTER))

unsigned short hal_swif_get_global(char regaddr,char offset,char len)
{
	GT_U16 data;
   
    if(hwGetGlobalRegField(dev,regaddr,offset,len,&data) != GT_OK)
        return 0;
    return data;
}

int hal_swif_atu_int_disable()  
{
	GT_U16 data;

	data = 0;
	if(eventSetActive(dev,data) != GT_OK)
		return GT_ERROR;
	return GT_OK;
}

int hal_swif_atu_int_enable()
{
	GT_U16 data;

	data = GT_ATU_PROB;
	if(eventSetActive(dev,data) != GT_OK)
		return GT_ERROR;
	return GT_OK;
}

int hal_swif_atu_clear_violation(GT_QD_DEV *dev, GT_ATU_OPERATION atuOp, GT_U8 *port, GT_U32 *intCause)
{
	GT_STATUS       retVal;
	GT_U16          data;
	GT_U16          opcodeData;
	GT_U8           i;
	GT_U16			portMask;

	portMask = (1 << dev->numOfPorts) - 1;	

	/* Wait until the ATU in ready. */
	data = 1;
	while(data == 1) {
		retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,15,1,&data);
		if(retVal != GT_OK) {
			return retVal;
		}
	}

	opcodeData = 0;    
	opcodeData |= ((1 << 15) | (atuOp << 12));
	retVal = hwWriteGlobalReg(dev,QD_REG_ATU_OPERATION,opcodeData);
	if(retVal != GT_OK) {
		return retVal;
	}

	/* get the Interrupt Cause */
	retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,4,3,&data);
	if(retVal != GT_OK) {
		return retVal;
	}

	switch (data) {
		case 4:	/* Member Violation */
		*intCause = GT_MEMBER_VIOLATION;
		break;
		case 2:	/* Miss Violation */
		*intCause = GT_MISS_VIOLATION;
		break;
		case 1:	/* Full Violation */
		*intCause = GT_FULL_VIOLATION;
		break;
		default:
		*intCause = 0;
		return GT_OK;
	}

	retVal = hwReadGlobalReg(dev,QD_REG_ATU_DATA_REG,&data);
	if(retVal != GT_OK) {
		return retVal;
	}

	*port = data & 0xF;

	return GT_OK;
}

int hal_swif_atu_get_violation(GT_QD_DEV *dev, GT_ATU_OPERATION atuOp, GT_ATU_ENTRY *entry)
{
	GT_STATUS       retVal;
	GT_U16          data=0;
	GT_U16          opcodeData;
	GT_U8           i;
	GT_U16			portMask;

	portMask = (1 << dev->numOfPorts) - 1;	

	/* Wait until the ATU in ready. */
	data = 1;
	while(data == 1) {
		retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,15,1,&data);
		if(retVal != GT_OK) {
			return retVal;
		}
	}

	/* If the operation is to service violation operation wait for the response   */
	if(atuOp == SERVICE_VIOLATIONS) {		
		entry->DBNum = 0;
		retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,12,3,&data);
		if((retVal != GT_OK)||(data != SERVICE_VIOLATIONS)) {
			return GT_FAIL;
		}

		/* Get the Mac address  */
		for(i = 0; i < 3; i++) {
			data = 0;
			retVal = hwReadGlobalReg(dev,(GT_U8)(QD_REG_ATU_MAC_BASE+i),&data);
			if(retVal != GT_OK) {
				return retVal;
			}
			entry->macAddr.arEther[2*i] = data >> 8;
			entry->macAddr.arEther[1 + 2*i] = data & 0xFF;
			hwWriteGlobalReg(dev,(GT_U8)(QD_REG_ATU_MAC_BASE+i),0);
		}
	} 

	return GT_OK;
}

int hal_swif_atu_add_mac(unsigned char hport, int i, GT_ATU_UC_STATE ucState)
{
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;

	memset(&macEntry,0,sizeof(GT_ATU_ENTRY));	
	macEntry.macAddr.arEther[0] = pSecurityInfos[i].MacAddr[0];
	macEntry.macAddr.arEther[1] = pSecurityInfos[i].MacAddr[1];
	macEntry.macAddr.arEther[2] = pSecurityInfos[i].MacAddr[2];
	macEntry.macAddr.arEther[3] = pSecurityInfos[i].MacAddr[3];
	macEntry.macAddr.arEther[4] = pSecurityInfos[i].MacAddr[4];
	macEntry.macAddr.arEther[5] = pSecurityInfos[i].MacAddr[5];
	macEntry.DBNum = 0;
	macEntry.portVec = (1 << hport);
	macEntry.prio = 0;
	macEntry.entryState.ucEntryState = ucState;

	if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK) {
		return HAL_SWIF_FAILURE;
	}

	return HAL_SWIF_SUCCESS;
}

void hal_swif_int_proc_task(void *arg)
{
	const unsigned char mac_all_00[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	GT_U16 intCause, phyIntCause=0;
	GT_U8 tempPort = 0;
	GT_U8 Lport = 0;
	GT_U32 tempintCause = 0;
	GT_ATU_ENTRY atuIntStatus;
	u32 SecurityPortVec = 0;
	int i;
	extern xSemaphoreHandle EXTI9_5_Semaphore;

	hal_swif_atu_int_enable();
	
	while(1) {
		if(EXTI9_5_Semaphore != NULL) {
			if(xSemaphoreTake(EXTI9_5_Semaphore, portMAX_DELAY) == pdTRUE) {
				/* for interrupt mode */
			}
		} else {
		/*********************************************************************************/
			intCause = 0;
			hwGetGlobalRegField(dev,0,0,7,&intCause);
			if(intCause & GT_ATU_PROB) {
				hal_swif_atu_int_disable();
				
				gtSemTake(dev,dev->atuRegsSem,0);
				if(hal_swif_atu_clear_violation(dev, SERVICE_VIOLATIONS,&tempPort,&tempintCause) != GT_OK) {
					hal_swif_atu_int_enable();
				 	gtSemGive(dev,dev->atuRegsSem);
					goto PhyInterrupt;
				}

				Lport = hal_swif_hport_2_lport(tempPort);
				#if 0
				if(tempintCause == GT_MEMBER_VIOLATION) {
					printf("Receive an ATU Member Violation interrupt, Port=%d\r\n", Lport);
				}

				if(tempintCause == GT_MISS_VIOLATION) {
					printf("Receive an Miss Violation interrupt, Port=%d\r\n", Lport);
				}
				#endif
				
				if((tempintCause != GT_MISS_VIOLATION) && (tempintCause != GT_MEMBER_VIOLATION)){
					gtSemGive(dev,dev->atuRegsSem);
					goto PhyInterrupt;
				}

				if(Lport == 0xFF) {
					hal_swif_atu_int_enable();
				 	gtSemGive(dev,dev->atuRegsSem);
					goto PhyInterrupt;
				}

				if(hal_swif_atu_get_violation(dev, SERVICE_VIOLATIONS, &atuIntStatus) != GT_OK) {
					hal_swif_atu_int_enable();
				 	gtSemGive(dev,dev->atuRegsSem);
					goto PhyInterrupt;
				 }

				#if 0
				printf("mac (%02x:%02x:%02x:%02x:%02x:%02x) \r\n", 
					atuIntStatus.macAddr.arEther[0],atuIntStatus.macAddr.arEther[1],atuIntStatus.macAddr.arEther[2],
					atuIntStatus.macAddr.arEther[3],atuIntStatus.macAddr.arEther[4],atuIntStatus.macAddr.arEther[5]);
				#endif
				
				gtSemGive(dev,dev->atuRegsSem);

				if(atuIntStatus.macAddr.arEther[0] & 0x01) {
					hal_swif_atu_int_enable();
					goto PhyInterrupt;
				}
				
				for(i=0; i<MAX_PORT_SECURITY_RECORD_COUNT; i++) {
					
					SecurityPortVec |= pSecurityInfos[i].PortVecCfg;
				}
									
				for(i=0; i<MAX_PORT_SECURITY_RECORD_COUNT; i++) {

					if(memcmp(pSecurityInfos[i].MacAddr, mac_all_00, 6) == 0) {
						hal_swif_atu_int_enable();
						goto PhyInterrupt;
					}
					
					if(memcmp(atuIntStatus.macAddr.arEther, pSecurityInfos[i].MacAddr, 6) == 0) {
						if(tempintCause == GT_MISS_VIOLATION) {
							if((1<<tempPort) & pSecurityInfos[i].PortVecCfg) {
								hal_swif_atu_add_mac(tempPort, i, GT_UC_STATIC);
							} 
						} 
						
						if(tempintCause == GT_MEMBER_VIOLATION) {
							if((1<<tempPort) & pSecurityInfos[i].PortVecCfg) {
								hal_swif_atu_add_mac(tempPort, i, GT_UC_STATIC);
							} else {
								if(((1<<tempPort) & SecurityPortVec) == 0) 
									hal_swif_atu_add_mac(tempPort, i, GT_UC_DYNAMIC);	
							}
						} 						
					}
				}
				
				hal_swif_atu_int_enable();
			}
			
PhyInterrupt:
			vTaskDelay(50);
		}
	}
}

void hal_interrupt_proc_entry(void)
{
	if(PortSecurityEnable == HAL_TRUE)
		xTaskCreate(hal_swif_int_proc_task, 	"tIntProc",	configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 4, NULL);
}


#endif /* (BOARD_GE22103MA && (BOARD_FEATURE & L2_MAC_FILTER)) */

#if MODULE_OBNMS

extern u8 NMS_TxBuffer[];
static obnet_maclist_stat_t MacListGetStat;
static obnet_record_get_stat_t SecurityRecGetStat;
static obnet_record_set_stat_t SecurityRecSetStat;

#if SWITCH_CHIP_88E6095	
static GT_ATU_ENTRY gMacEntry;
#endif

void nms_rsp_get_mac_list(u8 *DMA, u8 *RequestID, obnet_get_mac_list *pGetMacList)
{
	obnet_rsp_get_mac_list RspGetMacList;	
	u16 RspLength;
	port_security_conf_t port_security_cfg;
	obnet_mac_list_rec MacListRec[2];
	u32 hportVec, lportVec;
	u8 PortNum;
	u8 lport, hport;
	u8 bFirstRecFlag;
	int rec_index,j,loop;
	int stat;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetMacList, 0, sizeof(obnet_rsp_get_mac_list));
	memset(&MacListRec[0], 0, 2 * sizeof(obnet_mac_list_rec));
	RspGetMacList.GetCode = CODE_GET_MACLIST;

#if ((BOARD_FEATURE & L2_MAC_FILTER) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if(pGetMacList->OpCode == 0x00) {
		if(eeprom_read(NVRAM_PORT_CFG_BASE, &(MacListGetStat.PortNum), 1) != I2C_SUCCESS) {
			MacListGetStat.PortNum = MAX_PORT_NUM;
			goto ErrorGetMacList;
		} else {
			if((MacListGetStat.PortNum == 0) || (MacListGetStat.PortNum > MAX_PORT_NUM)) {
				MacListGetStat.PortNum = MAX_PORT_NUM;
				goto ErrorGetMacList;
			}
		}
		
		#if 0
		memset(&port_security_cfg, 0, sizeof(port_security_conf_t));
		if(eeprom_read(NVRAM_PORT_SECURITY_CFG_BASE, (u8 *)&(port_security_cfg.PortNum), sizeof(port_security_conf_t)) != I2C_SUCCESS) {
			MacListGetStat.PortNum = MAX_PORT_NUM;
			goto ErrorGetMacList;
		} else {
			if((port_security_cfg.PortNum == 0) || (port_security_cfg.PortNum > MAX_PORT_NUM) || \
				(port_security_cfg.TotalRecordCount > MAX_PORT_SECURITY_RECORD_COUNT) || (port_security_cfg.TotalRecordCount == 0))	{
				MacListGetStat.PortNum = MAX_PORT_NUM;
				goto ErrorGetMacList;		
			}
		}
		#endif
		
		MacListGetStat.PacketIndex = 1;
		MacListGetStat.RecSendCount = 0;
		MacListGetStat.LastMacRecFlag = 0;
		memset(&gMacEntry,0,sizeof(GT_ATU_ENTRY));
		bFirstRecFlag = 1;
	} else {
		bFirstRecFlag = 0;
	}

	loop = 2;
	rec_index=0;
	while(loop > 0) {
		if((stat = gfdbGetAtuEntryNext(dev,&gMacEntry)) != GT_OK) {
			MacListGetStat.LastMacRecFlag = 1;
			break;
		}
		
		if((gMacEntry.macAddr.arEther[0] == 0xff) && (gMacEntry.macAddr.arEther[1] == 0xff) && (gMacEntry.macAddr.arEther[2] == 0xff) && 
			(gMacEntry.macAddr.arEther[3] == 0xff) && (gMacEntry.macAddr.arEther[4] == 0xff) && (gMacEntry.macAddr.arEther[5] == 0xff)) {
			MacListGetStat.LastMacRecFlag = 1;
			break;
		}

		if((gMacEntry.macAddr.arEther[0] & 0x1) != 1) {	// unicast address
			if(gMacEntry.entryState.ucEntryState == GT_UC_DYNAMIC) {
				MacListRec[rec_index].Priority = gMacEntry.prio;
				memcpy(MacListRec[rec_index].MacAddr, gMacEntry.macAddr.arEther, 6);
				
				hportVec = gMacEntry.portVec;
				lportVec = 0;
				for(j=0; j<dev->numOfPorts; j++) {
					if((hportVec >> j) & 0x1) {
						if(j != dev->cpuPortNum) {
							lport = hal_swif_hport_2_lport(j);
							if(lport > MacListGetStat.PortNum)
								continue;
							else
								lportVec |= (1<<(lport-1));
						}
					}
				}
				
				MacListGetStat.RecSendCount++;
				MacListRec[rec_index].Index = MacListGetStat.RecSendCount;
				lportVec = htonl(lportVec);
				memcpy(&(MacListRec[rec_index].PortVec[0]), (u8 *)&lportVec, 4);
				
				rec_index++;
				loop--;
				if(MacListGetStat.RecSendCount == 64) {
					MacListGetStat.LastMacRecFlag = 1;
					break;
				}	
			}	
		}	
	}

	RspGetMacList.RetCode = 0x00;
	RspGetMacList.PortNum = MacListGetStat.PortNum;
	
	if(bFirstRecFlag) {
		bFirstRecFlag = 0;
		if(MacListGetStat.LastMacRecFlag == 1) {
			RspGetMacList.OpCode = (0x00 | SecurityRecGetStat.PacketIndex);
			RspGetMacList.RecordCount = 2-loop;
		} else {
			RspGetMacList.OpCode = (0x40 | SecurityRecGetStat.PacketIndex);
			RspGetMacList.RecordCount = 2;
		}
	} else {
		if(MacListGetStat.LastMacRecFlag == 1) {
			RspGetMacList.OpCode = (0x80 | SecurityRecGetStat.PacketIndex);
			RspGetMacList.RecordCount = 2-loop;
		} else {
			RspGetMacList.OpCode = (0xc0 | SecurityRecGetStat.PacketIndex);
			RspGetMacList.RecordCount = 2;
		}
	}

	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMacList, sizeof(obnet_rsp_get_mac_list));
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_mac_list)], (u8 *)&(MacListRec[0]), 2 * sizeof(obnet_mac_list_rec));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_mac_list) + 2 * sizeof(obnet_mac_list_rec);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
	
	MacListGetStat.PacketIndex++;	

	return;

ErrorGetMacList:
	RspGetMacList.GetCode = CODE_GET_MACLIST;
	RspGetMacList.RetCode = 0x01;
	RspGetMacList.PortNum = MacListGetStat.PortNum;
	RspGetMacList.OpCode = 0x00;
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMacList, sizeof(obnet_rsp_get_mac_list));
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_mac_list)], (u8 *)&(MacListRec[0]), 2 * sizeof(obnet_mac_list_rec));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_mac_list) + 2 * sizeof(obnet_mac_list_rec);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
#else

	RspGetMacList.GetCode = CODE_GET_MACLIST;
	RspGetMacList.RetCode = 0x01;
	RspGetMacList.PortNum = MacListGetStat.PortNum;
	RspGetMacList.OpCode = 0x00;	
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMacList, sizeof(obnet_rsp_get_mac_list));
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_mac_list)], (u8 *)&(MacListRec[0]), 2 * sizeof(obnet_mac_list_rec));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_mac_list) + 2 * sizeof(obnet_mac_list_rec);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);

#endif
}

void nms_rsp_set_port_security(u8 *DMA, u8 *RequestID, obnet_set_port_security *pPortSecurity)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	port_security_conf_t port_security_cfg;
	port_security_record_conf_t port_security_rec;
	obnet_port_security_rec *pPortSecurityRec = (obnet_port_security_rec *)((u8 *)pPortSecurity+sizeof(obnet_set_port_security));
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_PORT_SECURITY;

#if ((BOARD_FEATURE & L2_MAC_FILTER) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if((pPortSecurity->PortNum > MAX_PORT_NUM) || (pPortSecurity->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(((pPortSecurity->OpCode & 0xC0) >> 6) == 0x0) {
			SecurityRecSetStat.PacketIndex = 1;
			SecurityRecSetStat.RecordCount = pPortSecurity->RecordCount;
			SecurityRecSetStat.OffsetAddress = 0;
		} else if(((pPortSecurity->OpCode & 0xC0) >> 6) == 0x1) {
			SecurityRecSetStat.PacketIndex = 1;
			SecurityRecSetStat.RecordCount = pPortSecurity->RecordCount;
			SecurityRecSetStat.OffsetAddress = 0;
		} else if(((pPortSecurity->OpCode & 0xC0) >> 6) == 0x3) {
			if((pPortSecurity->OpCode & 0x3F) != SecurityRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				SecurityRecSetStat.PacketIndex++;
				SecurityRecSetStat.OffsetAddress = SecurityRecSetStat.RecordCount * sizeof(port_security_record_conf_t);
				SecurityRecSetStat.RecordCount += pPortSecurity->RecordCount;
			}
		} else if(((pPortSecurity->OpCode & 0xC0) >> 6) == 0x2) {
			if((pPortSecurity->OpCode & 0x3F) != SecurityRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				SecurityRecSetStat.PacketIndex++;
				SecurityRecSetStat.OffsetAddress = SecurityRecSetStat.RecordCount * sizeof(port_security_record_conf_t);
				SecurityRecSetStat.RecordCount += pPortSecurity->RecordCount;
			}
		} 

		/* Update the port security config */
		port_security_cfg.PortNum = pPortSecurity->PortNum;
		port_security_cfg.TotalRecordCount = SecurityRecSetStat.RecordCount;
		memcpy(port_security_cfg.SecurityConfig, pPortSecurity->SecurityConfig, 64);
		if(eeprom_page_write(NVRAM_PORT_SECURITY_CFG_BASE, (u8 *)&port_security_cfg, sizeof(port_security_conf_t)) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;
			goto Response;
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}
		
		/* Write the port security record configuration to EEPROM */
		if(eeprom_page_write(NVRAM_PORT_SECURITY_REC_CFG_BASE + SecurityRecSetStat.OffsetAddress, (u8 *)pPortSecurityRec, pPortSecurity->RecordCount * sizeof(port_security_record_conf_t)) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;
			goto Response;
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}
	}
#else
	RspSet.RetCode = 0x01;
	RspSet.Res = RSP_ERR_FEATURE_NOT_SUPPORT;
#endif

Response:
	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}

void nms_rsp_get_port_security(u8 *DMA, u8 *RequestID, obnet_get_port_security *pGetPortSecurity)
{
	obnet_rsp_get_port_security RspGetPortSecurity;	
	u16 RspLength;
	port_security_conf_t port_security_cfg;
	port_security_record_conf_t port_security_rec[2];	
	u8 bFirstRecFlag;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetPortSecurity, 0, sizeof(obnet_rsp_get_port_security));
	RspGetPortSecurity.GetCode = CODE_GET_PORT_SECURITY;
	
#if ((BOARD_FEATURE & L2_MAC_FILTER) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	memset(&port_security_cfg, 0, sizeof(port_security_conf_t));
	if(eeprom_read(NVRAM_PORT_SECURITY_CFG_BASE, (u8 *)&(port_security_cfg.PortNum), sizeof(port_security_conf_t)) != I2C_SUCCESS) {
		RspGetPortSecurity.RetCode = 0x01;
		goto error;
	} else {
		if((port_security_cfg.PortNum == 0) || (port_security_cfg.PortNum > MAX_PORT_NUM) || (port_security_cfg.TotalRecordCount > MAX_PORT_SECURITY_RECORD_COUNT)) {
			memset(&RspGetPortSecurity, 0, sizeof(obnet_rsp_get_port_security));
			RspGetPortSecurity.GetCode = CODE_GET_PORT_SECURITY;
			RspGetPortSecurity.RetCode = 0x01;
			goto error;
		} else {
			if(pGetPortSecurity->OpCode == 0x00) {
				SecurityRecGetStat.PacketIndex = 1;
				SecurityRecGetStat.RemainCount = port_security_cfg.TotalRecordCount;
				SecurityRecGetStat.OffsetAddress = 0;
				bFirstRecFlag = 1;
			} else {
				bFirstRecFlag = 0;
			}
			
			if(port_security_cfg.TotalRecordCount == 0) {
				RspGetPortSecurity.RetCode = 0x00;
				RspGetPortSecurity.PortNum = port_security_cfg.PortNum;
				RspGetPortSecurity.OpCode = 0x00;
				RspGetPortSecurity.RecordCount = 0x00;
				memcpy(RspGetPortSecurity.SecurityConfig, port_security_cfg.SecurityConfig, sizeof(port_security_conf_t)-2);
				
				memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortSecurity, sizeof(obnet_rsp_get_port_security));	
				RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_security);
				if (RspLength < MSG_MINSIZE)
					RspLength = MSG_MINSIZE;
				PrepareEtherHead(DMA);
				PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);				
				if(RspLength == MSG_MINSIZE) {
					RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
				} else {
					RspSend(NMS_TxBuffer, RspLength);	
				}				

			} else if((port_security_cfg.TotalRecordCount > 0) && (port_security_cfg.TotalRecordCount < 3)) {
				RspGetPortSecurity.RetCode = 0x00;
				RspGetPortSecurity.PortNum = port_security_cfg.PortNum;
				RspGetPortSecurity.OpCode = 0x00;
				RspGetPortSecurity.RecordCount = port_security_cfg.TotalRecordCount;
				memcpy(RspGetPortSecurity.SecurityConfig, port_security_cfg.SecurityConfig, sizeof(port_security_conf_t)-2);

				if(eeprom_read(NVRAM_PORT_SECURITY_REC_CFG_BASE, (u8 *)&(port_security_rec[0]), port_security_cfg.TotalRecordCount * sizeof(port_security_record_conf_t)) != I2C_SUCCESS) {
					goto error;
				} else {
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortSecurity, sizeof(obnet_rsp_get_port_security));
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_security)], (u8 *)&(port_security_rec[0]), port_security_cfg.TotalRecordCount * sizeof(port_security_record_conf_t));
					RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_security) + port_security_cfg.TotalRecordCount * sizeof(port_security_record_conf_t);
					if (RspLength < MSG_MINSIZE)
						RspLength = MSG_MINSIZE;
					PrepareEtherHead(DMA);
					PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
					if(RspLength == MSG_MINSIZE)
						RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
					else
						RspSend(NMS_TxBuffer, RspLength);
				}
			} else {
				if(SecurityRecGetStat.RemainCount >= 2) {
					if(eeprom_read(NVRAM_PORT_SECURITY_REC_CFG_BASE + SecurityRecGetStat.OffsetAddress, (u8 *)&(port_security_rec[0]), 2 * sizeof(port_security_record_conf_t)) != I2C_SUCCESS) {
						goto error;
					} else {
						RspGetPortSecurity.GetCode = CODE_GET_PORT_SECURITY;
						RspGetPortSecurity.RetCode = 0x00;
						RspGetPortSecurity.PortNum = port_security_cfg.PortNum;
						if(bFirstRecFlag) {
							bFirstRecFlag = 0;
							RspGetPortSecurity.OpCode = (0x40 | SecurityRecGetStat.PacketIndex);
						} else {
							if(SecurityRecGetStat.RemainCount == 2)
								RspGetPortSecurity.OpCode = (0x80 | SecurityRecGetStat.PacketIndex);
							else
								RspGetPortSecurity.OpCode = (0xc0 | SecurityRecGetStat.PacketIndex);
						}
						RspGetPortSecurity.RecordCount = 0x02;
						memcpy(RspGetPortSecurity.SecurityConfig, port_security_cfg.SecurityConfig, sizeof(port_security_conf_t)-2);

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortSecurity, sizeof(obnet_rsp_get_port_security));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_security)], (u8 *)&(port_security_rec[0]), 2 * sizeof(port_security_record_conf_t));
						RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_security) + 2 * sizeof(port_security_record_conf_t);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						
						SecurityRecGetStat.RemainCount -= 2;
						SecurityRecGetStat.OffsetAddress += 2 * sizeof(port_security_record_conf_t);
						SecurityRecGetStat.PacketIndex++;
					}
				} else {
					if(eeprom_read(NVRAM_PORT_SECURITY_REC_CFG_BASE + SecurityRecGetStat.OffsetAddress, (u8 *)&(port_security_rec[0]), SecurityRecGetStat.RemainCount * sizeof(port_security_record_conf_t)) != I2C_SUCCESS) {
						goto error;
					} else {
						RspGetPortSecurity.GetCode = CODE_GET_PORT_SECURITY;
						RspGetPortSecurity.RetCode = 0x00;
						RspGetPortSecurity.PortNum = port_security_cfg.PortNum;
						RspGetPortSecurity.OpCode = (0x80 | SecurityRecGetStat.PacketIndex);
						RspGetPortSecurity.RecordCount = SecurityRecGetStat.RemainCount;
						memcpy(RspGetPortSecurity.SecurityConfig, port_security_cfg.SecurityConfig, sizeof(port_security_conf_t)-2);
						
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortSecurity, sizeof(obnet_rsp_get_port_security));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_security)], (u8 *)&(port_security_rec[0]), SecurityRecGetStat.RemainCount * sizeof(port_security_record_conf_t));
						RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_security) + SecurityRecGetStat.RemainCount * sizeof(port_security_record_conf_t);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						
						SecurityRecGetStat.RemainCount = 0;
						SecurityRecGetStat.OffsetAddress = 0;
					}
				}
			}
		}
	}
	return;
#else
	RspGetPortSecurity.RetCode = 0x01;
#endif

error:
	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortSecurity, sizeof(obnet_rsp_get_port_security));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_security);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}
#endif


