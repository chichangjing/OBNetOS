
#ifndef __MCONFIG_H__
#define __MCONFIG_H__

#define MODULE_LWIP				1
#define MODULE_OBNMS			1
#define MODULE_CLI				1
#define MODULE_UART_SERVER		0
#define MODULE_RING				1

#define SERIAL_DEBUG            1

#define BOARD_GV3S_HONUE_QM_V1_00  	1
#if BOARD_GE22103MA
#define CONSOLE_USE_UART5		1
#define CONFIG_SWITCH_CPU_PORT	2
#elif BOARD_GE22103MA_V1_20
#define MODULE_SIGNAL           1
#define CONSOLE_USE_USART3		1
#define CONFIG_SWITCH_CPU_PORT	0
#elif BOARD_GV3S_HONUE_QM_V1_00
#define MODULE_SIGNAL           0
#define CONSOLE_USE_USART3		1
#define CONFIG_SWITCH_CPU_PORT	9
#elif BOARD_GE11144MD
#define CONSOLE_USE_USART3		1
#define CONFIG_SWITCH_CPU_PORT	9
#endif

#define SWITCH_CHIP_88E6095		1

#endif

