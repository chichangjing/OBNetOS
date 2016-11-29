
#ifndef __OB_RING_H__
#define __OB_RING_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "mconfig.h"
#include "stm32f2xx.h"
#include "hal_swif_port.h"

#ifndef MAC_LEN
#define MAC_LEN     			6
#endif

#if (BOARD_GE22103MA) || (BOARD_GV3S_HONUE_QM) || (BOARD_GE_EXT_22002EA)
#define RING_LOGIC_PORT_A		0
#define RING_LOGIC_PORT_B		1

#elif BOARD_GE20023MA
#define RING_LOGIC_PORT_A		0
#define RING_LOGIC_PORT_B		1

#else
#define RING_LOGIC_PORT_A		0
#define RING_LOGIC_PORT_B		1
#endif

#define RING_PAIR_NUM			1
#define	MAX_DEV_PORT_NUM		5

#define OP_BRCM_SND				0x24
#define TIM_TX_FRAME 			200
#define OB_RING_PROTOCOL		0x5231

/* Ring message type */
#define MsgRealBreak			0x03
#define MsgVirtualBreak			0x04
#define MsgRingFlushTbl			0x05
#define MsgChainFlushTbl		0x06

typedef struct {
	u32 TimTxPortA;
	u32 TimTxPortB;
} TimerGroup_t;

typedef struct _RingMsg {
	u8 MsgType;
	u8 MsgLength;
	u8 RBMac[6];					/* Real break MAC address */
	u8 VBMac[6];					/* Virtual break MAC address */
	u8 TxPort;
} RingMsg_t;

typedef struct _OBringFrame {
	u8	DstMAC[MAC_LEN];			/* Destination MAC address */ 
	u8	SrcMAC[MAC_LEN];			/* Source MAC address */ 
	u8	SwitchTag[4];				/* Switch Tag or 802.1Q vlan tag */ 
	
	u8	OUIExtEtherType[2];			/* OUI extended edthernet type, 0x88b7 */ 
	u8	OUICode[3];					/* Organizationally unique identifier code */
	u8	ProtocolID[2];				/* Protocal ID : 0x5231 */
	u8	Version;					/* Version : 0x00 */
	u8	Reserved1[8];
	
	u8	MsgData[32];
} OBringFrame_t;

typedef enum {
    SINGLE_NODE				= 0,		
    RING_MASTER_NODE		= 1,
    RING_TRANSFER_NODE		= 2,
    CHAIN_MASTER_NODE		= 3,
    CHAIN_TRANSFER_NODE		= 4,
    CHAIN_SLAVER_NODE		= 5
} eOBNodeType;

typedef enum {
    FLUSH_IDLE				= 0,		
    RING_FLUSH_START		= 1,
    RING_FLUSH_COMPLETE		= 2,
    CHAIN_FLUSH_START		= 3,
    CHAIN_FLUSH_COMPLETE	= 4
} eFlushTblState;

typedef struct {
	u8				SearchEnable;
	u8				DevPortNum;
	u8				ExtNeighborMac[MAC_LEN];
	u8				ExtNeighborPortNum;	
	u8				RingFlag;
	HAL_PORT_LINK_STATE	LinkState; 
	HAL_PORT_STP_STATE	StpState;
} RingPortInfo_t;

typedef struct {
	u8 				NodeMac[MAC_LEN];
	u8				RingEnable;
	eOBNodeType		NodeType;
	eFlushTblState	FlushState;
	RingPortInfo_t	PortInfo[RING_PAIR_NUM * 2];		
} DevNode_t;

DevNode_t *Ring_GetNode(void);

void Ring_Frame_Process(u8 *rxBuf, u16 len);
void Ring_Initialize(void);
void Ring_Task(void *arg);
void Ring_Start(void);
u8 Check_Ring_Enable(void);

#ifdef __cplusplus
}
#endif

#endif

