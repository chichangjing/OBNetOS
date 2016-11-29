

#ifndef __NMS_RING_H__
#define __NMS_RING_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"
#include "conf_ring.h"

#define MAX_RING 8
typedef struct obnet_set_ring
{
	u8	GetCode;
	u8	RetCode;
	u8	RingNum;
	u8	RingGate[4];
	u8	RingConfig[MAX_RING * 16];
} OBNET_SET_RING_CONFIG, *POBNET_SET_RING_CONFIG, OBNET_RSP_RING_CONFIG;

typedef struct obnet_ring_port_info
{
	u8	Flag;
	u8	DevPortNum;
	u8	ExtNeighborMac[6];
	u8	ExtNeighborPortNum;
}OBNET_RING_PORT_INFO, *POBNET_RING_PORT_INFO;

typedef struct obnet_rsp_topo
{
  	u8	GetCode;
	u8	RetCode;
	u8  Res;
    u8  SwitchType[8];
    u8  PowerNum;
    u8  PowerVol[4];
    u8  RingPairNum;
	OBNET_RING_PORT_INFO Info[2];
}OBNET_RSP_TOPO, *POBNET_RSP_TOPO;

void GetRingPortStatus(u8 RingLogicPort, POBNET_RING_PORT_INFO pRingPortInfo);
void Rsp_SetRingConfig(u8 *DMA, u8 *RequestID,u8 *RingConfig);
void Rsp_GetRingConfig(u8 *DMA, u8 *RequestID);
void Rsp_Topo(u8 *DMA, u8 *RequestID);


#ifdef __cplusplus
}
#endif

#endif

