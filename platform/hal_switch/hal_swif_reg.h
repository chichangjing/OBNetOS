

#ifndef _HAL_SWIF_REG_H_
#define _HAL_SWIF_REG_H_

#include "mconfig.h"
#include "hal_swif_types.h"


typedef union {
    uint16	addr_val;   /* Raw 32-bit value */
    struct {
		uint8	page;
		uint8	address;
	} bcm_addr;

    struct {
		uint8	phy_addr;
		uint8	offset;
	} mvl_addr;	
	
} hal_swif_addr_t;


#endif	/* _HAL_SWIF_REG_H_ */

