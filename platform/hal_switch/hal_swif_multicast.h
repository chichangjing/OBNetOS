
#ifndef _HAL_SWIF_MULTICAST_H_
#define _HAL_SWIF_MULTICAST_H_

#define MULTI_CFG_OPTIMIZAION	1
/*****************************************************************************************
 			Static Multicast Configuration			
 ******************************************************************************************/
#define MAX_MCAST_RECORD_COUNT 32

typedef struct {
	u8	Mac[6];
#if MULTI_CFG_OPTIMIZAION
	u8	Member[4];
#else
	u8	Member[2];
#endif
} multicast_rec;

typedef struct _mcast_config {
	u8	PortNum;
	u8	TotalRecordCount;
#if MULTI_CFG_OPTIMIZAION	
	u8	PortDefaultForward[4];
#endif	
} multicast_conf_t;

/*****************************************************************************************
 			Struct for OBNet NMS message			
 ******************************************************************************************/
typedef struct {
	u8	Mac[6];
#if MULTI_CFG_OPTIMIZAION
	u8	Member[4];
#else
	u8	Member[2];
#endif
} obnet_multicast_rec;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
#if MULTI_CFG_OPTIMIZAION	
	u8	PortDefaultForward[4];
#endif
	u8	OpCode;
	u8	RecordCount;
} obnet_set_multicast, obnet_rsp_get_multicast;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	OpCode;
} obnet_get_multicast;

/*****************************************************************************************
 			Local used			
 ******************************************************************************************/


/******************************************************************************************
	      Exported Functions
 ******************************************************************************************/
int hal_swif_mcast_conf_initialize(void);

#if MODULE_OBNMS
void nms_rsp_set_multicast(u8 *DMA, u8 *RequestID, obnet_set_multicast *pSetMcast);
void nms_rsp_get_multicast(u8 *DMA, u8 *RequestID, obnet_get_multicast *pGetMcast);
#endif

#endif


