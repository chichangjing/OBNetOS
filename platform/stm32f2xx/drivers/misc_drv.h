

#ifndef __MISC_DRV_H__
#define __MISC_DRV_H__

#include "stm32f2xx.h"


typedef struct {
	u8	BoardName[16];
	u8	BoardType[8];
	u8	PortNum;
	u8	HardwareVer[2];
	u8	FirmwareVer[2];
	u8	ChipType;
	u8	FeatureMask[4];
	u8	IpAddress[4];
} dev_base_info_t;

void board_early_initialize(void);
void misc_initialize(void);

#endif	/* __MISC_DRV_H__ */


