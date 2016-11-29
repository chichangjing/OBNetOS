
#ifndef _HAL_SWIF_TXRX_H_
#define _HAL_SWIF_TXRX_H_

#include "mconfig.h"
#include "hal_swif_types.h"


#if SWITCH_CHIP_88E6095
#define M88E6095_HDR_SIZE		4
#elif SWITCH_CHIP_BCM53101
#define BCM53101_BRCM_HDR_SIZE	4
#elif SWITCH_CHIP_BCM53115
#define BCM53115_BRCM_HDR_SIZE	4
#elif SWITCH_CHIP_BCM53286
#define BCM53286_BRCM_HDR_SIZE	8
#elif SWITCH_CHIP_BCM5396
#define BCM5396_BRCM_HDR_SIZE	6
uint32 bcm5396_crc32(uint32 crc, uint8 *data, uint32 len);
void bcm5396_tagged_buf_add_crc32(uint8 *buffer, uint16 *len);
#endif

#endif	/* _HAL_SWIF_TXRX_H_ */

