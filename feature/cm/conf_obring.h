
#ifndef __CONF_OBRING_H
#define __CONF_OBRING_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"
#include "obring.h"

int conf_obring_read_global(tRingConfigGlobal *RingGlobalCfg);
int conf_obring_write_global(tRingConfigGlobal *RingGlobalCfg);
int conf_obring_read_record(tRingConfigRec *RingRecordCfg, unsigned char RecIndex);
int conf_obring_write_record(tRingConfigRec *RingRecordCfg, unsigned char RecIndex);
int conf_obring_enable_demain_id(unsigned char RecIndex);
int conf_obring_disable_demain_id(unsigned char RecIndex);
int conf_obring_write_hello_times(unsigned char RecIndex, unsigned short HelloTimes);
int conf_obring_write_fail_times(unsigned char RecIndex, unsigned short FailTimes);
int conf_obring_write_ring_port(unsigned char RecIndex, unsigned char RingPort1, unsigned char RingPort2);

#ifdef __cplusplus
}
#endif

#endif



