
#ifndef __CONF_RING_H
#define __CONF_RING_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"
#include "obring.h"

enum {
	CUSTOM_MODE 	= 0x00,
	FAST_MODE		= 0x01,
	DIS_STORM_MODE 	= 0x02
};

typedef struct _ring_pair_conf {
	u8	logic_port_num;
	u8	ring_mode;
	u8	link_up_hold;
	u8	link_down_hold;
	u8	hello_time;
	u8	ballot_time;
	u8	res;
	u8	message_age;
} port_pair_conf_t;

typedef __packed struct _ring_conf {
	u8	reserved;
	u8	ring_num;
	u8	ring_gate[4];
	port_pair_conf_t config[MAX_RING_NUM * 2];
} ring_conf_t;


int conf_get_ring_num(u8 *ring_num);
int conf_set_ring_global(tRingConfigGlobal *RingGlobalCfg);
int conf_get_ring_global(tRingConfigGlobal *RingGlobalCfg);
int conf_set_ring_record(unsigned char RecIndex, tRingConfigRec *RingConfigRecord);
int conf_get_ring_record(unsigned char RecIndex, tRingConfigRec *RingConfigRecord);
int conf_set_ring_disable(unsigned char RecIndex);
int conf_set_ring_enable(unsigned char RecIndex);

#ifdef __cplusplus
}
#endif

#endif


