
#ifndef _NMS_SIGNAL_H__
#define _NMS_SIGNAL_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cpluscplus */
    
/* BSP includes */    
#include "stm32f2xx.h"

typedef __packed struct obnet_set_signal
{
	u8	GetCode;
	u8	RetCode;
    u8  enable;
    u8  AlarmType;
    u8  AlarmMode;
    u32 ip;
    u16 port;
    u16 time;
}OBNET_REQ_SET_SIGNAL, *POBNET_REQ_SET_SIGNAL, OBNET_RSP_GET_SIGNAL, *POBNET_RSP_GET_SIGNAL;

typedef struct obnet_get_signal
{
	u8	GetCode;
	u8	RetCode;
} OBNET_REQ_GET_SIGNAL, *POBNET_REQ_GET_SIGNAL;

void Rsp_GetSignalConfig(u8 *DMA, u8 *RequestID);
void Rsp_SetSignalConfig(u8 *DMA, u8 *RequestID, u8 *SignalCfg);

#ifdef __cpluscplus    
}
#endif /* __cpluscplus */

#endif /* _NMS_SIGNAL_H */