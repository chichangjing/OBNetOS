
/*************************************************************
 * Filename     : nms_sys.c
 * Description  : API for NMS interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#include "mconfig.h"

#if MODULE_OBNMS

/* Standard includes. */
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* BSP includes */
#include "stm32f2xx.h"

/* Other includes */
#include "nms_comm.h"
#include "nms_if.h"
#include "nms_sys.h"

#include "conf_comm.h"
#include "conf_sys.h"


extern u8 NMS_TxBuffer[];

void Rsp_SetMac(u8 *DMA, u8 *RequestID, POBNET_SET_MAC pSetMac)
{
	OBNET_SET_RSP RspSet;
	u16 RspLength;
	u8	retVal;
	int i2cRet;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	RspSet.GetCode = CODE_SET_MAC;
	/************************************************/
	/* To add */
	i2cRet = conf_set_mac_address(pSetMac->SwitchMac);
	if(i2cRet == CONF_ERR_INVALID_MAC) {
		RspSet.RetCode = 0x01;
		RspSet.Res = 0x10;
	} else if(i2cRet == CONF_ERR_I2C) {
		RspSet.RetCode = 0x01;
		RspSet.Res = 0x0F;	
	} else {
		RspSet.RetCode = 0x00;
		RspSet.Res = 0x00;	
	}
	/************************************************/
		
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);
}


void Rsp_SetNameID(u8 *DMA, u8 *RequestID, POBNET_SET_NAMEID pSetNameID)
{
	OBNET_SET_RSP RspSet;	
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
	RspSet.GetCode = CODE_SET_NAMEID;
	/************************************************/
	/* To add */
	Ret = conf_set_name_id(pSetNameID->Name, pSetNameID->SwitchIndex);
	if(Ret == CONF_ERR_OVERLENGTH) {
		RspSet.RetCode = 0x01;
		RspSet.Res = 0x11;
	} else if(Ret == CONF_ERR_I2C) {
		RspSet.RetCode = 0x01;
		RspSet.Res = 0x0F;	
	} else {
		RspSet.RetCode = 0x00;
		RspSet.Res = 0x00;	
	}
	/************************************************/
		
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);
}

void Rsp_GetNameID(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_NAMEID RspGetNameID;	
	u16 RspLength;
	u8	retVal;
	int Ret;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_NAMEID);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetNameID, 0, sizeof(OBNET_RSP_NAMEID));
	RspGetNameID.GetCode = CODE_GET_NAMEID;

	Ret = conf_get_name_id(RspGetNameID.Name, RspGetNameID.SwitchIndex);
	if(Ret == CONF_ERR_NONE) {
		RspGetNameID.RetCode = 0x00;
	} else {
		RspGetNameID.RetCode = 0x01;
	}

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetNameID, sizeof(OBNET_RSP_NAMEID));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}

void Rsp_SetVersion(u8 *DMA, u8 *RequestID, POBNET_SET_VERSION pSetVersion)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	int Ret;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	RspSet.GetCode = CODE_SET_VERSION;
	/************************************************/
	/* To add */
	Ret = conf_set_version((u8 *)&(pSetVersion->SystemVersion[0]));
	if(Ret == CONF_ERR_NONE) {
		RspSet.RetCode = 0x00;
		RspSet.Res = 0x00;	
	} else {
		RspSet.RetCode = 0x01;
		RspSet.Res = 0x0F;	
	}

	/************************************************/
		
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);
}

void Rsp_GetVersion(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_VERSION RspGetVersion;	
	u16 RspLength;
	int Ret;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_VERSION);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetVersion, 0, sizeof(OBNET_RSP_VERSION));
	RspGetVersion.GetCode = CODE_GET_VERSION;
	Ret = conf_get_version((u8 *)&(RspGetVersion.SystemVersion[0]));
	if(Ret == CONF_ERR_NONE) {
		RspGetVersion.RetCode = 0x00;
		RspGetVersion.Pad = 0x00;
	} else {
		memset(&RspGetVersion, 0, sizeof(OBNET_RSP_VERSION));
		RspGetVersion.RetCode = 0x01;
		RspGetVersion.Pad = 0x0F;		
	}
	
	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetVersion, sizeof(OBNET_RSP_VERSION));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}

void Rsp_SetIP(u8 *DMA, u8 *RequestID, POBNET_REQ_SET_IP pSetIP)
{
	OBNET_SET_RSP RspSet;	
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
	RspSet.GetCode = CODE_SET_IP;
	/************************************************/
	/* To add */
	if(conf_set_ip_info(pSetIP->IP) != CONF_ERR_NONE) {
		RspSet.RetCode = 0x01;
		RspSet.Res = 0x0F;
	} else {
		RspSet.RetCode = 0x00;
		RspSet.Res = 0x00;
	}

	/************************************************/
		
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);
}

void Rsp_GetIP(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_GET_IP RspGetIP;	
	u16 RspLength;
	u8	retVal;
	int Ret;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_IP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetIP, 0, sizeof(OBNET_RSP_GET_IP));
	RspGetIP.GetCode = CODE_GET_IP;

	if(conf_get_ip_info((u8 *)&(RspGetIP.IP[0])) != CONF_ERR_NONE) {
		RspGetIP.RetCode = 0x01;
		RspGetIP.Res = 0x00;
	} else {
		RspGetIP.RetCode = 0x00;
		RspGetIP.Res = 0x00;
	}

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetIP, sizeof(OBNET_RSP_GET_IP));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}

void Rsp_Reboot(u8 *DMA, u8 *RequestID)
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
	RspSet.GetCode = CODE_REBOOT;
	/**************************************************/
	/* To add */
	RspSet.RetCode = 0x00;
	RspSet.Res = 0x00;
	/**************************************************/
	
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);
	vTaskDelay(5000);
	NVIC_SystemReset();
}

#endif

