

#ifndef _HAL_SWIF_MESSAGE_H_
#define _HAL_SWIF_MESSAGE_H_

#include "mconfig.h"

#if SWITCH_CHIP_88E6095
typedef struct {
	uint8	DA[6];
	uint8	SA[6];
	uint8	SwitchTag[4];
	uint8	Type[2];
} hal_ether_header_t;
#define HAL_SWITCH_TAG_LEN		4
#define HAL_ETHER_HEAD_SIZE		18

#elif SWITCH_CHIP_BCM53101
typedef struct {
	uint8	DA[6];
	uint8	SA[6];
	uint8	SwitchTag[4];
	uint8	Type[2];
} hal_ether_header_t;
#define HAL_SWITCH_TAG_LEN		4
#define HAL_ETHER_HEAD_SIZE		18

#elif SWITCH_CHIP_BCM53286
typedef struct {
	uint8	SwitchTag[8];
	uint8	DA[6];
	uint8	SA[6];
	uint8	Type[2];
} hal_ether_header_t;
#define HAL_SWITCH_TAG_LEN		8
#define HAL_ETHER_HEAD_SIZE		22

#elif SWITCH_CHIP_BCM5396
typedef struct {
	uint8	DA[6];
	uint8	SA[6];
	uint8	SwitchTag[6];
	uint8	Type[2];
} hal_ether_header_t;
#define HAL_SWITCH_TAG_LEN		6
#define HAL_ETHER_HEAD_SIZE		20

#elif SWITCH_CHIP_BCM53115
typedef struct {
	uint8	DA[6];
	uint8	SA[6];
	uint8	SwitchTag[4];
	uint8	Type[2];
} hal_ether_header_t;
#define HAL_SWITCH_TAG_LEN		4
#define HAL_ETHER_HEAD_SIZE		18

#endif

typedef struct {
	uint8	OrgCode[3];
	uint8	ProtoType[2];
	uint8	Version;
	uint8	MessageType;
	uint8	MessageLength[2];
	uint8	RequestID[2];
	uint8	SwitchMac[6];
} hal_obnet_header_t;

#define HAL_OBNET_HEAD_SIZE		17
#define HAL_PAYLOAD_OFFSET		(HAL_ETHER_HEAD_SIZE + HAL_OBNET_HEAD_SIZE)

/* IMP Ingress (from CPU) Tag mode */
typedef enum {
	MODE_NORMAL		= 0,
	MODE_DIRECT		= 1
} HAL_IMP_TAG_MODE;

/* Ethernet Frame min size */
#if SWITCH_CHIP_BCM53286
#define HAL_MSG_MINSIZE			64
#elif SWITCH_CHIP_BCM5396
#define HAL_MSG_MINSIZE			64
#else
#define HAL_MSG_MINSIZE			60
#endif

#define HAL_MAX_MSG_SIZE			128

/* Private Protocol Type */
#define HAL_PROTOCOL_TYPE_RTOS		0x0102
#define HAL_PROTOCOL_TYPE_OBNET		0x0002

/* Message Type */
#define	HAL_MSG_GET					0x90
#define HAL_MSG_SET					0x91
#define HAL_MSG_RESPONSE			0x92
#define HAL_MSG_TRAP				0x93
#define HAL_MSG_TRAP_RESPONSE		0x94

/*********************************************************************
		Neighbor Search Feature
 *********************************************************************/
/* Code define */
#define HAL_CODE_NEIGHBOR_SEARCH	0x10

typedef enum {
	LOCAL_DISCONNECT	= 1,
	LOCAL_BLOCKING		= 2,
	LOCAL_FORWARDING	= 3
} HAL_LOCAL_PORT_STATUS;

typedef struct {
	uint8	GetCode;
	uint8	RetCode;
	uint8	PortSearchEnable;
	uint8	PortNo;
	uint8	PortStatus;
} hal_swif_msg_neighbor_req;

typedef struct {
	uint8	GetCode;
	uint8	RetCode;
	uint8	PortSearchEnable;
	uint8	PortNo;
	uint8	PortStatus;
	uint8	NeighborMac[6];
	uint8	NeighborPort;
	uint8	NeighborIP[4];
	uint8	NeighborSwitchType[8];
} hal_swif_msg_neighbor_req_rsponse;

/*********************************************************************
		Trap Feature
 *********************************************************************/
/* TrapGateMask define */
#define MAX_TRAP_TYPE_NUM			32

#define TRAP_MASK_DEV_REBOOT		0x00000001
#define	TRAP_INDEX_DEV_REBOOT		0
//#define TRAP_CODE_DEV_REBOOT		0x18

#define TRAP_MASK_PORT_STATUS		0x00000002
#define	TRAP_INDEX_PORT_STATUS		1
//#define TRAP_CODE_PORT_STATUS		0x11

#define TRAP_MASK_RING_STATUS		0x00000004
#define	TRAP_INDEX_RING_STATUS		2
//#define TRAP_CODE_RING_BREAK		0x12
//#define TRAP_CODE_RING_RECOVER	0x13

#define TRAP_MASK_VOL_OVER			0x00000008
#define	TRAP_INDEX_VOL_OVER			3
//#define TRAP_CODE_VOL_OVER		0x14

#define TRAP_MASK_TRAFFIC_OVER		0x00000010
#define TRAP_INDEX_TRAFFIC_OVER		4

typedef struct {
	HAL_BOOL	FeatureEnable;
	uint8		ServerMac[6];
	uint32		GateMask;
	uint16		RequestID[MAX_TRAP_TYPE_NUM];
	HAL_BOOL	SendEnable[MAX_TRAP_TYPE_NUM];
} hal_trap_info_t;

/* Trap for port status change */
#if 0
typedef struct {
	u8	TrapIndex;
	u8	RetCode;
	u8	Res;
	u8	PortNum;
	u8	PortStatus[MAX_PORT_NUM];
} hal_trap_port_status;
#else
typedef struct {
	u8	TrapIndex;
	u8	TimeStamp[6];
	u8	PortNum;
	u8	PortStatus[MAX_PORT_NUM];
} hal_trap_port_status;
#endif

/* Trap for ring status change */
#ifndef MAX_RING_NUM
#define MAX_RING_NUM		8
#endif
#define RING_DATA_LEN		9

typedef struct {
	u8	Flag;
	u8	RingPortNo;
	u8	ExtNeighborMac[6];
	u8	ExtNeighborPortNo;
} hal_ring_port_info;

typedef struct {
	u8	TrapIndex;
	u8	TimeStamp[6];
	u8	RingPairNum;
	hal_ring_port_info RingPairStatus[2 * MAX_RING_NUM];
} hal_trap_ring_status;

/* Trap for port traffic over */
typedef struct {
	u8	TrapIndex;
	u8	TimeStamp[6];
	u8	PortNum;
	u8	TrafficStatus[2*MAX_PORT_NUM];
} hal_trap_traffic_status;

/*********************************************************************
		Exported Functions
 *********************************************************************/
int hal_swif_neighbor_req_send(uint8 Lport, uint16 ReqID);
int hal_swif_neighbor_req_rsponse(uint8 *SMA, uint8 RxLport, uint16 ReqID, hal_swif_msg_neighbor_req *NeighborReq);
int hal_swif_neighbor_info_update(hal_swif_msg_neighbor_req_rsponse *pNeighBorReqRsp);
int hal_swif_neighbor_info_clear(uint8 Lport);

int hal_swif_trap_send(uint8 *ServerMac, uint8 *TrapMsgBuf, uint16 TrapMsgLen, uint16 RequestID);
int hal_swif_trap_complete(uint8 *SMA, uint16 ReqID, hal_trap_port_status *pTrapReponse);

#endif	/* _HAL_SWIF_MESSAGE_H_ */

