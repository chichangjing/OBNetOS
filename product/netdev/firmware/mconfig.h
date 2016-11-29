
#ifndef __MCONFIG_H__
#define __MCONFIG_H__

#define MODULE_LWIP				1
#define MODULE_OBNMS			1
#define MODULE_CLI				1
#define MODULE_RING				1
#define MODULE_SNMP				1
#define MODULE_SNMP_TRAP        0
#define MODULE_UDP_TCP_ECHO     0

/***************************************************************
	Serial Port Define
 ***************************************************************/
#define COM_USART1				0
#define COM_USART2				1
#define COM_USART3				2
#define COM_UART4				3
#define COM_UART5				4
#define COM_USART6				5

/***************************************************************
	Switch Chip Define
 ***************************************************************/
#define CHIP_88E6095			0x01
#define CHIP_BCM53101			0x41
#define CHIP_BCM53286			0x42
#define CHIP_BCM5396			0x43
#define CHIP_BCM53115           0x44

/***************************************************************
	Feature Define 
 ***************************************************************/
/* For Port-related */ 
#define L2_PORT_CONFIG			0x00000001
#define L2_PORT_STATISTIC		0x00000002
#define L2_PORT_MIRROR			0x00000004
#define L2_PORT_RATE_CTRL		0x00000008
/* For VLAN Isolation */ 
#define L2_PORT_VLAN			0x00000100
#define L2_8021Q_VLAN			0x00000200
/* For ARL Control */ 
#define L2_STATIC_MULTICAST		0x00001000
#define L2_MAC_FILTER			0x00002000
/* For Redundancy */
#define L2_OBRING				0x00010000
#define L2_LINK_AGGREGATION		0x00020000
/* Other L2 feature */
#define L2_QOS					0x00100000	
/* Other Custum Requirement */
#define LOCAL_TRAP				0x01000000
#define ALARM_KSIGNAL_INPUT		0x02000000

/***************************************************************
	Board Type Define
 ***************************************************************/
#define BT_GE22103MA			0xE22103FF
#define BT_GE20023MA			0xE20023FF
#define BT_GE11014MA			0xE11014FF
#define BT_GE1040PU				0xE10400FF
#define BT_GE2C400U				0xE2C400FF
#define BT_GE11144MD			0xE11144FF
#define BT_GE11500MD		    0xE11500FF
#define BT_GV3S_HONUE_QM		0xC00103FF
#define BT_GE204P0U				0xE20400FF
#define BT_GE_EXT_22002EA       0xE22002EA
#define BT_GE220044MD       	0xE22004FF


/*******************************************************************
	Configuration Your Board
	RELEASE_TRUNK_VERSION   = 0, if release from svn branches 
	RELEASE_TRUNK_VERSION   = 1, if release from svn trunk, 
	                             and edit FIRMWARE_VERSION
	OB_NMS_PROTOCOL_VERSION = 1, for OBNET protocol based on AOBO
	OB_NMS_PROTOCOL_VERSION = 2, for OBNET protocol after optimizing
 *******************************************************************/
#define OBRING_DEV			1
#define OB_NMS_PROTOCOL_VERSION	2
#define RELEASE_TRUNK_VERSION	1
#if RELEASE_TRUNK_VERSION
#define FIRMWARE_VERSION		0x1701
#endif

#define BOARD_TYPE				BT_GE220044MD

#define RURAL_CREDIT_PROJECT	1
/* The multi address for compatible credit project, if other project set 0 */
#define CREDIT_NEIG_SEARCH_MULTI_ADDR     0
/* Project set GE22103MA lport 3 to 1000M-full-linkup mode, and disable atuo, if other project set 0 */
#define LPORT3_FORCE_1000M_FULL_LINKUP	  0

#if (BOARD_TYPE == BT_GE22103MA)
#define	BOARD_GE22103MA			1
#define MARVELL_SWITCH			1
#define SWITCH_CHIP_88E6095		1
#define SWITCH_CHIP_TYPE		CHIP_88E6095
#define MAX_PORT_NUM			10
#define BOARD_FEATURE			(L2_PORT_CONFIG | L2_PORT_RATE_CTRL | LOCAL_TRAP | L2_MAC_FILTER | L2_PORT_MIRROR | L2_QOS | L2_OBRING | L2_PORT_VLAN | L2_8021Q_VLAN | L2_LINK_AGGREGATION | L2_STATIC_MULTICAST)
#define MODULE_UART_SERVER		0
#define MODULE_RS485			1
#define MODULE_SIGNAL           1
#define MODULE_IWDG             0
#define CONFIG_SWITCH_CPU_PORT	2

#elif (BOARD_TYPE == BT_GE20023MA)
#define	BOARD_GE20023MA			1
#define MAJOR_VERSION			0x01
#define CONSOLE_PORT			COM_USART3
#define ROBO_SWITCH				1
#define SWITCH_CHIP_BCM53101	1
#define SWITCH_CHIP_TYPE		CHIP_BCM53101
#define MAX_PORT_NUM			5
#define BOARD_FEATURE			(L2_PORT_CONFIG | L2_OBRING | LOCAL_TRAP)
#define MODULE_UART_SERVER		1
#define MODULE_RS485			0
#define MODULE_SIGNAL           0
#define MODULE_IWDG             0
#define CONFIG_SWITCH_CPU_PORT	2

#elif (BOARD_TYPE == BT_GE11014MA)
#define	BOARD_GE11014MA			1
#define MAJOR_VERSION			0x01
#define CONSOLE_PORT			COM_USART3
#define ROBO_SWITCH				1
#define SWITCH_CHIP_BCM53101	1
#define SWITCH_CHIP_TYPE		CHIP_BCM53101
#define MAX_PORT_NUM			6
#define BOARD_FEATURE			(L2_PORT_CONFIG)
#define MODULE_RS485			1
#define MODULE_SIGNAL           0
#define MODULE_IWDG             0
#define CONFIG_SWITCH_CPU_PORT	2

#elif (BOARD_TYPE == BT_GE1040PU)
#define	BOARD_GE1040PU			1
#define MAJOR_VERSION			0x01
#define CONSOLE_PORT			COM_USART6
#define ROBO_SWITCH				1
#define SWITCH_CHIP_BCM53286	1
#define SWITCH_CHIP_TYPE		CHIP_BCM53286
#define MAX_PORT_NUM			32
#define BOARD_FEATURE			(L2_PORT_CONFIG | LOCAL_TRAP)
#define MODULE_UART_SERVER		0
#define MODULE_RS485			1
#define MODULE_SIGNAL           1
#define MODULE_IWDG             0
#define CONFIG_SWITCH_CPU_PORT	2

#elif (BOARD_TYPE == BT_GE204P0U)
#define	BOARD_GE204P0U			1
#define MAJOR_VERSION			0x01
#define CONSOLE_PORT			COM_USART6
#define ROBO_SWITCH				1
#define SWITCH_CHIP_BCM53286	1
#define SWITCH_CHIP_TYPE		CHIP_BCM53286
#define MAX_PORT_NUM			32
#define BOARD_FEATURE			(L2_PORT_CONFIG | LOCAL_TRAP | L2_LINK_AGGREGATION)
#define MODULE_UART_SERVER		0
#define MODULE_RS485			1
#define MODULE_SIGNAL           1
#define MODULE_IWDG             0
#define CONFIG_SWITCH_CPU_PORT	2

#elif (BOARD_TYPE == BT_GE2C400U)
#define	BOARD_GE2C400U			1
#define MAJOR_VERSION			0x01
#define CONSOLE_PORT			COM_USART6
#define ROBO_SWITCH				1
#define SWITCH_CHIP_BCM5396		1
#define SWITCH_CHIP_TYPE		CHIP_BCM5396
#define MAX_PORT_NUM			16
#define BOARD_FEATURE			(L2_PORT_CONFIG | LOCAL_TRAP)
#define MODULE_RS485			1
#define CONFIG_SWITCH_CPU_PORT	2

#elif (BOARD_TYPE == BT_GE11144MD)
#define BOARD_GE11144MD			1
#define CONSOLE_PORT			COM_USART3
#define CONFIG_SWITCH_CPU_PORT	9

#elif (BOARD_TYPE == BT_GE11500MD)
#define	BOARD_GE11500MD			1
#define MAJOR_VERSION			0x01
#define CONSOLE_PORT			COM_USART6
#define ROBO_SWITCH				1
#define SWITCH_CHIP_BCM53115	1
#define SWITCH_CHIP_TYPE		CHIP_BCM53115
#define MAX_PORT_NUM			6
#define BOARD_FEATURE			(L2_PORT_CONFIG | LOCAL_TRAP)
#define MODULE_UART_SERVER		0
#define MODULE_RS485			1
#define MODULE_SIGNAL           0
#define MODULE_IWDG             0
#define CONFIG_SWITCH_CPU_PORT	2

#elif (BOARD_TYPE == BT_GV3S_HONUE_QM)
#define	BOARD_GV3S_HONUE_QM		1
#define MARVELL_SWITCH			1
#define SWITCH_CHIP_88E6095		1
#define SWITCH_CHIP_TYPE		CHIP_88E6095
#define MAX_PORT_NUM			10
#define BOARD_FEATURE			(L2_PORT_CONFIG | L2_PORT_RATE_CTRL | LOCAL_TRAP)
#define MODULE_UART_SERVER		0
#define MODULE_RS485			1
#define MODULE_SIGNAL           1
#define MODULE_IWDG             0
#define CONFIG_SWITCH_CPU_PORT	9

#elif (BOARD_TYPE == BT_GE_EXT_22002EA)
#define	BOARD_GE_EXT_22002EA	1
#define MARVELL_SWITCH			1
#define SWITCH_CHIP_88E6095		1
#define SWITCH_CHIP_TYPE		CHIP_88E6095
#define MAX_PORT_NUM			4
#define BOARD_FEATURE			(L2_PORT_CONFIG | L2_PORT_RATE_CTRL | LOCAL_TRAP | L2_MAC_FILTER | L2_PORT_MIRROR | L2_QOS | L2_OBRING | L2_PORT_VLAN | L2_8021Q_VLAN | L2_LINK_AGGREGATION | L2_STATIC_MULTICAST)
#define MODULE_UART_SERVER		0
#define MODULE_RS485			1
#define MODULE_SIGNAL           0
#define MODULE_IWDG             0
#define CONFIG_SWITCH_CPU_PORT	9

#elif (BOARD_TYPE == BT_GE220044MD)
#define	BOARD_GE220044MD		 1
#define MARVELL_SWITCH			 1
#define SWITCH_CHIP_88E6095		 1
#define SWITCH_CHIP_TYPE		 CHIP_88E6095
#define MAX_PORT_NUM			 8
#define BOARD_FEATURE			 (L2_PORT_CONFIG | L2_PORT_RATE_CTRL | LOCAL_TRAP | L2_MAC_FILTER | L2_PORT_MIRROR | L2_QOS | L2_OBRING | L2_PORT_VLAN | L2_8021Q_VLAN | L2_LINK_AGGREGATION | L2_STATIC_MULTICAST)
#define MODULE_UART_SERVER		 0
#define MODULE_RS485			 1
#define MODULE_SIGNAL            0
#define MODULE_IWDG              0
#define CONFIG_SWITCH_CPU_PORT	 9
#define CONFIG_SWITCH_DEBUG_PORT 2

#endif

#endif

