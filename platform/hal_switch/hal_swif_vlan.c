
/*******************************************************************
 * Filename     : hal_swif_vlan.c
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
#include "stm32f2x7_smi.h"
#include "soft_i2c.h"
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
#include "hal_swif_vlan.h"

/* Other includes */
#include "cli_util.h"

#if MARVELL_SWITCH
extern GT_QD_DEV *dev;
#endif

static HAL_VLAN_MODE VlanMode = VLAN_MODE_PORT_BASED;

/**************************************************************************
  * @brief  Set port vlan member, every port-vlan member include cpu port
  *         max member number is 32.
  * @param  
  * @retval 
  *************************************************************************/
int hal_swif_port_vlan_create(uint32 member_mask)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_LPORT memPorts[MAX_PORT_NUM+1];
	GT_U8 memPortsLen;
	GT_U8 hport;
	int i, j, k;

	memset(memPorts, 0, (MAX_PORT_NUM+1) * sizeof(GT_LPORT));
	k=0;

	/* For CPU port */
	for(j=1; j<=MAX_PORT_NUM; j++) {
		if(member_mask & (1<<(j-1))) {
			hport = hal_swif_lport_2_hport(j);
			memPorts[k] = hport;
			k++;
		}
	}
	memPortsLen = (GT_U8)(k&0xff);
	
	if((status = gvlnSetPortVlanPorts(dev, dev->cpuPortNum, memPorts, memPortsLen)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	/* For Other ports */
	for(i=1; i<=MAX_PORT_NUM; i++) {
		k=0;
		if(member_mask & (1<<(i-1))) {
			for(j=1; j<=MAX_PORT_NUM; j++) {
				if(j == i) {
					continue;
				}
				if(member_mask & (1<<(j-1))) {
					hport = hal_swif_lport_2_hport(j);
					memPorts[k] = hport;
					k++;
				}
			}
			memPorts[k] = dev->cpuPortNum;
			k++;
			memPortsLen = k;
			if((status = gvlnSetPortVlanPorts(dev, hport, memPorts, memPortsLen)) != GT_OK) {
				return HAL_SWIF_ERR_MSAPI;
			}
		}
	}
	
	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}


/**************************************************************************
  * @brief  Create 802.1Q VLAN
  * @param  vlan_id, lpbm, lubm
  * @retval 
  *************************************************************************/
int hal_swif_8021q_vlan_create(uint16 vlan_id, uint32 lpbm, uint32 lubm)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_VTU_ENTRY vtuEntry;
	GT_U16 vid;
	GT_U8 hport;
	int i;
	
	gtMemSet(&vtuEntry,0,sizeof(GT_VTU_ENTRY));
	vtuEntry.DBNum = 0;
	vtuEntry.vid = vlan_id;	

	for(i=1; i<=MAX_PORT_NUM; i++) {
		if(lpbm & (1<<(i-1))) {
			hport = hal_swif_lport_2_hport(i);
			if(lubm & (1<<(i-1)))
				vtuEntry.vtuData.memberTagP[hport] = MEMBER_EGRESS_UNTAGGED;
			else
				vtuEntry.vtuData.memberTagP[hport] = MEMBER_EGRESS_TAGGED;
		} else {
			vtuEntry.vtuData.memberTagP[hport] = NOT_A_MEMBER;
		}
	}
	vtuEntry.vtuData.memberTagP[dev->cpuPortNum] = MEMBER_EGRESS_UNTAGGED;

	if((status = gvtuAddEntry(dev,&vtuEntry)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	if(vlan_id == 1) {
		/* 1) Clear VLAN ID Table */
		if((status = gvtuFlush(dev)) != GT_OK) {
			return HAL_SWIF_ERR_MSAPI;	
		}

		/* 2) Enable 802.1Q for each port except CPU port as GT_SECURE mode */
		for(i=1; i<=MAX_PORT_NUM; i++) {
			hport = hal_swif_lport_2_hport(i);
			if((status = gvlnSetPortVlanDot1qMode(dev, hport, GT_SECURE)) != GT_OK) {
				return HAL_SWIF_ERR_MSAPI;
			}
		}

		/* 3) Enable 802.1Q for CPU port as GT_FALLBACK mode */
		if((status = gvlnSetPortVlanDot1qMode(dev, dev->cpuPortNum, GT_FALLBACK)) != GT_OK) {
			return HAL_SWIF_ERR_MSAPI;	
		}		
	}
	
	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}

/**************************************************************************
  * @brief  Add untag port
  * @param  vlan_id, lport
  * @return 0: SUCCESS -1:FAILURE
  *************************************************************************/
int hal_swif_8021q_vlan_add_untag_port(uint16 vlan_id, uint8 lport)
{
#if SWITCH_CHIP_88E6095
	int i;
	GT_STATUS status;
	GT_VTU_ENTRY vtuEntry;	
	GT_VTU_ENTRY tmpVtuEntry;
	GT_VTU_ENTRY pvidEntry;
	GT_BOOL found = GT_FALSE;
	GT_U8 hport;

	if(lport > MAX_PORT_NUM)
	return HAL_SWIF_ERR_INVALID_LPORT;

	/* Search VLAN ID vid entry */
	gtMemSet(&tmpVtuEntry, 0, sizeof(GT_VTU_ENTRY));
	tmpVtuEntry.vid = vlan_id;
	tmpVtuEntry.DBNum = 0;	
	if((status = gvtuFindVidEntry(dev,&tmpVtuEntry,&found)) != GT_OK) {
		return HAL_SWIF_FAILURE;
	}

	/* Set port as tag member of VLAN ID */
	gtMemSet(&vtuEntry,0,sizeof(GT_VTU_ENTRY));
	vtuEntry.DBNum = 0;
	vtuEntry.vid = vlan_id;
	for(i=0; i<dev->numOfPorts; i++) {		
		if(found == GT_TRUE)
			vtuEntry.vtuData.memberTagP[i] = tmpVtuEntry.vtuData.memberTagP[i];
		else			
			vtuEntry.vtuData.memberTagP[i] = NOT_A_MEMBER;
	}
	hport = hal_swif_lport_2_hport(lport);
	vtuEntry.vtuData.memberTagP[hport] = MEMBER_EGRESS_UNTAGGED;
	if((status = gvtuAddEntry(dev,&vtuEntry)) != GT_OK) {
		return HAL_SWIF_FAILURE;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}

/**************************************************************************
  * @brief  Add tag port
  * @param  vlan_id, lport
  * @return 0: SUCCESS -1:FAILURE
  *************************************************************************/
int hal_swif_8021q_vlan_add_tag_port(uint16 vlan_id, uint8 lport)
{
#if SWITCH_CHIP_88E6095
	int i;
	GT_STATUS status;
	GT_VTU_ENTRY vtuEntry;	
	GT_VTU_ENTRY tmpVtuEntry;
	GT_VTU_ENTRY pvidEntry;
	GT_BOOL found = GT_FALSE;
	GT_U8 hport;

	if(lport > MAX_PORT_NUM)
	return HAL_SWIF_ERR_INVALID_LPORT;

	/* Search VLAN ID vid entry */
	gtMemSet(&tmpVtuEntry, 0, sizeof(GT_VTU_ENTRY));
	tmpVtuEntry.vid = vlan_id;
	tmpVtuEntry.DBNum = 0;	
	if((status = gvtuFindVidEntry(dev,&tmpVtuEntry,&found)) != GT_OK) {
		return HAL_SWIF_FAILURE;
	}

	/* Set port as tag member of VLAN ID */
	gtMemSet(&vtuEntry,0,sizeof(GT_VTU_ENTRY));
	vtuEntry.DBNum = 0;
	vtuEntry.vid = vlan_id;
	for(i=0; i<dev->numOfPorts; i++) {		
		if(found == GT_TRUE)
			vtuEntry.vtuData.memberTagP[i] = tmpVtuEntry.vtuData.memberTagP[i];
		else			
			vtuEntry.vtuData.memberTagP[i] = NOT_A_MEMBER;
	}
	hport = hal_swif_lport_2_hport(lport);
	vtuEntry.vtuData.memberTagP[hport] = MEMBER_EGRESS_TAGGED;
	if((status = gvtuAddEntry(dev,&vtuEntry)) != GT_OK) {
		return HAL_SWIF_FAILURE;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}

/**************************************************************************
  * @brief  Set pvid for logic port bit map
  * @param  lpbm, pvid
  * @return 0: SUCCESS -1:FAILURE
  *************************************************************************/
int hal_swif_8021q_vlan_set_pvid(uint32 lpbm, u16 pvid)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_U8 hport;
	int i;

	for(i=1; i<=MAX_PORT_NUM; i++) {
		if(lpbm & (1<<(i-1))) {
			hport = hal_swif_lport_2_hport(i);
			if((status = gvlnSetPortVid(dev, hport, pvid)) != GT_OK) {
				return HAL_SWIF_ERR_MSAPI;	
			}
		} 
	}
	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}

/**************************************************************************
  * @brief  Initialize configuration for port isolation
  * @param  none
  * @retval none
  *************************************************************************/
int hal_swif_port_isolation_conf_initialize(void)
{
#if SWITCH_CHIP_88E6095
	u8 i, j, k, hport, hport_tmp;
	u16 single_port_isolation_cfg;
	u16 single_port_member_ports;
	GT_LPORT memPorts[MAX_PORT_NUM+1];
	GT_U8 memPortsLen;
	hal_port_isolation_conf_t PortIsolationConfig;
		
	if(eeprom_read(NVRAM_PORT_ISOLATION_CFG_BASE, &(PortIsolationConfig.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;

	if((PortIsolationConfig.PortNum == 0) || (PortIsolationConfig.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;

	for(i=0; i<PortIsolationConfig.PortNum; i++) {
		single_port_isolation_cfg = *(u16 *)&(PortIsolationConfig.VlanMap[i*2]);
		single_port_isolation_cfg = ntohs(single_port_isolation_cfg);
		hport = hal_swif_lport_2_hport(i+1);

		if(single_port_isolation_cfg & PORT_ISOLATION_MASK_LIMIT_VID_SWITCH) {
			/* TO ADD ? */
		}
		
		single_port_member_ports = single_port_isolation_cfg & PORT_ISOLATION_MASK_PORTS_MEMBER;
		memset(memPorts, 0, (MAX_PORT_NUM+1) * sizeof(GT_LPORT));
		k=0;
		for(j=0; j<PortIsolationConfig.PortNum; j++) {
			if(j == i)
				continue;
			if(single_port_member_ports & (1<<j)) {
				hport_tmp = hal_swif_lport_2_hport(j+1);
				memPorts[k] = GT_PORT_2_LPORT(hport_tmp);
				k++;
			}
		}
		memPorts[k] = dev->cpuPortNum;
		k++;		
		memPortsLen = k;
		
		if(gvlnSetPortVlanPorts(dev, GT_PORT_2_LPORT(hport), memPorts, memPortsLen) != GT_OK) {
			return CONF_ERR_MSAPI;
		}		

		#if 0
		if((ret = gvlnGetPortVlanPorts(dev,GT_PORT_2_LPORT(hport),memPorts,&memPortsLen)) != GT_OK) {
        	printf("Error: gvlnGetPortVlanPorts, ret=%d\r\n", ret);
			return CONF_ERR_MSAPI;
		}
	    printf("PortMember: %d,%d,%d,%d,%d,%d,%d\r\n",  
			GT_LPORT_2_PORT(memPorts[0]), GT_LPORT_2_PORT(memPorts[1]), GT_LPORT_2_PORT(memPorts[2]), 
			GT_LPORT_2_PORT(memPorts[3]), GT_LPORT_2_PORT(memPorts[4]), GT_LPORT_2_PORT(memPorts[5]), GT_LPORT_2_PORT(memPorts[6]));
		#endif
	}
	
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}

/**************************************************************************
  * @brief  Initialize configuration for pvid
  * @param  none
  * @retval none
  *************************************************************************/
int hal_swif_pvid_conf_initialize(void)
{
#if SWITCH_CHIP_88E6095
	int ret;
	u8 i, hport;
	u16 single_port_isolation_cfg;
	u16 pvid, regval;
	u16 ingress_filter_rule;
	hal_port_isolation_conf_t PortIsolationConfig;
	
	if(eeprom_read(NVRAM_PORT_VLAN_CFG_BASE, &(PortIsolationConfig.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;

	if((PortIsolationConfig.PortNum == 0) || (PortIsolationConfig.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;
	
	for(i=0; i<PortIsolationConfig.PortNum; i++) {
		single_port_isolation_cfg = *(u16 *)&(PortIsolationConfig.VlanMap[i*2]);
		single_port_isolation_cfg = ntohs(single_port_isolation_cfg);
		hport = hal_swif_lport_2_hport(i+1);

		pvid = single_port_isolation_cfg & PVID_MASK_VID_NUMBER;
		ingress_filter_rule = (single_port_isolation_cfg & PVID_MASK_INGRESS_FILTER_RULE) >> 13;

		/* refer to datasheet page 185 */
		if((pvid<1) || (pvid>4094)) {
			printf("Error: invalid pvid number %d\r\n", pvid);
			return CONF_ERR_MSAPI;
		}
		
		if((ret = gvlnSetPortVid(dev, hport, pvid)) != GT_OK) {
        	printf("Error: gvlnSetPortVid, ret=%d\r\n", ret);
			return CONF_ERR_MSAPI;	
		}	
		
		if(single_port_isolation_cfg & PVID_MASK_VID_FORCE) {
			if((ret = gvlnSetPortVlanForceDefaultVID(dev, hport, GT_TRUE)) != GT_OK) {
            	printf("Error: gvlnGetPortVlanForceDefaultVID, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;	
			}
		}
		
		/* refer to datasheet page 187, 802.1Q mode */
		switch(ingress_filter_rule) {
			case VLAN_INGRESS_RULE_NOT_CHECK:
			if((ret = gvlnSetPortVlanDot1qMode(dev, hport, GT_DISABLE)) != GT_OK) {
            	printf("Error: gvlnSetPortVlanDot1qMode, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;	
			}	
			VlanMode = VLAN_MODE_PORT_BASED;
			break;

			case VLAN_INGRESS_RULE_PRIO_CHECK:
			if((ret = gvlnSetPortVlanDot1qMode(dev, hport, GT_FALLBACK)) != GT_OK) {
            	printf("Error: gvlnSetPortVlanDot1qMode, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;	
			}	
			VlanMode = VLAN_MODE_8021Q;
			break;

			case VLAN_INGRESS_RULE_CHECK:
			if((ret = gvlnSetPortVlanDot1qMode(dev, hport, GT_CHECK)) != GT_OK) {
            	printf("Error: gvlnSetPortVlanDot1qMode, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;	
			}
			VlanMode = VLAN_MODE_8021Q;
			break;

			case VLAN_INGRESS_RULE_SECURE:
			if((ret = gvlnSetPortVlanDot1qMode(dev, hport, GT_SECURE)) != GT_OK) {
            	printf("Error: gvlnSetPortVlanDot1qMode, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;	
			}
			VlanMode = VLAN_MODE_8021Q;
			break;

			default:
			break;
		}	
	}
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}

/**************************************************************************
  * @brief  Initialize configuration for 8021.q VLAN
  * @param  none
  * @retval none
  *************************************************************************/
int hal_swif_8021q_vlan_conf_initialize(void)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_VTU_ENTRY vtuEntry;
	GT_U16 vid;
	GT_U8 hport;
	hal_8021q_vlan_conf_t vlan_cfg;
	hal_8021q_vlan_record_t vlan_rec;
	GT_U8 vlan_setup[32];
	int i,j,x,y;


	if(VlanMode != VLAN_MODE_8021Q)
		return CONF_ERR_NO_CFG;
	
	if(eeprom_read(NVRAM_VLAN_CFG_BASE, (u8 *)&vlan_cfg, sizeof(hal_8021q_vlan_conf_t)) != I2C_SUCCESS) {
		return CONF_ERR_I2C;
	}

	if((vlan_cfg.PortNum == 0) || (vlan_cfg.PortNum > MAX_PORT_NUM) || (vlan_cfg.TotalRecordCount > MAX_8021Q_VLAN_RECORD_COUNT) || (vlan_cfg.TotalRecordCount == 0)) {
		return CONF_ERR_NO_CFG;
	}

	
	/* 1) Clear VLAN ID Table */
	if((status = gvtuFlush(dev)) != GT_OK) {
		printf("gvtuFlush returned failed\r\n");
		return CONF_ERR_MSAPI;
	}

	/* 2) Setup 802.1Q mode for each port except CPU port. Refer to function hal_swif_pvid_conf_initialize() */

	/* 3) Enable 802.1Q for CPU port as GT_FALLBACK mode */
	if((status = gvlnSetPortVlanDot1qMode(dev, dev->cpuPortNum, GT_FALLBACK)) != GT_OK) {
		printf("gvlnSetPortVlanDot1qMode return Failed, port=%d, ret=%d\r\n", dev->cpuPortNum, status);
		return CONF_ERR_MSAPI;
	}
#if 0
	/* 3) Configure the default vid for CPU port as 0xfff */
	if((status = gvlnSetPortVid(dev,dev->cpuPortNum, 0xfff)) != GT_OK) {
		printf("gvlnSetPortVid returned fail.\n");
		return status;
	}
#endif
	/* 4) Add VLAN ID and add vlan members */
	for(i=0; i<vlan_cfg.TotalRecordCount; i++) {
		if(eeprom_read(NVRAM_VLAN_RECORD_CFG_BASE + i * sizeof(hal_8021q_vlan_record_t), (u8 *)&vlan_rec, sizeof(hal_8021q_vlan_record_t)) != I2C_SUCCESS) {
			return CONF_ERR_I2C;
		}
		vid = *(GT_U16 *)&(vlan_rec.VLanID[0]);
		vid = ntohs(vid);
		
		y=0;
		for(j=0; j<=7; j++) {
			for(x=0; x<4; x++) {
				vlan_setup[y] = (vlan_rec.VLanSetting[j] >> x*2) & 0x3;
				y++;
			}
		}
        
		#if 0
        printf("vid=%d, vlan_setup: ",vid);
        for(j=0; j<vlan_cfg.PortNum; j++) {
            printf("%d ",vlan_setup[j]);
        }
        printf("\r\n");
        #endif
		
		gtMemSet(&vtuEntry,0,sizeof(GT_VTU_ENTRY));
		vtuEntry.DBNum = 0;
		vtuEntry.vid = vid;			
		for(j=0; j<vlan_cfg.PortNum; j++) {
			hport = hal_swif_lport_2_hport(j+1);
			vtuEntry.vtuData.memberTagP[hport] = vlan_setup[j];
		}
		vtuEntry.vtuData.memberTagP[dev->cpuPortNum] = MEMBER_EGRESS_UNTAGGED;

		if((status = gvtuAddEntry(dev,&vtuEntry)) != GT_OK) {
			printf("gvtuAddEntry return Failed\n");
			return CONF_ERR_MSAPI;
		}
	
	}
	
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}

/**************************************************************************
  * OBNet NMS
  *************************************************************************/
#if MODULE_OBNMS
extern u8 NMS_TxBuffer[];
static obnet_record_set_stat_t VlanRecSetStat;
static obnet_record_get_stat_t VlanRecGetStat;

void nms_rsp_set_port_isolation(u8 *DMA, u8 *RequestID, obnet_set_port_isolation *pPortIsolation)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_ISOLATION;

#if ((BOARD_FEATURE & L2_PORT_VLAN) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if((pPortIsolation->PortNum > MAX_PORT_NUM) || (pPortIsolation->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(eeprom_page_write(NVRAM_PORT_ISOLATION_CFG_BASE, (u8 *)&(pPortIsolation->PortNum), pPortIsolation->PortNum * 2 + 1) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}
	}
#else
	RspSet.RetCode = 0x01;
	RspSet.Res = RSP_ERR_FEATURE_NOT_SUPPORT;
#endif

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

void nms_rsp_get_port_isolation(u8 *DMA, u8 *RequestID)
{
	obnet_rsp_get_port_isolation RspGetPortIsolation;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_isolation);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetPortIsolation, 0, sizeof(obnet_rsp_get_port_isolation));
	RspGetPortIsolation.GetCode = CODE_GET_ISOLATION;

#if ((BOARD_FEATURE & L2_PORT_VLAN) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if(eeprom_read(NVRAM_PORT_ISOLATION_CFG_BASE, (u8 *)&(RspGetPortIsolation.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) {
		RspGetPortIsolation.RetCode = 0x01;
	} else {
		if((RspGetPortIsolation.PortNum == 0) || (RspGetPortIsolation.PortNum > MAX_PORT_NUM)) {
			memset(&RspGetPortIsolation, 0, sizeof(obnet_rsp_get_port_isolation));
			RspGetPortIsolation.GetCode = CODE_GET_ISOLATION;
			RspGetPortIsolation.RetCode = 0x01;
		} else
			RspGetPortIsolation.RetCode = 0x00;
	}
#else
	RspGetPortIsolation.RetCode = 0x01;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortIsolation, sizeof(obnet_rsp_get_port_isolation));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_isolation);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}

void nms_rsp_set_port_vlan(u8 *DMA, u8 *RequestID, obnet_set_port_vlan *pSetPortVlan)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_PORT_VLAN;

#if ((BOARD_FEATURE & L2_8021Q_VLAN) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if((pSetPortVlan->PortNum > MAX_PORT_NUM) || (pSetPortVlan->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(eeprom_page_write(NVRAM_PORT_VLAN_CFG_BASE, (u8 *)&(pSetPortVlan->PortNum), pSetPortVlan->PortNum * 2 + 1) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}	
	}
#else
	RspSet.RetCode = 0x01;
	RspSet.Res = RSP_ERR_FEATURE_NOT_SUPPORT;
#endif

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

void nms_rsp_get_port_vlan(u8 *DMA, u8 *RequestID)
{
	obnet_rsp_get_port_vlan RspGetPortVlan;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetPortVlan, 0, sizeof(obnet_rsp_get_port_vlan));
	RspGetPortVlan.GetCode = CODE_GET_PORT_VLAN;

#if ((BOARD_FEATURE & L2_8021Q_VLAN) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if(eeprom_read(NVRAM_PORT_VLAN_CFG_BASE, &(RspGetPortVlan.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) {
		RspGetPortVlan.RetCode = 0x01;
	} else {
		if((RspGetPortVlan.PortNum == 0) || (RspGetPortVlan.PortNum > MAX_PORT_NUM)) {
			memset(&RspGetPortVlan, 0, sizeof(obnet_rsp_get_port_vlan));
			RspGetPortVlan.GetCode = CODE_GET_PORT_VLAN;
			RspGetPortVlan.RetCode = 0x01;
		} else
			RspGetPortVlan.RetCode = 0x00;
	}
#else
	RspGetPortVlan.RetCode = 0x01;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortVlan, sizeof(obnet_rsp_get_port_vlan));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_vlan);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}

void nms_rsp_set_adm_vlan(u8 *DMA, u8 *RequestID, obnet_set_adm_vlan *pSetAdmVlan)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_ADM_VLAN;

#if ((BOARD_FEATURE & L2_8021Q_VLAN) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if(eeprom_page_write(NVRAM_ADM_VLAN_CFG_BASE, (u8 *)&(pSetAdmVlan->VLanID[0]), 2) != I2C_SUCCESS) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
	} else {
		RspSet.RetCode = 0x00;
		RspSet.Res = 0x00;
	}	
#else
	RspSet.RetCode = 0x01;
	RspSet.Res = RSP_ERR_FEATURE_NOT_SUPPORT;
#endif

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

void nms_rsp_get_adm_vlan(u8 *DMA, u8 *RequestID)
{
	obnet_rsp_get_adm_vlan RspGetAdmVlan;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetAdmVlan, 0, sizeof(obnet_rsp_get_adm_vlan));
	RspGetAdmVlan.GetCode = CODE_GET_ADM_VLAN;

#if ((BOARD_FEATURE & L2_8021Q_VLAN) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if(eeprom_read(NVRAM_ADM_VLAN_CFG_BASE, (u8 *)&(RspGetAdmVlan.VLanID[0]), 2) != I2C_SUCCESS) {
		RspGetAdmVlan.RetCode = 0x01;
	} else {
		if(ntohs(*(u16 *)&(RspGetAdmVlan.VLanID[0])) > 4095)
			RspGetAdmVlan.RetCode = 0x01;
		else
			RspGetAdmVlan.RetCode = 0x00;
	}
#else
	RspGetAdmVlan.RetCode = 0x01;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetAdmVlan, sizeof(obnet_rsp_get_adm_vlan));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_adm_vlan);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}

void nms_rsp_set_vlan(u8 *DMA, u8 *RequestID, obnet_set_vlan *pSetVlan)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	hal_8021q_vlan_conf_t vlan_cfg;
	hal_8021q_vlan_record_t vlan_rec;
	obnet_vlan_rec *pVlanRec = (obnet_vlan_rec *)((u8 *)pSetVlan+sizeof(obnet_set_vlan));
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_VLAN;

#if ((BOARD_FEATURE & L2_8021Q_VLAN) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if((pSetVlan->PortNum > MAX_PORT_NUM) || (pSetVlan->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(((pSetVlan->OpCode & 0xC0) >> 6) == 0x0) {
			VlanRecSetStat.PacketIndex = 1;
			VlanRecSetStat.RecordCount = pSetVlan->RecordCount;
			VlanRecSetStat.OffsetAddress = 0;
		} else if(((pSetVlan->OpCode & 0xC0) >> 6) == 0x1) {
			VlanRecSetStat.PacketIndex = 1;
			VlanRecSetStat.RecordCount = pSetVlan->RecordCount;
			VlanRecSetStat.OffsetAddress = 0;
		} else if(((pSetVlan->OpCode & 0xC0) >> 6) == 0x3) {
			if((pSetVlan->OpCode & 0x3F) != VlanRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				VlanRecSetStat.PacketIndex++;
				VlanRecSetStat.OffsetAddress = VlanRecSetStat.RecordCount * sizeof(hal_8021q_vlan_record_t);
				VlanRecSetStat.RecordCount += pSetVlan->RecordCount;
			}
		} else if(((pSetVlan->OpCode & 0xC0) >> 6) == 0x2) {
			if((pSetVlan->OpCode & 0x3F) != VlanRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				VlanRecSetStat.PacketIndex++;
				VlanRecSetStat.OffsetAddress = VlanRecSetStat.RecordCount * sizeof(hal_8021q_vlan_record_t);
				VlanRecSetStat.RecordCount += pSetVlan->RecordCount;
			}
		} 

		/* Update the vlan config */
		vlan_cfg.PortNum = pSetVlan->PortNum;
		vlan_cfg.TotalRecordCount = VlanRecSetStat.RecordCount;
		if(eeprom_page_write(NVRAM_VLAN_CFG_BASE, (u8 *)&vlan_cfg, sizeof(hal_8021q_vlan_conf_t)) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;
			goto Response;
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}

		/* Write the vlan record configuration to EEPROM */
		if(eeprom_page_write(NVRAM_VLAN_RECORD_CFG_BASE + VlanRecSetStat.OffsetAddress, (u8 *)pVlanRec, pSetVlan->RecordCount * sizeof(hal_8021q_vlan_record_t)) != I2C_SUCCESS) {
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

void nms_rsp_get_vlan(u8 *DMA, u8 *RequestID, obnet_get_vlan *pGetVlan)
{
	obnet_rsp_get_vlan RspGetVlan;	
	u16 RspLength;
	hal_8021q_vlan_conf_t vlan_cfg;
	hal_8021q_vlan_record_t vlan_rec[2];	
	u8 bFirstRecFlag;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetVlan, 0, sizeof(obnet_rsp_get_vlan));
	RspGetVlan.GetCode = CODE_GET_VLAN;
	
#if ((BOARD_FEATURE & L2_8021Q_VLAN) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if(eeprom_read(NVRAM_VLAN_CFG_BASE, (u8 *)&vlan_cfg, sizeof(hal_8021q_vlan_conf_t)) != I2C_SUCCESS) {
		goto ErrorVlan;
	} else {
		if((vlan_cfg.PortNum == 0) || (vlan_cfg.PortNum > MAX_PORT_NUM) || (vlan_cfg.TotalRecordCount > MAX_8021Q_VLAN_RECORD_COUNT)) {
			goto ErrorVlan;
		} else {
			if(pGetVlan->OpCode == 0x00) {
				VlanRecGetStat.PacketIndex = 1;
				VlanRecGetStat.RemainCount = vlan_cfg.TotalRecordCount;
				VlanRecGetStat.OffsetAddress = 0;
				bFirstRecFlag = 1;
			} else {
				bFirstRecFlag = 0;
			}
			
			if(vlan_cfg.TotalRecordCount == 0) {
				RspGetVlan.RetCode = 0x00;
				RspGetVlan.PortNum = vlan_cfg.PortNum;
				RspGetVlan.OpCode = 0x00;
				RspGetVlan.RecordCount = 0x00;

				memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetVlan, sizeof(obnet_rsp_get_vlan));
				RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_vlan);
				if (RspLength < MSG_MINSIZE)
					RspLength = MSG_MINSIZE;
				PrepareEtherHead(DMA);
				PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
				if(RspLength == MSG_MINSIZE)
					RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
				else
					RspSend(NMS_TxBuffer, RspLength);				

			} else if((vlan_cfg.TotalRecordCount > 0) && (vlan_cfg.TotalRecordCount < 3)) {
				RspGetVlan.RetCode = 0x00;
				RspGetVlan.PortNum = vlan_cfg.PortNum;
				RspGetVlan.OpCode = 0x00;
				RspGetVlan.RecordCount = vlan_cfg.TotalRecordCount;

				if(eeprom_read(NVRAM_VLAN_RECORD_CFG_BASE, (u8 *)&(vlan_rec[0]), vlan_cfg.TotalRecordCount * sizeof(hal_8021q_vlan_record_t)) != I2C_SUCCESS) {
					goto ErrorVlan;
				} else {
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetVlan, sizeof(obnet_rsp_get_vlan));
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_vlan)], (u8 *)&(vlan_rec[0]), vlan_cfg.TotalRecordCount * sizeof(hal_8021q_vlan_record_t));
					RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_vlan) + vlan_cfg.TotalRecordCount * sizeof(hal_8021q_vlan_record_t);
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
				if(VlanRecGetStat.RemainCount >= 2) {
					if(eeprom_read(NVRAM_VLAN_RECORD_CFG_BASE + VlanRecGetStat.OffsetAddress, (u8 *)&(vlan_rec[0]), 2 * sizeof(hal_8021q_vlan_record_t)) != I2C_SUCCESS) {
						goto ErrorVlan;
					} else {
						RspGetVlan.GetCode = CODE_GET_VLAN;
						RspGetVlan.RetCode = 0x00;
						RspGetVlan.PortNum = vlan_cfg.PortNum;
						if(bFirstRecFlag) {
							bFirstRecFlag = 0;
							RspGetVlan.OpCode = (0x40 | VlanRecGetStat.PacketIndex);
						} else {
							if(VlanRecGetStat.RemainCount == 2)
								RspGetVlan.OpCode = (0x80 | VlanRecGetStat.PacketIndex);
							else
								RspGetVlan.OpCode = (0xc0 | VlanRecGetStat.PacketIndex);
						}
						RspGetVlan.RecordCount = 0x02;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetVlan, sizeof(obnet_rsp_get_vlan));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_vlan)], (u8 *)&(vlan_rec[0]), 2 * sizeof(hal_8021q_vlan_record_t));
						RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_vlan) + 2 * sizeof(hal_8021q_vlan_record_t);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						VlanRecGetStat.RemainCount -= 2;
						VlanRecGetStat.OffsetAddress += 2 * sizeof(hal_8021q_vlan_record_t);
						VlanRecGetStat.PacketIndex++;
					}
				} else {
					if(eeprom_read(NVRAM_VLAN_RECORD_CFG_BASE + VlanRecGetStat.OffsetAddress, (u8 *)&(vlan_rec[0]), VlanRecGetStat.RemainCount * sizeof(hal_8021q_vlan_record_t)) != I2C_SUCCESS) {
						goto ErrorVlan;
					} else {
						RspGetVlan.GetCode = CODE_GET_VLAN;
						RspGetVlan.RetCode = 0x00;
						RspGetVlan.PortNum = vlan_cfg.PortNum;
						RspGetVlan.OpCode = (0x80 | VlanRecGetStat.PacketIndex);
						RspGetVlan.RecordCount = VlanRecGetStat.RemainCount;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetVlan, sizeof(obnet_rsp_get_vlan));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_vlan)], (u8 *)&(vlan_rec[0]), VlanRecGetStat.RemainCount * sizeof(hal_8021q_vlan_record_t));
						RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_vlan) + VlanRecGetStat.RemainCount * sizeof(hal_8021q_vlan_record_t);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						VlanRecGetStat.RemainCount = 0;
						VlanRecGetStat.OffsetAddress = 0;
					}
				}
			}
		}
	}

	return;
#endif

ErrorVlan:
	memset(&RspGetVlan, 0, sizeof(obnet_rsp_get_vlan));
	RspGetVlan.GetCode = CODE_GET_VLAN;
	RspGetVlan.RetCode = 0x01;
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetVlan, sizeof(obnet_rsp_get_vlan));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_vlan);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}

#endif /* MODULE_OBNMS */


