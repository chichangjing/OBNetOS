
/*************************************************************
 * Filename     : nms_signal.c
 * Description  : API for NMS interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/

/* Standard includes */
#include "string.h"

/* LwIP includes */
#include "lwip/inet.h"

/* Other includes */
#include "nms_comm.h"
#include "nms_if.h"
#include "nms_signal.h"
#include "conf_comm.h"
#include "conf_signal.h"
#include "mconfig.h"

extern u8 NMS_TxBuffer[];

void Rsp_GetSignalConfig(u8 *DMA, u8 *RequestID)
{
    OBNET_RSP_GET_SIGNAL RspGetSig;	
	u16 RspLength;
	u8	retVal;
    signal_cfg_t sigcfg;
	int Ret;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_SIGNAL);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
    
    /* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetSig, 0, sizeof(OBNET_RSP_GET_SIGNAL));
	RspGetSig.GetCode = CODE_GET_SIGNAL;

#if 0//MODULE_SIGNAL
	if(signal_cfg_fetch(&sigcfg) != CONF_ERR_NONE) {
		RspGetSig.RetCode = 0x01;
	} else {
		RspGetSig.RetCode = 0x00;
	}
    
    RspGetSig.enable = sigcfg.enable;
    RspGetSig.AlarmType = sigcfg.AlarmType;
    RspGetSig.AlarmMode = sigcfg.AlarmMode;
    RspGetSig.ip = sigcfg.ip;
    RspGetSig.port = htons(sigcfg.port);
    RspGetSig.time = htons(sigcfg.time);
#else
	RspGetSig.RetCode = 0x01;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetSig, sizeof(OBNET_RSP_GET_SIGNAL));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	  
    
}

void Rsp_SetSignalConfig(u8 *DMA, u8 *RequestID, u8 *SignalCfg)
{
    POBNET_REQ_SET_SIGNAL pSignalCfg = (POBNET_REQ_SET_SIGNAL)SignalCfg;
	u16 RspLength;
    OBNET_SET_RSP RspSet;
    signal_cfg_t sigcfg;
	int ret;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/**************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_SIGNAL;
	
#if MODULE_SIGNAL
    sigcfg.enable = pSignalCfg->enable;
    sigcfg.AlarmType = pSignalCfg->AlarmType;
    sigcfg.AlarmMode = pSignalCfg->AlarmMode;
    sigcfg.ip = pSignalCfg->ip;
    sigcfg.port = ntohs(pSignalCfg->port);
    sigcfg.time = ntohs(pSignalCfg->time);
    
    if((ret = set_signal_cfg(&sigcfg)) == CONF_ERR_NONE){
         RspSet.RetCode = 0x00;
         RspSet.Res = 0x00;
    }else{
         RspSet.RetCode = 0x01;
         RspSet.Res = 0x00;
    }  
#else
     RspSet.RetCode = 0x01;
     RspSet.Res = RSP_ERR_FEATURE_NOT_SUPPORT;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
    
}
