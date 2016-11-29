
#ifndef __MCONFIG_H__
#define __MCONFIG_H__

#define MODULE_LWIP				1
#define MODULE_OBNMS			1
#define MODULE_CLI				1
#define MODULE_UART_SERVER		1
#define MODULE_SIGNALE          1
#define MODULE_RING				1

#define BOARD_GE20023MA			1
#if BOARD_GE20023MA
#define CONFIG_SWITCH_CPU_PORT	0x02
#define CONSOLE_USE_UART5		1
#define SWITCH_CHIP_BCM53101	1
#endif

#endif

