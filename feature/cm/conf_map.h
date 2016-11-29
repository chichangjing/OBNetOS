

#ifndef _CONF_MAP_H
#define _CONF_MAP_H

#define CONF_SUCCESS							1
#define CONF_FAILURE							0

/* 0x0000 Special define */
#define NVRAM_ADDR_SPECIAL						0x0000				
#define NVRAM_UPGRADE_FLAG						(NVRAM_ADDR_SPECIAL + 0x40)
#define NVRAM_BOOT_DELAY						(NVRAM_ADDR_SPECIAL + 0x41)
#define NVRAM_CONSOLE_ENABLE					(NVRAM_ADDR_SPECIAL + 0x42)
#define NVRAM_CLI_LOGIN_DISABLE					(NVRAM_ADDR_SPECIAL + 0x43)
#define NVRAM_LOADER_VERSION					(NVRAM_ADDR_SPECIAL + 0x48)
#define NVRAM_FIRMWARE_SIZE						(NVRAM_ADDR_SPECIAL + 0x50)
#define NVRAM_FIRMWARE_CRC32					(NVRAM_ADDR_SPECIAL + 0x54)	

#define MAX_LOADER_VERSION_SIZE					8

/* 0x0100 Factory information */
#define NVRAM_ADDR_FACTORY_INFO					0x0100
#define NVRAM_MAC								(NVRAM_ADDR_FACTORY_INFO + 0x60)


/* 0x0200 Device property configuration*/
#define NVRAM_DEVICE_NAME						0x0200
#define NVRAM_SERIAL_NUMBER						0x0220
#define NVRAM_VERSION_BASE						0x0230	/* 32+20+20 Bytes */
#define NVRAM_SYS_VERSION						0x0230	/* 32 Bytes */
#define NVRAM_HARDWARE_VERSION					0x0250	/* 20 Bytes */
#define NVRAM_SOFT_VERSION						0x0264	/* 20 Bytes */
#define NVRAM_IP								0x02C0
#define NVRAM_NETMASK							0x02C4
#define NVRAM_GATEWAY							0x02C8

#define CONF_VERSION_SIZE						72
#define CONF_SYS_VERSION_SIZE					32
#define CONF_HARDWARE_VERSION_SIZE				20
#define CONF_SOFT_VERSION_SIZE					20

/* 0x0300 UART Server configuration */
#define NVRAM_UART_CFG_BASE						0x0300
#define NVRAM_UART_EN(port)						(NVRAM_UART_CFG_BASE+0x80*port)	
#define NVRAM_BAUDRATE(port)					(NVRAM_UART_CFG_BASE+0x80*port+0x0001)	
#define NVRAM_WORDLENGTH(port)					(NVRAM_UART_CFG_BASE+0x80*port+0x0002)			
#define NVRAM_STOPBITS(port)					(NVRAM_UART_CFG_BASE+0x80*port+0x0003)	
#define NVRAM_PARITY(port)						(NVRAM_UART_CFG_BASE+0x80*port+0x0004)	
#define NVRAM_FLOWCTRL(port)					(NVRAM_UART_CFG_BASE+0x80*port+0x0005)	
#define NVRAM_WORK_MODE(port)					(NVRAM_UART_CFG_BASE+0x80*port+0x0008)	
#define NVRAM_UARTCFG_MODE_BASE(port)			(NVRAM_UART_CFG_BASE+0x80*port+0x0010)


/* 0x0600 Port configuration */
#define NVRAM_PORT_CFG_BASE						0x0600
#define NVRAM_PORT_CFG_DATA						0x0601

/* 0x0700 Global configuration */
#define NVRAM_GLOBAL_CFG_BASE					0x0700

#define NVRAM_TRAP_BASE							0x0710
#define NVRAM_TRAP_ENABLE						(NVRAM_TRAP_BASE+0x0)
#define NVRAM_TRAP_SERVER_MAC					(NVRAM_TRAP_BASE+0x1)
#define NVRAM_TRAP_FRAME_GATE					(NVRAM_TRAP_BASE+0x7)	

/* 0x0720 switch signal configuration*/
#define NVRAM_ALARM_KIN_BASE					0x0720
#define NVRAM_ALARM_KIN_ENABLE                  (NVRAM_ALARM_KIN_BASE+0x0000)
#define NVRAM_ALARM_KIN_SAMPCYCLE               (NVRAM_ALARM_KIN_ENABLE+0x0001)
#define NVRAM_ALARM_KIN_JITTEREN                (NVRAM_ALARM_KIN_SAMPCYCLE+0x0001)
#define NVARM_ALARM_KIN_JITTERTIM               (NVRAM_ALARM_KIN_JITTEREN+0x0001)
#define NVARM_ALARM_KIN_WORKMODE                (NVARM_ALARM_KIN_JITTERTIM+0x0001)
#define NVARM_ALARM_KIN_IP                      (NVARM_ALARM_KIN_WORKMODE+0x0001)
#define NVARM_ALARM_KIN_PORT                    (NVARM_ALARM_KIN_IP+0x0004)
#define NVARM_ALARM_KIN_CHAN_NUM                (NVARM_ALARM_KIN_PORT+0x0008)
#define NVARM_ALARM_KIN_CHAN_PARAM(chanid)      (NVARM_ALARM_KIN_CHAN_NUM+(chanid))

/* 0x0800 Port Rate configuration */
#define NVRAM_PORT_RATE_CFG_BASE				0x0800

/* 0x0900 Port Mirror configuration */
#define NVRAM_PORT_MIRROR_CFG_BASE				0x0900

/* 0x0A00 ~ 0x0BFF Port Security configuration */
#define NVRAM_PORT_SECURITY_CFG_BASE			0x0A00
#define NVRAM_PORT_SECURITY_REC_CFG_BASE		0x0A80

/* 0x0C00 QoS configuration */
#define NVRAM_QOS_CFG_BASE						0x0C00

/* 0x0C80 PVID configuration */
#define NVRAM_PORT_VLAN_CFG_BASE				0x0C80

/* 0x0D00 Port Isolation configuration */
#define NVRAM_PORT_ISOLATION_CFG_BASE			0x0D00

/* 0x0D80 802.1Q VLAN configuration */
#define NVRAM_VLAN_CFG_BASE						0x0D80

/* 0x0D88 ADM VLAN configuration */
#define NVRAM_ADM_VLAN_CFG_BASE					0x0D88

/* 0x0D90 802.1Q VLAN record configuration */
#define NVRAM_VLAN_RECORD_CFG_BASE				0x0D90

/* 0x1000 Ring configuration */
#define NVRAM_RING_CFG_BASE						0x1000
#define NVRAM_RING_GLOBAL_CFG_BASE				0x1000
#define NVRAM_RING_RECORD_CFG_BASE				0x1010
#define NVRAM_RING_RECORD_SIZE					32
#define NVRAM_RING_PAIR_NUM						0x1001
#define NVRAM_RING_PAIR_MASK					0x1002

/* 0x1100 Multicast configuration */
#define NVRAM_MCAST_CFG_BASE					0x1100
#define NVRAM_MCAST_RECORD_CFG_BASE				0x1120

/* 0x1200 Port trunk configuration */
#define NVRAM_PORT_TRUNK_CFG_BASE				0x1200
#define NVRAM_PORT_TRUNK_RECORD_CFG_BASE		0x1220

/* 0x1300 OBRing configuration */
#define NVRAM_OBRING_CFG_BASE					0x1300
#define NVRAM_OBRING_GLOBAL_CFG_BASE			0x1300
#define NVRAM_OBRING_RECORD_CFG_BASE			0x1310
#define NVRAM_OBRING_RECORD_SIZE				64

/* 0x1500 MIB-2 system */
#define NVRAM_SYS_CONTACT_SIZE                  16
#define NVRAM_SYS_NAME_SIZE                     16
#define NVRAM_SYS_LOCATION_SIZE                 16

#define NVRAM_SYSTEM_CFG_BASE				    0x1500
#define NVRAM_SYS_CONTACT				        (NVRAM_SYSTEM_CFG_BASE)
#define NVRAM_SYS_NAME				            (NVRAM_SYS_CONTACT + NVRAM_SYS_CONTACT_SIZE)
#define NVRAM_SYS_LOCATION				        (NVRAM_SYS_NAME + NVRAM_SYS_NAME_SIZE)

#endif


