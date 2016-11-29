

/*****************************************************************
 * Filename     : hal_swif_aggregation.c
 * Description  : Hardware Abstraction Layer for L2 
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *****************************************************************/
#include "mconfig.h"

/* Standard includes */
#include <stdio.h>

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
#include "hal_swif_aggregation.h"

/* Other includes */
#include "cli_util.h"

#if MARVELL_SWITCH
extern GT_QD_DEV *dev;
#endif

aggr_info_t PortAggrInfo;

/**************************************************************************
  * @brief  find aggregation group
  * @param  lport
  * @retval 
  *************************************************************************/
u8 hal_swif_aggr_find_group(u8 lport)
{
	u8 aggr_group, i;

	for(i=0; i<=PortAggrInfo.AggrCount; i++) {
		if((PortAggrInfo.AggrGroupRec[i].PortMask & (1<<(lport-1))) != 0)
			return PortAggrInfo.AggrGroupRec[i].AggrId;
	}

	return 0xFF;
}

/**************************************************************************
  * @brief  port aggregation add members
  * @param  aggr_id, lport
  * @retval none
  *************************************************************************/
int hal_swif_aggr_add_member(u8 aggr_id, u8 lport)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_U32 trunk_mask, mask;
	u8	aggr_port_array[MAX_AGGREGATION_GROUP_PORT_NUM];
	GT_U32 hwport_vec;
	u8 hport;
	int j, k, index, aggr_port_num, aggr_reg_idx;

	if((PortAggrInfo.AggrCount == 0) || (PortAggrInfo.AggrCount > MAX_AGGREGATION_RECORD_COUNT))
		return HAL_SWIF_FAILURE;
	
	hport = hal_swif_lport_2_hport(lport);
	for(aggr_reg_idx=0; aggr_reg_idx<PortAggrInfo.AggrCount; aggr_reg_idx++) {
		if(PortAggrInfo.AggrGroupRec[aggr_reg_idx].AggrId == aggr_id)
			break;
	}
	if(aggr_reg_idx == PortAggrInfo.AggrCount)
		return HAL_SWIF_FAILURE;
	/* Get Trunk Route Table for the given Trunk ID */
	if((status = gsysGetTrunkRouting(dev, aggr_id, (GT_U32 *)&hwport_vec)) != GT_OK) {
		return CONF_ERR_MSAPI;
	}

	hwport_vec |= (1 << hport);

	/* Enable trunk on the given port */
	if((status = gprtSetTrunkPort(dev, hport, GT_TRUE, aggr_id)) != GT_OK) {
		return CONF_ERR_MSAPI;
	}

	
	/* Set Trunk Route Table for the given Trunk ID */
	if((status = gsysSetTrunkRouting(dev, aggr_id, hwport_vec)) != GT_OK) {
		return CONF_ERR_MSAPI;
	}

	memset(aggr_port_array, 0, MAX_AGGREGATION_GROUP_PORT_NUM);
	index = 0;
	for(j=0; j<dev->numOfPorts; j++) {
		if(hwport_vec & (1<<j)) {
			aggr_port_array[index] = j;
			index++;
			if(index == MAX_AGGREGATION_GROUP_PORT_NUM)
				break;					
		}
	}

	aggr_port_num = index;
	
	/* Set Trunk Mask Table for load balancing. */
	trunk_mask = 0x7FF;
	trunk_mask &= ~hwport_vec;
	for(k=0; k<8; k++) {
		index = k % aggr_port_num;
		mask = trunk_mask | (1 << aggr_port_array[index]);
		if((status = gsysSetTrunkMaskTable(dev, k, mask)) != GT_OK) {
			return CONF_ERR_MSAPI;
		}
	}

	hal_swif_port_set_stp_state(lport, FORWARDING);
	
	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53286
	//GT_STATUS status;
	u32 trunk_mask, mask;
	u8	aggr_port_array[MAX_AGGREGATION_GROUP_PORT_NUM];
	u32 hwport_vec;
	u8 hport;
	u32 u32Data;
	int j, k, index, aggr_port_num, aggr_reg_idx;

	if((PortAggrInfo.AggrCount == 0) || (PortAggrInfo.AggrCount > MAX_AGGREGATION_RECORD_COUNT))
		return HAL_SWIF_FAILURE;
	
	hport = hal_swif_lport_2_hport(lport);
	for(aggr_reg_idx=0; aggr_reg_idx<PortAggrInfo.AggrCount; aggr_reg_idx++) {

		if(PortAggrInfo.AggrGroupRec[aggr_reg_idx].AggrId == aggr_id)
			break;
	}
	if(aggr_reg_idx == PortAggrInfo.AggrCount)
		return HAL_SWIF_FAILURE;
	/* Get Trunk_Port_Map */
	if(robo_read(0x31, 0x10 + (aggr_id - 1) * 8, (u8 *)&hwport_vec, 4) != 0) 
		return HAL_SWIF_ERR_SPI_RW;
	
	hwport_vec |= (1 << hport);

	/* Add Trunk Port to Trunk_Port_Map */
	u32Data = 1<<hport;
	if(robo_write(0x31, 0x10 + (aggr_id - 1) * 8, (u8 *)&u32Data, 4) != 0) 
			return HAL_SWIF_ERR_SPI_RW;

	if(robo_write(0x31, 0x10 + (aggr_id - 1) * 8, (u8 *)&hwport_vec, 4) != 0) 
		return HAL_SWIF_ERR_SPI_RW;

	hal_swif_port_set_stp_state(lport, FORWARDING);
	
	return HAL_SWIF_SUCCESS;	
	
#else
	return HAL_SWIF_FAILURE;
#endif
}

/**************************************************************************
  * @brief  port aggregation del members
  * @param  none
  * @retval none
  *************************************************************************/
int hal_swif_aggr_del_member(u8 aggr_id, u8 lport)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_U32 trunk_mask, mask;
	u8	aggr_port_array[MAX_AGGREGATION_GROUP_PORT_NUM];
	GT_U32 hwport_vec;
	u8 hport;
	int j, k, index, aggr_port_num, aggr_reg_idx;

	if((PortAggrInfo.AggrCount == 0) || (PortAggrInfo.AggrCount > MAX_AGGREGATION_RECORD_COUNT))
		return HAL_SWIF_FAILURE;
	
	hport = hal_swif_lport_2_hport(lport);
	for(aggr_reg_idx=0; aggr_reg_idx<PortAggrInfo.AggrCount; aggr_reg_idx++) {
		if(PortAggrInfo.AggrGroupRec[aggr_reg_idx].AggrId == aggr_id)
			break;
	}
	if(aggr_reg_idx == PortAggrInfo.AggrCount)
		return HAL_SWIF_FAILURE;
	
	/* Get Trunk Route Table for the given Trunk ID */
	if((status = gsysGetTrunkRouting(dev, aggr_id, (GT_U32 *)&hwport_vec)) != GT_OK) {
		return CONF_ERR_MSAPI;
	}

	hwport_vec &= ~(1 << hport);	
	
	/* Disable trunk on the given port */
	if((status = gprtSetTrunkPort(dev, hport, GT_FALSE, aggr_id)) != GT_OK) {
		return CONF_ERR_MSAPI;
	}
	
	/* Set Trunk Route Table for the given Trunk ID */
	if((status = gsysSetTrunkRouting(dev, aggr_id, hwport_vec)) != GT_OK) {
		return CONF_ERR_MSAPI;
	}

	aggr_port_num = PortAggrInfo.AggrGroupRec[aggr_reg_idx].PortNumber - 1;
	memset(aggr_port_array, 0, MAX_AGGREGATION_GROUP_PORT_NUM);
	index = 0;
	for(j=0; j<dev->numOfPorts; j++) {
		if(hwport_vec & (1<<j)) {
			aggr_port_array[index] = j;
			index++;
			if(index == MAX_AGGREGATION_GROUP_PORT_NUM)
				break;					
		}
	}

	/* Set Trunk Mask Table for load balancing. */
	trunk_mask = 0x7FF;
	trunk_mask &= ~hwport_vec;
	for(k=0; k<8; k++) {
		index = k % aggr_port_num;
		mask = trunk_mask | (1 << aggr_port_array[index]);
		if((status = gsysSetTrunkMaskTable(dev, k, mask)) != GT_OK) {
			return CONF_ERR_MSAPI;
		}
	}

	hal_swif_port_set_stp_state(lport, BLOCKING);

	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53286
	u32 trunk_mask, mask;
	u8	aggr_port_array[MAX_AGGREGATION_GROUP_PORT_NUM];
	u32 hwport_vec;
	u8 hport;
	int j, k, index, aggr_port_num, aggr_reg_idx;

	if((PortAggrInfo.AggrCount == 0) || (PortAggrInfo.AggrCount > MAX_AGGREGATION_RECORD_COUNT))
		return HAL_SWIF_FAILURE;
	
	hport = hal_swif_lport_2_hport(lport);
	for(aggr_reg_idx=0; aggr_reg_idx<PortAggrInfo.AggrCount; aggr_reg_idx++) {
		if(PortAggrInfo.AggrGroupRec[aggr_reg_idx].AggrId == aggr_id)
			break;
	}
	if(aggr_reg_idx == PortAggrInfo.AggrCount)
		return HAL_SWIF_FAILURE;
	
	/* Get Trunk_Port_Map, Trunk Group (aggr_id - 1) */
	if(robo_read(0x31, 0x10 + (aggr_id - 1) * 8, (u8 *)&hwport_vec, 4) != 0) 
		return HAL_SWIF_ERR_SPI_RW;

	hwport_vec &= ~(1 << hport);
	
	/* Delete Trunk Prt from Trunk_Port_Map, Trunk Group (aggr_id - 1) */
	if(robo_write(0x31, 0x10 + (aggr_id - 1) * 8, (u8 *)&hwport_vec, 4) != 0) 
		return HAL_SWIF_ERR_SPI_RW;
	
	hal_swif_port_set_stp_state(lport, BLOCKING);

	return HAL_SWIF_SUCCESS;

#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  aggregation group member port link state change process
  * @param  lport, new_state
  * @retval 
  *************************************************************************/
int hal_swif_aggr_link_changed(u8 lport, HAL_PORT_LINK_STATE new_state)
{
	u8 aggr_id,flag;
	int ret;

	if((aggr_id = hal_swif_aggr_find_group(lport)) == 0xFF)
		return HAL_SWIF_FAILURE;

	switch(new_state) {
		case LINK_DOWN:
		ret = hal_swif_aggr_del_member(aggr_id, lport);
		break;

		default: /* LINK_UP */
		ret = hal_swif_aggr_add_member(aggr_id, lport);
		break;
	}
	
	return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  show aggregation member logic port status
  * @param  
  * @retval 
  *************************************************************************/
int hal_swif_aggr_show_status(void *pCliEnv)
{
#if SWITCH_CHIP_88E6095
	int i, j, k;
	uint8	lport, hport;
	u8	AggrId;
	uint8	portType;
    u16     data;
	HAL_PORT_LINK_STATE linkState;
	HAL_PORT_DUPLEX_STATE duplexState;
	HAL_PORT_SPEED_STATE speedState;
	HAL_PORT_STP_STATE stpState;
	char	port_member_str[36];
	GT_STATUS status;
	GT_U32 hwport_vec, trunk_mask;

	if((PortAggrInfo.AggrCount == 0) || (PortAggrInfo.AggrCount > MAX_AGGREGATION_RECORD_COUNT))
		return HAL_SWIF_FAILURE;
	
	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  Port Aggregation Status Information Display :\r\n");
	cli_printf(pCliEnv, "  ====================================================\r\n");
	cli_printf(pCliEnv, "  AggrID      Port-Member                             \r\n");
	cli_printf(pCliEnv, "  ====================================================\r\n");

	memset(port_member_str, 0, 36);
	
	for(i=0; i<PortAggrInfo.AggrCount; i++) {
		AggrId = PortAggrInfo.AggrGroupRec[i].AggrId;
		/* Get Trunk Route Table for the given Trunk ID */
		if((status = gsysGetTrunkRouting(dev, AggrId, (GT_U32 *)&hwport_vec)) != GT_OK) {
			return CONF_ERR_MSAPI;
		}
 
		if(PortAggrInfo.AggrGroupRec[i].PortNumber > 8) {
			cli_printf(pCliEnv, "Error: Port number error for aggregator id %d\r\n", AggrId);
			return HAL_SWIF_FAILURE;
		}
		j=0;
		for(k=0; k<dev->maxPorts; k++) {
			if(hwport_vec & (1<<k)) {
				hport = k;
				lport = hal_swif_hport_2_lport(hport);
				if(j>8)
					break;
				sprintf(&port_member_str[j*4],"P%02d ", lport);
				j++;
			}
		}
		
		cli_printf(pCliEnv, "    %02d        %s\r\n", AggrId, port_member_str);
	}
	cli_printf(pCliEnv, "\r\n");

	cli_printf(pCliEnv, "  Trunk Mask Table Display :\r\n");
	cli_printf(pCliEnv, "  ====================================================\r\n");
	for(i=0; i<8; i++) {
		if((status = gsysGetTrunkMaskTable(dev, i, &trunk_mask)) != GT_OK) {
			return CONF_ERR_MSAPI;
		}
		cli_printf(pCliEnv, "  TrunkMask[%d]   ...   0x%04X\r\n", i, trunk_mask);
	}
	cli_printf(pCliEnv, "  ====================================================\r\n");
	cli_printf(pCliEnv, "\r\n");
	
	return HAL_SWIF_SUCCESS;

#elif SWITCH_CHIP_BCM53286
	int i, group_port, k;
	uint8	lport, hport;
	u8	AggrId;
	uint8	portType;
    u16     data;
	HAL_PORT_LINK_STATE linkState;
	HAL_PORT_DUPLEX_STATE duplexState;
	HAL_PORT_SPEED_STATE speedState;
	HAL_PORT_STP_STATE stpState;
	char	port_member_str[36];
	u32 hwport_vec, trunk_mask,mask;
	int index, aggr_port_num;
	u8	aggr_port_array[MAX_AGGREGATION_GROUP_PORT_NUM];

	if((PortAggrInfo.AggrCount == 0) || (PortAggrInfo.AggrCount > MAX_AGGREGATION_RECORD_COUNT))
		return HAL_SWIF_FAILURE;
	
	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  Port Aggregation Status Information Display :\r\n");
	cli_printf(pCliEnv, "  ====================================================\r\n");
	cli_printf(pCliEnv, "  AggrID      Port-Member                             \r\n");
	cli_printf(pCliEnv, "  ====================================================\r\n");

	memset(port_member_str, 0, 36);
	
	for(i=0; i<PortAggrInfo.AggrCount; i++) {
		AggrId = PortAggrInfo.AggrGroupRec[i].AggrId;
		
		/* Get Trunk_Port_Map */
		if(robo_read(0x31, 0x10 + i * 8, (u8 *)&hwport_vec, 4) != 0) 
			return HAL_SWIF_ERR_SPI_RW;
 
		if(PortAggrInfo.AggrGroupRec[i].PortNumber > 8) {
			cli_printf(pCliEnv, "Error: Port number error for aggregator id %d\r\n", AggrId);
			return HAL_SWIF_FAILURE;
		}
		group_port=0;
		index = 0;
		memset(aggr_port_array, 0, MAX_AGGREGATION_GROUP_PORT_NUM);
		for(k=0; k<MAX_PORT_NUM; k++) {
			if(hwport_vec & (1<<k)) {				
				aggr_port_array[index] = k;
				index++;				
				hport = k;
				lport = hal_swif_hport_2_lport(hport);
				if(group_port>8)
					break;
				sprintf(&port_member_str[group_port*4],"P%02d ", lport);
				group_port++;
			}
		}
		
		cli_printf(pCliEnv, "    %02d        %s\r\n", AggrId, port_member_str);
	}
	cli_printf(pCliEnv, "\r\n");

	cli_printf(pCliEnv, "  Trunk Mask Table Display :\r\n");
	cli_printf(pCliEnv, "  ====================================================\r\n");
	aggr_port_num = group_port;
	for(i=0; i<8; i++) {		
		trunk_mask = 0xFFFFFFFF;
		trunk_mask &= ~hwport_vec;
		index = i % aggr_port_num;
		mask = trunk_mask | (1 << aggr_port_array[index]);
		
		cli_printf(pCliEnv, "  TrunkMask[%d]   ...   0x%08X\r\n", i, mask);
	}
	cli_printf(pCliEnv, "  ====================================================\r\n");
	cli_printf(pCliEnv, "\r\n");
	
	return HAL_SWIF_SUCCESS;

#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  port aggregation initialize use configuration
  * @param  none
  * @retval none
  *************************************************************************/
int hal_swif_aggr_conf_initialize(void)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_U32 AggrId, trunk_mask, mask;
	link_aggregation_conf_t aggr_cfg;
	link_aggregation_rec aggr_rec;
	u8	aggr_port_array[MAX_AGGREGATION_GROUP_PORT_NUM];
	u32 port_list_vec;
	u32 hwport_vec;
	u8 hport;
	int i,j,k;
	int index, aggr_port_num;

	memset(&PortAggrInfo, 0, sizeof(aggr_info_t));
	
	if(eeprom_read(NVRAM_PORT_TRUNK_CFG_BASE, (u8 *)&aggr_cfg, sizeof(link_aggregation_conf_t)) != I2C_SUCCESS) {
		return CONF_ERR_I2C;
	}

	if((aggr_cfg.PortNum == 0) || (aggr_cfg.PortNum > MAX_PORT_NUM) || \
		(aggr_cfg.TotalRecordCount > MAX_AGGREGATION_RECORD_COUNT) || (aggr_cfg.TotalRecordCount == 0)) {
		return CONF_ERR_NO_CFG;
	}

	for(i=0; i<aggr_cfg.TotalRecordCount; i++) {
		if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + i * sizeof(link_aggregation_rec), (u8 *)&aggr_rec, sizeof(link_aggregation_rec)) != I2C_SUCCESS) {
			return CONF_ERR_I2C;
		}

		AggrId = aggr_rec.AggrId;
		if((AggrId > MAX_AGGREGATION_GROUP_ID) || (AggrId == 0))
			continue;

		PortAggrInfo.AggrCount++;
		PortAggrInfo.AggrGroupRec[PortAggrInfo.AggrCount-1].AggrId = AggrId;
		port_list_vec = *(u32 *)&(aggr_rec.MemberVec[0]);
		port_list_vec = ntohl(port_list_vec);
		PortAggrInfo.AggrGroupRec[PortAggrInfo.AggrCount-1].PortMask = port_list_vec;
		hwport_vec = 0;
		aggr_port_num = 0;
		for(j=0; j<aggr_cfg.PortNum; j++) {
			if(port_list_vec & (1<<j)) {
				hport = hal_swif_lport_2_hport(j+1);
				hwport_vec |= 1<<hport;
				
				/* enabled trunk on the given port */
				if((status = gprtSetTrunkPort(dev, hport, GT_TRUE, AggrId)) != GT_OK) {
					return CONF_ERR_MSAPI;
				}
				
				aggr_port_num++;
				if(aggr_port_num == MAX_AGGREGATION_GROUP_PORT_NUM)
					break;				
			}
		}
		PortAggrInfo.AggrGroupRec[PortAggrInfo.AggrCount-1].PortNumber = aggr_port_num;
		#if 0
		/* Set Trunk Route Table for the given Trunk ID */
		if((status = gsysSetTrunkRouting(dev, AggrId, hwport_vec)) != GT_OK) {
			return CONF_ERR_MSAPI;
		}
		#endif
		memset(aggr_port_array, 0, MAX_AGGREGATION_GROUP_PORT_NUM);
		index = 0;
		for(j=0; j<dev->numOfPorts; j++) {
			if(hwport_vec & (1<<j)) {
				aggr_port_array[index] = j;
				index++;
				if(index == MAX_AGGREGATION_GROUP_PORT_NUM)
					break;					
			}
		}

		/* Set Trunk Mask Table for load balancing. */
		trunk_mask = 0x7FF;
		trunk_mask &= ~hwport_vec;
		for(k=0; k<8; k++) {
			index = k % aggr_port_num;
			mask = trunk_mask | (1 << aggr_port_array[index]);
			#if 0
			if((status = gsysSetTrunkMaskTable(dev, k, mask)) != GT_OK) {
				return CONF_ERR_MSAPI;
			}
			#endif
		}
		
	}
	
	return CONF_ERR_NONE;

#elif SWITCH_CHIP_BCM53286
	u32 AggrId, trunk_mask, mask;
	link_aggregation_conf_t aggr_cfg;
	link_aggregation_rec aggr_rec;
	u8	aggr_port_array[MAX_AGGREGATION_GROUP_PORT_NUM];
	u32 port_list_vec;
	u32 hwport_vec;
	u8 hport,u8Data;
	int i,j,k;
	int index, aggr_port_num;

	memset(&PortAggrInfo, 0, sizeof(aggr_info_t));

	/* refer page351 */
	/* Set Trunk_Seed MAC_DA/SA Trunk hash ,En_Trunk */
	u8Data = 0x03<<6 | 0x02;
	if(robo_write(0x31, 0x00, (u8 *)&u8Data, 1) != 0) 
		return HAL_SWIF_ERR_SPI_RW;
	
	if(eeprom_read(NVRAM_PORT_TRUNK_CFG_BASE, (u8 *)&aggr_cfg, sizeof(link_aggregation_conf_t)) != I2C_SUCCESS) {
		return CONF_ERR_I2C;
	}

	if((aggr_cfg.PortNum == 0) || (aggr_cfg.PortNum > MAX_PORT_NUM) || \
		(aggr_cfg.TotalRecordCount > MAX_AGGREGATION_RECORD_COUNT) || (aggr_cfg.TotalRecordCount == 0)) {
		return CONF_ERR_NO_CFG;
	}

	for(i=0; i<aggr_cfg.TotalRecordCount; i++) {
		if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + i * sizeof(link_aggregation_rec), (u8 *)&aggr_rec, sizeof(link_aggregation_rec)) != I2C_SUCCESS) {
			return CONF_ERR_I2C;
		}

		AggrId = aggr_rec.AggrId;
		if((AggrId > MAX_AGGREGATION_GROUP_ID) || (AggrId == 0))
			continue;

		PortAggrInfo.AggrCount++;
		PortAggrInfo.AggrGroupRec[PortAggrInfo.AggrCount-1].AggrId = AggrId;
		port_list_vec = *(u32 *)&(aggr_rec.MemberVec[0]);
		port_list_vec = ntohl(port_list_vec);
		PortAggrInfo.AggrGroupRec[PortAggrInfo.AggrCount-1].PortMask = port_list_vec;
		hwport_vec = 0;
		aggr_port_num = 0;
		for(j=0; j<aggr_cfg.PortNum; j++) {
			if(port_list_vec & (1<<j)) {
				hport = hal_swif_lport_2_hport(j+1);
				hwport_vec |= 1<<hport;				
				aggr_port_num++;
				if(aggr_port_num == MAX_AGGREGATION_GROUP_PORT_NUM)
					break;				
			}
		}
						
		PortAggrInfo.AggrGroupRec[PortAggrInfo.AggrCount-1].PortNumber = aggr_port_num;

/*		memset(aggr_port_array, 0, MAX_AGGREGATION_GROUP_PORT_NUM);
		index = 0;
		for(j=0; j<MAX_PORT_NUM; j++) {
			if(hwport_vec & (1<<j)) {
				aggr_port_array[index] = j;
				index++;
				if(index == MAX_AGGREGATION_GROUP_PORT_NUM)
					break;					
			}
		}	*/
	}
	
	return CONF_ERR_NONE;

#else
	return CONF_ERR_NOT_SUPPORT;
#endif

}


#if MODULE_OBNMS

extern u8 NMS_TxBuffer[];
static obnet_record_set_stat_t PortAggrRecSetStat;
static obnet_record_get_stat_t PortAggrRecGetStat;

void nms_rsp_set_port_aggregation(u8 *DMA, u8 *RequestID, obnet_set_port_aggregation *pSetPortAggr)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	link_aggregation_conf_t aggr_cfg;
	link_aggregation_rec aggr_rec;
	obnet_port_aggregation_rec *pPortTrunkRec = (obnet_port_aggregation_rec *)((u8 *)pSetPortAggr+sizeof(obnet_set_port_aggregation));
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_PORT_TRUNK;

#if ((BOARD_FEATURE & L2_LINK_AGGREGATION) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if((pSetPortAggr->PortNum > MAX_PORT_NUM) || (pSetPortAggr->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(((pSetPortAggr->OpCode & 0xC0) >> 6) == 0x0) {
			PortAggrRecSetStat.PacketIndex = 1;
			PortAggrRecSetStat.RecordCount = pSetPortAggr->RecordCount;
			PortAggrRecSetStat.OffsetAddress = 0;
		} else if(((pSetPortAggr->OpCode & 0xC0) >> 6) == 0x1) {
			PortAggrRecSetStat.PacketIndex = 1;
			PortAggrRecSetStat.RecordCount = pSetPortAggr->RecordCount;
			PortAggrRecSetStat.OffsetAddress = 0;
		} else if(((pSetPortAggr->OpCode & 0xC0) >> 6) == 0x3) {
			if((pSetPortAggr->OpCode & 0x3F) != PortAggrRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				PortAggrRecSetStat.PacketIndex++;
				PortAggrRecSetStat.OffsetAddress = PortAggrRecSetStat.RecordCount * sizeof(link_aggregation_rec);
				PortAggrRecSetStat.RecordCount += pSetPortAggr->RecordCount;
			}
		} else if(((pSetPortAggr->OpCode & 0xC0) >> 6) == 0x2) {
			if((pSetPortAggr->OpCode & 0x3F) != PortAggrRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				PortAggrRecSetStat.PacketIndex++;
				PortAggrRecSetStat.OffsetAddress = PortAggrRecSetStat.RecordCount * sizeof(link_aggregation_rec);
				PortAggrRecSetStat.RecordCount += pSetPortAggr->RecordCount;
			}
		} 

		/* Update the port trunk config */
		aggr_cfg.PortNum = pSetPortAggr->PortNum;
		aggr_cfg.TotalRecordCount = PortAggrRecSetStat.RecordCount;
		if(eeprom_page_write(NVRAM_PORT_TRUNK_CFG_BASE, (u8 *)&aggr_cfg, sizeof(link_aggregation_conf_t)) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;
			goto Response;
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}

		/* Write the port trunk record configuration to EEPROM */
		if(eeprom_page_write(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + PortAggrRecSetStat.OffsetAddress, (u8 *)pPortTrunkRec, pSetPortAggr->RecordCount * sizeof(link_aggregation_rec)) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;
			goto Response;
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}
	}
	
#elif ((BOARD_FEATURE & L2_LINK_AGGREGATION) && (SWITCH_CHIP_TYPE == CHIP_BCM53286))
	if((pSetPortAggr->PortNum > MAX_PORT_NUM) || (pSetPortAggr->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(((pSetPortAggr->OpCode & 0xC0) >> 6) == 0x0) {
			PortAggrRecSetStat.PacketIndex = 1;
			PortAggrRecSetStat.RecordCount = pSetPortAggr->RecordCount;
			PortAggrRecSetStat.OffsetAddress = 0;
		} else if(((pSetPortAggr->OpCode & 0xC0) >> 6) == 0x1) {
			PortAggrRecSetStat.PacketIndex = 1;
			PortAggrRecSetStat.RecordCount = pSetPortAggr->RecordCount;
			PortAggrRecSetStat.OffsetAddress = 0;
		} else if(((pSetPortAggr->OpCode & 0xC0) >> 6) == 0x3) {
			if((pSetPortAggr->OpCode & 0x3F) != PortAggrRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				PortAggrRecSetStat.PacketIndex++;
				PortAggrRecSetStat.OffsetAddress = PortAggrRecSetStat.RecordCount * sizeof(link_aggregation_rec);
				PortAggrRecSetStat.RecordCount += pSetPortAggr->RecordCount;
			}
		} else if(((pSetPortAggr->OpCode & 0xC0) >> 6) == 0x2) {
			if((pSetPortAggr->OpCode & 0x3F) != PortAggrRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				PortAggrRecSetStat.PacketIndex++;
				PortAggrRecSetStat.OffsetAddress = PortAggrRecSetStat.RecordCount * sizeof(link_aggregation_rec);
				PortAggrRecSetStat.RecordCount += pSetPortAggr->RecordCount;
			}
		} 

		/* Update the port trunk config */
		aggr_cfg.PortNum = pSetPortAggr->PortNum;
		aggr_cfg.TotalRecordCount = PortAggrRecSetStat.RecordCount;
		if(eeprom_page_write(NVRAM_PORT_TRUNK_CFG_BASE, (u8 *)&aggr_cfg, sizeof(link_aggregation_conf_t)) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;
			goto Response;
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}

		/* Write the port trunk record configuration to EEPROM */
		if(eeprom_page_write(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + PortAggrRecSetStat.OffsetAddress, (u8 *)pPortTrunkRec, pSetPortAggr->RecordCount * sizeof(link_aggregation_rec)) != I2C_SUCCESS) {
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

void nms_rsp_get_port_aggregation(u8 *DMA, u8 *RequestID, obnet_get_port_aggregation *pGetPortAggr)
{
	obnet_rsp_get_port_aggregation RspGetPortAggr;	
	u16 RspLength;
	link_aggregation_conf_t aggr_cfg;
	link_aggregation_rec aggr_rec[2];
	u8 bFirstRecFlag;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetPortAggr, 0, sizeof(obnet_rsp_get_port_aggregation));
	RspGetPortAggr.GetCode = CODE_GET_PORT_TRUNK;

#if ((BOARD_FEATURE & L2_LINK_AGGREGATION) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if(eeprom_read(NVRAM_PORT_TRUNK_CFG_BASE, (u8 *)&aggr_cfg, sizeof(link_aggregation_conf_t)) != I2C_SUCCESS) {
		goto ErrorPortAggr;
	} else {
		if((aggr_cfg.PortNum == 0) || (aggr_cfg.PortNum > MAX_PORT_NUM) || (aggr_cfg.TotalRecordCount > MAX_AGGREGATION_RECORD_COUNT) || (aggr_cfg.TotalRecordCount == 0)) {
			goto ErrorPortAggr;
		} else {
			if(pGetPortAggr->OpCode == 0x00) {
				PortAggrRecGetStat.PacketIndex = 1;
				PortAggrRecGetStat.RemainCount = aggr_cfg.TotalRecordCount;
				PortAggrRecGetStat.OffsetAddress = 0;
				bFirstRecFlag = 1;
			} else {
				bFirstRecFlag = 0;
			}
			
			if(aggr_cfg.TotalRecordCount == 0) {
				RspGetPortAggr.RetCode = 0x00;
				RspGetPortAggr.PortNum = aggr_cfg.PortNum;
				RspGetPortAggr.OpCode = 0x00;
				RspGetPortAggr.RecordCount = 0x00;

				memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortAggr, sizeof(obnet_rsp_get_port_aggregation));
				RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation);
				PrepareEtherHead(DMA);
				PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);	
				if(RspLength == MSG_MINSIZE)
					RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
				else
					RspSend(NMS_TxBuffer, RspLength);

			} else if((aggr_cfg.TotalRecordCount > 0) && (aggr_cfg.TotalRecordCount < 3)) {
				RspGetPortAggr.RetCode = 0x00;
				RspGetPortAggr.PortNum = aggr_cfg.PortNum;
				RspGetPortAggr.OpCode = 0x00;
				RspGetPortAggr.RecordCount = aggr_cfg.TotalRecordCount;

				if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE, (u8 *)&(aggr_rec[0]), aggr_cfg.TotalRecordCount * sizeof(link_aggregation_rec)) != I2C_SUCCESS) {
					goto ErrorPortAggr;
				} else {
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortAggr, sizeof(obnet_rsp_get_port_aggregation));
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation)], (u8 *)&(aggr_rec[0]), aggr_cfg.TotalRecordCount * sizeof(link_aggregation_rec));
					RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation) + aggr_cfg.TotalRecordCount * sizeof(link_aggregation_rec);
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
				if(PortAggrRecGetStat.RemainCount >= 2) {
					if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + PortAggrRecGetStat.OffsetAddress, (u8 *)&(aggr_rec[0]), 2 * sizeof(link_aggregation_rec)) != I2C_SUCCESS) {
						goto ErrorPortAggr;
					} else {
						RspGetPortAggr.GetCode = CODE_GET_PORT_TRUNK;
						RspGetPortAggr.RetCode = 0x00;
						RspGetPortAggr.PortNum = aggr_cfg.PortNum;
						if(bFirstRecFlag) {
							bFirstRecFlag = 0;
							RspGetPortAggr.OpCode = (0x40 | PortAggrRecGetStat.PacketIndex);
						} else {
							if(PortAggrRecGetStat.RemainCount == 2)
								RspGetPortAggr.OpCode = (0x80 | PortAggrRecGetStat.PacketIndex);
							else
								RspGetPortAggr.OpCode = (0xc0 | PortAggrRecGetStat.PacketIndex);
						}
						RspGetPortAggr.RecordCount = 0x02;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortAggr, sizeof(obnet_rsp_get_port_aggregation));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation)], (u8 *)&(aggr_rec[0]), 2 * sizeof(link_aggregation_rec));
						RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation) + 2 * sizeof(link_aggregation_rec);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						PortAggrRecGetStat.RemainCount -= 2;
						PortAggrRecGetStat.OffsetAddress += 2 * sizeof(link_aggregation_rec);
						PortAggrRecGetStat.PacketIndex++;
					}
				} else {
					if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + PortAggrRecGetStat.OffsetAddress, (u8 *)&(aggr_rec[0]), PortAggrRecGetStat.RemainCount * sizeof(link_aggregation_rec)) != I2C_SUCCESS) {
						goto ErrorPortAggr;
					} else {
						RspGetPortAggr.GetCode = CODE_GET_PORT_TRUNK;
						RspGetPortAggr.RetCode = 0x00;
						RspGetPortAggr.PortNum = aggr_cfg.PortNum;
						RspGetPortAggr.OpCode = (0x80 | PortAggrRecGetStat.PacketIndex);
						RspGetPortAggr.RecordCount = PortAggrRecGetStat.RemainCount;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortAggr, sizeof(obnet_rsp_get_port_aggregation));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation)], (u8 *)&(aggr_rec[0]), PortAggrRecGetStat.RemainCount * sizeof(link_aggregation_rec));
						RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation) + PortAggrRecGetStat.RemainCount * sizeof(link_aggregation_rec);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						PortAggrRecGetStat.RemainCount = 0;
						PortAggrRecGetStat.OffsetAddress = 0;
					}
				}
			}
		}
	}

	return;

#elif ((BOARD_FEATURE & L2_LINK_AGGREGATION) && (SWITCH_CHIP_TYPE == CHIP_BCM53286))
	if(eeprom_read(NVRAM_PORT_TRUNK_CFG_BASE, (u8 *)&aggr_cfg, sizeof(link_aggregation_conf_t)) != I2C_SUCCESS) {
		goto ErrorPortAggr;
	} else {
		if((aggr_cfg.PortNum == 0) || (aggr_cfg.PortNum > MAX_PORT_NUM) || (aggr_cfg.TotalRecordCount > MAX_AGGREGATION_RECORD_COUNT) || (aggr_cfg.TotalRecordCount == 0)) {
			goto ErrorPortAggr;
		} else {
			if(pGetPortAggr->OpCode == 0x00) {
				PortAggrRecGetStat.PacketIndex = 1;
				PortAggrRecGetStat.RemainCount = aggr_cfg.TotalRecordCount;
				PortAggrRecGetStat.OffsetAddress = 0;
				bFirstRecFlag = 1;
			} else {
				bFirstRecFlag = 0;
			}
			
			if(aggr_cfg.TotalRecordCount == 0) {
				RspGetPortAggr.RetCode = 0x00;
				RspGetPortAggr.PortNum = aggr_cfg.PortNum;
				RspGetPortAggr.OpCode = 0x00;
				RspGetPortAggr.RecordCount = 0x00;

				memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortAggr, sizeof(obnet_rsp_get_port_aggregation));
				RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation);
				PrepareEtherHead(DMA);
				PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);	
				if(RspLength == MSG_MINSIZE)
					RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
				else
					RspSend(NMS_TxBuffer, RspLength);

			} else if((aggr_cfg.TotalRecordCount > 0) && (aggr_cfg.TotalRecordCount < 3)) {
				RspGetPortAggr.RetCode = 0x00;
				RspGetPortAggr.PortNum = aggr_cfg.PortNum;
				RspGetPortAggr.OpCode = 0x00;
				RspGetPortAggr.RecordCount = aggr_cfg.TotalRecordCount;

				if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE, (u8 *)&(aggr_rec[0]), aggr_cfg.TotalRecordCount * sizeof(link_aggregation_rec)) != I2C_SUCCESS) {
					goto ErrorPortAggr;
				} else {
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortAggr, sizeof(obnet_rsp_get_port_aggregation));
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation)], (u8 *)&(aggr_rec[0]), aggr_cfg.TotalRecordCount * sizeof(link_aggregation_rec));
					RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation) + aggr_cfg.TotalRecordCount * sizeof(link_aggregation_rec);
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
				if(PortAggrRecGetStat.RemainCount >= 2) {
					if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + PortAggrRecGetStat.OffsetAddress, (u8 *)&(aggr_rec[0]), 2 * sizeof(link_aggregation_rec)) != I2C_SUCCESS) {
						goto ErrorPortAggr;
					} else {
						RspGetPortAggr.GetCode = CODE_GET_PORT_TRUNK;
						RspGetPortAggr.RetCode = 0x00;
						RspGetPortAggr.PortNum = aggr_cfg.PortNum;
						if(bFirstRecFlag) {
							bFirstRecFlag = 0;
							RspGetPortAggr.OpCode = (0x40 | PortAggrRecGetStat.PacketIndex);
						} else {
							if(PortAggrRecGetStat.RemainCount == 2)
								RspGetPortAggr.OpCode = (0x80 | PortAggrRecGetStat.PacketIndex);
							else
								RspGetPortAggr.OpCode = (0xc0 | PortAggrRecGetStat.PacketIndex);
						}
						RspGetPortAggr.RecordCount = 0x02;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortAggr, sizeof(obnet_rsp_get_port_aggregation));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation)], (u8 *)&(aggr_rec[0]), 2 * sizeof(link_aggregation_rec));
						RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation) + 2 * sizeof(link_aggregation_rec);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						PortAggrRecGetStat.RemainCount -= 2;
						PortAggrRecGetStat.OffsetAddress += 2 * sizeof(link_aggregation_rec);
						PortAggrRecGetStat.PacketIndex++;
					}
				} else {
					if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + PortAggrRecGetStat.OffsetAddress, (u8 *)&(aggr_rec[0]), PortAggrRecGetStat.RemainCount * sizeof(link_aggregation_rec)) != I2C_SUCCESS) {
						goto ErrorPortAggr;
					} else {
						RspGetPortAggr.GetCode = CODE_GET_PORT_TRUNK;
						RspGetPortAggr.RetCode = 0x00;
						RspGetPortAggr.PortNum = aggr_cfg.PortNum;
						RspGetPortAggr.OpCode = (0x80 | PortAggrRecGetStat.PacketIndex);
						RspGetPortAggr.RecordCount = PortAggrRecGetStat.RemainCount;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortAggr, sizeof(obnet_rsp_get_port_aggregation));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation)], (u8 *)&(aggr_rec[0]), PortAggrRecGetStat.RemainCount * sizeof(link_aggregation_rec));
						RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation) + PortAggrRecGetStat.RemainCount * sizeof(link_aggregation_rec);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						PortAggrRecGetStat.RemainCount = 0;
						PortAggrRecGetStat.OffsetAddress = 0;
					}
				}
			}
		}
	}

	return;	
#endif

	/************************************************/
ErrorPortAggr:
	memset(&RspGetPortAggr, 0, sizeof(obnet_rsp_get_port_aggregation));
	RspGetPortAggr.GetCode = CODE_GET_PORT_TRUNK;
	RspGetPortAggr.RetCode = 0x01;
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortAggr, sizeof(obnet_rsp_get_port_aggregation));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_aggregation);
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);	
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}

#endif /* MODULE_OBNMS */



