
#ifndef _HAL_SWIF_PORT_H_
#define _HAL_SWIF_PORT_H_

#include "mconfig.h"
#include "hal_swif_types.h"

#if 0
/* Port type        */
typedef enum {
	S100M_CABLE 	= 0,
	S1000M_CABLE	= 1,
	S100M_OPTICAL	= 2,
	S1000M_OPTICAL	= 3
} HAL_PORT_TYPE;
#else
/* Port type        */
typedef enum {
	S100M_CABLE 	= 0,
	S100M_OPTICAL	= 1,		
	S1000M_CABLE	= 2,
	S1000M_OPTICAL	= 3,
    PORT_UNKOWN     = 4
} HAL_PORT_TYPE;
#endif

/* Port link status */
typedef enum {
    LINK_DOWN		= 0,
    LINK_UP			= 1
} HAL_PORT_LINK_STATE;

/* Port duplex mode */
typedef enum {
    HALF_DUPLEX		= 0,
    FULL_DUPLEX		= 1,
    UNKOWN_DUPLEX   = 2
} HAL_PORT_DUPLEX_STATE;

/* Port speed       */
typedef enum {
    SPEED_10M		= 0,
    SPEED_100M		= 1,
    SPEED_1000M		= 2,
    SPEED_UNKOWN	= 3
} HAL_PORT_SPEED_STATE;

/* Port stp status  */
typedef enum {
    DISABLED		= 0,
    BLOCKING		= 1,
    LEARNING		= 2,
    FORWARDING		= 3,
    STP_UNKOWN		= 4
} HAL_PORT_STP_STATE;

 /* Port speed duplex */
typedef enum {
	S10M_HALF		= 0,
	S10M_FULL		= 1,
	S100M_HALF		= 2,
	S100M_FULL		= 3,
	S1000M_HALF		= 4,
	S1000M_FULL		= 5,
	AUTO_NEGO		= 8
} HAL_PORT_SPEED_DUPLEX;

typedef enum {
    STATE_00 		= 0,		/* PORTA link down, PORTB link down */
    STATE_01 		= 1,		/* PORTA link down, PORTB link up   */
    STATE_10 		= 2,		/* PORTA link up,   PORTB link down */
    STATE_11 		= 3,		/* PORTA link up,   PORTB link up   */
    STATE_POWER_ON = STATE_00
} HAL_NODE_LINK_STATE;

typedef enum {
    MODE_MDIX		= 0,
    MODE_MDI		= 1,
    MODE_AutoMDIX	= 2
} HAL_MDI_MDIX_STATE;

typedef enum {
    MODE_COUNT_RX_ONLY = 0,
    MODE_COUNT_TX_ONLY,
	MODE_COUNT_RX_TX
} HAL_HISTOGRAM_MODE;


typedef struct {
	unsigned char lport;
	unsigned char hport;
	unsigned char port_type;
} hal_port_map_t;

typedef __packed struct {
	uint32	RxGoodOctetsLo;
	uint32	RxGoodOctetsHi;
	uint32	RxUnicastPkts;
	uint32	RxBroadcastPkts;
	uint32	RxMulticastPkts;
	uint32	RxPausePkts;

	uint32	TxOctetsLo;
	uint32	TxOctetsHi;
	uint32	TxUnicastPkts;
	uint32	TxBroadcastPkts;
	uint32	TxMulticastPkts;
	uint32	TxPausePkts;
	
	uint32	Reserved[4];
} hal_port_counters_t;

#if SWITCH_CHIP_88E6095
typedef __packed struct {
	uint32	InGoodOctetsLo;	/* offset 0 */
	uint32	InGoodOctetsHi;	/* offset 1, not supported by 88E6065 */
	uint32	InBadOctets;	/* offset 2 */
	uint32	OutFCSErr;		/* offset 3 */
	uint32	InUnicasts;		/* offset 4 */
	uint32	Deferred;		/* offset 5 */
	uint32	InBroadcasts;	/* offset 6 */
	uint32	InMulticasts;	/* offset 7 */
	uint32	Octets64;		/* 64 Octets, offset 8 */
	uint32	Octets127;		/* 65 to 127 Octets, offset 9 */
	uint32	Octets255;		/* 128 to 255 Octets, offset 10 */
	uint32	Octets511;		/* 256 to 511 Octets, offset 11 */
	uint32	Octets1023;		/* 512 to 1023 Octets, offset 12 */
	uint32	OctetsMax;		/* 1024 to Max Octets, offset 13 */
	uint32	OutOctetsLo;	/* offset 14 */
	uint32	OutOctetsHi;	/* offset 15, not supported by 88E6065 */
	uint32	OutUnicasts;	/* offset 16 */
	uint32	Excessive;		/* offset 17 */
	uint32	OutMulticasts;	/* offset 18 */
	uint32	OutBroadcasts;	/* offset 19 */
	uint32	Single;			/* offset 20 */

	uint32	OutPause;		/* offset 21 */
	uint32	InPause;		/* offset 22 */
	uint32	Multiple;		/* offset 23 */
	uint32	Undersize;		/* offset 24 */
	uint32	Fragments;		/* offset 25 */
	uint32	Oversize;		/* offset 26 */
	uint32	Jabber;			/* offset 27 */
	uint32	InMACRcvErr;	/* offset 28 */
	uint32	InFCSErr;		/* offset 29 */
	uint32	Collisions;		/* offset 30 */
	uint32	Late;			/* offset 31 */
} m88e6095_counters_t;
#endif

typedef struct {
	u8	PacketIndex;
	u8	RecordCount;
	u16	OffsetAddress;
} obnet_record_set_stat_t;

typedef struct {
	u8	PacketIndex;
	u8	RemainCount;
	u16	OffsetAddress;
} obnet_record_get_stat_t;

/*****************************************************************************************
 			Port Configuration for OBNet NMS                                               
 ******************************************************************************************/
#if (OB_NMS_PROTOCOL_VERSION == 1)
#define PORT_CFG_MASK_TYPE					0x40000000	/* Port type */
#define PORT_CFG_MASK_NEIGHBOR_SEARCH		0x00008000	/* Port neighbor search */
#define PORT_CFG_MASK_ENABLE				0x00004000	/* Port enable */
#define PORT_CFG_MASK_AUTO_NEG				0x00000040	/* Port auto-negotiation */
#define PORT_CFG_MASK_SPEED					0x00000030	/* Port speed */
#define PORT_CFG_MASK_DUPLEX				0x00000008	/* Port duplex */
#define PORT_CFG_MASK_SPEED_DUPLEX			0x00000038	/* Port speed-duplex */
#define PORT_CFG_MASK_MDIX					0x00000006	/* Port MDI-MDIX or Reserved */
#define PORT_CFG_MASK_FLOW_CTRL				0x00000001	/* Port flow control */
#elif (OB_NMS_PROTOCOL_VERSION > 1)
#define PORT_CFG_MASK_TYPE					0xC0000000	/* Port type */
#define PORT_CFG_MASK_TX_RATE_THRESHOLD		0x00F00000	/* Port rate threshold for TX */
#define PORT_CFG_MASK_RX_RATE_THRESHOLD		0x000F0000	/* Port rate threshold for RX */
#define PORT_CFG_MASK_NEIGHBOR_SEARCH		0x00008000	/* Port neighbor search */
#define PORT_CFG_MASK_ENABLE				0x00004000	/* Port enable */
#define PORT_CFG_MASK_AUTO_NEG				0x00000040	/* Port auto-negotiation */
#define PORT_CFG_MASK_SPEED					0x00000030	/* Port speed */
#define PORT_CFG_MASK_DUPLEX				0x00000008	/* Port duplex */
#define PORT_CFG_MASK_SPEED_DUPLEX			0x00000038	/* Port speed-duplex */
#define PORT_CFG_MASK_FLOW_CTRL				0x00000001	/* Port flow control */

typedef enum {
	PERCENTAGE_10	= 0,
	PERCENTAGE_20	= 1,
	PERCENTAGE_30	= 2,
	PERCENTAGE_40	= 3,
	PERCENTAGE_50	= 4,
	PERCENTAGE_60	= 5,
	PERCENTAGE_70	= 6,
	PERCENTAGE_80	= 7,
	PERCENTAGE_90	= 8,
	PERCENTAGE_100	= 9
} HAL_PERCENTAGE_E;

typedef enum {
	SPEED_10	= 0,
	SPEED_100	= 1,
	SPEED_1000	= 2
} HAL_PORT_MAX_SPEED_E;

typedef struct {
	HAL_PORT_MAX_SPEED_E PortMaxSpeed;
	uint8	TxThreshold;
	uint8	RxThreshold;
} hal_threshold_t;

#endif

typedef struct {
	uint8	PortNum;
	uint8	PortConfig[MAX_PORT_NUM*4];
} hal_port_conf_t;

typedef struct {
	uint8	PortSearchEnable;
	uint8	PortNo;
	uint8	PortStatus;
	uint8	NeighborMac[6];
	uint8	NeighborPort;
	uint8	NeighborIP[4];
	uint8	NeighborSwitchType[8];
} hal_neighbor_record_t;

typedef struct {
	uint8					PortNo;
	HAL_PORT_TYPE			PortType;
	HAL_BOOL				PortEnable;
	HAL_PORT_SPEED_DUPLEX	SpeedDuplex;
	HAL_BOOL				FlowCtrl;
	HAL_BOOL				NeigborSearch;
	HAL_PERCENTAGE_E		TxThreshold;
	HAL_PERCENTAGE_E		RxThreshold;
} hal_port_config_info_t;

#define HAL_DEFAULT_TRAFFIC_INTERVAL	5	/* 5 Second */
typedef struct {
	uint8	PortNo;
	uint8	Interval;
	uint32	Threshold_TxOctets;
	uint32	Threshold_RxOctets;
	uint32	PrevTxOctets;
	uint32	PrevRxOctets;
	uint32	Interval_TxOctets;
	uint32	Interval_RxOctets;
} hal_port_traffic_info_t;

/************************* Struct for OBNet NMS message **********************************/
#if MODULE_OBNMS

/****** Get port status and Get/Set port config ********/
#define FRAME_MAX_PORT_CONFIG_REC	18
typedef struct {
  	uint8	GetCode;
	uint8	RetCode;
	uint8	PortNum;
	uint8	PortStatus[MAX_PORT_NUM];
} obnet_rsp_port_status;

typedef struct {
	uint8	GetCode;
	uint8	RetCode;
	uint8	PortNum;
	uint8	PortConfig[MAX_PORT_NUM*4];
} obnet_rsp_port_config, obnet_set_port_config;

typedef struct {
	uint8	GetCode;
	uint8	RetCode;
	uint8	OpCode;
} obnet_get_port_config;

typedef struct {
	uint8	GetCode;
	uint8	RetCode;
	uint8	Res;
	uint8	OpCode;
	uint8	ItemsInData;
	uint8	PortConfig[MAX_PORT_NUM*4];
} obnet_rsp_port_config2, obnet_set_port_config2;

/****************** Get neighbor ***********************/
typedef struct {
	uint8	GetCode;
	uint8	RetCode;
	uint8	OpCode;
} obnet_get_port_neighbor;

typedef struct {
	uint8	GetCode; 
	uint8	RetCode;
	uint8	PortNum;
	uint8	OpCode;
	uint8	RecordCount;
} obnet_rsp_get_port_neighbor;

/****************** Port Statistics *********************/
typedef enum {
	COUNTERS_READ	= 0,
	COUNTERS_CLEAR	= 1
} HAL_PORT_STATISTIC_OPCODE;

#define MAX_STATISTIC_PORT_NUM	4
typedef struct {
  	uint8	GetCode;
	uint8	RetCode;
	uint8	PortNum;
	uint8	OpCode;
	uint8	PortMap[4];
} obnet_port_statistic;

typedef struct {
	uint8	GetCode;
	uint8	RetCode;
	uint8	PortNum;
	uint8	OpCode;
	uint8	PortMap[4];
	uint8	ValidItemsMask[2];
	hal_port_counters_t CountersInfo[MAX_STATISTIC_PORT_NUM];
} obnet_rsp_port_statistic;

#endif

/*****************************************************************************************
	      Exported Functions                                                               
 ******************************************************************************************/
/* HAL Function */
unsigned char hal_swif_lport_2_hport(unsigned char lport);
unsigned char hal_swif_hport_2_lport(unsigned char hport);
int hal_swif_port_get_link_state(uint8 lport, HAL_PORT_LINK_STATE *link_state);
int hal_swif_port_set_stp_state(uint8 lport, HAL_PORT_STP_STATE stp_state);
int hal_swif_port_get_stp_state(uint8 lport, HAL_PORT_STP_STATE *stp_state);
int hal_swif_port_get_duplex(uint8 lport, HAL_PORT_DUPLEX_STATE *duplex_state);
int hal_swif_port_get_speed(uint8 lport, HAL_PORT_SPEED_STATE *speed_state);
int hal_swif_port_set_mdi_mdix(uint8 lport, HAL_MDI_MDIX_STATE mdi_mdix);
int hal_swif_port_get_mdi_mdix(uint8 lport, HAL_MDI_MDIX_STATE *mdi_mdix);
int hal_swif_port_get_counters(uint8 lport, hal_port_counters_t *port_counters, uint16 *valid_bit_mask);
int	hal_swif_port_clear_counters(uint8 lport);
int cnt_clr_mutex_init(void);
int hal_swif_port_get_clear_counters_flag(uint8 lport);
int hal_swif_port_clear_counters_flag(uint8 lport);

/* For CLI */
int hal_swif_port_show_status(void *pCliEnv);
int hal_swif_port_show_config(void *pCliEnv);
int hal_swif_port_show_neigbor(void *pCliEnv);
int hal_swif_port_show_traffic(void *pCliEnv);
int hal_swif_port_show_counters(void *pCliEnv, uint8 lport);

/* Configuration */
int hal_swif_port_conf_initialize(void);

/* Functions for NMS */
#if MODULE_OBNMS
void nms_rsp_get_port_status(uint8 *DMA, uint8 *RequestID);
void nms_rsp_set_port_config(uint8 *DMA, uint8 *RequestID, obnet_set_port_config *pSetPortConfig);
void nms_rsp_get_port_config(uint8 *DMA, uint8 *RequestID, obnet_get_port_config *pGetPortConfig);
void nms_rsp_get_port_neighbor(u8 *DMA, u8 *RequestID, obnet_get_port_neighbor *pGetNeighbor);
void nms_rsp_port_statistics(u8 *DMA, u8 *RequestID, obnet_port_statistic *pSetPortStatistics);
#endif

#endif

