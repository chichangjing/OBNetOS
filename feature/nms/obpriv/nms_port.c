
/*************************************************************
 * Filename     : nms_port.c
 * Description  : API for NMS interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#include "mconfig.h"

#if MODULE_OBNMS
/* Standard includes */
#include "string.h"

/* Kernel includes */

/* LwIP includes */
#include "lwip/inet.h"

/* BSP includes */
#include "stm32f2xx.h"
#include "soft_i2c.h"

/* Other includes */
#include "hal_swif_port.h"
#include "conf_comm.h"
#include "conf_map.h"
#include "conf_port.h"

#if SWITCH_CHIP_88E6095
#include "msApi.h"
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>
#endif

#include "nms_comm.h"
#include "nms_if.h"
#include "nms_port.h"
#include "cli_util.h"

extern u8 NMS_TxBuffer[];

static record_set_stat_t VlanRecSetStat, McastRecSetStat, SecurityRecSetStat, PortTrunkRecSetStat;
static record_get_stat_t VlanRecGetStat, McastRecGetStat, SecurityRecGetStat, PortTrunkRecGetStat, PortConfigGetStat;
static get_maclist_stat_t MacListGetStat;
static int PortCfgPktIdx=0;

#if SWITCH_CHIP_88E6095	
static GT_ATU_ENTRY gMacEntry;
#endif

#if SWITCH_CHIP_88E6095
extern GT_QD_DEV *dev;
#endif

void Rsp_GetPortStatus(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_PORT_STATUS RspPortStatus;	
	u16 RspLength;
	u8 i, hport;
	HAL_PORT_LINK_STATE link_state;
	HAL_PORT_SPEED_STATE speed;
	HAL_PORT_DUPLEX_STATE duplex;
	HAL_MDI_MDIX_STATE mdi_mdix;
	u16 regval;
		
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_PORT_STATUS);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspPortStatus, 0, sizeof(OBNET_RSP_PORT_STATUS));
	RspPortStatus.GetCode = CODE_PORT_STATUS;
	RspPortStatus.RetCode = 0x00;
	RspPortStatus.PortNum = MAX_PORT_NUM;
	
	for(i=0; i<MAX_PORT_NUM; i++) {
		RspPortStatus.PortStatus[i] = 0;
		hport = hal_swif_lport_2_hport(i+1);
		hal_swif_port_get_link_state(i+1, &link_state);
		hal_swif_port_get_speed(i+1, &speed);
		hal_swif_port_get_duplex(i+1, &duplex);
		hal_swif_port_get_mdi_mdix(i+1, &mdi_mdix);
		
		RspPortStatus.PortStatus[i] = 	((link_state == LINK_UP)? 0x80 : 0x00) | \
										((speed == SPEED_10M)? 0x00 : (speed == SPEED_100M)? 0x10 : (speed == SPEED_1000M)? 0x20 : 0x00) | \
										((duplex == FULL_DUPLEX)? 0x08 : 0x00) | \
										((mdi_mdix == MODE_MDI)? 0x02 : 0x00);
	}

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspPortStatus, sizeof(OBNET_RSP_PORT_STATUS));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}

void Rsp_SetPortConfig(u8 *DMA, u8 *RequestID, POBNET_SET_PORT_CONFIG pPortConfig)
{
	OBNET_SET_RSP RspSet;
	POBNET_SET_PORT_CONFIG2	pPortConfig2 = (POBNET_SET_PORT_CONFIG2)pPortConfig;
	u16 RspLength;
	u8	retVal;
	int Ret;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_PORT_CFG;

	if(PortCfgPktIdx == 0) {
		if((pPortConfig->PortNum > MAX_PORT_NUM) || (pPortConfig->PortNum == 0)) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
		} else {
			if(pPortConfig->PortNum > MAX_PORT_CONFIG_REC) {
				if(eeprom_page_write(NVRAM_PORT_CFG_BASE, (u8 *)&(pPortConfig->PortNum), MAX_PORT_CONFIG_REC * 4 + 1) != I2C_SUCCESS) {
					RspSet.RetCode = 0x01;
					RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
				} else {
					RspSet.RetCode = 0x00;
					RspSet.Res = 0x00;
					PortCfgPktIdx = 1;
				}			
			} else {
				if(eeprom_page_write(NVRAM_PORT_CFG_BASE, (u8 *)&(pPortConfig->PortNum), pPortConfig->PortNum * 4 + 1) != I2C_SUCCESS) {
					RspSet.RetCode = 0x01;
					RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
				} else {
					RspSet.RetCode = 0x00;
					RspSet.Res = 0x00;
				}			
			}
		}
	} else {
		if(eeprom_page_write(NVRAM_PORT_CFG_BASE + MAX_PORT_CONFIG_REC * 4 + 1, (u8 *)&(pPortConfig2->PortConfig[0]), pPortConfig2->ItemsInData * 4) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
			PortCfgPktIdx = 0;
		}
	}
	
	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}


void Rsp_GetPortConfig(u8 *DMA, u8 *RequestID, POBNET_GET_PORT_CONFIG pGetPortConfig)
{
	OBNET_RSP_PORT_CONFIG 	RspPortConfig;
	OBNET_RSP_PORT_CONFIG2	RspPortConfig2;
	u8	PortNum;
	u16 RspLength;
	u8	bFirstRecFlag;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_PORT_CONFIG);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspPortConfig, 0, sizeof(OBNET_RSP_PORT_CONFIG));
	memset(&RspPortConfig2, 0, sizeof(OBNET_RSP_PORT_CONFIG2));
	RspPortConfig.GetCode = CODE_PORT_CONFIG;
	RspPortConfig2.GetCode = CODE_PORT_CONFIG;
	
	if(eeprom_read(NVRAM_PORT_CFG_BASE, &PortNum, 1) != I2C_SUCCESS) {
		goto ErrorPortConfig;
	} else {
		if((PortNum == 0) || (PortNum > MAX_PORT_NUM)) {
			goto ErrorPortConfig;
		} else {
			if(pGetPortConfig->OpCode == 0x00) {
				PortConfigGetStat.PacketIndex = 1;
				PortConfigGetStat.RemainCount = PortNum;
				PortConfigGetStat.OffsetAddress = 0;
				bFirstRecFlag = 1;
			} else {
				bFirstRecFlag = 0;
			}

			if(bFirstRecFlag) {
				if(PortNum > MAX_PORT_CONFIG_REC) {
					if(eeprom_read(NVRAM_PORT_CFG_DATA, (u8 *)&(RspPortConfig.PortConfig[0]), MAX_PORT_CONFIG_REC * 4) != I2C_SUCCESS) {
						goto ErrorPortConfig;
					} else {
						RspPortConfig.RetCode = 0x00;
						RspPortConfig.PortNum = PortNum;
						
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspPortConfig, 3 + MAX_PORT_CONFIG_REC * 4);
						RspLength = PAYLOAD_OFFSET + 3 + MAX_PORT_CONFIG_REC * 4;
						RspSend(NMS_TxBuffer, RspLength);

						PortConfigGetStat.PacketIndex++;
						PortConfigGetStat.RemainCount -= MAX_PORT_CONFIG_REC;
						PortConfigGetStat.OffsetAddress += MAX_PORT_CONFIG_REC * 4;
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
						
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
					}
				}
			} else {
				if(PortConfigGetStat.RemainCount > MAX_PORT_CONFIG_REC) {
					if(eeprom_read(NVRAM_PORT_CFG_DATA + PortConfigGetStat.OffsetAddress, (u8 *)&(RspPortConfig2.PortConfig[0]), MAX_PORT_CONFIG_REC * 4) != I2C_SUCCESS) {
						goto ErrorPortConfig;
					} else {
						RspPortConfig2.RetCode = 0x00;
						RspPortConfig2.Res = 0x00;
						RspPortConfig2.OpCode = (0xc0 | (PortConfigGetStat.PacketIndex - 1));
						RspPortConfig2.ItemsInData = MAX_PORT_CONFIG_REC;
						
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspPortConfig2, 5 + MAX_PORT_CONFIG_REC * 4);
						RspLength = PAYLOAD_OFFSET + 5 + MAX_PORT_CONFIG_REC * 4;
						RspSend(NMS_TxBuffer, RspLength);

						PortConfigGetStat.PacketIndex++;
						PortConfigGetStat.RemainCount -= MAX_PORT_CONFIG_REC;
						PortConfigGetStat.OffsetAddress += MAX_PORT_CONFIG_REC * 4;
					}
				} else {
					if(eeprom_read(NVRAM_PORT_CFG_DATA + PortConfigGetStat.OffsetAddress, (u8 *)&(RspPortConfig2.PortConfig[0]), PortConfigGetStat.RemainCount * 4) != I2C_SUCCESS) {
						goto ErrorPortConfig;
					} else {
						RspPortConfig2.RetCode = 0x00;
						RspPortConfig2.Res = 0x00;
						RspPortConfig2.OpCode = (0x80 | (PortConfigGetStat.PacketIndex - 1));
						RspPortConfig2.ItemsInData = PortConfigGetStat.RemainCount;
						
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspPortConfig2, 5 + PortConfigGetStat.RemainCount * 4);
						RspLength = PAYLOAD_OFFSET + 5 + PortConfigGetStat.RemainCount * 4;
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);

						PortConfigGetStat.PacketIndex++;
						PortConfigGetStat.RemainCount = 0;
						PortConfigGetStat.OffsetAddress += PortConfigGetStat.RemainCount;
					}
				}
			}
		}
	}

	return;
	
ErrorPortConfig:
	memset(&RspPortConfig, 0, sizeof(OBNET_RSP_PORT_CONFIG));
	RspPortConfig.GetCode = CODE_PORT_CONFIG;
	RspPortConfig.RetCode = 0x01;
	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_PORT_CONFIG);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}


void Rsp_SetPortMirror(u8 *DMA, u8 *RequestID, POBNET_SET_MIRROR pPortMirror)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_MIRROR;

#if SWITCH_CHIP_88E6095
	if((pPortMirror->PortNum > MAX_PORT_NUM) || (pPortMirror->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(eeprom_write(NVRAM_PORT_MIRROR_CFG_BASE, (u8 *)&(pPortMirror->PortNum), pPortMirror->PortNum * 4 + 1) != I2C_SUCCESS) {
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
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}

void Rsp_GetPortMirror(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_GET_MIRROR RspGetMirror;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MIRROR);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetMirror, 0, sizeof(OBNET_RSP_GET_MIRROR));
	RspGetMirror.GetCode = CODE_GET_MIRROR;

#if SWITCH_CHIP_88E6095
	if(eeprom_read(NVRAM_PORT_MIRROR_CFG_BASE, &(RspGetMirror.PortNum), MAX_PORT_NUM * 4 + 1) != I2C_SUCCESS) {
		RspGetMirror.RetCode = 0x01;
	} else {
		if((RspGetMirror.PortNum == 0) || (RspGetMirror.PortNum > MAX_PORT_NUM)) {
			memset(&RspGetMirror, 0, sizeof(OBNET_RSP_GET_MIRROR));
			RspGetMirror.GetCode = CODE_GET_MIRROR;
			RspGetMirror.RetCode = 0x01;
		} else
			RspGetMirror.RetCode = 0x00;		
	}
#else
	RspGetMirror.RetCode = 0x01;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMirror, sizeof(OBNET_RSP_GET_MIRROR));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}


void Rsp_SetPortRate(u8 *DMA, u8 *RequestID, POBNET_SET_RATE pPortRate)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_RATE;

#if SWITCH_CHIP_88E6095
	if((pPortRate->PortNum > MAX_PORT_NUM) || (pPortRate->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(eeprom_write(NVRAM_PORT_RATE_CFG_BASE, (u8 *)&(pPortRate->PortNum), pPortRate->PortNum * 2 + 1) != I2C_SUCCESS) {
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
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}

void Rsp_GetPortRate(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_GET_RATE RspGetRate;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_RATE);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetRate, 0, sizeof(OBNET_RSP_GET_RATE));
	RspGetRate.GetCode = CODE_GET_RATE;

#if SWITCH_CHIP_88E6095
	if(eeprom_read(NVRAM_PORT_RATE_CFG_BASE, &(RspGetRate.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) {
		RspGetRate.RetCode = 0x01;
	} else {
		if((RspGetRate.PortNum == 0) || (RspGetRate.PortNum > MAX_PORT_NUM)) {
			memset(&RspGetRate, 0, sizeof(OBNET_RSP_GET_RATE));
			RspGetRate.GetCode = CODE_GET_RATE;
			RspGetRate.RetCode = 0x01;
		} else
			RspGetRate.RetCode = 0x00;
	}
#else
	RspGetRate.RetCode = 0x01;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetRate, sizeof(OBNET_RSP_GET_RATE));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}



void Rsp_SetQos(u8 *DMA, u8 *RequestID, POBNET_SET_QOS pQos)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_QOS;

#if 0
	RspSet.RetCode = 0x01;
	RspSet.Res = RSP_ERR_FEATURE_NOT_SUPPORT;
#else
#if SWITCH_CHIP_88E6095
	if((pQos->PortNum > MAX_PORT_NUM) || (pQos->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = 0x10;
	} else {
		if(eeprom_write(NVRAM_QOS_CFG_BASE, (u8 *)&(pQos->CosMapping[0]), 20 + pQos->PortNum + 1) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = 0x0F;	
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}	
	}
#else
	RspSet.RetCode = 0x01;
	RspSet.Res = 0x1F;
#endif
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}

void Rsp_GetQos(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_GET_QOS RspGetQos;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_QOS);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetQos, 0, sizeof(OBNET_RSP_GET_QOS));
	RspGetQos.GetCode = CODE_GET_QOS;

#if 0
	RspGetQos.RetCode = 0x01;
#else	
#if SWITCH_CHIP_88E6095
	if(eeprom_read(NVRAM_QOS_CFG_BASE, (u8 *)&(RspGetQos.CosMapping[0]), 20 + MAX_PORT_NUM + 1) != I2C_SUCCESS) {
		RspGetQos.RetCode = 0x01;
	} else {
		if((RspGetQos.PortNum == 0) || (RspGetQos.PortNum > MAX_PORT_NUM)) {
			memset(&RspGetQos, 0, sizeof(OBNET_RSP_GET_QOS));
			RspGetQos.GetCode = CODE_GET_QOS;
			RspGetQos.RetCode = 0x01;
		} else
			RspGetQos.RetCode = 0x00;
	}
#else
	RspGetQos.RetCode = 0x01;
#endif
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetQos, sizeof(OBNET_RSP_GET_QOS));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}


void Rsp_SetPortIsolation(u8 *DMA, u8 *RequestID, POBNET_SET_ISOLATION pPortIsolation)
{
	OBNET_SET_RSP RspSet;
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_ISOLATION;

#if SWITCH_CHIP_88E6095
	if((pPortIsolation->PortNum > MAX_PORT_NUM) || (pPortIsolation->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(eeprom_write(NVRAM_PORT_ISOLATION_CFG_BASE, (u8 *)&(pPortIsolation->PortNum), pPortIsolation->PortNum * 2 + 1) != I2C_SUCCESS) {
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
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}

void Rsp_GetPortIsolation(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_GET_ISOLATION RspGetIsolation;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_ISOLATION);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetIsolation, 0, sizeof(OBNET_RSP_GET_ISOLATION));
	RspGetIsolation.GetCode = CODE_GET_ISOLATION;

#if SWITCH_CHIP_88E6095
	if(eeprom_read(NVRAM_PORT_ISOLATION_CFG_BASE, (u8 *)&(RspGetIsolation.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) {
		RspGetIsolation.RetCode = 0x01;
	} else {
		if((RspGetIsolation.PortNum == 0) || (RspGetIsolation.PortNum > MAX_PORT_NUM)) {
			memset(&RspGetIsolation, 0, sizeof(OBNET_RSP_GET_ISOLATION));
			RspGetIsolation.GetCode = CODE_GET_ISOLATION;
			RspGetIsolation.RetCode = 0x01;
		} else
			RspGetIsolation.RetCode = 0x00;
	}
#else
	RspGetIsolation.RetCode = 0x01;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetIsolation, sizeof(OBNET_RSP_GET_ISOLATION));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}

void Rsp_SetPortVlan(u8 *DMA, u8 *RequestID, POBNET_SET_PORTVLAN pPortVlan)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_PORT_VLAN;

#if SWITCH_CHIP_88E6095
	if((pPortVlan->PortNum > MAX_PORT_NUM) || (pPortVlan->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(eeprom_write(NVRAM_PORT_VLAN_CFG_BASE, (u8 *)&(pPortVlan->PortNum), pPortVlan->PortNum * 2 + 1) != I2C_SUCCESS) {
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
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}

void Rsp_GetPortVlan(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_GET_PORTVLAN RspGetPortVlan;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORTVLAN);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetPortVlan, 0, sizeof(OBNET_RSP_GET_PORTVLAN));
	RspGetPortVlan.GetCode = CODE_GET_PORT_VLAN;

#if SWITCH_CHIP_88E6095
	if(eeprom_read(NVRAM_PORT_VLAN_CFG_BASE, &(RspGetPortVlan.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) {
		RspGetPortVlan.RetCode = 0x01;
	} else {
		if((RspGetPortVlan.PortNum == 0) || (RspGetPortVlan.PortNum > MAX_PORT_NUM)) {
			memset(&RspGetPortVlan, 0, sizeof(OBNET_RSP_GET_RATE));
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
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortVlan, sizeof(OBNET_RSP_GET_PORTVLAN));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}

void Rsp_SetAdmVlan(u8 *DMA, u8 *RequestID, POBNET_SET_ADM_VLAN pAdmVlan)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_ADM_VLAN;

#if 0
	RspSet.RetCode = 0x01;
	RspSet.Res = RSP_ERR_FEATURE_NOT_SUPPORT;
#else
#if SWITCH_CHIP_88E6095
	if(eeprom_write(NVRAM_ADM_VLAN_CFG_BASE, (u8 *)&(pAdmVlan->VLanID[0]), 2) != I2C_SUCCESS) {
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
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}

void Rsp_GetAdmVlan(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_GET_ADM_VLAN RspGetAdmVlan;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_ADM_VLAN);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetAdmVlan, 0, sizeof(OBNET_RSP_GET_ADM_VLAN));
	RspGetAdmVlan.GetCode = CODE_GET_ADM_VLAN;

#if 0
	RspGetAdmVlan.RetCode = 0x01;
#else	
#if SWITCH_CHIP_88E6095
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
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetAdmVlan, sizeof(OBNET_RSP_GET_ADM_VLAN));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}

void Rsp_SetVlan(u8 *DMA, u8 *RequestID, POBNET_SET_VLAN pVlan)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	vlan_conf_t vlan_cfg;
	vlan_record_conf_t vlan_rec;
	OBNET_VLAN_REC *pVlanRec = (OBNET_VLAN_REC *)((u8 *)pVlan+sizeof(OBNET_REQ_SET_VLAN));
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_VLAN;

#if SWITCH_CHIP_88E6095
	if((pVlan->PortNum > MAX_PORT_NUM) || (pVlan->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(((pVlan->OpCode & 0xC0) >> 6) == 0x0) {
			VlanRecSetStat.PacketIndex = 1;
			VlanRecSetStat.RecordCount = pVlan->RecordCount;
			VlanRecSetStat.OffsetAddress = 0;
		} else if(((pVlan->OpCode & 0xC0) >> 6) == 0x1) {
			VlanRecSetStat.PacketIndex = 1;
			VlanRecSetStat.RecordCount = pVlan->RecordCount;
			VlanRecSetStat.OffsetAddress = 0;
		} else if(((pVlan->OpCode & 0xC0) >> 6) == 0x3) {
			if((pVlan->OpCode & 0x3F) != VlanRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				VlanRecSetStat.PacketIndex++;
				VlanRecSetStat.OffsetAddress = VlanRecSetStat.RecordCount * sizeof(vlan_record_conf_t);
				VlanRecSetStat.RecordCount += pVlan->RecordCount;
			}
		} else if(((pVlan->OpCode & 0xC0) >> 6) == 0x2) {
			if((pVlan->OpCode & 0x3F) != VlanRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				VlanRecSetStat.PacketIndex++;
				VlanRecSetStat.OffsetAddress = VlanRecSetStat.RecordCount * sizeof(vlan_record_conf_t);
				VlanRecSetStat.RecordCount += pVlan->RecordCount;
			}
		} 

		/* Update the vlan config */
		vlan_cfg.PortNum = pVlan->PortNum;
		vlan_cfg.TotalRecordCount = VlanRecSetStat.RecordCount;
		if(eeprom_page_write(NVRAM_VLAN_CFG_BASE, (u8 *)&vlan_cfg, sizeof(vlan_conf_t)) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;
			goto Response;
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}

		/* Write the vlan record configuration to EEPROM */
		if(eeprom_page_write(NVRAM_VLAN_RECORD_CFG_BASE + VlanRecSetStat.OffsetAddress, (u8 *)pVlanRec, pVlan->RecordCount * sizeof(vlan_record_conf_t)) != I2C_SUCCESS) {
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
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}

void Rsp_GetVlan(u8 *DMA, u8 *RequestID, POBNET_GET_VLAN pGetVlan)
{
	OBNET_RSP_GET_VLAN RspGetVlan;	
	u16 RspLength;
	vlan_conf_t vlan_cfg;
	vlan_record_conf_t vlan_rec[2];
	u8 bFirstRecFlag;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_VLAN);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetVlan, 0, sizeof(OBNET_RSP_GET_VLAN));
	RspGetVlan.GetCode = CODE_GET_VLAN;
	
#if 0
	RspGetVlan.RetCode = 0x01;
#else	
#if SWITCH_CHIP_88E6095

	if(eeprom_read(NVRAM_VLAN_CFG_BASE, (u8 *)&vlan_cfg, sizeof(vlan_conf_t)) != I2C_SUCCESS) {
		RspGetVlan.RetCode = 0x01;
	} else {
		if((vlan_cfg.PortNum == 0) || (vlan_cfg.PortNum > MAX_PORT_NUM) || (vlan_cfg.TotalRecordCount > MAX_VLAN_RECORD_COUNT)) {
			memset(&RspGetVlan, 0, sizeof(OBNET_RSP_GET_VLAN));
			RspGetVlan.GetCode = CODE_GET_VLAN;
			RspGetVlan.RetCode = 0x01;
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

				memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetVlan, sizeof(OBNET_RSP_GET_VLAN));
				RspSend(NMS_TxBuffer, MSG_MINSIZE + SWITCH_TAG_LEN);	

			} else if((vlan_cfg.TotalRecordCount > 0) && (vlan_cfg.TotalRecordCount < 3)) {
				RspGetVlan.RetCode = 0x00;
				RspGetVlan.PortNum = vlan_cfg.PortNum;
				RspGetVlan.OpCode = 0x00;
				RspGetVlan.RecordCount = vlan_cfg.TotalRecordCount;

				if(eeprom_read(NVRAM_VLAN_RECORD_CFG_BASE, (u8 *)&(vlan_rec[0]), vlan_cfg.TotalRecordCount * sizeof(vlan_record_conf_t)) != I2C_SUCCESS) {
					goto ErrorVlan;
				} else {
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetVlan, sizeof(OBNET_RSP_GET_VLAN));
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_VLAN)], (u8 *)&(vlan_rec[0]), vlan_cfg.TotalRecordCount * sizeof(vlan_record_conf_t));
					RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_VLAN) + vlan_cfg.TotalRecordCount * sizeof(vlan_record_conf_t);
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
					if(eeprom_read(NVRAM_VLAN_RECORD_CFG_BASE + VlanRecGetStat.OffsetAddress, (u8 *)&(vlan_rec[0]), 2 * sizeof(vlan_record_conf_t)) != I2C_SUCCESS) {
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

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetVlan, sizeof(OBNET_RSP_GET_VLAN));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_VLAN)], (u8 *)&(vlan_rec[0]), 2 * sizeof(vlan_record_conf_t));
						RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_VLAN) + 2 * sizeof(vlan_record_conf_t);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						VlanRecGetStat.RemainCount -= 2;
						VlanRecGetStat.OffsetAddress += 2 * sizeof(vlan_record_conf_t);
						VlanRecGetStat.PacketIndex++;
					}
				} else {
					if(eeprom_read(NVRAM_VLAN_RECORD_CFG_BASE + VlanRecGetStat.OffsetAddress, (u8 *)&(vlan_rec[0]), VlanRecGetStat.RemainCount * sizeof(vlan_record_conf_t)) != I2C_SUCCESS) {
						goto ErrorVlan;
					} else {
						RspGetVlan.GetCode = CODE_GET_VLAN;
						RspGetVlan.RetCode = 0x00;
						RspGetVlan.PortNum = vlan_cfg.PortNum;
						RspGetVlan.OpCode = (0x80 | VlanRecGetStat.PacketIndex);
						RspGetVlan.RecordCount = VlanRecGetStat.RemainCount;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetVlan, sizeof(OBNET_RSP_GET_VLAN));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_VLAN)], (u8 *)&(vlan_rec[0]), VlanRecGetStat.RemainCount * sizeof(vlan_record_conf_t));
						RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_VLAN) + VlanRecGetStat.RemainCount * sizeof(vlan_record_conf_t);
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
#else
	RspGetVlan.RetCode = 0x01;
#endif
#endif

ErrorVlan:
	memset(&RspGetVlan, 0, sizeof(OBNET_RSP_GET_VLAN));
	RspGetVlan.GetCode = CODE_GET_VLAN;
	RspGetVlan.RetCode = 0x01;
	RspSend(NMS_TxBuffer, MSG_MINSIZE + SWITCH_TAG_LEN);
}

void Rsp_SetMcast(u8 *DMA, u8 *RequestID, POBNET_SET_MCAST pMcast)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	mcast_conf_t mcast_cfg;
	mcast_record_conf_t mcast_rec;
	OBNET_MCAST_REC *pMcastRec = (OBNET_MCAST_REC *)((u8 *)pMcast+sizeof(OBNET_REQ_SET_MCAST));
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_MCAST;

#if SWITCH_CHIP_88E6095
	if((pMcast->PortNum > MAX_PORT_NUM) || (pMcast->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(((pMcast->OpCode & 0xC0) >> 6) == 0x0) {
			McastRecSetStat.PacketIndex = 1;
			McastRecSetStat.RecordCount = pMcast->RecordCount;
			McastRecSetStat.OffsetAddress = 0;
		} else if(((pMcast->OpCode & 0xC0) >> 6) == 0x1) {
			McastRecSetStat.PacketIndex = 1;
			McastRecSetStat.RecordCount = pMcast->RecordCount;
			McastRecSetStat.OffsetAddress = 0;
		} else if(((pMcast->OpCode & 0xC0) >> 6) == 0x3) {
			if((pMcast->OpCode & 0x3F) != McastRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				McastRecSetStat.PacketIndex++;
				McastRecSetStat.OffsetAddress = McastRecSetStat.RecordCount * sizeof(mcast_record_conf_t);
				McastRecSetStat.RecordCount += pMcast->RecordCount;
			}
		} else if(((pMcast->OpCode & 0xC0) >> 6) == 0x2) {
			if((pMcast->OpCode & 0x3F) != McastRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				McastRecSetStat.PacketIndex++;
				McastRecSetStat.OffsetAddress = McastRecSetStat.RecordCount * sizeof(mcast_record_conf_t);
				McastRecSetStat.RecordCount += pMcast->RecordCount;
			}
		} 

		/* Update the mcast config */
		mcast_cfg.PortNum = pMcast->PortNum;
		mcast_cfg.TotalRecordCount = McastRecSetStat.RecordCount;
		if(eeprom_page_write(NVRAM_MCAST_CFG_BASE, (u8 *)&mcast_cfg, sizeof(mcast_conf_t)) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;
			goto Response;
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}

		/* Write the mcast record configuration to EEPROM */
		if(eeprom_page_write(NVRAM_MCAST_RECORD_CFG_BASE + McastRecSetStat.OffsetAddress, (u8 *)pMcastRec, pMcast->RecordCount * sizeof(mcast_record_conf_t)) != I2C_SUCCESS) {
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
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}

void Rsp_GetMcast(u8 *DMA, u8 *RequestID, POBNET_GET_MCAST pGetMcast)
{
	OBNET_RSP_GET_MCAST RspGetMcast;	
	u16 RspLength;
	mcast_conf_t mcast_cfg;
	mcast_record_conf_t mcast_rec[2];
	u8 bFirstRecFlag;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MCAST);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetMcast, 0, sizeof(OBNET_RSP_GET_MCAST));
	RspGetMcast.GetCode = CODE_GET_MCAST;

#if 0
	RspGetMcast.RetCode = 0x01;
#else	
#if SWITCH_CHIP_88E6095

	if(eeprom_read(NVRAM_MCAST_CFG_BASE, (u8 *)&mcast_cfg, sizeof(mcast_conf_t)) != I2C_SUCCESS) {
		RspGetMcast.RetCode = 0x01;
	} else {
		if((mcast_cfg.PortNum == 0) || (mcast_cfg.PortNum > MAX_PORT_NUM) || (mcast_cfg.TotalRecordCount > MAX_MCAST_RECORD_COUNT)) {
			memset(&RspGetMcast, 0, sizeof(OBNET_RSP_GET_MCAST));
			RspGetMcast.GetCode = CODE_GET_MCAST;
			RspGetMcast.RetCode = 0x01;
		} else {
			if(pGetMcast->OpCode == 0x00) {
				McastRecGetStat.PacketIndex = 1;
				McastRecGetStat.RemainCount = mcast_cfg.TotalRecordCount;
				McastRecGetStat.OffsetAddress = 0;
				bFirstRecFlag = 1;
			} else {
				bFirstRecFlag = 0;
			}
			
			if(mcast_cfg.TotalRecordCount == 0) {
				RspGetMcast.RetCode = 0x00;
				RspGetMcast.PortNum = mcast_cfg.PortNum;
				RspGetMcast.OpCode = 0x00;
				RspGetMcast.RecordCount = 0x00;

				memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMcast, sizeof(OBNET_RSP_GET_MCAST));
				RspSend(NMS_TxBuffer, MSG_MINSIZE + SWITCH_TAG_LEN);	

			} else if((mcast_cfg.TotalRecordCount > 0) && (mcast_cfg.TotalRecordCount < 3)) {
				RspGetMcast.RetCode = 0x00;
				RspGetMcast.PortNum = mcast_cfg.PortNum;
				RspGetMcast.OpCode = 0x00;
				RspGetMcast.RecordCount = mcast_cfg.TotalRecordCount;

				if(eeprom_read(NVRAM_MCAST_RECORD_CFG_BASE, (u8 *)&(mcast_rec[0]), mcast_cfg.TotalRecordCount * sizeof(mcast_record_conf_t)) != I2C_SUCCESS) {
					goto ErrorMcast;
				} else {
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMcast, sizeof(OBNET_RSP_GET_MCAST));
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MCAST)], (u8 *)&(mcast_rec[0]), mcast_cfg.TotalRecordCount * sizeof(mcast_record_conf_t));
					RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MCAST) + mcast_cfg.TotalRecordCount * sizeof(mcast_record_conf_t);
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
				if(McastRecGetStat.RemainCount >= 2) {
					if(eeprom_read(NVRAM_MCAST_RECORD_CFG_BASE + McastRecGetStat.OffsetAddress, (u8 *)&(mcast_rec[0]), 2 * sizeof(mcast_record_conf_t)) != I2C_SUCCESS) {
						goto ErrorMcast;
					} else {
						RspGetMcast.GetCode = CODE_GET_MCAST;
						RspGetMcast.RetCode = 0x00;
						RspGetMcast.PortNum = mcast_cfg.PortNum;
						if(bFirstRecFlag) {
							bFirstRecFlag = 0;
							RspGetMcast.OpCode = (0x40 | McastRecGetStat.PacketIndex);
						} else {
							if(McastRecGetStat.RemainCount == 2)
								RspGetMcast.OpCode = (0x80 | McastRecGetStat.PacketIndex);
							else
								RspGetMcast.OpCode = (0xc0 | McastRecGetStat.PacketIndex);
						}
						RspGetMcast.RecordCount = 0x02;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMcast, sizeof(OBNET_RSP_GET_MCAST));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MCAST)], (u8 *)&(mcast_rec[0]), 2 * sizeof(mcast_record_conf_t));
						RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MCAST) + 2 * sizeof(mcast_record_conf_t);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						McastRecGetStat.RemainCount -= 2;
						McastRecGetStat.OffsetAddress += 2 * sizeof(mcast_record_conf_t);
						McastRecGetStat.PacketIndex++;
					}
				} else {
					if(eeprom_read(NVRAM_MCAST_RECORD_CFG_BASE + McastRecGetStat.OffsetAddress, (u8 *)&(mcast_rec[0]), McastRecGetStat.RemainCount * sizeof(mcast_record_conf_t)) != I2C_SUCCESS) {
						goto ErrorMcast;
					} else {
						RspGetMcast.GetCode = CODE_GET_MCAST;
						RspGetMcast.RetCode = 0x00;
						RspGetMcast.PortNum = mcast_cfg.PortNum;
						RspGetMcast.OpCode = (0x80 | McastRecGetStat.PacketIndex);
						RspGetMcast.RecordCount = McastRecGetStat.RemainCount;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMcast, sizeof(OBNET_RSP_GET_MCAST));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MCAST)], (u8 *)&(mcast_rec[0]), McastRecGetStat.RemainCount * sizeof(mcast_record_conf_t));
						RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MCAST) + McastRecGetStat.RemainCount * sizeof(mcast_record_conf_t);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						McastRecGetStat.RemainCount = 0;
						McastRecGetStat.OffsetAddress = 0;
					}
				}
			}
		}
	}

	return;
#else
	RspGetMcast.RetCode = 0x01;
#endif
#endif

	/************************************************/
ErrorMcast:
	memset(&RspGetMcast, 0, sizeof(OBNET_RSP_GET_MCAST));
	RspGetMcast.GetCode = CODE_GET_MCAST;
	RspGetMcast.RetCode = 0x01;
	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MCAST);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
	
}

void Rsp_SetPortSecurity(u8 *DMA, u8 *RequestID, POBNET_SET_PORT_SECURITY pPortSecurity)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	port_security_conf_t port_security_cfg;
	port_security_record_conf_t port_security_rec;
	OBNET_PORT_SECURITY_REC *pPortSecurityRec = (OBNET_PORT_SECURITY_REC *)((u8 *)pPortSecurity+sizeof(OBNET_SET_PORT_SECURITY));
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_PORT_SECURITY;

#if SWITCH_CHIP_88E6095
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
		memcpy(port_security_cfg.SecuriyConfig, pPortSecurity->SecuriyConfig, 64);
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
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}

void Rsp_GetPortSecurity(u8 *DMA, u8 *RequestID, POBNET_GET_PORT_SECURITY pGetPortSecurity)
{
	OBNET_RSP_GET_PORT_SECURITY RspGetPortSecurity;	
	u16 RspLength;
	port_security_conf_t port_security_cfg;
	port_security_record_conf_t port_security_rec[2];	
	u8 bFirstRecFlag;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_SECURITY);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetPortSecurity, 0, sizeof(OBNET_RSP_GET_PORT_SECURITY));
	RspGetPortSecurity.GetCode = CODE_GET_PORT_SECURITY;

	/*
	if(eeprom_read(NVRAM_PORT_SECURITY_CFG_BASE, testbuf, 64) == I2C_SUCCESS) {
		buffer_dump_console(testbuf, 64);
	}
	*/
	
#if SWITCH_CHIP_88E6095
	memset(&port_security_cfg, 0, sizeof(port_security_conf_t));
	if(eeprom_read(NVRAM_PORT_SECURITY_CFG_BASE, (u8 *)&(port_security_cfg.PortNum), sizeof(port_security_conf_t)) != I2C_SUCCESS) {
		RspGetPortSecurity.RetCode = 0x01;
		goto error;
	} else {
		if((port_security_cfg.PortNum == 0) || (port_security_cfg.PortNum > MAX_PORT_NUM) || (port_security_cfg.TotalRecordCount > MAX_PORT_SECURITY_RECORD_COUNT)) {
			memset(&RspGetPortSecurity, 0, sizeof(OBNET_RSP_GET_PORT_SECURITY));
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
				memcpy(RspGetPortSecurity.SecuriyConfig, port_security_cfg.SecuriyConfig, sizeof(port_security_conf_t)-2);
				
				memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortSecurity, sizeof(OBNET_RSP_GET_PORT_SECURITY));	
				RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_SECURITY);
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
				memcpy(RspGetPortSecurity.SecuriyConfig, port_security_cfg.SecuriyConfig, sizeof(port_security_conf_t)-2);

				if(eeprom_read(NVRAM_PORT_SECURITY_REC_CFG_BASE, (u8 *)&(port_security_rec[0]), port_security_cfg.TotalRecordCount * sizeof(port_security_record_conf_t)) != I2C_SUCCESS) {
					goto error;
				} else {
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortSecurity, sizeof(OBNET_RSP_GET_PORT_SECURITY));
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_SECURITY)], (u8 *)&(port_security_rec[0]), port_security_cfg.TotalRecordCount * sizeof(port_security_record_conf_t));
					RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_SECURITY) + port_security_cfg.TotalRecordCount * sizeof(port_security_record_conf_t);
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
						memcpy(RspGetPortSecurity.SecuriyConfig, port_security_cfg.SecuriyConfig, sizeof(port_security_conf_t)-2);

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortSecurity, sizeof(OBNET_RSP_GET_PORT_SECURITY));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_SECURITY)], (u8 *)&(port_security_rec[0]), 2 * sizeof(port_security_record_conf_t));
						RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_SECURITY) + 2 * sizeof(port_security_record_conf_t);
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
						memcpy(RspGetPortSecurity.SecuriyConfig, port_security_cfg.SecuriyConfig, sizeof(port_security_conf_t)-2);
						
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortSecurity, sizeof(OBNET_RSP_GET_PORT_SECURITY));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_SECURITY)], (u8 *)&(port_security_rec[0]), SecurityRecGetStat.RemainCount * sizeof(port_security_record_conf_t));
						RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_SECURITY) + SecurityRecGetStat.RemainCount * sizeof(port_security_record_conf_t);
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
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortSecurity, sizeof(OBNET_RSP_GET_PORT_SECURITY));
	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_SECURITY);
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}


void Rsp_GetMacList(u8 *DMA, u8 *RequestID, POBNET_GET_MAC_LIST pGetMacList)
{
	OBNET_RSP_GET_MAC_LIST RspGetMacList;	
	u16 RspLength;
	OBNET_MAC_LIST_REC MacListRec[2];
	u32 hportVec, lportVec;
	u8 PortNum;
	u8 lport, hport;
	u8 bFirstRecFlag;
	int rec_index,j,loop;
#if SWITCH_CHIP_88E6095	
	GT_STATUS stat;
#endif

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MAC_LIST);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetMacList, 0, sizeof(OBNET_RSP_GET_MAC_LIST));
	memset(&MacListRec[0], 0, 2 * sizeof(OBNET_MAC_LIST_REC));
	RspGetMacList.GetCode = CODE_GET_MACLIST;

#if SWITCH_CHIP_88E6095
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
		if((stat = gfdbGetAtuEntryNext(dev,&gMacEntry)) != GT_OK)
			break;
		
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
				//hportVec &= (u32)(dev->validPortVec);
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

	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMacList, sizeof(OBNET_RSP_GET_MAC_LIST));
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MAC_LIST)], (u8 *)&(MacListRec[0]), 2 * sizeof(OBNET_MAC_LIST_REC));
	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MAC_LIST) + 2 * sizeof(OBNET_MAC_LIST_REC);
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
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMacList, sizeof(OBNET_RSP_GET_MAC_LIST));
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MAC_LIST)], (u8 *)&(MacListRec[0]), 2 * sizeof(OBNET_MAC_LIST_REC));
	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MAC_LIST) + 2 * sizeof(OBNET_MAC_LIST_REC);
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
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMacList, sizeof(OBNET_RSP_GET_MAC_LIST));
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MAC_LIST)], (u8 *)&(MacListRec[0]), 2 * sizeof(OBNET_MAC_LIST_REC));
	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_MAC_LIST) + 2 * sizeof(OBNET_MAC_LIST_REC);
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


#if (OB_NMS_PROTOCOL_VERSION == 1)

void Rsp_SetPortStatistics(u8 *DMA, u8 *RequestID, POBNET_SET_PORT_STATISTICS pPortStatistics)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	u8 hport;
#if SWITCH_CHIP_88E6095	
	GT_STATUS status;
#endif

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_PORT_STATISTICS;

#if SWITCH_CHIP_88E6095
	if( (pPortStatistics->PortNum == 0) || \
		(pPortStatistics->PortNum > MAX_PORT_NUM) || \
		(pPortStatistics->CurrentPort > pPortStatistics->PortNum) || \
		(pPortStatistics->CurrentPort == 0) || \
		(pPortStatistics->HistogramMode > 3) || (pPortStatistics->HistogramMode == 0)) {
		
		memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
		RspSet.GetCode = CODE_SET_PORT_STATISTICS;
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_PORT_CFG;
	} else {
		if((status = gstatsSetHistogramMode(dev, (GT_HISTOGRAM_MODE)(pPortStatistics->HistogramMode - 1))) != GT_OK) {
			goto ErrorSetPortStatistics;
		} 
		hport = hal_swif_lport_2_hport(pPortStatistics->CurrentPort);
		if((status = gstatsFlushPort(dev, hport)) != GT_OK) {
			goto ErrorSetPortStatistics;
		} 
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
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
	return;
	
ErrorSetPortStatistics:
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_PORT_STATISTICS;
	RspSet.RetCode = 0x01;
	RspSet.Res = RSP_ERR_INVALID_PORT_CFG;
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);		
}

void Rsp_GetPortStatistics(u8 *DMA, u8 *RequestID, POBNET_GET_PORT_STATISTICS pGetPortStatistics)
{
	OBNET_RSP_GET_PORT_STATISTICS RspGetPortStatistics;	
	u16 RspLength;
	u8 hport;
#if SWITCH_CHIP_88E6095	
	GT_STATUS status;
#endif

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_STATISTICS);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetPortStatistics, 0, sizeof(OBNET_RSP_GET_PORT_STATISTICS));
	RspGetPortStatistics.GetCode = CODE_GET_PORT_STATISTICS;

#if SWITCH_CHIP_88E6095

	if((pGetPortStatistics->PortNum == 0) || (pGetPortStatistics->PortNum > MAX_PORT_NUM) || (pGetPortStatistics->CurrentPort > pGetPortStatistics->PortNum) || (pGetPortStatistics->CurrentPort == 0)) {
		memset(&RspGetPortStatistics, 0, sizeof(OBNET_RSP_GET_PORT_STATISTICS));
		RspGetPortStatistics.GetCode = CODE_GET_PORT_STATISTICS;
		RspGetPortStatistics.RetCode = 0x01;
	} else {
		hport = hal_swif_lport_2_hport(pGetPortStatistics->CurrentPort);
		if((status = gstatsGetPortAllCounters3(dev, hport, (GT_STATS_COUNTER_SET3 *)&(RspGetPortStatistics.StatsCounter))) != GT_OK) {
			goto ErrorGetPortStatistics;
		}
		if((status = gstatsGetHistogramMode(dev, (GT_HISTOGRAM_MODE *)&(RspGetPortStatistics.HistogramMode))) != GT_OK) {
			goto ErrorGetPortStatistics;
		} 
		
		RspGetPortStatistics.RetCode = 0x00;
		RspGetPortStatistics.PortNum = pGetPortStatistics->PortNum;
		RspGetPortStatistics.CurrentPort = pGetPortStatistics->CurrentPort;
	}
	
#else
	RspGetPortStatistics.RetCode = 0x01;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortStatistics, sizeof(OBNET_RSP_GET_PORT_STATISTICS));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	

	return;
	
ErrorGetPortStatistics:
	memset(&RspGetPortStatistics, 0, sizeof(OBNET_RSP_GET_PORT_STATISTICS));
	RspGetPortStatistics.GetCode = CODE_GET_PORT_STATISTICS;
	RspGetPortStatistics.RetCode = 0x01;
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortStatistics, sizeof(OBNET_RSP_GET_PORT_STATISTICS));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}

#endif

void Rsp_SetPortTrunk(u8 *DMA, u8 *RequestID, POBNET_SET_PORT_TRUNK pSetPortTrunk)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	port_trunk_conf_t port_trunk_cfg;
	port_trunk_record_conf_t port_trunk_rec;
	OBNET_PORT_TRUNK_REC *pPortTrunkRec = (OBNET_PORT_TRUNK_REC *)((u8 *)pSetPortTrunk+sizeof(OBNET_REQ_SET_PORT_TRUNK));
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_PORT_TRUNK;

#if SWITCH_CHIP_88E6095
	if((pSetPortTrunk->PortNum > MAX_PORT_NUM) || (pSetPortTrunk->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(((pSetPortTrunk->OpCode & 0xC0) >> 6) == 0x0) {
			PortTrunkRecSetStat.PacketIndex = 1;
			PortTrunkRecSetStat.RecordCount = pSetPortTrunk->RecordCount;
			PortTrunkRecSetStat.OffsetAddress = 0;
		} else if(((pSetPortTrunk->OpCode & 0xC0) >> 6) == 0x1) {
			PortTrunkRecSetStat.PacketIndex = 1;
			PortTrunkRecSetStat.RecordCount = pSetPortTrunk->RecordCount;
			PortTrunkRecSetStat.OffsetAddress = 0;
		} else if(((pSetPortTrunk->OpCode & 0xC0) >> 6) == 0x3) {
			if((pSetPortTrunk->OpCode & 0x3F) != PortTrunkRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				PortTrunkRecSetStat.PacketIndex++;
				PortTrunkRecSetStat.OffsetAddress = PortTrunkRecSetStat.RecordCount * sizeof(port_trunk_record_conf_t);
				PortTrunkRecSetStat.RecordCount += pSetPortTrunk->RecordCount;
			}
		} else if(((pSetPortTrunk->OpCode & 0xC0) >> 6) == 0x2) {
			if((pSetPortTrunk->OpCode & 0x3F) != PortTrunkRecSetStat.PacketIndex+1) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_INVALID_PKT_INDEX;	
				goto Response;
			} else {
				PortTrunkRecSetStat.PacketIndex++;
				PortTrunkRecSetStat.OffsetAddress = PortTrunkRecSetStat.RecordCount * sizeof(port_trunk_record_conf_t);
				PortTrunkRecSetStat.RecordCount += pSetPortTrunk->RecordCount;
			}
		} 

		/* Update the port trunk config */
		port_trunk_cfg.PortNum = pSetPortTrunk->PortNum;
		port_trunk_cfg.TotalRecordCount = PortTrunkRecSetStat.RecordCount;
		if(eeprom_page_write(NVRAM_PORT_TRUNK_CFG_BASE, (u8 *)&port_trunk_cfg, sizeof(port_trunk_conf_t)) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;
			goto Response;
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}

		/* Write the port trunk record configuration to EEPROM */
		if(eeprom_page_write(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + PortTrunkRecSetStat.OffsetAddress, (u8 *)pPortTrunkRec, pSetPortTrunk->RecordCount * sizeof(port_trunk_record_conf_t)) != I2C_SUCCESS) {
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
	if(RspLength == MSG_MINSIZE) {
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	} else {
		RspSend(NMS_TxBuffer, RspLength);	
	}
}

void Rsp_GetPortTrunk(u8 *DMA, u8 *RequestID, POBNET_GET_PORT_TRUNK pGetPortTrunk)
{
	OBNET_RSP_GET_PORT_TRUNK RspGetPortTrunk;	
	u16 RspLength;
	port_trunk_conf_t port_trunk_cfg;
	port_trunk_record_conf_t port_trunk_rec[2];
	u8 bFirstRecFlag;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_TRUNK);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetPortTrunk, 0, sizeof(OBNET_RSP_GET_PORT_TRUNK));
	RspGetPortTrunk.GetCode = CODE_GET_PORT_TRUNK;

#if 0
	RspGetPortTrunk.RetCode = 0x01;
#else
#if SWITCH_CHIP_88E6095

	if(eeprom_read(NVRAM_PORT_TRUNK_CFG_BASE, (u8 *)&port_trunk_cfg, sizeof(port_trunk_conf_t)) != I2C_SUCCESS) {
		RspGetPortTrunk.RetCode = 0x01;
	} else {
		if((port_trunk_cfg.PortNum == 0) || (port_trunk_cfg.PortNum > MAX_PORT_NUM) || (port_trunk_cfg.TotalRecordCount > MAX_PORT_TRUNK_RECORD_COUNT)) {
			memset(&RspGetPortTrunk, 0, sizeof(OBNET_RSP_GET_PORT_TRUNK));
			RspGetPortTrunk.GetCode = CODE_GET_PORT_TRUNK;
			RspGetPortTrunk.RetCode = 0x01;
		} else {
			if(pGetPortTrunk->OpCode == 0x00) {
				PortTrunkRecGetStat.PacketIndex = 1;
				PortTrunkRecGetStat.RemainCount = port_trunk_cfg.TotalRecordCount;
				PortTrunkRecGetStat.OffsetAddress = 0;
				bFirstRecFlag = 1;
			} else {
				bFirstRecFlag = 0;
			}
			
			if(port_trunk_cfg.TotalRecordCount == 0) {
				RspGetPortTrunk.RetCode = 0x00;
				RspGetPortTrunk.PortNum = port_trunk_cfg.PortNum;
				RspGetPortTrunk.OpCode = 0x00;
				RspGetPortTrunk.RecordCount = 0x00;

				memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortTrunk, sizeof(OBNET_RSP_GET_PORT_TRUNK));
				RspSend(NMS_TxBuffer, MSG_MINSIZE + SWITCH_TAG_LEN);	

			} else if((port_trunk_cfg.TotalRecordCount > 0) && (port_trunk_cfg.TotalRecordCount < 3)) {
				RspGetPortTrunk.RetCode = 0x00;
				RspGetPortTrunk.PortNum = port_trunk_cfg.PortNum;
				RspGetPortTrunk.OpCode = 0x00;
				RspGetPortTrunk.RecordCount = port_trunk_cfg.TotalRecordCount;

				if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE, (u8 *)&(port_trunk_rec[0]), port_trunk_cfg.TotalRecordCount * sizeof(port_trunk_record_conf_t)) != I2C_SUCCESS) {
					goto ErrorPortTrunk;
				} else {
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortTrunk, sizeof(OBNET_RSP_GET_PORT_TRUNK));
					memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_TRUNK)], (u8 *)&(port_trunk_rec[0]), port_trunk_cfg.TotalRecordCount * sizeof(port_trunk_record_conf_t));
					RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_TRUNK) + port_trunk_cfg.TotalRecordCount * sizeof(port_trunk_record_conf_t);
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
				if(PortTrunkRecGetStat.RemainCount >= 2) {
					if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + PortTrunkRecGetStat.OffsetAddress, (u8 *)&(port_trunk_rec[0]), 2 * sizeof(port_trunk_record_conf_t)) != I2C_SUCCESS) {
						goto ErrorPortTrunk;
					} else {
						RspGetPortTrunk.GetCode = CODE_GET_PORT_TRUNK;
						RspGetPortTrunk.RetCode = 0x00;
						RspGetPortTrunk.PortNum = port_trunk_cfg.PortNum;
						if(bFirstRecFlag) {
							bFirstRecFlag = 0;
							RspGetPortTrunk.OpCode = (0x40 | PortTrunkRecGetStat.PacketIndex);
						} else {
							if(PortTrunkRecGetStat.RemainCount == 2)
								RspGetPortTrunk.OpCode = (0x80 | PortTrunkRecGetStat.PacketIndex);
							else
								RspGetPortTrunk.OpCode = (0xc0 | PortTrunkRecGetStat.PacketIndex);
						}
						RspGetPortTrunk.RecordCount = 0x02;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortTrunk, sizeof(OBNET_RSP_GET_PORT_TRUNK));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_TRUNK)], (u8 *)&(port_trunk_rec[0]), 2 * sizeof(port_trunk_record_conf_t));
						RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_TRUNK) + 2 * sizeof(port_trunk_record_conf_t);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						PortTrunkRecGetStat.RemainCount -= 2;
						PortTrunkRecGetStat.OffsetAddress += 2 * sizeof(port_trunk_record_conf_t);
						PortTrunkRecGetStat.PacketIndex++;
					}
				} else {
					if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + PortTrunkRecGetStat.OffsetAddress, (u8 *)&(port_trunk_rec[0]), PortTrunkRecGetStat.RemainCount * sizeof(port_trunk_record_conf_t)) != I2C_SUCCESS) {
						goto ErrorPortTrunk;
					} else {
						RspGetPortTrunk.GetCode = CODE_GET_PORT_TRUNK;
						RspGetPortTrunk.RetCode = 0x00;
						RspGetPortTrunk.PortNum = port_trunk_cfg.PortNum;
						RspGetPortTrunk.OpCode = (0x80 | PortTrunkRecGetStat.PacketIndex);
						RspGetPortTrunk.RecordCount = PortTrunkRecGetStat.RemainCount;

						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetPortTrunk, sizeof(OBNET_RSP_GET_PORT_TRUNK));
						memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_TRUNK)], (u8 *)&(port_trunk_rec[0]), PortTrunkRecGetStat.RemainCount * sizeof(port_trunk_record_conf_t));
						RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_TRUNK) + PortTrunkRecGetStat.RemainCount * sizeof(port_trunk_record_conf_t);
						if (RspLength < MSG_MINSIZE)
							RspLength = MSG_MINSIZE;
						PrepareEtherHead(DMA);
						PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
						if(RspLength == MSG_MINSIZE)
							RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
						else
							RspSend(NMS_TxBuffer, RspLength);
						PortTrunkRecGetStat.RemainCount = 0;
						PortTrunkRecGetStat.OffsetAddress = 0;
					}
				}
			}
		}
	}

	return;
#else
	RspGetPortTrunk.RetCode = 0x01;
#endif
#endif

	/************************************************/
ErrorPortTrunk:
	memset(&RspGetPortTrunk, 0, sizeof(OBNET_RSP_GET_PORT_TRUNK));
	RspGetPortTrunk.GetCode = CODE_GET_PORT_TRUNK;
	RspGetPortTrunk.RetCode = 0x01;
	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_PORT_TRUNK);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
	
}

void Rsp_GetNeighbor(u8 *DMA, u8 *RequestID, POBNET_GET_NEIGHBOR pGetNeighbor)
{
	
}

#endif

