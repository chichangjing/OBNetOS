
#ifndef __NMS_PORT_H__
#define __NMS_PORT_H__

#ifdef __cplusplus
 extern "C" {
#endif
#include "mconfig.h"
#include "stm32f2xx.h"

#define MAX_PORT_CONFIG_REC	18

typedef struct {
  	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	PortStatus[MAX_PORT_NUM];
}OBNET_RSP_PORT_STATUS, *POBNET_RSP_PORT_STATUS;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	PortConfig[MAX_PORT_NUM * 4];
}OBNET_RSP_PORT_CONFIG, *POBNET_RSP_PORT_CONFIG, OBNET_SET_PORT_CONFIG, *POBNET_SET_PORT_CONFIG;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	Res;
	u8	OpCode;
	u8	ItemsInData;
	u8	PortConfig[MAX_PORT_NUM * 4];
}OBNET_RSP_PORT_CONFIG2, *POBNET_RSP_PORT_CONFIG2, OBNET_SET_PORT_CONFIG2, *POBNET_SET_PORT_CONFIG2;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	OpCode;
} OBNET_REQ_GET_PORT_CONFIG, *POBNET_GET_PORT_CONFIG;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	SourcePort[MAX_PORT_NUM * 4];
}OBNET_RSP_GET_MIRROR, *POBNET_SET_MIRROR, OBNET_SET_MIRROR;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	RateControl[MAX_PORT_NUM * 2];
}OBNET_SET_RATE, *POBNET_SET_RATE, OBNET_RSP_GET_RATE;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	Res;
	u8	CosMapping[2];
	u8	TosMapping[16];
	u8	QosFlag[2];
	u8	PortNum;
	u8	PortQosConfig[MAX_PORT_NUM];
}OBNET_REQ_SET_QOS, *POBNET_SET_QOS, OBNET_RSP_GET_QOS;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	OutPortSel[2 * MAX_PORT_NUM];
}OBNET_REQ_SET_ISOLATION, *POBNET_SET_ISOLATION, OBNET_RSP_GET_ISOLATION;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	VLanSetting[2 * MAX_PORT_NUM];
}OBNET_REQ_SET_PORTVLAN, *POBNET_SET_PORTVLAN, OBNET_RSP_GET_PORTVLAN;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	VLanID[2];
}OBNET_REQ_SET_ADM_VLAN, *POBNET_SET_ADM_VLAN, OBNET_RSP_GET_ADM_VLAN;

typedef struct {
	u8	VLanID[2];
	u8	VLanName[8];
	u8	VLanSetting[3];
}OBNET_VLAN_REC;

typedef struct {
	u8	Mac[6];
	u8	Member[2];
}OBNET_MCAST_REC;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	OpCode;
	u8	RecordCount;
}OBNET_REQ_SET_VLAN, *POBNET_SET_VLAN, OBNET_REQ_SET_MCAST, *POBNET_SET_MCAST, OBNET_RSP_GET_VLAN, OBNET_RSP_GET_MCAST, OBNET_RSP_GET_NEIGHBOR, OBNET_RSP_PORT_CONFIG_EX, OBNET_RSP_RING_CONFIG_EX, OBNET_RSP_TOPO_EX;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	OpCode;
}OBNET_REQ_GET_VLAN, *POBNET_GET_VLAN, OBNET_REQ_GET_MCAST, *POBNET_GET_MCAST, OBNET_REQ_GET_PORT_SECURITY, *POBNET_GET_PORT_SECURITY, OBNET_REQ_GET_NEIGHBOR, OBNET_REQ_GET_RING_CONFIG, OBNET_REQ_TOPO_EX;

typedef struct {
	u8	PacketIndex;
	u8	RecordCount;
	u16	OffsetAddress;
} record_set_stat_t;

typedef struct {
	u8	PacketIndex;
	u8	RemainCount;
	u16	OffsetAddress;
} record_get_stat_t;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	OpCode;
	u8	RecordCount;
	u8	SecuriyConfig[32 * 2];
}OBNET_SET_PORT_SECURITY, *POBNET_SET_PORT_SECURITY, OBNET_RSP_GET_PORT_SECURITY;

typedef struct {
	u8	Index;
	u8	MacAddr[6];
	u8	PortVec[4];
	u8	Priority;
	u8	Res;
}OBNET_PORT_SECURITY_REC;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	OpCode;
}OBNET_GET_MAC_LIST, *POBNET_GET_MAC_LIST;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	OpCode;
	u8	RecordCount;
}OBNET_REQ_SET_MAC_LIST, *POBNET_SET_MAC_LIST, OBNET_RSP_GET_MAC_LIST;

typedef struct {
	u8	Index;
	u8	Priority;
	u8	MacAddr[6];
	u8	PortVec[4];
}OBNET_MAC_LIST_REC;

typedef struct {
	u8	PortNum;
	u8	LastMacRecFlag;
	u8	PacketIndex;
	u8	RecSendCount;
} get_maclist_stat_t;

typedef struct {
	u8	TrunkId;
	u8	MemberVec[4];
} OBNET_PORT_TRUNK_REC;

typedef struct {
	u8	GetCode; 
	u8	RetCode;
	u8	PortNum;
	u8	OpCode;
	u8	RecordCount;
} OBNET_REQ_SET_PORT_TRUNK, *POBNET_SET_PORT_TRUNK, OBNET_RSP_GET_PORT_TRUNK;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	OpCode;
} OBNET_REQ_GET_PORT_TRUNK, *POBNET_GET_PORT_TRUNK, *POBNET_GET_NEIGHBOR;

typedef __packed struct {
	u32	InGoodOctetsLo;	/* offset 0 */
	u32	InGoodOctetsHi;	/* offset 1, not supported by 88E6065 */
	u32	InBadOctets;	/* offset 2 */
	u32	OutFCSErr;		/* offset 3 */
	u32	InUnicasts;		/* offset 4 */
	u32	Deferred;		/* offset 5 */
	u32	InBroadcasts;	/* offset 6 */
	u32	InMulticasts;	/* offset 7 */
	u32	Octets64;		/* 64 Octets, offset 8 */
	u32	Octets127;		/* 65 to 127 Octets, offset 9 */
	u32	Octets255;		/* 128 to 255 Octets, offset 10 */
	u32	Octets511;		/* 256 to 511 Octets, offset 11 */
	u32	Octets1023;		/* 512 to 1023 Octets, offset 12 */
	u32	OctetsMax;		/* 1024 to Max Octets, offset 13 */
	u32	OutOctetsLo;	/* offset 14 */
	u32	OutOctetsHi;	/* offset 15, not supported by 88E6065 */
	u32	OutUnicasts;	/* offset 16 */
	u32	Excessive;		/* offset 17 */
	u32	OutMulticasts;	/* offset 18 */
	u32	OutBroadcasts;	/* offset 19 */
	u32	Single;			/* offset 20 */

	u32	OutPause;		/* offset 21 */
	u32	InPause;		/* offset 22 */
	u32	Multiple;		/* offset 23 */
	u32	Undersize;		/* offset 24 */
	u32	Fragments;		/* offset 25 */
	u32	Oversize;		/* offset 26 */
	u32	Jabber;			/* offset 27 */
	u32	InMACRcvErr;	/* offset 28 */
	u32	InFCSErr;		/* offset 29 */
	u32	Collisions;		/* offset 30 */
	u32	Late;			/* offset 31 */
} StatsCounter_t;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	CurrentPort;
	u8	HistogramMode;
} OBNET_GET_PORT_STATISTICS, *POBNET_GET_PORT_STATISTICS, OBNET_SET_PORT_STATISTICS, *POBNET_SET_PORT_STATISTICS;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	CurrentPort;
	u8	HistogramMode;
	StatsCounter_t StatsCounter;
} OBNET_RSP_GET_PORT_STATISTICS;


void Rsp_GetPortStatus(u8 *DMA, u8 *RequestID);
void Rsp_SetPortConfig(u8 *DMA, u8 *RequestID, POBNET_SET_PORT_CONFIG pPortConfig);
void Rsp_GetPortConfig(u8 *DMA, u8 *RequestID, POBNET_GET_PORT_CONFIG pGetPortConfig);
void Rsp_SetPortMirror(u8 *DMA, u8 *RequestID, POBNET_SET_MIRROR pPortMirror);
void Rsp_GetPortMirror(u8 *DMA, u8 *RequestID);
void Rsp_SetPortRate(u8 *DMA, u8 *RequestID, POBNET_SET_RATE pPortRate);
void Rsp_GetPortRate(u8 *DMA, u8 *RequestID);
void Rsp_SetQos(u8 *DMA, u8 *RequestID, POBNET_SET_QOS pQos);
void Rsp_GetQos(u8 *DMA, u8 *RequestID);
void Rsp_SetPortIsolation(u8 *DMA, u8 *RequestID, POBNET_SET_ISOLATION pPortIsolation);
void Rsp_GetPortIsolation(u8 *DMA, u8 *RequestID);
void Rsp_SetPortVlan(u8 *DMA, u8 *RequestID, POBNET_SET_PORTVLAN pPortVlan);
void Rsp_GetPortVlan(u8 *DMA, u8 *RequestID);
void Rsp_SetAdmVlan(u8 *DMA, u8 *RequestID, POBNET_SET_ADM_VLAN pAdmVlan);
void Rsp_GetAdmVlan(u8 *DMA, u8 *RequestID);
void Rsp_SetVlan(u8 *DMA, u8 *RequestID, POBNET_SET_VLAN pVlan);
void Rsp_GetVlan(u8 *DMA, u8 *RequestID, POBNET_GET_VLAN pGetVlan);
void Rsp_SetMcast(u8 *DMA, u8 *RequestID, POBNET_SET_MCAST pMcast);
void Rsp_GetMcast(u8 *DMA, u8 *RequestID, POBNET_GET_MCAST pGetMcast);
void Rsp_SetPortSecurity(u8 *DMA, u8 *RequestID, POBNET_SET_PORT_SECURITY pPortSecurity);
void Rsp_GetPortSecurity(u8 *DMA, u8 *RequestID, POBNET_GET_PORT_SECURITY pGetPortSecurity);
void Rsp_GetMacList(u8 *DMA, u8 *RequestID, POBNET_GET_MAC_LIST pGetMacList);
void Rsp_SetPortStatistics(u8 *DMA, u8 *RequestID, POBNET_SET_PORT_STATISTICS pPortStatistics);
void Rsp_GetPortStatistics(u8 *DMA, u8 *RequestID, POBNET_GET_PORT_STATISTICS pGetPortStatistics);
void Rsp_SetPortTrunk(u8 *DMA, u8 *RequestID, POBNET_SET_PORT_TRUNK pSetPortTrunk);
void Rsp_GetPortTrunk(u8 *DMA, u8 *RequestID, POBNET_GET_PORT_TRUNK pGetPortTrunk);

#ifdef __cplusplus
}
#endif

#endif


