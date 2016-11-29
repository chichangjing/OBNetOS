
#ifndef _HAL_SWIF_RATE_CTRL_H_
#define _HAL_SWIF_RATE_CTRL_H_

#include "mconfig.h"


/* Ingress/Egress Rate define */
typedef enum {
#if (OB_NMS_PROTOCOL_VERSION == 1)
	RATE_NO_LIMIT 			= 0x0, 		/* Not limited   */
	RATE_128K				= 0x1,      /* 128K bits/sec */
	RATE_256K				= 0x2,      /* 256K bits/sec */
	RATE_512K				= 0x3,      /* 512 bits/sec */
	RATE_1M					= 0x4,      /* 1M  bits/sec */
	RATE_2M					= 0x5,      /* 2M  bits/sec */
	RATE_4M					= 0x6,      /* 4M  bits/sec */
	RATE_8M					= 0x7	    /* 8M  bits/sec */
#else
	RATE_NO_LIMIT 			= 0x0, 		/* Not limited   */
	RATE_128K				= 0x1,      /* 128K bits/sec */
	RATE_256K				= 0x2,      /* 256K bits/sec */
	RATE_512K				= 0x3,      /* 512 bits/sec */
	RATE_1M					= 0x4,      /* 1M  bits/sec */
	RATE_2M					= 0x5,      /* 2M  bits/sec */
	RATE_4M					= 0x6,      /* 4M  bits/sec */
	RATE_8M					= 0x7,      /* 8M  bits/sec */
	RATE_16M				= 0x8,      /* 16M  bits/sec */
	RATE_32M				= 0xA,      /* 32M  bits/sec */
	RATE_64M				= 0xB,      /* 64M  bits/sec */
	RATE_128M				= 0xC,      /* 128M  bits/sec */
	RATE_256M				= 0xD      	/* 256M  bits/sec */
#endif
} HAL_INGRESS_RATE,HAL_EGRESS_RATE;

/* Ingress packets type define */
typedef enum {
	FRAME_ALL 				= 0,		/* limit and count all frames */
	FRAME_FLOOD				= 1,		/* limit and count Broadcast, Multicast and flooded unicast frames */
	FRAME_BRDCST_MLTCST		= 2,		/* limit and count Broadcast and Multicast frames */
	FRAME_BRDCST			= 3			/* limit and count Broadcast frames */
} HAL_INGRESS_FRAME_TYPE;

/******************************************************************************************
	      Rate Control Configuration for NMS
 ******************************************************************************************/
#if (OB_NMS_PROTOCOL_VERSION == 1)
#define RATE_CTRL_MASK_INGRESS_LIMIT_MODE	0xC000		/* Port rate control config mask for ingress limit mode */
#define RATE_CTRL_MASK_INGRESS_PRI3_RATE	0x2000		/* Port rate control config mask for priority 3 rate limit */
#define RATE_CTRL_MASK_INGRESS_PRI2_RATE	0x1000		/* Port rate control config mask for priority 2 rate limit */
#define RATE_CTRL_MASK_INGRESS_PRI1_RATE	0x0800		/* Port rate control config mask for priority 1 rate limit */
#define RATE_CTRL_MASK_INGRESS_PRI0_RATE	0x0700		/* Port rate control config mask for priority 0 rate limit */
#define RATE_CTRL_MASK_EGRESS_RATE			0x0007		/* Port rate control config mask for egress rate limit */
#else
#define RATE_CTRL_MASK_INGRESS_LIMIT_MODE	0xC000		/* Port rate control config mask for ingress limit mode */
#define RATE_CTRL_MASK_INGRESS_ENBALE		0x2000		/* Port rate control config mask for ingress limit enable */
#define RATE_CTRL_MASK_EGRESS_ENABLE		0x1000		/* Port rate control config mask for egress limit enable */
#define RATE_CTRL_MASK_INGRESS_RATE			0x0F00		/* Port rate control config mask for priority 0 rate limit */
#define RATE_CTRL_MASK_EGRESS_RATE			0x000F		/* Port rate control config mask for egress rate limit */
#endif

typedef struct {
	uint8	PortNum;
	uint8	RateCtrlConfig[MAX_PORT_NUM * 2];
} hal_rate_conf_t;

/************************* Struct for OBNet NMS message **********************************/
#if MODULE_OBNMS

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	RateControl[MAX_PORT_NUM * 2];
} obnet_set_rate_ctrl, obnet_rsp_get_rate_ctrl;

#endif

/******************************************************************************************
	      Exported Functions
 ******************************************************************************************/
int hal_swif_set_ingress_rate(uint8 lport, HAL_INGRESS_FRAME_TYPE frame_type, HAL_INGRESS_RATE ingress_rate, HAL_BOOL pri1_mode, HAL_BOOL pri2_mode, HAL_BOOL pri3_mode);
int hal_swif_set_egress_rate(uint8 lport, HAL_EGRESS_RATE egress_rate);
int hal_swif_rate_ctrl_conf_initialize(void);

#if MODULE_OBNMS
void nms_rsp_set_rate_ctrl(u8 *DMA, u8 *RequestID, obnet_set_rate_ctrl * pRateCtrl);
void nms_rsp_get_rate_ctrl(u8 *DMA, u8 *RequestID);
#endif




#endif	/* _HAL_SWIF_RATE_CTRL_H_ */

