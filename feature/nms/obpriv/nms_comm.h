

#ifndef __NMS_COMM_H__
#define __NMS_COMM_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "mconfig.h"

#if SWITCH_CHIP_88E6095
typedef struct ethernet_head {
	u8	dma[6];
	u8	sma[6];
	u8	switchHeader[4];
	u8	type[2];
}ETHER_HEAD, *PETHER_HEAD;
#define SWITCH_TAG_LEN		4
#define ETHER_HEAD_SIZE		18

#elif SWITCH_CHIP_BCM53101
typedef struct ethernet_head {
	u8	dma[6];
	u8	sma[6];
	u8	switchHeader[4];
	u8	type[2];
}ETHER_HEAD, *PETHER_HEAD;
#define SWITCH_TAG_LEN		4
#define ETHER_HEAD_SIZE		18

#elif SWITCH_CHIP_BCM53286
typedef struct ethernet_head {
	u8	switchHeader[8];
	u8	dma[6];
	u8	sma[6];
	u8	type[2];
}ETHER_HEAD, *PETHER_HEAD;
#define SWITCH_TAG_LEN		8
#define ETHER_HEAD_SIZE		22

#elif SWITCH_CHIP_BCM5396
typedef struct ethernet_head {
	u8	dma[6];
	u8	sma[6];
	u8	switchHeader[6];
	u8	type[2];
}ETHER_HEAD, *PETHER_HEAD;
#define SWITCH_TAG_LEN		6
#define ETHER_HEAD_SIZE		20

#elif SWITCH_CHIP_BCM53115
typedef struct ethernet_head {
	u8	dma[6];
	u8	sma[6];
	u8	switchHeader[4];
	u8	type[2];
}ETHER_HEAD, *PETHER_HEAD;
#define SWITCH_TAG_LEN		4
#define ETHER_HEAD_SIZE		18

#endif

typedef __packed struct OBNET_head
{
	u8	OrgCode[3];
	u8	ProtoType[2];
	u8	Version;
	u8	MessageType;
	u16	MessageLength;
	u8	RequestID[2];
	u8	SwitchMac[6];
}OBNET_HEAD, *POBNET_HEAD;

#define OBNET_HEAD_SIZE		17
#define PAYLOAD_OFFSET		(ETHER_HEAD_SIZE + OBNET_HEAD_SIZE) //18+17

/*********************************************************************************************************/
#ifndef MAC_LEN
#define MAC_LEN     		6
#endif
#define SWITCHTYPE_LEN		8

/*********************************************************************************************************
						Frame Message Type Definitions
 *********************************************************************************************************/
//#define USE_ABCODE

#define MSG_POSITION			20
#if SWITCH_CHIP_BCM53286
#define MSG_MINSIZE				64
#elif SWITCH_CHIP_BCM5396
#define MSG_MINSIZE				64
#elif SWITCH_CHIP_BCM53115
#define MSG_MINSIZE				64
#elif SWITCH_CHIP_BCM53101
#define MSG_MINSIZE				64
#else
#define MSG_MINSIZE				60
#endif
#define MSG_MINSIZE_EX			0x80
#define MSG_MAXSIZE				512//1024

#ifdef USE_ABCODE
#define	MSG_GET					0x80
#define MSG_SET					0x81
#define	MSG_RESPONSE			0x82
#define MSG_TRAP				0x83
#define MSG_TRAP_RESPONSE		0x84
#else
#define	MSG_GET					0x90
#define MSG_SET					0x91
#define	MSG_RESPONSE			0x92
#define MSG_TRAP				0x93
#define MSG_TRAP_RESPONSE		0x94
#endif

/*********************************************************************************************************
						Get Code Definitions for Protocol(0x0002)
 *********************************************************************************************************/

#define CODE_POSITION				31

#ifdef USE_ABCODE
#define CODE_PAGING					0x00
#else
#define CODE_PAGING					0xff
#endif
#define CODE_TOPO					0x01
#define CODE_MODEL					0x10
#define CODE_GET_VERSION			0x22

#define CODE_GLOBAL_CONFIG			0x40
#define CODE_PORT_CONFIG			0x41
#define CODE_RING_CONFIG			0x42

#define CODE_PORT_STATUS			0x02
#define CODE_GET_NAMEID				0x21
#define CODE_INDICATION				0xf0
#define CODE_REBOOT					0xf2
#define CODE_RESETCONFIG			0xf3
#define CODE_BATCHREBOOT			0xf9

#define CODE_START					0xf4
#define CODE_COMPLETE				0xf5
#ifdef USE_ABCODE
#define CODE_FIRMWARE_START			0xf6
#define CODE_FIRMWARE_COMPLETE		0xf7
#define CODE_FIRMWARE				0xf8
#else
#define CODE_FIRMWARE_START			0xe1
#define CODE_FIRMWARE_COMPLETE		0xe3
#define CODE_FIRMWARE				0xe2
#endif
#define CODE_SET_MAC				0x80
#define CODE_SET_NAMEID				0x81
#define CODE_SET_VERSION			0x82
#define CODE_SET_GLOBAL_CFG			0xa0
#define CODE_SET_PORT_CFG			0xa1
#define CODE_SET_RING_CFG			0xa2
#define CODE_SET_IP					0xa4
#define CODE_GET_IP					0x44
#define CODE_SET_RATE				0xa5
#define CODE_GET_RATE				0x45
#define CODE_SET_QOS				0xa6
#define CODE_GET_QOS				0x46
#define CODE_SET_ISOLATION			0xa7
#define CODE_GET_ISOLATION			0x47
#define CODE_SET_PORT_VLAN			0xa8
#define CODE_GET_PORT_VLAN			0x48
#define CODE_SET_ADM_VLAN			0xa9
#define CODE_GET_ADM_VLAN			0x49
#define CODE_SET_VLAN				0xaa
#define CODE_GET_VLAN				0x4a
#define CODE_SET_MCAST				0xac
#define CODE_GET_MCAST				0x4c
#define CODE_SET_MIRROR				0xab
#define CODE_GET_MIRROR				0x4b
#define CODE_GET_NEIGHBOR			0x03
#define CODE_GET_PORTSTATISTICS 	0x04

#define CODE_SET_UART				0xad
#define CODE_GET_UART				0x4d
#define CODE_SET_PORT_SECURITY		0xb0
#define CODE_GET_PORT_SECURITY		0x50
#define CODE_SET_MACLIST			0xb1
#define CODE_GET_MACLIST			0x51
#if (OB_NMS_PROTOCOL_VERSION == 1)
#define CODE_SET_PORT_STATISTICS	0xb2
#define CODE_GET_PORT_STATISTICS	0x52
#endif
#define CODE_PORT_STATISTICS		0xb2
#define CODE_SET_PORT_TRUNK			0xb3
#define CODE_GET_PORT_TRUNK			0x53


/*********************************************************************************************************
						Response error code Definitions
 *********************************************************************************************************/
#define RSP_ERR_EEPROM_OPERATION				0x0F	/* EEPROM read/write error */
#define RSP_ERR_FEATURE_NOT_SUPPORT				0xE0	/* Feature not support */ 
#define RSP_ERR_INVALID_CONFIGURATION			0xE1	/* Invalid configuration */ 
#define RSP_ERR_INVALID_MAC						0x10	/* Invalid mac address */
#define RSP_ERR_INVALID_DEVICE_NAME				0x11	/* Invalid device name */
#define RSP_ERR_INVALID_SERIAL_NUMBER			0x12	/* Invalid serial number */
#define RSP_ERR_INVALID_SYSTEM_REV				0x13	/* Invalid system version */
#define RSP_ERR_INVALID_HARDWARE_REV			0x14	/* Invalid hardware version */
#define RSP_ERR_INVALID_SOFTWARE_REV			0x15	/* Invalid software version */
#define RSP_ERR_INVALID_RELAY_ALARM_SWITCH		0x16	/* Invalid relay alarm switch configuration */
#define RSP_ERR_INVALID_ALARM_MESSAGE_SWITCH	0x17	/* Invalid alarm message switch configuration */
#define RSP_ERR_INVALID_SERVER_MAC_CFG			0x18	/* Invalid server mac address */
#define RSP_ERR_INVALID_AGE_TIME_CFG			0x19	/* Invalid age time configuration */
#define RSP_ERR_INVALID_PORT_CFG				0x1A	/* Invalid port configuration */
#define RSP_ERR_INVALID_RING_PAIR_SWITCH_CFG	0x1B	/* Invalid ring port pair switch configuration */
#define RSP_ERR_INVALID_RING_PORT_CFG			0x1C	/* Invalid ring port configuration */
#define RSP_ERR_INVALID_IP_CFG					0x20	/* Invalid ip address */
#define RSP_ERR_INVALID_NETMASK_CFG				0x21	/* Invalid subnet mask */
#define RSP_ERR_INVALID_GATEWAY_CFG				0x22	/* Invalid gateway ip address */
#define RSP_ERR_INVALID_PKT_INDEX				0x2F	/* Invalid packet index */

/*********************************************************************************************************
						
 *********************************************************************************************************/

typedef struct {
	u16	BufLen;
	u8	Buffer[MSG_MAXSIZE];	
} OBNET_NMS_MSG, *POBNET_NMS_MSG;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	Res;
} OBNET_GET_REQ, *POBNET_GET_REQ, OBNET_SET_RSP, *POBNET_SET_RSP;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8  Res;
	u8	SwitchType[SWITCHTYPE_LEN];
	u8	PortNum;
	u8	HardwareVer[2];
	u8	FirmwareVer[2];
	u8	ChipType;
	u8	FeatureMask[4];
	u8	IpAddress[4];
} OBNET_RSP_PAGING, *POBNET_RSP_PAGING;

#ifdef __cplusplus
}
#endif

#endif


