
#ifndef __NMS_SYS_H__
#define __NMS_SYS_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"

typedef struct obnet_req_setmac
{
	u8	GetCode;
	u8	RetCode;
	u8	Res;
	u8	SwitchMac[MAC_LEN];
}OBNET_SET_MAC, *POBNET_SET_MAC;

typedef struct obnet_rsp_nameid
{
	u8	GetCode;
	u8	RetCode;
	u8	Res;
	u8	Name[48];
	u8	SwitchIndex[16];
}OBNET_RSP_NAMEID, *POBNET_RSP_NAMEID, OBNET_SET_NAMEID, *POBNET_SET_NAMEID;

typedef struct obnet_rsp_version
{
	u8	GetCode;
	u8	RetCode;
	u8	Pad;
	u8	SystemVersion[32];
	u8	HardwareVersion[20];
	u8	SoftwareVersion[20];
}OBNET_RSP_VERSION, *POBNET_RSP_VERSION, OBNET_SET_VERSION, *POBNET_SET_VERSION;

typedef struct obnet_set_ip
{
	u8	GetCode;
	u8	RetCode;
	u8	Res;
	u8	IP[4];
	u8	Mask[4];
	u8	Gateway[4];
}OBNET_REQ_SET_IP, *POBNET_REQ_SET_IP, OBNET_RSP_GET_IP, *POBNET_RSP_GET_IP;

void Rsp_SetMac(u8 *DMA, u8 *RequestID, POBNET_SET_MAC pSetMac);
void Rsp_SetNameID(u8 *DMA, u8 *RequestID, POBNET_SET_NAMEID pSetNameID);
void Rsp_GetNameID(u8 *DMA, u8 *RequestID);
void Rsp_SetVersion(u8 *DMA, u8 *RequestID, POBNET_SET_VERSION pSetVersion);
void Rsp_GetVersion(u8 *DMA, u8 *RequestID)	;
void Rsp_SetIP(u8 *DMA, u8 *RequestID, POBNET_REQ_SET_IP pSetIP);
void Rsp_GetIP(u8 *DMA, u8 *RequestID);
void Rsp_Reboot(u8 *DMA, u8 *RequestID);

#ifdef __cplusplus
}
#endif

#endif

