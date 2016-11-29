
#ifndef __NMS_UPGRADE_H__
#define __NMS_UPGRADE_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"

void RspNMS_FirmwareUpgradeStart(u8 *DMA, u8 *RequestID);
void RspNMS_FirmwareUpgradeDoing(u8 *DMA, u8 *RequestID, u8 *DataBuffer);
void RspNMS_FirmwareUpgradeComplete(u8 *DMA, u8 *RequestID);
	
#ifdef __cplusplus
}
#endif

#endif


