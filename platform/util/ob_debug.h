
#ifndef __OB_DEBUG_H__
#define __OB_DEBUG_H__

#include "stm32f2xx.h"

#define DBG_ALL					0xFFFFFFFF
#define DBG_NMS					0x00000001
#define DBG_OBRING				0x00000002
#define DBG_CLI					0x00000004
#define DBG_CONFIG				0x00000008
#define DBG_UART_SRV			0x00000010
#define DBG_CMD					0x00001000

#define OB_DEBUG(m, fmt, params...) ob_debug_print(m,fmt,##params)

void ob_debug_init(void);
void ob_debug_print(int module, char* fmt, ...);
u32 ob_debug_module_get(void);
void ob_debug_module_set(u32 module);
void ob_debug_module_add(u32 module);
void ob_debug_module_del(u32 module);
u32 ob_debug_module_check(u32 module);
#endif



