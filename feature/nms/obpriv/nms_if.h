
#ifndef __NMS_IF_H__
#define __NMS_IF_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"

void RspSend(u8 *txBuffer, u16 len);
void PrepareEtherHead(u8 *DMA);
void PrepareOBHead(u8 MessageType, u16 MessageLength, u8 *RequestID);

void Rsp_Paging(u8 *DMA, u8 *RequestID);
void Rsp_LoadStart(u8 *DMA, u8 *RequestID);
void Rsp_LoadComplete(u8 *DMA, u8 *RequestID);

void NMS_Init(void);
void NMS_Msg_Receive(u8 *rxBuf, u16 rxLen);
void NMS_Task(void *arg);
void NmsMsgDump(const char *string, u8 *buf, int len, int parseFlag);

#ifdef __cplusplus
}
#endif

#endif

