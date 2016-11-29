
#ifndef __NMS_GLOBAL_H__
#define __NMS_GLOBAL_H__

#include "conf_global.h"

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	Res;
	tTrapConfig TrapCfg;
	tKinAlarmConfig KinAlarmCfg;
} obnet_rsp_get_global_config, obnet_set_global_config;

void nms_rsp_get_global_config(u8 *DMA, u8 *RequestID);
void nms_rsp_set_global_config(u8 *DMA, u8 *RequestID, obnet_set_global_config *pGlobalConfig);

#ifdef __cplusplus
}
#endif

#endif







