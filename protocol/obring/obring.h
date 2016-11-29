
#ifndef __OBRING_H__
#define __OBRING_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "mconfig.h"
#include "stm32f2xx.h"
#include "hal_swif_port.h"

#ifndef MAC_LEN
#define MAC_LEN     			6
#endif

#define CPU_PORT				2
#define MAX_RING_NUM			8

#define LINK_POLL_DELAY			1			/* 1 ms */

#define MAX_RING_MSG_QUEUE_LEN	16
#define MAX_RING_MSG_SIZE		94
#define MAX_RING_DATA_SIZE		36
#define OFFSET_RING_HEADER		30
#define OFFSET_RING_DATA		58

#define MAX_AUTH_TIMEOUT_COUNT	10
#define DEFAULT_AUTH_TIME		2			/* 2 s */
#define DEFAULT_HELLO_TIME		1			/* 1 s */
#define DEFAULT_FAIL_TIME		3			/* 3 s */
#define DEFAULT_BALLOT_TIME		2			/* 2 s */

#define RING_TICK_VAL			1
#define MAX_DOMAIN_NAME_LEN 	7

#define INDEX_PRIMARY			0
#define INDEX_SECONDARY			1

#define OP_BRCM_SND				0x24
#define USE_OWN_MULTI_ADDR		1
#define BALLOT_MODE_1			1

typedef enum {
	PORT_CUSTUM_MODE			= 0,
	PORT_FAST_MODE				= 1,
} eRingMode;

typedef enum {
	RING_FAULT					= 0,
	RING_HEALTH					= 1
} eRingState;

typedef enum {
	NODE_TYPE_MASTER			= 0,
	NODE_TYPE_TRANSIT			= 1
} eNodeType;

typedef enum {
	PACKET_AUTHENTICATION		= 1,
	PACKET_BALLOT				= 2,
    PACKET_HELLO				= 5,
    PACKET_COMPLETE				= 6,
    PACKET_COMMON				= 7,
    PACKET_LINKDOWN				= 8,
    PACKET_EDGE_HELLO			= 10,
    PACKET_MAJOR_FAULT			= 11,
    PACKET_COMMAND				= 20
} ePacketType;

typedef enum {
	NODE_STATE_IDLE				= 0,
    NODE_STATE_COMPLETE			= 1,
    NODE_STATE_FAIL				= 2,
    NODE_STATE_LINK_UP			= 3,
    NODE_STATE_LINK_DOWN		= 4,
    NODE_STATE_PRE_FORWARDING	= 5  
} eNodeState;

typedef enum {
	PRIO_HIGH					= 0,
	PRIO_MEDIUM					= 1,
	PRIO_LOW					= 2
} eNodePrio;

typedef enum {
	PORT_IDLE					= 0,
	PORT_DOWN					= 1,
	PORT_AUTH_REQ				= 2,
	PORT_AUTH_FAIL				= 3,
	PORT_AUTH_PASS				= 4,
	PORT_BALLOT_ACTIVE			= 5,
	PORT_BALLOT_FINISH			= 7
} ePortRunState;

/*************************************************************
	Ring message data type define
 *************************************************************/
typedef enum {
	MSG_AUTH_REQ					= 0x01,	/* Auth Request */
	MSG_AUTH_ACK					= 0x02	/* Auth Ack */
} eMsgAuthType;

#if BALLOT_MODE_1
typedef enum {
	MSG_BALLOT_ACTIVE				= 0x01,	/* Start ballot timer */
	MSG_BALLOT_PASSIVE				= 0x02,	
	MSG_BALLOT_LOOPBACK				= 0x03
} eMsgBallotType;
#else
typedef enum {
	MSG_BALLOT_ACTIVE				= 0x01,	/* Start ballot timer */
	MSG_BALLOT_CHAIN_BACK			= 0x02,	
	MSG_BALLOT_RING_BACK			= 0x03		
} eMsgBallotType;
#endif

typedef enum {
	MSG_COMPLETE_UPDATE_NODE		= 0x01,	/* Update NodeState */
	MSG_COMPLETE_WITH_FLUSH_FDB		= 0x02	/* Update NodeState and StpState */
} eMsgCompleteType;

typedef enum {
	MSG_LINKDOWN_REQ_UPDATE_NODE	= 0x01,	/* Request update NodeState */
	MSG_LINKDOWN_REQ_FLUSH_FDB		= 0x02	/* Request update NodeState and FlushFDB */
} eMsgLinkDownType;

typedef enum {
	MSG_COMMON_UPDATE_NODE			= 0x01,
	MSG_COMMON_WITH_FLUSH_FDB		= 0x02,
	MSG_COMMON_RING_CHAIN_READY		= 0x03
} eMsgCommonType;

typedef enum {
	CMD_GET_NODE_REQ				= 0x01,
	CMD_GET_NODE_RSP				= 0x81,
	
	CMD_RING_DISABLE_REQ			= 0x02,	
	CMD_RING_DISABLE_RSP			= 0x82,	
	
	CMD_RING_ENABLE_REQ				= 0x03,	
	CMD_RING_ENABLE_RSP				= 0x83,	
	
	CMD_RING_REBOOT_REQ				= 0x04,	
	CMD_RING_REBOOT_RSP				= 0x84		
} eMsgCmdCode;

typedef enum {
	CMD_RET_CODE_CONTINUE			= 0x01,
	CMD_RET_CODE_END				= 0x02,
	CMD_RET_CODE_TIMEOUT			= 0x03
} eMsgCmdRetCode;

typedef enum {
	CMD_RET_NODE_NOT_LAST			= 0x00,
	CMD_RET_LAST_NODE_CHAIN			= 0x01,
	CMD_RET_LAST_NODE_RING			= 0x02		
} eMsgCmdRetLastNode;

/*************************************************************
	Ring timers define
 *************************************************************/
typedef enum {
	T_MS		= 0,
	T_SEC		= 1,
} eTimingUnit;

typedef struct {
	unsigned short	Value;
	unsigned short	Active;
} tRingTimer;

typedef struct {
	tRingTimer		Hello;
	tRingTimer		Fail;
	tRingTimer		BallotP;
	tRingTimer		BallotS;
	tRingTimer		AuthP;
	tRingTimer		AuthS;
} tRingTimers;

/*************************************************************
	OB-Ring Message body struct
 *************************************************************/
/* Message Ballot */
typedef struct {
	unsigned char 	Prio;
	unsigned char 	Mac[MAC_LEN];
} tBallotId;

typedef struct {
	unsigned char	Type;
	unsigned char	Port;
	tBallotId 		Id;
	ePortRunState	State;
} tRMsgBallot;

/* Message Authentication */
typedef struct {
	unsigned char	Type; 
	tBallotId		BallotId;
} tRMsgAuth;

/* Message Hello */
typedef struct {
	unsigned char		Tick[4];
	HAL_PORT_STP_STATE	MasterSecondaryStp;
	HAL_PORT_STP_STATE	TxPortStpState;
	unsigned char		BlockLineNum[2];
	eRingState			RingState;
} tRMsgHello;

/* Message Complete */
typedef struct {
	unsigned char	Type;
} tRMsgComplete;

/* Message Common */
typedef struct {
	unsigned char	Type;
} tRMsgCommon;

/* Message LinkDown */
typedef struct {
	unsigned char	Type;
	unsigned char	ExtNeighborMac[MAC_LEN];
	unsigned char	ExtNeighborPortNo;
} tRMsgLinkDown;

/*********************************************************
	One-To-One Command Mode
 *********************************************************/
typedef struct {
	unsigned char		ReqestId;
	unsigned char		SrcNodeIndex;
	unsigned char		DstNodeIndex;
	unsigned char		NodeIndexInc;
} tCmdReqGetNode, tCmdReqRingDisable, tCmdReqRingEnable, tCmdReqRingReboot;

typedef struct {
	unsigned char		DstMac[MAC_LEN];
	unsigned char		ReqestId;
	unsigned char		RetCode;
	unsigned char		NodeMac[MAC_LEN];
	unsigned char		LastNodeFlag;
} tCmdRspRingDisable, tCmdRspRingEnable, tCmdRspRingReboot;

typedef struct {
	unsigned char		DstMac[MAC_LEN];
	unsigned char		ReqestId;
	unsigned char		RetCode;
	unsigned char		NodeMac[MAC_LEN];
	unsigned char		NodeVersion[4];
	unsigned char		NodeType;
	unsigned char		RingEnable;
	eRingState			RingState;
	unsigned char		WestPortNo;
	HAL_PORT_LINK_STATE	WestPortLink;
	HAL_PORT_STP_STATE	WestPortStp;
	unsigned char		EastPortNo;
	HAL_PORT_LINK_STATE	EastPortLink;
	HAL_PORT_STP_STATE	EastPortStp;
	unsigned char		LastNodeFlag;
} tCmdRspGetNode;

typedef struct {
	unsigned char				Code;
	union {
		unsigned char 			Data[MAX_RING_DATA_SIZE-1]; 
		tCmdReqGetNode			ReqGetNode;
		tCmdRspGetNode			RspGetNode;
		tCmdReqRingDisable		ReqRingDisable;
		tCmdRspRingDisable		RspRingDisable;
		tCmdReqRingEnable		ReqRingEnable;
		tCmdRspRingEnable		RspRingEnable;
		tCmdReqRingReboot		ReqRingReboot;
		tCmdRspRingReboot		RspRingReboot;
	} Action;	
} tRMsgCmd;

/* Message Body */
typedef union {
    unsigned char 	Data[MAX_RING_DATA_SIZE]; 
	tRMsgAuth		Auth;
	tRMsgBallot		Ballot;
	tRMsgHello		Hello;
	tRMsgComplete	Complete;
	tRMsgCommon		Common;
	tRMsgLinkDown	LinkDown;
	tRMsgCmd		Command;
} tRingMsgBody;

/*************************************************************
	OB-Ring Frame struct
	802.3 LLC header + Switch Tag + VLAN Tag + 64 Data
 *************************************************************/
typedef struct {
	/* 802.3 LLC header, 30 bytes */
	unsigned char	dst_mac[6];
	unsigned char	src_mac[6];
	unsigned char	switch_tag[4];
	unsigned char	ether_type[2];
	unsigned char	vlan_tag[2];
	unsigned char	frame_length[2];
	unsigned char	dsap;
	unsigned char	ssap;
	unsigned char	llc;
	unsigned char	oui_code[3];
	unsigned char	pid[2];
	
	/* OB-Ring header, 28 bytes */
  	unsigned char	obr_res1[2];
	unsigned char	obr_length[2];
	unsigned char	obr_version;
	unsigned char	obr_type;
	unsigned char	obr_domain_id[2];
	unsigned char	obr_ring_id[2];
	unsigned char	obr_res2[2];
	unsigned char	obr_sys_mac[6];
	unsigned char	obr_hello_time[2];
	unsigned char	obr_fail_time[2];
	unsigned char	obr_port_id;
	unsigned char	obr_ring_level;
	unsigned char	obr_hello_seq[2];
	unsigned char	obr_res3[2];

	/* OB-Ring data, 36 bytes */
	tRingMsgBody	obr_msg;
} tRingFrame;

typedef struct {
	unsigned short	BufLen;
	unsigned char	Buf[MAX_RING_MSG_SIZE];	
} tRingMessage;

/*************************************************************
 * OBNet Configuration struct
 *************************************************************/
typedef struct {
	unsigned char		ucRingPort;
	unsigned char		ucRingMode;
	unsigned char		ucNodePrio;		/* ucLinkUpHold */
	unsigned char		ucRes2;			/* ucLinkDownHold */
	unsigned char		ucHelloTime;
	unsigned char		ucBallotTime;
	unsigned char		ucFailTime;
	unsigned char		ucRes3;			/* ucMessageAge */
} tRingNmsPortConfig;

typedef struct {
	tRingNmsPortConfig	stPrimaryPortConfig;
	tRingNmsPortConfig	stSecondaryPortConfig;
} tRingNmsConfigRec;

typedef struct {
	unsigned char		ucGlobalEnable;
	unsigned char		ucRecordNum;
	unsigned char		uiRingGate[4];
} tRingNmsConfigGlobal;

typedef struct {
	tRingNmsConfigGlobal stGlobal;
	tRingNmsConfigRec	 stRecord[MAX_RING_NUM];
} tRingNmsConfigInfo;


/* Single Ring Configuration, 32 bytes */
typedef struct {
	unsigned char 		ucRingIndex;
	unsigned char 		ucEnable;
	unsigned char 		ucDomainName[8];
	unsigned char 		usDomainId[2];
	unsigned char 		usRingId[2];
	unsigned char 		ucRingMode;
	unsigned char 		ucNodePrio;
	
	unsigned char 		usAuthTime[2];
	unsigned char 		usBallotTime[2];
	unsigned char 		usHelloTime[2];
	unsigned char 		usFailTime[2];
	unsigned char 		ucPrimaryPort;
	unsigned char 		ucSecondaryPort;
	unsigned char 		usControlVlan[2];
	unsigned char 		usProtectVlan[2];
	unsigned char 		ucRes[2];
} tRingConfigRec;

typedef struct {
	unsigned char 		ucGlobalEnable;
	unsigned char		ucRecordNum;
} tRingConfigGlobal;

/*************************************************************
 	Struct for OB-Ring State
 *************************************************************/
#define BALLOT_TAG_CANDIDATE	0x01
#define BALLOT_TAG_MASTER		0x02

typedef struct {
	HAL_PORT_LINK_STATE	LinkState; 
	HAL_PORT_STP_STATE	StpState;
	ePortRunState		RunState;
	unsigned char		AuthTimoutCount;
	//unsigned char		BallotTag;
	tBallotId			BallotId;
	HAL_BOOL			NeighborValid;
	unsigned char		NeighborMac[MAC_LEN];
	unsigned char		NeighborPortNo;	
} tRingPortState;

typedef struct {
	unsigned short		SwitchTimes;
	unsigned short		StormCount;
	unsigned short		HelloSeq;
	unsigned int		HelloElapsed;
	eNodeType			NodeType;
	eNodeState			NodeState;
	eRingState			RingState;
	tRingPortState		PortState[2];
} tRingState;

typedef struct {
	tRingConfigGlobal	GlobalConfig;
	tRingConfigRec		RingConfig[MAX_RING_NUM];
	tRingState			DevState[MAX_RING_NUM];
} tRingInfo;


/*************************************************************
 	Functions
 *************************************************************/
unsigned char obring_get_ring_idx_by_port(unsigned char Lport);
unsigned char obring_get_port_idx_by_port(tRingConfigRec *pRingConfig, unsigned char Lport);
int obring_disable(unsigned char RingIndex);
int obring_enable(unsigned char RingIndex);
int obring_reboot(unsigned char RingIndex);
int obring_command_send(void *pCliEnv, unsigned char RingIndex, unsigned char TxLport, tRMsgCmd *pCmdRequest, tRMsgCmd *pCmdResponse);
void obring_frame_receive(unsigned char *rxBuf, u16 rxLen);
void obring_initialize(void);
u8 obring_check_enable(unsigned char RingIndex);

#ifdef __cplusplus
}
#endif

#endif


