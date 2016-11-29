
#ifndef _HAL_SWIF_MIRROR_H_
#define _HAL_SWIF_MIRROR_H_

#include "mconfig.h"

/* Port mirror mode */
typedef enum {
	MIRROR_NONE		= 0,
	MIRROR_RX 		= 1,
	MIRROR_TX		= 2,
	MIRROR_ALL		= 3
} HAL_MIRROR_MODE;

/******************************************************************************************
	      Port mirror Configuration for NMS
 ******************************************************************************************/
#define MIRROR_SRC_MASK_MODE			0xC0		/*  Bit[7:6]=0 	: No mirror source, 
																=1 	: Ingress monitor only, 
																=2	: Egress monitor only,
																=3	: Ingress/Egress monitor */
#define MIRROR_DEST_MASK_INGRESS_ENABLE	0x80		/* Enable ingress monitor dest port */
#define MIRROR_DEST_MASK_INGRESS_PORT	0x3F		/* Ingress monitor dest port */
#define MIRROR_DEST_MASK_EGRESS_ENABLE	0x80		/* Enable egress monitor dest port */
#define MIRROR_DEST_MASK_EGRESS_PORT	0x3F		/* Egress monitor dest port */

typedef struct {
	uint8	PortNum;
	uint8	IngressMirrorDestConfig;
	uint8	EgressMirrorDestConfig;
	uint8	MirrorSourceConfig[MAX_PORT_NUM];
} hal_mirror_conf_t;

/* OBNet configuration define */
#define MIRROR_MASK_MODE	0xC0000000		/* Bit[31:30]=0 : Egress monitor, 
												   Bit[31:30]=1 : Ingress monitor, 
												   Bit[31:30]=2 : Egress/Ingress monitor */
#define MIRROR_MASK_DEST	0x3FFFFFFF		/* Egress/Ingress monitor dest */

typedef struct {
	u8	PortNum;
	u8	SourcePort[MAX_PORT_NUM * 4];
} mirror_conf_t;

/*****************************************************************************************
 			Struct for OBNet NMS message			
 ******************************************************************************************/
typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	SourcePort[MAX_PORT_NUM * 4];
} obnet_set_port_mirror, obnet_rsp_get_port_mirror;

/******************************************************************************************
	      Exported Functions
 ******************************************************************************************/
int hal_swif_set_ingress_mirror_dest(uint8 IngressDestLport);
int hal_swif_set_egress_mirror_dest(uint8 EgressDestLPort);
int hal_swif_add_mirror_source(uint8 lport, HAL_MIRROR_MODE MirrorMode);

int hal_swif_mirror_conf_initialize(void);

 /* Functions for NMS */
#if MODULE_OBNMS
void nms_rsp_set_port_mirror(u8 *DMA, u8 *RequestID, obnet_set_port_mirror *pPortMirror);
void nms_rsp_get_port_mirror(u8 *DMA, u8 *RequestID);
#endif

#endif	/* _HAL_SWIF_MIRROR_H_ */


