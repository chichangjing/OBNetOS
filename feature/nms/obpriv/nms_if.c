
/*************************************************************
 * Filename     : nms_if.c
 * Description  : API for NMS interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#include "mconfig.h"

#if MODULE_OBNMS
/* Standard includes */
#include "stdio.h"
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "os_mutex.h"

/* LwIP include */
#include "lwip/netif.h"

/* BSP includes */
#include "stm32f2xx.h"
#include "soft_i2c.h"
#include "misc_drv.h"

/* Other includes */
#include "nms_comm.h"
#include "nms_if.h"
#include "nms_sys.h"
#include "nms_upgrade.h"
#include "nms_port.h"
#include "nms_global.h"
#include "nms_signal.h"
#include "nms_uart.h"
#include "nms_ring.h"

#include "hal_swif_port.h"
#include "hal_swif_rate_ctrl.h"
#include "hal_swif_mirror.h"
#include "hal_swif_message.h"
#include "hal_swif_mac.h"
#include "hal_swif_qos.h"
#include "hal_swif_vlan.h"
#include "hal_swif_aggregation.h"
#include "hal_swif_multicast.h"

#include "cli_util.h"

static xQueueHandle xNmsMsgRxQueue;
static OBNET_NMS_MSG NMS_TxMessage;
static OBNET_NMS_MSG NMS_RxMessage;

u8 NMS_TxBuffer[MSG_MAXSIZE];

static char *CodeParse(u8 code)
{
	switch (code) {
		case CODE_PAGING: 				return "Paging"; break;
		case CODE_TOPO: 				return "Get TOPO"; break;
		case CODE_PORT_STATUS:			return "Get Port Stauts"; break;		
		case CODE_GET_NAMEID:			return "Get NameID"; break;
		case CODE_GET_VERSION:			return "Get Version"; break;
		case CODE_GLOBAL_CONFIG:		return "Get Global Configuration"; break;
		case CODE_PORT_CONFIG:			return "Get Port Configuration"; break;
		case CODE_RING_CONFIG:			return "Get Ring Configuration"; break;
		case CODE_GET_MIRROR:			return "Get Mirror"; break;
		case CODE_GET_IP:				return "Get IP information"; break;
		case CODE_GET_RATE:				return "Get Rate"; break;		
		case CODE_GET_QOS:				return "Get QoS"; break;		
		case CODE_GET_ISOLATION:		return "Get Isolation"; break;		
		case CODE_GET_PORT_VLAN:		return "Get Port VLAN"; break;	
		case CODE_GET_ADM_VLAN:			return "Get ADM VLAN"; break;
		case CODE_GET_VLAN:				return "Get VLAN"; break;	
		case CODE_GET_MCAST:			return "Get Multicast"; break;			
		case CODE_GET_UART:				return "Get UART Configuration"; break;
		case CODE_GET_PORT_SECURITY:	return "Get Port Security configuration"; break;
		case CODE_GET_MACLIST:			return "Get Port MacList"; break;
		case CODE_GET_PORT_TRUNK:		return "Get Port Trunk"; break;
		case CODE_GET_NEIGHBOR:			return "Get NeighborInfo"; break;
		
		case CODE_START:				return "Set Start"; break;
		case CODE_COMPLETE:				return "Set Complete"; break;
		case CODE_RESETCONFIG:			return "Reset Configuration"; break;
		case CODE_SET_MAC:				return "Set MAC address"; break;
		case CODE_SET_NAMEID:			return "Set NameID"; break;
		case CODE_SET_VERSION:			return "Set Version"; break;
		case CODE_SET_GLOBAL_CFG:		return "Set Global Configuration"; break;
		case CODE_SET_PORT_CFG:			return "Set Port Configuration"; break;
		case CODE_SET_IP:				return "Set IP Information"; break;
		case CODE_SET_MIRROR:			return "Set Port Mirror"; break;
		case CODE_SET_RATE:				return "Set Rate"; break;
		case CODE_SET_QOS:				return "Set QoS"; break;
		case CODE_SET_ISOLATION:		return "Set Isolation"; break;
		case CODE_SET_PORT_VLAN:		return "Set Port VLAN"; break;
		case CODE_SET_ADM_VLAN:			return "Set ADM VLAN"; break;
		case CODE_SET_VLAN:				return "Set VLAN"; break;
		case CODE_SET_MCAST:			return "Set Multicast"; break;
		case CODE_REBOOT:				return "Set Reboot"; break;
		case CODE_FIRMWARE_START:		return "Set firmare upgrade start"; break;
		case CODE_FIRMWARE:				return "Set firmare upgrade doing"; break;				
		case CODE_FIRMWARE_COMPLETE:	return "Set firmare upgrade complete"; break;	
		case CODE_SET_UART:				return "Set UART configuration"; break;
		case CODE_SET_RING_CFG:			return "Set Ring configuration"; break;	
		case CODE_SET_PORT_SECURITY:	return "Set Port Security configuration"; break;
		case CODE_SET_MACLIST:			return "Set Port MacList"; break; 
#if (OB_NMS_PROTOCOL_VERSION == 1)
		case CODE_GET_PORT_STATISTICS:	return "Get Port Statistics"; break;
		case CODE_SET_PORT_STATISTICS:	return "Set Port Statistics"; break;
#elif (OB_NMS_PROTOCOL_VERSION > 1)
		case CODE_PORT_STATISTICS:		return "Port Statistics"; break;
#endif
		case CODE_SET_PORT_TRUNK:		return "Set Port Trunk"; break;
		default: 						return "Unkown code"; break;
	}
}

void NmsMsgDump(const char *string, u8 *buf, int len, int parseFlag)
{
	unsigned int i, nbytes, linebytes;
	u8 *cp=buf;
	
	if(parseFlag) {
		cli_debug(DBG_NMS, "\r\n%s: %s\r\n", string, CodeParse(*(buf+PAYLOAD_OFFSET)));
	} else {
		cli_debug(DBG_NMS, "\r\n%s: \r\n", string);
	}
	
	nbytes = len;
	do {
		unsigned char	linebuf[16];
		unsigned char	*ucp = linebuf;
		cli_debug(DBG_NMS, "     ");
		linebytes = (nbytes > 16)?16:nbytes;
		for (i=0; i<linebytes; i+= 1) {
			cli_debug(DBG_NMS, "%02X ", (*ucp++ = *cp));
			cp += 1;
		}
		cli_debug(DBG_NMS, "\r\n");
		nbytes -= linebytes;

	} while (nbytes > 0);

	if(parseFlag) {
		POBNET_HEAD	obnethdr = (POBNET_HEAD)(buf + ETHER_HEAD_SIZE);
		cli_debug(DBG_NMS, "     (msg_type: 0x%02x, req_id: 0x%04x, code: 0x%02x)\r\n", obnethdr->MessageType, ntohs(*(u16 *)(obnethdr->RequestID)), *(buf+PAYLOAD_OFFSET));
	}
}

void buffer_dump_console(u8 *buf, int len)
{
	unsigned int i, nbytes, linebytes;
	u8 *cp=buf;

	printf("\r\n");
	
	nbytes = len;
	do {
		unsigned char	linebuf[16];
		unsigned char	*ucp = linebuf;
		printf("     ");
		linebytes = (nbytes > 16)?16:nbytes;
		for (i=0; i<linebytes; i+= 1) {
			printf("%02x ", (*ucp++ = *cp));
			cp += 1;
		}
		printf("\r\n");
		nbytes -= linebytes;

	} while (nbytes > 0);
}

#if SWITCH_CHIP_BCM5396
static void utag_switch_header(u8 *txBuffer, u16 *len)
{
	u16 i, size;

	size=*len;
	for(i=0; i<size-12-SWITCH_TAG_LEN; i++) {
		txBuffer[12+i] = txBuffer[12+SWITCH_TAG_LEN+i];
	}
	
	*len -= SWITCH_TAG_LEN;
}

static void tag_switch_header(u8 *txBuffer, u16 *len)
{
	u16 i, size;

	size=*len;
	for(i=0; i<size-12; i++) {
		txBuffer[size-1+SWITCH_TAG_LEN-i] = txBuffer[size-1-i];
	}
	txBuffer[12] = 0x88;
	txBuffer[13] = 0x74;
	txBuffer[14] = 0x00;
	txBuffer[15] = 0x00;
	txBuffer[16] = 0x00;
	txBuffer[17] = 0x00;
	
	*len += SWITCH_TAG_LEN;
}

#endif

void RspSend(u8 *txBuffer, u16 len)
{
#if SWITCH_CHIP_BCM5396
	u8	brcm_tag_bak[6];
	u8 *crcbuf;
	u32	crc;
	extern u32 bcm5396_crc32(u32 crc, u8 *data, u32 len);
#endif
	extern void EthSend(u8 *txBuffer, u16 len);

#if SWITCH_CHIP_BCM5396
	utag_switch_header(txBuffer, &len);
    crc = ~ bcm5396_crc32(~0, &txBuffer[0], len);
	tag_switch_header(txBuffer, &len);
    crcbuf = &txBuffer[len];
    *crcbuf++ = (u8)(crc >> 24);
    *crcbuf++ = (u8)(crc >> 16);
    *crcbuf++ = (u8)(crc >> 8);
    *crcbuf++ = (u8)(crc);
	NmsMsgDump("TxMsg", txBuffer, len+4, 0);
	EthSend(txBuffer, len+4);
#else
	NmsMsgDump("TxMsg", txBuffer, len, 1);
	EthSend(txBuffer, len);
#endif
}

void PrepareEtherHead(u8 *DMA)
{
	PETHER_HEAD	ethhdr = (PETHER_HEAD)&NMS_TxBuffer[0];
	extern u8 DevMac[];
	
	if (DMA != NULL)
		memcpy(ethhdr->dma, DMA, MAC_LEN);
	else
		memset(ethhdr->dma, 0xFF, MAC_LEN);
	memcpy(ethhdr->sma, DevMac, MAC_LEN);
#if SWITCH_CHIP_88E6095
	NMS_TxBuffer[12] = 0xC0;
	NMS_TxBuffer[13] = 0x00;
	NMS_TxBuffer[14] = 0x00;
	NMS_TxBuffer[15] = 0x01;
	
#elif SWITCH_CHIP_BCM53101
	NMS_TxBuffer[12] = 0x00;
	NMS_TxBuffer[13] = 0x00;
	NMS_TxBuffer[14] = 0x00;
	NMS_TxBuffer[15] = 0x00;
	
#elif SWITCH_CHIP_BCM53286
	NMS_TxBuffer[0] = 0xF0;
	NMS_TxBuffer[1] = 0x00;
	NMS_TxBuffer[2] = 0x00;
	NMS_TxBuffer[3] = 0x00;
	NMS_TxBuffer[4] = 0x00;
	NMS_TxBuffer[5] = 0x00;
	NMS_TxBuffer[6] = 0x00;
	NMS_TxBuffer[7] = 0x00;
#elif SWITCH_CHIP_BCM5396
	NMS_TxBuffer[12] = 0x88;
	NMS_TxBuffer[13] = 0x74;
	NMS_TxBuffer[14] = 0x00;
	NMS_TxBuffer[15] = 0x00;
	NMS_TxBuffer[14] = 0x00;
	NMS_TxBuffer[15] = 0x00;
#endif
	ethhdr->type[0] = 0x88;
	ethhdr->type[1] = 0xB7;
}

void PrepareOBHead(u8 MessageType, u16 MessageLength, u8 *RequestID)
{
	POBNET_HEAD	obnetHdr = (POBNET_HEAD)&NMS_TxBuffer[ETHER_HEAD_SIZE];
	extern u8 DevMac[];
	
	memcpy(obnetHdr->OrgCode, DevMac, 3);
	obnetHdr->ProtoType[0] = 0x00;
	obnetHdr->ProtoType[1] = 0x02;
	obnetHdr->Version = 0x00;
	obnetHdr->MessageType = MessageType;
	obnetHdr->MessageLength = ntohs(MessageLength);
	memcpy(obnetHdr->RequestID, RequestID, 2);
	memcpy(obnetHdr->SwitchMac, DevMac, MAC_LEN);
}


void Rsp_Paging(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_PAGING RspData;
	u16 RspLength;
	extern dev_base_info_t	DeviceBaseInfo;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_PAGING);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	RspData.GetCode = CODE_PAGING;
	RspData.RetCode = 0x00;
	RspData.Res = 0x00;
	RspData.SwitchType[0] = DeviceBaseInfo.BoardType[0];
	RspData.SwitchType[1] = DeviceBaseInfo.BoardType[1];
	RspData.SwitchType[2] = DeviceBaseInfo.BoardType[2];
	RspData.SwitchType[3] = DeviceBaseInfo.BoardType[3];
	RspData.SwitchType[4] = DeviceBaseInfo.BoardType[4];
	RspData.SwitchType[5] = DeviceBaseInfo.BoardType[5];
	RspData.SwitchType[6] = DeviceBaseInfo.BoardType[6];
	RspData.SwitchType[7] = DeviceBaseInfo.BoardType[7];
	RspData.PortNum = DeviceBaseInfo.PortNum;
	RspData.HardwareVer[0] = DeviceBaseInfo.HardwareVer[0];
	RspData.HardwareVer[1] = DeviceBaseInfo.HardwareVer[1];
	RspData.FirmwareVer[0] = DeviceBaseInfo.FirmwareVer[0];
	RspData.FirmwareVer[1] = DeviceBaseInfo.FirmwareVer[1];
	RspData.ChipType = DeviceBaseInfo.ChipType;
	RspData.FeatureMask[0] = DeviceBaseInfo.FeatureMask[0];
	RspData.FeatureMask[1] = DeviceBaseInfo.FeatureMask[1];
	RspData.FeatureMask[2] = DeviceBaseInfo.FeatureMask[2];
	RspData.FeatureMask[3] = DeviceBaseInfo.FeatureMask[3];
	RspData.IpAddress[0] = DeviceBaseInfo.IpAddress[0];
	RspData.IpAddress[1] = DeviceBaseInfo.IpAddress[1];
	RspData.IpAddress[2] = DeviceBaseInfo.IpAddress[2];
	RspData.IpAddress[3] = DeviceBaseInfo.IpAddress[3];
	
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspData, sizeof(OBNET_RSP_PAGING));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);		
}

void Rsp_LoadStart(u8 *DMA, u8 *RequestID)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	RspSet.GetCode = CODE_START;
	RspSet.RetCode = 0x00;
	RspSet.Res = 0x00;
	
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);
}

void Rsp_LoadComplete(u8 *DMA, u8 *RequestID)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	RspSet.GetCode = CODE_COMPLETE;
	RspSet.RetCode = 0x00;
	RspSet.Res = 0x00;

	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);
}

void Rsp_ResetConfig(u8 *DMA, u8 *RequestID)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	u16	eeprom_start_addr;
	u16	i, loop, left_size;
	u8	tempdata[0x80];
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	/************************************************/
	/* To add */
	RspSet.GetCode = CODE_RESETCONFIG;

	eeprom_start_addr = 0x300;
	for(i=0; i<0x80; i++)
		tempdata[i] = 0xFF;

	loop = (EEPROM_SIZE - eeprom_start_addr) / 0x80;
	left_size = (EEPROM_SIZE - eeprom_start_addr) % 0x80;

	RspSet.RetCode = 0x00;
	RspSet.Res = 0x00;
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);
	
	for(i=0; i<loop; i++) {
		if(eeprom_page_write((u16)(eeprom_start_addr + i*0x80), tempdata, 0x80) != I2C_SUCCESS) {
			goto ErrRstCfg;
		}
	}
	
	if(left_size > 0) {
		if(eeprom_page_write((u16)(eeprom_start_addr + loop*0x80), tempdata, left_size) != I2C_SUCCESS) {
			goto ErrRstCfg;
		}
	}

	return;
	
ErrRstCfg:
	RspSet.RetCode = 0x01;
	RspSet.Res = RSP_ERR_EEPROM_OPERATION;
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);
}

void NMS_Frame_Process(u8 *rxBuf, u16 len)
{
	PETHER_HEAD	ethhdr = (PETHER_HEAD)rxBuf;
	POBNET_HEAD	obnethdr = (POBNET_HEAD)(rxBuf + ETHER_HEAD_SIZE);
	OBNET_GET_REQ stReq;
	u8 getCode;
	u8 dstMac[6];
	u8 RxHport, RxLport;
	u16 ReqID;
	HAL_PORT_LINK_STATE PortLinkStatus;

	if((obnethdr->ProtoType[0] == 0x00) && (obnethdr->ProtoType[1] == 0x02)) {	/* Protocol Type = 0x0002 */
		NmsMsgDump("RxMsg", rxBuf, len, 1);
		switch(obnethdr->MessageType) {
			case MSG_GET:
			getCode = *(rxBuf+PAYLOAD_OFFSET);
			switch(getCode) {
				case CODE_PAGING:
				Rsp_Paging(ethhdr->sma, obnethdr->RequestID);
				break;

				case CODE_TOPO:
				Rsp_Topo(ethhdr->sma, obnethdr->RequestID);
				break;

				case CODE_GET_NEIGHBOR:
				nms_rsp_get_port_neighbor(ethhdr->sma, obnethdr->RequestID, (obnet_get_port_neighbor *)(rxBuf+PAYLOAD_OFFSET));
				break;
				
				case CODE_PORT_STATUS:
				nms_rsp_get_port_status(ethhdr->sma, obnethdr->RequestID);
				break;	
				
				case CODE_GET_NAMEID:
				Rsp_GetNameID(ethhdr->sma, obnethdr->RequestID);
				break;	

				case CODE_GET_VERSION:
				Rsp_GetVersion(ethhdr->sma, obnethdr->RequestID);
				break;	

				case CODE_GLOBAL_CONFIG:
				nms_rsp_get_global_config(ethhdr->sma, obnethdr->RequestID);
				break;	

				case CODE_PORT_CONFIG:
				nms_rsp_get_port_config(ethhdr->sma, obnethdr->RequestID, (obnet_get_port_config *)(rxBuf+PAYLOAD_OFFSET));
				break;	

				case CODE_RING_CONFIG:  
				Rsp_GetRingConfig(ethhdr->sma, obnethdr->RequestID);
	            break;	

				case CODE_GET_MIRROR:
				nms_rsp_get_port_mirror(ethhdr->sma, obnethdr->RequestID);
				break;
				
				case CODE_GET_IP:
				Rsp_GetIP(ethhdr->sma, obnethdr->RequestID);
				break;	

				case CODE_GET_RATE:
				nms_rsp_get_rate_ctrl(ethhdr->sma, obnethdr->RequestID);
				break;

				case CODE_GET_QOS:
				nms_rsp_get_qos(ethhdr->sma, obnethdr->RequestID);
				break;	

				case CODE_GET_ISOLATION:
				nms_rsp_get_port_isolation(ethhdr->sma, obnethdr->RequestID);
				break;

				case CODE_GET_PORT_VLAN:
				nms_rsp_get_port_vlan(ethhdr->sma, obnethdr->RequestID);	
				break;

				case CODE_GET_ADM_VLAN:
				nms_rsp_get_adm_vlan(ethhdr->sma, obnethdr->RequestID);	
				break;

				case CODE_GET_VLAN:
				nms_rsp_get_vlan(ethhdr->sma, obnethdr->RequestID, (obnet_get_vlan *)(rxBuf+PAYLOAD_OFFSET));	
				break;

				case CODE_GET_MCAST:					
				nms_rsp_get_multicast(ethhdr->sma, obnethdr->RequestID, (obnet_get_multicast *)(rxBuf+PAYLOAD_OFFSET));
				break;
				
				case CODE_GET_UART:
				Rsp_GetUart(ethhdr->sma, obnethdr->RequestID, (u8 *)(rxBuf+PAYLOAD_OFFSET));
				break;

				case CODE_GET_PORT_SECURITY:				
				nms_rsp_get_port_security(ethhdr->sma, obnethdr->RequestID, (obnet_get_port_security *)(rxBuf+PAYLOAD_OFFSET));
	            break;

				case CODE_GET_MACLIST:					
				nms_rsp_get_mac_list(ethhdr->sma, obnethdr->RequestID, (obnet_get_mac_list *)(rxBuf+PAYLOAD_OFFSET));
	            break;

#if (OB_NMS_PROTOCOL_VERSION == 1)
				case CODE_GET_PORT_STATISTICS:
				Rsp_GetPortStatistics(ethhdr->sma, obnethdr->RequestID, (POBNET_GET_PORT_STATISTICS)(rxBuf+PAYLOAD_OFFSET));	
	            break;
#endif
				case CODE_GET_PORT_TRUNK:				
				nms_rsp_get_port_aggregation(ethhdr->sma, obnethdr->RequestID, (obnet_get_port_aggregation *)(rxBuf+PAYLOAD_OFFSET));
	            break;
				
				default:
				break;
			}
			
			break;
			/****************************************************************************************************/
			case MSG_SET:
			getCode = *(rxBuf+PAYLOAD_OFFSET);
			switch(getCode) {
				case CODE_START:
				Rsp_LoadStart(ethhdr->sma, obnethdr->RequestID);
				break;
				
				case CODE_COMPLETE:
				Rsp_LoadComplete(ethhdr->sma, obnethdr->RequestID);
				break;		

				case CODE_RESETCONFIG:
				//Rsp_ResetConfig(ethhdr->sma, obnethdr->RequestID);
				break;
				
				case CODE_SET_MAC:
				Rsp_SetMac(ethhdr->sma, obnethdr->RequestID, (POBNET_SET_MAC)(rxBuf+PAYLOAD_OFFSET));
				break;
		
				case CODE_SET_NAMEID:
				Rsp_SetNameID(ethhdr->sma, obnethdr->RequestID, (POBNET_SET_NAMEID)(rxBuf+PAYLOAD_OFFSET));
				break;	

				case CODE_SET_VERSION:
				Rsp_SetVersion(ethhdr->sma, obnethdr->RequestID, (POBNET_SET_VERSION)(rxBuf+PAYLOAD_OFFSET));
				break;	

				case CODE_SET_GLOBAL_CFG:
				nms_rsp_set_global_config(ethhdr->sma, obnethdr->RequestID, (obnet_set_global_config *)(rxBuf+PAYLOAD_OFFSET));
				break;	

				case CODE_SET_PORT_CFG:
				nms_rsp_set_port_config(ethhdr->sma, obnethdr->RequestID, (obnet_set_port_config *)(rxBuf+PAYLOAD_OFFSET));
				break;	
				
				case CODE_SET_IP:
				Rsp_SetIP(ethhdr->sma, obnethdr->RequestID, (POBNET_REQ_SET_IP)(rxBuf+PAYLOAD_OFFSET));
				break;	

				case CODE_SET_RATE:					
				nms_rsp_set_rate_ctrl(ethhdr->sma, obnethdr->RequestID, (obnet_set_rate_ctrl *)(rxBuf+PAYLOAD_OFFSET));
				break;

				case CODE_SET_QOS:					
				nms_rsp_set_qos(ethhdr->sma, obnethdr->RequestID, (obnet_set_qos *)(rxBuf+PAYLOAD_OFFSET));
				break;

				case CODE_SET_ISOLATION:
				nms_rsp_set_port_isolation(ethhdr->sma, obnethdr->RequestID, (obnet_set_port_isolation *)(rxBuf+PAYLOAD_OFFSET));
				break;

				case CODE_SET_PORT_VLAN:
				nms_rsp_set_port_vlan(ethhdr->sma, obnethdr->RequestID, (obnet_set_port_vlan *)(rxBuf+PAYLOAD_OFFSET));
				break;

				case CODE_SET_ADM_VLAN:
				nms_rsp_set_adm_vlan(ethhdr->sma, obnethdr->RequestID, (obnet_set_adm_vlan *)(rxBuf+PAYLOAD_OFFSET));
				break;

				case CODE_SET_VLAN:
				nms_rsp_set_vlan(ethhdr->sma, obnethdr->RequestID, (obnet_set_vlan *)(rxBuf+PAYLOAD_OFFSET));
				break;

				case CODE_SET_MCAST:					
				nms_rsp_set_multicast(ethhdr->sma, obnethdr->RequestID, (obnet_set_multicast *)(rxBuf+PAYLOAD_OFFSET));
				break;
				
				case CODE_SET_MIRROR:
				nms_rsp_set_port_mirror(ethhdr->sma, obnethdr->RequestID, (obnet_set_port_mirror *)(rxBuf+PAYLOAD_OFFSET));
				break;	

				case CODE_SET_RING_CFG:
				Rsp_SetRingConfig(ethhdr->sma, obnethdr->RequestID, (u8 *)(rxBuf+PAYLOAD_OFFSET));
				break;	
				
				case CODE_REBOOT:
				Rsp_Reboot(ethhdr->sma, obnethdr->RequestID);
				break;

				case CODE_FIRMWARE_START:
				RspNMS_FirmwareUpgradeStart(ethhdr->sma, obnethdr->RequestID);
				break;	

				case CODE_FIRMWARE:
				RspNMS_FirmwareUpgradeDoing(ethhdr->sma, obnethdr->RequestID, (u8 *)(rxBuf+PAYLOAD_OFFSET+2));
				break;	
				case CODE_FIRMWARE_COMPLETE:
				RspNMS_FirmwareUpgradeComplete(ethhdr->sma, obnethdr->RequestID);
				break;	

				case CODE_SET_UART:
				Rsp_SetUart(ethhdr->sma, obnethdr->RequestID, (u8 *)(rxBuf+PAYLOAD_OFFSET));
				break;	

	            case CODE_SET_PORT_SECURITY:				
	            nms_rsp_set_port_security(ethhdr->sma, obnethdr->RequestID, (obnet_set_port_security *)(rxBuf+PAYLOAD_OFFSET));
	            break;

#if (OB_NMS_PROTOCOL_VERSION == 1)
	            case CODE_SET_PORT_STATISTICS:
	            Rsp_SetPortStatistics(ethhdr->sma, obnethdr->RequestID, (POBNET_SET_PORT_STATISTICS)(rxBuf+PAYLOAD_OFFSET));
	            break;
#elif (OB_NMS_PROTOCOL_VERSION > 1)
	            case CODE_PORT_STATISTICS:
	            nms_rsp_port_statistics(ethhdr->sma, obnethdr->RequestID, (obnet_port_statistic *)(rxBuf+PAYLOAD_OFFSET));
	            break;
#endif
	            case CODE_SET_PORT_TRUNK:					
	            nms_rsp_set_port_aggregation(ethhdr->sma, obnethdr->RequestID, (obnet_set_port_aggregation *)(rxBuf+PAYLOAD_OFFSET));
	            break;
				
				default:
				break;
			}		
			break;
			
			/****************************************************************************************************/
			case MSG_RESPONSE:
			break;

			case MSG_TRAP_RESPONSE:
			ReqID = obnethdr->RequestID[0];
			ReqID = (ReqID << 8) | obnethdr->RequestID[1];
			hal_swif_trap_complete(ethhdr->sma, ReqID, (hal_trap_port_status *)(rxBuf+HAL_PAYLOAD_OFFSET));
			break;
			
			default:
			break;
		}
	} else if ( (obnethdr->ProtoType[0] == (u8)((HAL_PROTOCOL_TYPE_RTOS & 0xFF00) >> 8)) && 
				(obnethdr->ProtoType[1] == (u8)(HAL_PROTOCOL_TYPE_RTOS & 0x00FF))) {	/* Protocol Type = 0x0102 */
		NmsMsgDump("Rx-Protocol: 0x0102", rxBuf, len, 0);
#if SWITCH_CHIP_88E6095
		RxHport = (ethhdr->switchHeader[1] & 0xF8) >> 3;
#elif SWITCH_CHIP_BCM5396
		RxHport = ethhdr->switchHeader[5];
#elif SWITCH_CHIP_BCM53101
		RxHport = ethhdr->switchHeader[3];
#elif SWITCH_CHIP_BCM53115
		RxHport = ethhdr->switchHeader[3];
#elif SWITCH_CHIP_BCM53286
		RxHport = ethhdr->switchHeader[3];
#else
		RxHport = 0xFF;
#endif

		RxLport = hal_swif_hport_2_lport(RxHport);

		ReqID = obnethdr->RequestID[0];
		ReqID = (ReqID << 8) | obnethdr->RequestID[1];

		PortLinkStatus = LINK_DOWN;
		hal_swif_port_get_link_state(RxLport, &PortLinkStatus);
		
		switch(obnethdr->MessageType) {
			case MSG_GET:
			getCode = *(rxBuf+PAYLOAD_OFFSET);
			switch(getCode) {
				case HAL_CODE_NEIGHBOR_SEARCH:
				if(PortLinkStatus == LINK_UP)
					hal_swif_neighbor_req_rsponse(ethhdr->sma, RxLport, ReqID, (hal_swif_msg_neighbor_req *)(rxBuf+PAYLOAD_OFFSET));
				break;
				
				default:
				break;
			}
			break;

			case MSG_RESPONSE:
			getCode = *(rxBuf+PAYLOAD_OFFSET);
			switch(getCode) {
				case HAL_CODE_NEIGHBOR_SEARCH:
				if(PortLinkStatus == LINK_UP)
					hal_swif_neighbor_info_update((hal_swif_msg_neighbor_req_rsponse *)(rxBuf+PAYLOAD_OFFSET));				
				break;
				
				default:
				break;
			}				
			break;
			
			default:
			break;
		}	
	}
}

void NMS_Msg_Receive(u8 *rxBuf, u16 rxLen)
{
	if(rxLen <= MSG_MAXSIZE) {
		NMS_TxMessage.BufLen = rxLen;
		memcpy((u8 *)&(NMS_TxMessage.Buffer[0]), rxBuf, rxLen);

		if(xNmsMsgRxQueue != NULL) {
			while(xQueueSendToBack(xNmsMsgRxQueue, &NMS_TxMessage, portMAX_DELAY) != pdTRUE) {}
		}
	}
}

void NMS_Task(void *arg)
{
	xNmsMsgRxQueue = xQueueCreate(5, sizeof(OBNET_NMS_MSG));
	if(xNmsMsgRxQueue == NULL)
		printf("NMS message queue create error\r\n");
	
	for(;;) {
		if(xNmsMsgRxQueue != NULL) {
			while(xQueueReceive(xNmsMsgRxQueue, &NMS_RxMessage, 100) != pdTRUE) {}
			NMS_Frame_Process((u8 *)&(NMS_RxMessage.Buffer[0]), NMS_RxMessage.BufLen);
		} else {
			vTaskDelay(100);
		}
	}
}

#endif

