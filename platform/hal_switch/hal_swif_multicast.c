
/*****************************************************************
 * Filename     : hal_swif_multicast.c
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
#include "hal_swif_multicast.h"

/* Other includes */
#include "cli_util.h"

#if MARVELL_SWITCH
extern GT_QD_DEV *dev;
#endif

/**************************************************************************
  * @brief  static muticast initialize use configuration
  * @param  none
  * @retval none
  *************************************************************************/
int hal_swif_mcast_conf_initialize(void)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;
	multicast_conf_t mcast_cfg;
	multicast_rec mcast_rec;
#if MULTI_CFG_OPTIMIZAION
	u32 port_list_vec;
#else
	u16 port_list_vec;
#endif
	u32 hwport_vec;
	u8 hport;
	int i,j;
	
	if(eeprom_read(NVRAM_MCAST_CFG_BASE, (u8 *)&mcast_cfg, sizeof(multicast_conf_t)) != I2C_SUCCESS) {
		return CONF_ERR_I2C;
	}

	if((mcast_cfg.PortNum == 0) || (mcast_cfg.PortNum > MAX_PORT_NUM) || (mcast_cfg.TotalRecordCount > MAX_MCAST_RECORD_COUNT) || (mcast_cfg.TotalRecordCount == 0)) {
		return CONF_ERR_NO_CFG;
	}
	
#if MULTI_CFG_OPTIMIZAION
	port_list_vec = *(u32 *)&(mcast_cfg.PortDefaultForward[0]);
	port_list_vec = ntohl(port_list_vec);
	
	for(j=0; j<mcast_cfg.PortNum; j++) {
		if(port_list_vec & (1<<j)) {
			hport = hal_swif_lport_2_hport(j+1);
			switch(dev->deviceId) {
				case GT_88E6095:
				/* DF=1, Multicast frames with unknown DAs are allowed to egress out this port */
			    if((status = gprtSetDefaultForward(dev, hport, GT_TRUE)) != GT_OK) {
			        printf("Error: gprtSetDefaultForward failed\r\n");
			        return status;
			    }

				case GT_88E6097:
			    if((status = hwSetPortRegField(dev, hport, QD_REG_PORT_CONTROL, 2, 2, 0x3)) != GT_OK) {
			        printf("Error: hwSetPortRegField failed\r\n");
			        return status;
			    }
				break;

				default:
				break;
			}
		} else {
			hport = hal_swif_lport_2_hport(j+1);
			switch(dev->deviceId) {
				case GT_88E6095:
				/* DF=0, Multicast frames with unknown DAs do not egress from this port */
			    if((status = gprtSetDefaultForward(dev, hport, GT_FALSE)) != GT_OK) {
			        printf("Error: gprtSetDefaultForward failed\r\n");
			        return status;
			    }

				case GT_88E6097:
			    if((status = hwSetPortRegField(dev, hport, QD_REG_PORT_CONTROL, 2, 2, 0x1)) != GT_OK) {
			        printf("Error: hwSetPortRegField failed\r\n");
			        return status;
			    }					
				break;

				default:
				break;
			}			
		}
	}
#endif	

	for(i=0; i<mcast_cfg.TotalRecordCount; i++) {
		if(eeprom_read(NVRAM_MCAST_RECORD_CFG_BASE + i * sizeof(multicast_rec), (u8 *)&mcast_rec, sizeof(multicast_rec)) != I2C_SUCCESS) {
			return CONF_ERR_I2C;
		}

		if((mcast_rec.Mac[0] & 0x1) != 1)
			continue;
		
#if MULTI_CFG_OPTIMIZAION
		port_list_vec = *(u32 *)&(mcast_rec.Member[0]);
		port_list_vec = ntohl(port_list_vec);
#else
		port_list_vec = *(u16 *)&(mcast_rec.Member[0]);
		port_list_vec = ntohs(port_list_vec);
#endif		
		hwport_vec = 0;
		for(j=0; j<mcast_cfg.PortNum; j++) {
			if(port_list_vec & (1<<j)) {
				hport = hal_swif_lport_2_hport(j+1);
				hwport_vec |= 1<<hport;
			}
		}
		
		memset(&macEntry,0,sizeof(GT_ATU_ENTRY));	
		macEntry.macAddr.arEther[0] = mcast_rec.Mac[0];
		macEntry.macAddr.arEther[1] = mcast_rec.Mac[1];
		macEntry.macAddr.arEther[2] = mcast_rec.Mac[2];
		macEntry.macAddr.arEther[3] = mcast_rec.Mac[3];
		macEntry.macAddr.arEther[4] = mcast_rec.Mac[4];
		macEntry.macAddr.arEther[5] = mcast_rec.Mac[5];	

		macEntry.DBNum = 0;
		macEntry.portVec = hwport_vec;
		macEntry.prio = 0;
		macEntry.entryState.mcEntryState = GT_MC_STATIC;

		if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK) {
			printf("gfdbAddMacEntry return failed, ret=%d\r\n", status);
			return CONF_ERR_MSAPI;
		}
	}
	
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}


#if MODULE_OBNMS

extern u8 NMS_TxBuffer[];
static obnet_record_set_stat_t MulticastRecSetStat;
static obnet_record_get_stat_t MulticastRecGetStat;

void nms_rsp_set_multicast(u8 *DMA, u8 *RequestID, obnet_set_multicast *pSetMcast)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	multicast_conf_t mcast_cfg;
	multicast_rec mcast_rec;
	multicast_rec *pMcastRec = (multicast_rec *)((u8 *)pSetMcast+sizeof(obnet_set_multicast));
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_MCAST;

#if ((BOARD_FEATURE & L2_STATIC_MULTICAST) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if((pSetMcast->PortNum > MAX_PORT_NUM) || (pSetMcast->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(((pSetMcast->OpCode & 0xC0) >> 6) == 0x0) {
			MulticastRecSetStat.PacketIndex = 1;
			MulticastRecSetStat.RecordCount = pSetMcast->RecordCount;
			MulticastRecSetStat.OffsetAddress = 0;
		} else if(((pSetMcast->OpCode & 0xC0) >> 6) == 0x1) {
			MulticastRecSetStat.PacketIndex = 1;
			MulticastRecSetStat.RecordCount = pSetMcast->RecordCount;
			MulticastRecSetStat.OffsetAddress = 0;
		} else if(((pSetMcast->OpCode & 0xC0) >> 6) == 0x3) {
			if((pSetMcast->OpCode & 0x3F) != MulticastRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				MulticastRecSetStat.PacketIndex++;
				MulticastRecSetStat.OffsetAddress = MulticastRecSetStat.RecordCount * sizeof(multicast_rec);
				MulticastRecSetStat.RecordCount += pSetMcast->RecordCount;
			}
		} else if(((pSetMcast->OpCode & 0xC0) >> 6) == 0x2) {
			if((pSetMcast->OpCode & 0x3F) != MulticastRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				MulticastRecSetStat.PacketIndex++;
				MulticastRecSetStat.OffsetAddress = MulticastRecSetStat.RecordCount * sizeof(multicast_rec);
				MulticastRecSetStat.RecordCount += pSetMcast->RecordCount;
			}
		} 

		/* Update the mcast config */
		mcast_cfg.PortNum = pSetMcast->PortNum;
		mcast_cfg.TotalRecordCount = MulticastRecSetStat.RecordCount;
#if MULTI_CFG_OPTIMIZAION
		memcpy(mcast_cfg.PortDefaultForward, pSetMcast->PortDefaultForward, 4);
#endif
		if(eeprom_page_write(NVRAM_MCAST_CFG_BASE, (u8 *)&mcast_cfg, sizeof(multicast_conf_t)) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;
			goto Response;
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}

		/* Write the mcast record configuration to EEPROM */
		if(eeprom_page_write(NVRAM_MCAST_RECORD_CFG_BASE + MulticastRecSetStat.OffsetAddress, (u8 *)pMcastRec, pSetMcast->RecordCount * sizeof(multicast_rec)) != I2C_SUCCESS) {
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

void nms_rsp_get_multicast(u8 *DMA, u8 *RequestID, obnet_get_multicast *pGetMcast)
{
	obnet_rsp_get_multicast RspGetMcast;	
	u16 RspLength;
	multicast_conf_t mcast_cfg;
	multicast_rec mcast_rec[2];
	u8 bFirstRecFlag;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetMcast, 0, sizeof(obnet_rsp_get_multicast));
	RspGetMcast.GetCode = CODE_GET_MCAST;

#if ((BOARD_FEATURE & L2_STATIC_MULTICAST) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if(eeprom_read(NVRAM_MCAST_CFG_BASE, (u8 *)&mcast_cfg, sizeof(multicast_conf_t)) != I2C_SUCCESS) {
		goto ErrorMcast;
	} else {
		if((mcast_cfg.PortNum == 0) || (mcast_cfg.PortNum > MAX_PORT_NUM) || (mcast_cfg.TotalRecordCount > MAX_MCAST_RECORD_COUNT)) {
			goto ErrorMcast;
		} else {
			if(pGetMcast->OpCode == 0x00) {
				MulticastRecGetStat.PacketIndex = 1;
				MulticastRecGetStat.RemainCount = mcast_cfg.TotalRecordCount;
				MulticastRecGetStat.OffsetAddress = 0;
				bFirstRecFlag = 1;
			} else {
				bFirstRecFlag = 0;
			}
			
			if(mcast_cfg.TotalRecordCount == 0) {
				RspGetMcast.RetCode = 0x00;
				RspGetMcast.PortNum = mcast_cfg.PortNum;
#if MULTI_CFG_OPTIMIZAION
				memcpy(RspGetMcast.PortDefaultForward, mcast_cfg.PortDefaultForward, 4);
#endif				
				RspGetMcast.OpCode = 0x00;
				RspGetMcast.RecordCount = 0x00;

				memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMcast, sizeof(obnet_rsp_get_multicast));
				RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_multicast);
				PrepareEtherHead(DMA);
				PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);	
				if(RspLength == MSG_MINSIZE)
					RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
				else
					RspSend(NMS_TxBuffer, RspLength);
				
			} else if((mcast_cfg.TotalRecordCount > 0) && (mcast_cfg.TotalRecordCount < 3)) {
				RspGetMcast.RetCode = 0x00;
				RspGetMcast.PortNum = mcast_cfg.PortNum;
#if MULTI_CFG_OPTIMIZAION
				memcpy(RspGetMcast.PortDefaultForward, mcast_cfg.PortDefaultForward, 4);
#endif					
				RspGetMcast.OpCode = 0x00;
				RspGetMcast.RecordCount = mcast_cfg.TotalRecordCount;

				if(eeprom_read(NVRAM_MCAST_RECORD_CFG_BASE, (u8 *)&(mcast_rec[0]), mcast_cfg.TotalRecordCount * sizeof(multicast_rec)) != I2C_SUCCESS) {
					goto ErrorMcast;
				} else {
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMcast, sizeof(obnet_rsp_get_multicast));
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_multicast)], (u8 *)&(mcast_rec[0]), mcast_cfg.TotalRecordCount * sizeof(multicast_rec));
					RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_multicast) + mcast_cfg.TotalRecordCount * sizeof(multicast_rec);
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
				if(MulticastRecGetStat.RemainCount >= 2) {
					if(eeprom_read(NVRAM_MCAST_RECORD_CFG_BASE + MulticastRecGetStat.OffsetAddress, (u8 *)&(mcast_rec[0]), 2 * sizeof(multicast_rec)) != I2C_SUCCESS) {
						goto ErrorMcast;
					} else {
						RspGetMcast.GetCode = CODE_GET_MCAST;
						RspGetMcast.RetCode = 0x00;
						RspGetMcast.PortNum = mcast_cfg.PortNum;
#if MULTI_CFG_OPTIMIZAION
						memcpy(RspGetMcast.PortDefaultForward, mcast_cfg.PortDefaultForward, 4);
#endif							
						if(bFirstRecFlag) {
							bFirstRecFlag = 0;
							RspGetMcast.OpCode = (0x40 | MulticastRecGetStat.PacketIndex);
						} else {
							if(MulticastRecGetStat.RemainCount == 2)
								RspGetMcast.OpCode = (0x80 | MulticastRecGetStat.PacketIndex);
							else
								RspGetMcast.OpCode = (0xc0 | MulticastRecGetStat.PacketIndex);
						}
						RspGetMcast.RecordCount = 0x02;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMcast, sizeof(obnet_rsp_get_multicast));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_multicast)], (u8 *)&(mcast_rec[0]), 2 * sizeof(multicast_rec));
						RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_multicast) + 2 * sizeof(multicast_rec);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						MulticastRecGetStat.RemainCount -= 2;
						MulticastRecGetStat.OffsetAddress += 2 * sizeof(multicast_rec);
						MulticastRecGetStat.PacketIndex++;
					}
				} else {
					if(eeprom_read(NVRAM_MCAST_RECORD_CFG_BASE + MulticastRecGetStat.OffsetAddress, (u8 *)&(mcast_rec[0]), MulticastRecGetStat.RemainCount * sizeof(multicast_rec)) != I2C_SUCCESS) {
						goto ErrorMcast;
					} else {
						RspGetMcast.GetCode = CODE_GET_MCAST;
						RspGetMcast.RetCode = 0x00;
						RspGetMcast.PortNum = mcast_cfg.PortNum;
#if MULTI_CFG_OPTIMIZAION
						memcpy(RspGetMcast.PortDefaultForward, mcast_cfg.PortDefaultForward, 4);
#endif							
						RspGetMcast.OpCode = (0x80 | MulticastRecGetStat.PacketIndex);
						RspGetMcast.RecordCount = MulticastRecGetStat.RemainCount;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMcast, sizeof(obnet_rsp_get_multicast));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(obnet_rsp_get_multicast)], (u8 *)&(mcast_rec[0]), MulticastRecGetStat.RemainCount * sizeof(multicast_rec));
						RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_multicast) + MulticastRecGetStat.RemainCount * sizeof(multicast_rec);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						MulticastRecGetStat.RemainCount = 0;
						MulticastRecGetStat.OffsetAddress = 0;
					}
				}
			}
		}
	}

	return;
#endif

	/************************************************/
ErrorMcast:
	memset(&RspGetMcast, 0, sizeof(obnet_rsp_get_multicast));
	RspGetMcast.GetCode = CODE_GET_MCAST;
	RspGetMcast.RetCode = 0x01;
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMcast, sizeof(obnet_rsp_get_multicast));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_multicast);
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


