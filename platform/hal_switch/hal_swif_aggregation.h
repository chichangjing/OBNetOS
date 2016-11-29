
#ifndef _HAL_SWIF_AGGREGATION_H_
#define _HAL_SWIF_AGGREGATION_H_

/*****************************************************************************************
 			Port Aggregation Configuration			
 ******************************************************************************************/
#if SWITCH_CHIP_88E6095
	#define MAX_AGGREGATION_RECORD_COUNT 	8
	#define MAX_AGGREGATION_GROUP_ID		15
	#define MAX_AGGREGATION_GROUP_PORT_NUM	8	// for M88E6095
#elif SWITCH_CHIP_BCM53286
	#define MAX_AGGREGATION_RECORD_COUNT 	8
	#define MAX_AGGREGATION_GROUP_ID		14
	#define MAX_AGGREGATION_GROUP_PORT_NUM	8	
#else
	#define MAX_AGGREGATION_RECORD_COUNT 	8
	#define MAX_AGGREGATION_GROUP_ID		8
	#define MAX_AGGREGATION_GROUP_PORT_NUM	8	
#endif 	
	
typedef struct {
	u8	PortNum;
	u8	TotalRecordCount;
} link_aggregation_conf_t;

typedef struct {
	u8	AggrId;
	u8	MemberVec[4];
} link_aggregation_rec;

typedef struct {
	u8	AggrId;
	u8	AggrPortNum;
	u8	PortNo[MAX_AGGREGATION_GROUP_PORT_NUM];
} link_aggregation_member;

/*****************************************************************************************
 			Struct for OBNet NMS message			
 ******************************************************************************************/
typedef struct {
	u8	TrunkId;
	u8	MemberVec[4];
} obnet_port_aggregation_rec;

typedef struct {
	u8	GetCode; 
	u8	RetCode;
	u8	PortNum;
	u8	OpCode;
	u8	RecordCount;
} obnet_set_port_aggregation, obnet_rsp_get_port_aggregation;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	OpCode;
} obnet_get_port_aggregation;

/*****************************************************************************************
 			Local used			
 ******************************************************************************************/
typedef struct {
	u8	AggrId;
	u8	PortNumber;
	u32	PortMask;
} aggr_rec;

typedef struct {
	u8	AggrCount;
	aggr_rec AggrGroupRec[MAX_AGGREGATION_RECORD_COUNT];
} aggr_info_t;

/******************************************************************************************
	      Exported Functions
 ******************************************************************************************/
int hal_swif_aggr_link_changed(u8 lport, HAL_PORT_LINK_STATE new_state);
int hal_swif_aggr_show_status(void *pCliEnv);

int hal_swif_aggr_conf_initialize(void);

#if MODULE_OBNMS
void nms_rsp_set_port_aggregation(u8 *DMA, u8 *RequestID, obnet_set_port_aggregation *pSetPortAggr);
void nms_rsp_get_port_aggregation(u8 *DMA, u8 *RequestID, obnet_get_port_aggregation *pGetPortAggr);
#endif

#endif

