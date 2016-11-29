


#ifndef _HAL_SWIF_QOS_H_
#define _HAL_SWIF_QOS_H_

#include "mconfig.h"
#include "msApiDefs.h"

/* QoS schedule mode */
typedef enum {  
	QOS_MODE_WWR	= 0,	/* SP: Strict  Priority */
	QOS_MODE_SP		= 1		/* WRR: Weight Round Robin 8:4:2:1 */
} HAL_QOS_MODE;

/******************************************************************************************
	      QOS Configuration for NMS
 ******************************************************************************************/
#define QOS_MASK_PRIORITY_MAP_COS	0x10			/*  Bit[4]  =0 	: Disable IEEETag priority map, 
																=1 	: Enable IEEETag priority map */
#define QOS_MASK_PRIORITY_MAP_TOS	0x20			/*  Bit[5]  =0 	: Disable IPTosOrDSCP priority map, 
																=1 	: Enable IPTosOrDSCP priority map */
#define QOS_MASK_PRIORITY_MAP_BOTH	0x40			/*  Bit[6]  =0 	: IPTosOrDSCP high priority than IEEETag, 
																=1 	: IEEETag high priority than IPTosOrDSCP */
#define QOS_MASK_DEFAULT_PRIORITY	0x0C			/* Default port priority */
#define QOS_MASK_EGRESS_MODE		0x03			/* Port egress mode */

typedef struct _hal_qos_config {
	uint8	CosMapping[2];
	uint8	TosMapping[16];
	uint8	QosFlag[2];
	uint8	PortNum;
	uint8	PortQosConfig[MAX_PORT_NUM];
} hal_qos_conf_t;

/*****************************************************************************************
 			Struct for OBNet NMS message			
 ******************************************************************************************/
typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	Res;
	u8	CosMapping[2];
	u8	TosMapping[16];
	u8	QosFlag[2];
	u8	PortNum;
	u8	PortQosConfig[MAX_PORT_NUM];
} obnet_set_qos, obnet_rsp_get_qos;

/******************************************************************************************
	      Exported Functions
 ******************************************************************************************/
int hal_swif_qos_conf_initialize(void);
int hal_swif_get_qos_schedule_mode(HAL_QOS_MODE *qos_mode);
int hal_swif_get_qos_cos_to_queue_map(uint8 priority, uint8 *queue_num);
int hal_swif_get_qos_tos_to_queue_map(uint8 priority, uint8 *queue_num);
int hal_swif_get_qos_priority_cos_enable(uint8 lport, HAL_BOOL *enable);
int hal_swif_get_qos_priority_tos_enable(uint8 lport, HAL_BOOL *enable);
int hal_swif_get_qos_priority_both_enable(uint8 lport, HAL_BOOL *enable);
int hal_swif_get_qos_port_default_prority_level(uint8 lport, uint8 *priority_level);
int hal_swif_get_port_egress_mode(uint8 lport, GT_EGRESS_MODE *egress_mode);

 /* Functions for NMS */
#if MODULE_OBNMS
void nms_rsp_set_qos(u8 *DMA, u8 *RequestID, obnet_set_qos *pQos);
void nms_rsp_get_qos(u8 *DMA, u8 *RequestID);
#endif


#endif	/* _HAL_SWIF_QOS_H_ */

