
/*******************************************************************
 * Filename     : hal_swif_message.c
 * Description  : Hardware Abstraction Layer for L2 Switch Port API
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *******************************************************************/
#include "mconfig.h"
#include "nms_if.h"

/* Standard includes */
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "os_mutex.h"

/* BSP includes */
#include "stm32f2x7_smi.h"
#include "soft_i2c.h"
#include "misc_drv.h"
#if ROBO_SWITCH
#include "robo_drv.h"
#elif MARVELL_SWITCH
#include "msApi.h"
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>
#endif

/* Configuration includes */
#include "conf_comm.h"
#include "conf_map.h"

/* HAL for L2 includes */
#include "hal_swif_error.h"
#include "hal_swif_types.h"
#include "hal_swif_comm.h"
#include "hal_swif_port.h"
#include "hal_swif_txrx.h"
#include "hal_swif_message.h"

/* Other includes */
#include "cli_util.h"

#if MARVELL_SWITCH
extern GT_QD_DEV *dev;
#endif

static uint8 OBMsgTxBuf[HAL_MAX_MSG_SIZE];
OS_MUTEX_T OBMsgTxMutex;
OS_MUTEX_T NeighborInfoMutex;
hal_trap_info_t gTrapInfo;

extern hal_port_config_info_t gPortConfigInfo[];
extern unsigned char DevMac[];
extern unsigned char MultiAddress[];
extern unsigned char NeigSearchMultiAddr[];
extern unsigned char MacAllOne[];
extern void EthSend(uint8 *, uint16);

/**************************************************************************
  * @brief  Prepare ethernet header use normal mode
  * @param  DMA
  * @retval 
  *************************************************************************/
int hal_swif_normal_mode_ether_header(uint8 *DMA)
{
	hal_ether_header_t *pEtherHead = (hal_ether_header_t *)&OBMsgTxBuf[0];
	
	memcpy(pEtherHead->DA, DMA, 6);
	memcpy(pEtherHead->SA, DevMac, 6);

#if SWITCH_CHIP_88E6095
	pEtherHead->SwitchTag[0] = 0xC0;
	pEtherHead->SwitchTag[1] = 0x00;
	pEtherHead->SwitchTag[2] = 0x00;
	pEtherHead->SwitchTag[3] = 0x01;
#elif SWITCH_CHIP_BCM53101
    pEtherHead->SwitchTag[0] = 0x00;
	pEtherHead->SwitchTag[1] = 0x00;
	pEtherHead->SwitchTag[2] = 0x00;
	pEtherHead->SwitchTag[3] = 0x00;
#elif SWITCH_CHIP_BCM53286
	pEtherHead->SwitchTag[0] = 0xF0;
	pEtherHead->SwitchTag[1] = 0x00;
	pEtherHead->SwitchTag[2] = 0x00;
	pEtherHead->SwitchTag[3] = 0x00;
	pEtherHead->SwitchTag[4] = 0x00;
	pEtherHead->SwitchTag[5] = 0x00;
	pEtherHead->SwitchTag[6] = 0x00;
	pEtherHead->SwitchTag[7] = 0x00;	
#elif SWITCH_CHIP_BCM5396

#elif SWITCH_CHIP_BCM53115
	pEtherHead->SwitchTag[0] = 0x00;
	pEtherHead->SwitchTag[1] = 0x00;
	pEtherHead->SwitchTag[2] = 0x00;
	pEtherHead->SwitchTag[3] = 0x00;
#else
#error Add SwitchTag unkown chip type
#endif
	pEtherHead->Type[0] = 0x88;
	pEtherHead->Type[1] = 0xB7;
        
	return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  Prepare ethernet header use direct mode
  * @param  DMA, Lport
  * @retval 
  *************************************************************************/
int hal_swif_direct_mode_ether_header(uint8 *DMA, uint8 Lport)
{
	uint8 Hport;
	hal_ether_header_t *pEtherHead = (hal_ether_header_t *)&OBMsgTxBuf[0];

	if(Lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	Hport = hal_swif_lport_2_hport(Lport);
	
	memcpy(pEtherHead->DA, DMA, 6);
	memcpy(pEtherHead->SA, DevMac, 6);

#if SWITCH_CHIP_88E6095
	pEtherHead->SwitchTag[0] = 0x40;
	pEtherHead->SwitchTag[1] = Hport << 3;
	pEtherHead->SwitchTag[2] = 0xe0;
	pEtherHead->SwitchTag[3] = 0x00;	
#elif SWITCH_CHIP_BCM53101
	pEtherHead->SwitchTag[0] = 0x24;
	pEtherHead->SwitchTag[1] = 0x00;
	pEtherHead->SwitchTag[2] = 0x00;
	pEtherHead->SwitchTag[3] = 1 << Hport;
#elif SWITCH_CHIP_BCM53115
	pEtherHead->SwitchTag[0] = 0x24;    /*Opcode = 001 */
	pEtherHead->SwitchTag[1] = 0x00;
	pEtherHead->SwitchTag[2] = 0x00;
	pEtherHead->SwitchTag[3] = 1 << Hport;
#elif SWITCH_CHIP_BCM53286	
	pEtherHead->SwitchTag[0] = 0x10;	/* Opcode = 0001, TC = 0000 */
	pEtherHead->SwitchTag[1] = 0x3F;	/* DP = 00, Filter_ByPass[7:2] = 111111 */
	pEtherHead->SwitchTag[2] = 0xC0;	/* Filter_ByPass[1:0] = 11, R = 0, DST_ID[12:11] = 00, DST_ID[10:8] = 000 */
	pEtherHead->SwitchTag[3] = Hport;	/* DST_ID[7:5] = 000, DST_ID[4:0] = port_id */
	pEtherHead->SwitchTag[4] = 0x00;	/* R = 00000000 */
	pEtherHead->SwitchTag[5] = 0x00;	
	pEtherHead->SwitchTag[6] = 0x10;	/* Set vid to default vlan 1 */
	pEtherHead->SwitchTag[7] = 0x00;	
#elif SWITCH_CHIP_BCM5396
	pEtherHead->SwitchTag[0] = 0x88;
	pEtherHead->SwitchTag[1] = 0x74;
	pEtherHead->SwitchTag[2] = 0x40;
	pEtherHead->SwitchTag[3] = 0x00;
	pEtherHead->SwitchTag[4] = 0x00;
	pEtherHead->SwitchTag[5] = Hport;	
#endif
	pEtherHead->Type[0] = 0x88;
	pEtherHead->Type[1] = 0xB7;
        
	return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  Prepare obnet header
  * @param  MessageType, MessageLength, RequestID, OrgCode, SwitchMac
  * @retval 
  *************************************************************************/
void hal_swif_prepare_obnet_header(uint16 ProtocolType, uint8 MessageType, uint16 MessageLength, uint16 RequestID, uint8 *OrgCode, uint8 *SwitchMac)
{
	hal_obnet_header_t *pOBNetHead = (hal_obnet_header_t *)&OBMsgTxBuf[HAL_ETHER_HEAD_SIZE];
	
	memcpy(pOBNetHead->OrgCode, OrgCode, 3);
	pOBNetHead->ProtoType[0] = (uint8)((ProtocolType >> 8) & 0xff);;
	pOBNetHead->ProtoType[1] = (uint8)(ProtocolType & 0xff);;
	pOBNetHead->Version	= 0x00;
	pOBNetHead->MessageType	= MessageType;
	pOBNetHead->MessageLength[0] = (uint8)((MessageLength >> 8) & 0xff);
	pOBNetHead->MessageLength[1] = (uint8)(MessageLength & 0xff);	
	pOBNetHead->RequestID[0] = (uint8)((RequestID >> 8) & 0xff);
	pOBNetHead->RequestID[1] = (uint8)(RequestID & 0xff);
	memcpy(pOBNetHead->SwitchMac, SwitchMac, 6);
}

/**************************************************************************
  * @brief  Send Neighbor Search Request Message
  * @param  Lport, ReqID
  * @retval 
  *************************************************************************/
int hal_swif_neighbor_req_send(uint8 Lport, uint16 ReqID)
{
#if SWITCH_CHIP_BCM5396
	uint32 i, crc32;
	uint8 tag_bak[HAL_SWITCH_TAG_LEN];
	uint8 *crc_buf;
#endif
	uint8 Hport;
	hal_swif_msg_neighbor_req NeighborReq;	
	uint16 MsgLength;
	HAL_PORT_STP_STATE StpStatus;
	
	if(Lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	if((Hport = hal_swif_lport_2_hport(Lport)) == 0xFF) 
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	MsgLength = HAL_PAYLOAD_OFFSET + sizeof(hal_swif_msg_neighbor_req);
	if (MsgLength < HAL_MSG_MINSIZE)//40<60
		MsgLength = HAL_MSG_MINSIZE;

#if SWITCH_CHIP_BCM5396
	if(MsgLength > HAL_MAX_MSG_SIZE - 4)
		return HAL_SWIF_FAILURE;
#else
	if(MsgLength > HAL_MAX_MSG_SIZE)//60>128
		return HAL_SWIF_FAILURE;
#endif

	os_mutex_lock(&OBMsgTxMutex, OS_MUTEX_WAIT_FOREVER);

	memset(OBMsgTxBuf, 0, HAL_MAX_MSG_SIZE);
	
	/* fill the frame header */
	hal_swif_direct_mode_ether_header(NeigSearchMultiAddr, Lport);    //DA SA   88 b7
	hal_swif_prepare_obnet_header(HAL_PROTOCOL_TYPE_RTOS, HAL_MSG_GET, MsgLength, ReqID, DevMac, MacAllOne);

	/* fill the request code */
	hal_swif_port_get_stp_state(Lport, &StpStatus);
	NeighborReq.GetCode = HAL_CODE_NEIGHBOR_SEARCH;      /*******/
	NeighborReq.RetCode = 0x00;
	NeighborReq.PortSearchEnable = gPortConfigInfo[Lport-1].NeigborSearch;
	NeighborReq.PortNo = Lport;
	NeighborReq.PortStatus = (StpStatus == FORWARDING)? 0x3 : (StpStatus == BLOCKING)? 0x2 : 0x1;
	
	/* prepare the data to send */
	memcpy(&OBMsgTxBuf[HAL_PAYLOAD_OFFSET], (u8 *)&NeighborReq, sizeof(hal_swif_msg_neighbor_req));
	if(MsgLength == HAL_MSG_MINSIZE)
		MsgLength += HAL_SWITCH_TAG_LEN;
	
#if SWITCH_CHIP_BCM5396
	/* Backup Switch TAG */
	for(i=0; i<HAL_SWITCH_TAG_LEN; i++) {
		tag_bak[i] = OBMsgTxBuf[12+i];
	}

	/* Untag Switch TAG */
	for(i=0; i<MsgLength-12-HAL_SWITCH_TAG_LEN; i++) {
		OBMsgTxBuf[12+i] = OBMsgTxBuf[12+HAL_SWITCH_TAG_LEN+i];
	}
	MsgLength -= HAL_SWITCH_TAG_LEN;

	/* Calculate CRC32 value */
    crc32 = ~ bcm5396_crc32(~0, &OBMsgTxBuf[0], MsgLength);

	/* Tag Switch TAG */
	for(i=0; i<MsgLength-12; i++) {
		OBMsgTxBuf[MsgLength-1+HAL_SWITCH_TAG_LEN-i] = OBMsgTxBuf[MsgLength-1-i];
	}
	for(i=0; i<HAL_SWITCH_TAG_LEN; i++) {
		OBMsgTxBuf[12+i] = tag_bak[i];
	}
	MsgLength += HAL_SWITCH_TAG_LEN;

	/* Added CRC32 at the buffer end */
    crc_buf = &OBMsgTxBuf[MsgLength];
    *crc_buf++ = (uint8)(crc32 >> 24);
    *crc_buf++ = (uint8)(crc32 >> 16);
    *crc_buf++ = (uint8)(crc32 >> 8);
    *crc_buf++ = (uint8)(crc32);
	MsgLength += 4;
	
	EthSend(OBMsgTxBuf, MsgLength);
#else
	EthSend(OBMsgTxBuf, MsgLength);
#endif

	os_mutex_unlock(&OBMsgTxMutex);

	return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  Response for Neighbor request from other device
  * @param  SMA, RxLport, ReqID, *NeighborReq
  * @retval 
  *************************************************************************/
int hal_swif_neighbor_req_rsponse(uint8 *SMA, uint8 RxLport, uint16 ReqID, hal_swif_msg_neighbor_req *NeighborReq)
{
#if SWITCH_CHIP_BCM5396
	uint32 i, crc32;
	uint8 tag_bak[HAL_SWITCH_TAG_LEN];
	uint8 *crc_buf;
#endif
	uint8 Hport;
	hal_swif_msg_neighbor_req_rsponse NeighborReqResponse;	
	uint16 MsgLength;
	extern dev_base_info_t DeviceBaseInfo;

	if(RxLport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	if((Hport = hal_swif_lport_2_hport(RxLport)) == 0xFF) 
		return HAL_SWIF_ERR_INVALID_LPORT;

	MsgLength = HAL_PAYLOAD_OFFSET + sizeof(hal_swif_msg_neighbor_req_rsponse);
	if (MsgLength < HAL_MSG_MINSIZE)
		MsgLength = HAL_MSG_MINSIZE;

#if SWITCH_CHIP_BCM5396
	if(MsgLength > HAL_MAX_MSG_SIZE - 4)
		return HAL_SWIF_FAILURE;
#else
	if(MsgLength > HAL_MAX_MSG_SIZE)
		return HAL_SWIF_FAILURE;
#endif

	os_mutex_lock(&OBMsgTxMutex, OS_MUTEX_WAIT_FOREVER);

	memset(OBMsgTxBuf, 0, HAL_MAX_MSG_SIZE);

	/* fill the frame header */
	hal_swif_direct_mode_ether_header(NeigSearchMultiAddr, RxLport);
	hal_swif_prepare_obnet_header(HAL_PROTOCOL_TYPE_RTOS, HAL_MSG_RESPONSE, MsgLength, ReqID, DevMac, MacAllOne);

	/* fill the request code */
	NeighborReqResponse.GetCode = 0x10;
	NeighborReqResponse.RetCode = 0x00;
	NeighborReqResponse.PortSearchEnable = NeighborReq->PortSearchEnable;
	NeighborReqResponse.PortNo = NeighborReq->PortNo;
	NeighborReqResponse.PortStatus = NeighborReq->PortStatus;
	memcpy(NeighborReqResponse.NeighborMac, DevMac, 6);
	NeighborReqResponse.NeighborPort = RxLport;
	memcpy(NeighborReqResponse.NeighborIP, DeviceBaseInfo.IpAddress, 4);
	memcpy(NeighborReqResponse.NeighborSwitchType, DeviceBaseInfo.BoardType, 8);

	/* prepare the data to send */
	memcpy(&OBMsgTxBuf[HAL_PAYLOAD_OFFSET], (u8 *)&NeighborReqResponse, sizeof(hal_swif_msg_neighbor_req_rsponse));
	if(MsgLength == HAL_MSG_MINSIZE)
		MsgLength += HAL_SWITCH_TAG_LEN;

#if SWITCH_CHIP_BCM5396
	/* Backup Switch TAG */
	for(i=0; i<HAL_SWITCH_TAG_LEN; i++) {
		tag_bak[i] = OBMsgTxBuf[12+i];
	}

	/* Untag Switch TAG */
	for(i=0; i<MsgLength-12-HAL_SWITCH_TAG_LEN; i++) {
		OBMsgTxBuf[12+i] = OBMsgTxBuf[12+HAL_SWITCH_TAG_LEN+i];
	}
	MsgLength -= HAL_SWITCH_TAG_LEN;

	/* Calculate CRC32 value */
    crc32 = ~ bcm5396_crc32(~0, &OBMsgTxBuf[0], MsgLength);

	/* Tag Switch TAG */
	for(i=0; i<MsgLength-12; i++) {
		OBMsgTxBuf[MsgLength-1+HAL_SWITCH_TAG_LEN-i] = OBMsgTxBuf[MsgLength-1-i];
	}
	for(i=0; i<HAL_SWITCH_TAG_LEN; i++) {
		OBMsgTxBuf[12+i] = tag_bak[i];
	}
	MsgLength += HAL_SWITCH_TAG_LEN;

	/* Added CRC32 at the buffer end */
    crc_buf = &OBMsgTxBuf[MsgLength];
    *crc_buf++ = (uint8)(crc32 >> 24);
    *crc_buf++ = (uint8)(crc32 >> 16);
    *crc_buf++ = (uint8)(crc32 >> 8);
    *crc_buf++ = (uint8)(crc32);
	MsgLength += 4;
	
	EthSend(OBMsgTxBuf, MsgLength);
#else
	EthSend(OBMsgTxBuf, MsgLength);
#endif

	os_mutex_unlock(&OBMsgTxMutex);

	return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  Update the Neighbor information
  * @param  pNeighBorReqRsp
  * @retval 
  *************************************************************************/
int hal_swif_neighbor_info_update(hal_swif_msg_neighbor_req_rsponse *pNeighBorReqRsp)
{
	uint8 LocalLport;
	HAL_PORT_STP_STATE	stp_state;
	extern hal_neighbor_record_t gNeighborInformation[];
	extern uint32 gNeighborReqEnBitMap;
	
	if((pNeighBorReqRsp->PortNo == 0) || (pNeighBorReqRsp->PortNo > MAX_PORT_NUM))
		return HAL_SWIF_FAILURE;

	if(pNeighBorReqRsp->PortSearchEnable == 0)
		return HAL_SWIF_FAILURE;	


	os_mutex_lock(&NeighborInfoMutex, OS_MUTEX_WAIT_FOREVER);
	
	LocalLport = pNeighBorReqRsp->PortNo;
	gNeighborInformation[LocalLport - 1].PortSearchEnable = pNeighBorReqRsp->PortSearchEnable;
	gNeighborInformation[LocalLport - 1].PortNo = pNeighBorReqRsp->PortNo;
	gNeighborInformation[LocalLport - 1].PortStatus = pNeighBorReqRsp->PortStatus;
	memcpy(gNeighborInformation[LocalLport - 1].NeighborMac, pNeighBorReqRsp->NeighborMac, 6);
	gNeighborInformation[LocalLport - 1].NeighborPort = pNeighBorReqRsp->NeighborPort;
	memcpy(gNeighborInformation[LocalLport - 1].NeighborIP, pNeighBorReqRsp->NeighborIP, 4);
	memcpy(gNeighborInformation[LocalLport - 1].NeighborSwitchType, pNeighBorReqRsp->NeighborSwitchType, 8);

	gNeighborReqEnBitMap &= ~(uint32)(1<<(LocalLport-1));
	
	os_mutex_unlock(&NeighborInfoMutex);

	return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  Clear the Neighbor information
  * @param  
  * @retval 
  *************************************************************************/
int hal_swif_neighbor_info_clear(uint8 Lport)
{
	HAL_PORT_STP_STATE	stp_state;
	extern hal_neighbor_record_t gNeighborInformation[];
	
	os_mutex_lock(&NeighborInfoMutex, OS_MUTEX_WAIT_FOREVER);
	
	gNeighborInformation[Lport-1].PortSearchEnable = gPortConfigInfo[Lport-1].NeigborSearch;
	gNeighborInformation[Lport-1].PortNo = Lport;
	gNeighborInformation[Lport-1].PortStatus = 0x01;
	memset(gNeighborInformation[Lport-1].NeighborMac, 0, 6);
	gNeighborInformation[Lport-1].NeighborPort = 0;
	memset(gNeighborInformation[Lport-1].NeighborIP, 0, 4);
	memset(gNeighborInformation[Lport-1].NeighborSwitchType, 0, 8);
	
	os_mutex_unlock(&NeighborInfoMutex);
	
	return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  Trap message send
  * @param  ServerMac, TrapMsgBuf, TrapMsgLen, RequestID
  * @retval 
  *************************************************************************/
int hal_swif_trap_send(uint8 *ServerMac, uint8 *TrapMsgBuf, uint16 TrapMsgLen, uint16 RequestID)
{
#if SWITCH_CHIP_BCM5396
	uint32 i, crc32;
	uint8 tag_bak[HAL_SWITCH_TAG_LEN];
	uint8 *crc_buf;
#endif
	uint16 MsgLength;

	if(check_mac(ServerMac) != CHK_OK)
		return HAL_SWIF_FAILURE;
		
	MsgLength = HAL_PAYLOAD_OFFSET + TrapMsgLen; //35 + 14
	if (MsgLength < HAL_MSG_MINSIZE) //49 < 60
		MsgLength = HAL_MSG_MINSIZE;

#if SWITCH_CHIP_BCM5396
	if(MsgLength > HAL_MAX_MSG_SIZE - 4)
		return HAL_SWIF_FAILURE;
#else
	if(MsgLength > HAL_MAX_MSG_SIZE)
		return HAL_SWIF_FAILURE;
#endif

	os_mutex_lock(&OBMsgTxMutex, OS_MUTEX_WAIT_FOREVER);

	memset(OBMsgTxBuf, 0, HAL_MAX_MSG_SIZE);

	/* fill the frame header */
	hal_swif_normal_mode_ether_header(ServerMac);
	hal_swif_prepare_obnet_header(HAL_PROTOCOL_TYPE_OBNET, HAL_MSG_TRAP, MsgLength, RequestID, DevMac, DevMac);

	/* prepare the data to send */
	memcpy(&OBMsgTxBuf[HAL_PAYLOAD_OFFSET], TrapMsgBuf, TrapMsgLen);
	if(MsgLength == HAL_MSG_MINSIZE)
		MsgLength += HAL_SWITCH_TAG_LEN;
	
#if SWITCH_CHIP_BCM5396
	/* Backup Switch TAG */
	for(i=0; i<HAL_SWITCH_TAG_LEN; i++) {
		tag_bak[i] = OBMsgTxBuf[12+i];
	}

	/* Untag Switch TAG */
	for(i=0; i<MsgLength-12-HAL_SWITCH_TAG_LEN; i++) {
		OBMsgTxBuf[12+i] = OBMsgTxBuf[12+HAL_SWITCH_TAG_LEN+i];
	}
	MsgLength -= HAL_SWITCH_TAG_LEN;

	/* Calculate CRC32 value */
    crc32 = ~ bcm5396_crc32(~0, &OBMsgTxBuf[0], MsgLength);

	/* Tag Switch TAG */
	for(i=0; i<MsgLength-12; i++) {
		OBMsgTxBuf[MsgLength-1+HAL_SWITCH_TAG_LEN-i] = OBMsgTxBuf[MsgLength-1-i];
	}
	for(i=0; i<HAL_SWITCH_TAG_LEN; i++) {
		OBMsgTxBuf[12+i] = tag_bak[i];
	}
	MsgLength += HAL_SWITCH_TAG_LEN;

	/* Added CRC32 at the buffer end */
    crc_buf = &OBMsgTxBuf[MsgLength];
    *crc_buf++ = (uint8)(crc32 >> 24);
    *crc_buf++ = (uint8)(crc32 >> 16);
    *crc_buf++ = (uint8)(crc32 >> 8);
    *crc_buf++ = (uint8)(crc32);
	MsgLength += 4;

	NmsMsgDump("TxMsg", OBMsgTxBuf, MsgLength, 1);
	EthSend(OBMsgTxBuf, MsgLength);
#else
	NmsMsgDump("TxMsg", OBMsgTxBuf, MsgLength, 1);
	EthSend(OBMsgTxBuf, MsgLength);
#endif
	
	os_mutex_unlock(&OBMsgTxMutex);

	return HAL_SWIF_SUCCESS;
}

/**************************************************************************
  * @brief  Trap send complete handle for this time
  * @param  SMA, ReqID, pTrapReponse
  * @retval 
  *************************************************************************/
int hal_swif_trap_complete(uint8 *SMA, uint16 ReqID, hal_trap_port_status *pTrapReponse)
{
	if(memcmp(SMA, gTrapInfo.ServerMac, 6) != 0)
		return HAL_SWIF_FAILURE;

	if(pTrapReponse->TrapIndex >= MAX_TRAP_TYPE_NUM)
		return HAL_SWIF_FAILURE;

	if(ReqID != gTrapInfo.RequestID[pTrapReponse->TrapIndex])
		return HAL_SWIF_FAILURE;

	gTrapInfo.SendEnable[pTrapReponse->TrapIndex] = HAL_FALSE;

	return HAL_SWIF_SUCCESS;
}

