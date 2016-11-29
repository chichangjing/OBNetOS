
/*************************************************************
 * Filename     : nms_global.c
 * Description  : API for NMS interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#include "mconfig.h"

/* Standard includes */
#include "string.h"

/* LwIP include */
#include "lwip/netif.h"

/* BSP includes */
#include "stm32f2xx.h"
#include "soft_i2c.h"

/* Other includes */
#include "conf_comm.h"
#include "conf_map.h"
#include "conf_global.h"

#include "nms_comm.h"
#include "nms_if.h"
#include "nms_global.h"

extern u8 NMS_TxBuffer[];

void nms_rsp_get_global_config(u8 *DMA, u8 *RequestID)
{
	const u8 temp_mac1[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	const u8 temp_mac2[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	obnet_rsp_get_global_config RspGlobalConfig;	
	u16 RspLength;
	u8 TrapCfgCheckFlag, KsigCfgCheckFlag;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	memset(&RspGlobalConfig, 0, sizeof(obnet_rsp_get_global_config));
	RspGlobalConfig.GetCode = CODE_GLOBAL_CONFIG;
	RspGlobalConfig.RetCode = 0x00;
	RspGlobalConfig.Res = 0x00;

	if(eeprom_read(NVRAM_TRAP_BASE, (u8 *)&(RspGlobalConfig.TrapCfg), sizeof(tTrapConfig)) != I2C_SUCCESS) {
		RspGlobalConfig.RetCode = 0x01;
		goto Response;
	}
	if(eeprom_read(NVRAM_ALARM_KIN_BASE, (u8 *)&(RspGlobalConfig.KinAlarmCfg), sizeof(tKinAlarmConfig)) != I2C_SUCCESS) {
		RspGlobalConfig.RetCode = 0x01;
		goto Response;
	} 

	/* Check trap configuration */
	TrapCfgCheckFlag = 0;
	if((memcmp(RspGlobalConfig.TrapCfg.TrapServerMac, temp_mac1, 6) == 0) || (memcmp(RspGlobalConfig.TrapCfg.TrapServerMac, temp_mac2, 6) == 0)) 
		TrapCfgCheckFlag = 1;
	if((RspGlobalConfig.TrapCfg.TrapEnable != 0x00) && (RspGlobalConfig.TrapCfg.TrapEnable != 0x01)) 
		TrapCfgCheckFlag = 1;
	if(TrapCfgCheckFlag)
		memset(&(RspGlobalConfig.TrapCfg), 0, sizeof(tTrapConfig));

	/* Check Ksig input alarm configuration */
	KsigCfgCheckFlag = 0;
	if((RspGlobalConfig.KinAlarmCfg.FeatureEnable != 0x00) && (RspGlobalConfig.KinAlarmCfg.FeatureEnable != 0x01)) 
		KsigCfgCheckFlag = 1;
	else {
		if(RspGlobalConfig.KinAlarmCfg.SampleCycle > 100)
			RspGlobalConfig.KinAlarmCfg.SampleCycle = KSIG_IN_ALARM_DEFAULT_SAMPLE_CYCLE;

		if(RspGlobalConfig.KinAlarmCfg.RemoveJitterEnable == 0x01) {
			if(RspGlobalConfig.KinAlarmCfg.JitterProbeTime > 200)
				RspGlobalConfig.KinAlarmCfg.JitterProbeTime = KSIG_IN_ALARM_DEFAULT_JITTER_TIME;
			
			if(RspGlobalConfig.KinAlarmCfg.SampleCycle >= RspGlobalConfig.KinAlarmCfg.JitterProbeTime) {
				RspGlobalConfig.KinAlarmCfg.SampleCycle = KSIG_IN_ALARM_DEFAULT_SAMPLE_CYCLE;
				RspGlobalConfig.KinAlarmCfg.JitterProbeTime = KSIG_IN_ALARM_DEFAULT_JITTER_TIME;
			}	
		}

		if(RspGlobalConfig.KinAlarmCfg.ChanConfig.ChanNum > MAX_KSIG_IN_ALARM_CHANNEL_NUM)
			KsigCfgCheckFlag = 1;
	}
	
	if(KsigCfgCheckFlag) {
		memset(&(RspGlobalConfig.KinAlarmCfg), 0, sizeof(tKinAlarmConfig));
		RspGlobalConfig.KinAlarmCfg.FeatureEnable = 0x00;
		RspGlobalConfig.KinAlarmCfg.SampleCycle = KSIG_IN_ALARM_DEFAULT_SAMPLE_CYCLE;
        RspGlobalConfig.KinAlarmCfg.RemoveJitterEnable = 0x00;
		RspGlobalConfig.KinAlarmCfg.JitterProbeTime = KSIG_IN_ALARM_DEFAULT_JITTER_TIME;
		RspGlobalConfig.KinAlarmCfg.WorkMode = KSIG_IN_ALARM_WORK_MODE_UDP;
		RspGlobalConfig.KinAlarmCfg.ChanConfig.ChanNum = 0;
	}

Response:	
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGlobalConfig, sizeof(obnet_rsp_get_global_config));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_global_config);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}

void nms_rsp_set_global_config(u8 *DMA, u8 *RequestID, obnet_set_global_config *pGlobalConfig)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
		
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_GLOBAL_CFG;

	if(eeprom_page_write(NVRAM_TRAP_BASE, (u8 *)&(pGlobalConfig->TrapCfg), sizeof(tTrapConfig)) != I2C_SUCCESS) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
		goto Response;
	}

	if(eeprom_page_write(NVRAM_ALARM_KIN_BASE, (u8 *)&(pGlobalConfig->KinAlarmCfg), sizeof(tKinAlarmConfig)) != I2C_SUCCESS) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
		goto Response;
	}

Response:
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


