

#ifndef __LOADER_CONFIG_H__
#define __LOADER_CONFIG_H__


#define COM_USART1				0
#define COM_USART2				1
#define COM_USART3				2
#define COM_UART4				3
#define COM_UART5				4
#define COM_USART6				5

#define LOADER_VERSION			"1.5"

#define BOARD_GE22103MA			0
#define BOARD_GE20023MA			0
#define BOARD_GE11014MA			0
#define BOARD_GE1040PU			0
#define BOARD_GE204P0U			0
#define	BOARD_GE2C400U			0
#define BOARD_GE11144MD         0
#define BOARD_GE11500MD         0
#define BOARD_GV3S_HONUE_QM     0
#define BOARD_GE_EXT_22002EA    0
#define BOARD_GE220044MD		1

#if BOARD_GE22103MA
#if 0
#define MAJOR_VER_1_1			0x11
#define MAJOR_VER_1_3			0x13
#define MAJOR_VERSION			MAJOR_VER_1_3		/* PCB: v1.1(0x11), v1.2(0x12) */
#if (MAJOR_VERSION > MAJOR_VER_1_1)
#define CONSOLE_PORT			COM_USART3
#else
#define CONSOLE_PORT			COM_UART5
#endif
#endif
#define BOARD_NAME				"GE22103MA"

#elif BOARD_GE20023MA
#define CONSOLE_PORT			COM_UART5
#define BOARD_NAME				"GE20023MA"

#elif BOARD_GE11014MA
#define CONSOLE_PORT			COM_USART3
#define BOARD_NAME				"GE11014MA"

#elif BOARD_GE11144MD
#define CONSOLE_PORT			COM_USART3
#define BOARD_NAME				"GE11144MD"

#elif BOARD_GE1040PU
#define CONSOLE_PORT			COM_USART6
#define BOARD_NAME				"GE1040PU"

#elif BOARD_GE204P0U
#define CONSOLE_PORT			COM_USART6
#define BOARD_NAME				"GE204P0U"

#elif BOARD_GE2C400U
#define CONSOLE_PORT			COM_USART6
#define BOARD_NAME				"GE2C400U"

#elif BOARD_GE11500MD
#define CONSOLE_PORT			COM_USART6
#define BOARD_NAME				"GE11500MD"

#elif BOARD_GV3S_HONUE_QM
#define CONSOLE_PORT			COM_USART3
#define BOARD_NAME				"GV3S_HONUE_QM"

#elif BOARD_GE_EXT_22002EA
#define CONSOLE_PORT			COM_USART3
#define BOARD_NAME				"BOARD_GE_EXT_22002EA"

#elif BOARD_GE220044MD
#define CONSOLE_PORT			COM_USART3
#define BOARD_NAME				"BOARD_GE220044MD"


#endif

#endif

