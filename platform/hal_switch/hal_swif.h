
#ifndef _HAL_SWIF_H_
#define _HAL_SWIF_H_

#include "mconfig.h"

#define LINK_STATUS_POLLING_DELAY	100

#if SWITCH_CHIP_BCM53286
#define	BCM53286_GE25_LINK_MASK	0x02000000	/* Bit25 */
#define	BCM53286_GE26_LINK_MASK	0x04000000	/* Bit26 */
#define	BCM53286_GE27_LINK_MASK	0x08000000	/* Bit27 */
#define	BCM53286_GE28_LINK_MASK	0x10000000	/* Bit28 */

#define	BCM53286_GE25_PORT	0x01
#define	BCM53286_GE26_PORT	0x02
#define	BCM53286_GE27_PORT	0x04
#define	BCM53286_GE28_PORT	0x08

#endif

int hal_swif_init(void);
void hal_swif_conf_initialize(void);
void hal_swif_poll_task(void *arg);
#if (BOARD_GE1040PU || BOARD_GE204P0U)
void combo_led_control_task(void *arg);
#endif
void hal_swif_traffic_entry(void);

#endif	/* _HAL_SWIF_H_ */

