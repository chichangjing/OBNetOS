/*************************************************************
 * Filename     : obring.c
 * Description  : OBRing protocol
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#include "mconfig.h"

#if OBRING_DEV
/* Standard includes. */
#include "stdio.h"
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "os_mutex.h"

/* BSP includes */
#if MARVELL_SWITCH
#include "msApi.h"
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>
#endif

/* LwIP includes */
#include "lwip/api.h"
#include "lwip/inet.h"

/* BSP include */
#include "stm32f2xx.h"
#include "timer.h"
#include "led_drv.h"
#include "stm32f2x7_smi.h"
#if ROBO_SWITCH
#include "robo_drv.h"
#endif

/* HAL for L2 includes */
#include "hal_swif_error.h"
#include "hal_swif_types.h"
#include "hal_swif_comm.h"
#include "hal_swif_mac.h"
#include "hal_swif_message.h"

#include "cli_util.h"
#include "obring.h"

#include "conf_comm.h"
#include "conf_ring.h"

#if MARVELL_SWITCH
extern GT_QD_DEV *dev;
#endif

tRingInfo RingInfo;
static OS_MUTEX_T	RingMutex; 
tRingTimers	RingTimer[MAX_RING_NUM];
static xQueueHandle RingMsgQueue = NULL;
static xQueueHandle ObrMsgRxQueue = NULL;
static tRingMessage RingTxMsg;
static tRingMessage RingRxMsg;
static unsigned char RingTxBuf[MAX_RING_MSG_SIZE];
static unsigned char RingTxBufTest[MAX_RING_MSG_SIZE];
static unsigned char RingMsgRxBuffer[MAX_RING_DATA_SIZE];
static unsigned int RingHportVec = 0;
#if USE_OWN_MULTI_ADDR 
unsigned char RingMgmtMultiDA[6] = {0x0D, 0xA4, 0x2A, 0x00, 0x00, 0x05};
#else
unsigned char RingMgmtMultiDA[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0B};
#endif

int	RingGlobalEnable = 0;

void obring_frame_dump(u8 *buf, int len)
{
	unsigned int i, nbytes, linebytes;
	u8 *cp=buf;
	
	nbytes = len;
	do {
		unsigned char	linebuf[16];
		unsigned char	*ucp = linebuf;
		cli_debug(DBG_OBRING, "     ");
		linebytes = (nbytes > 16)?16:nbytes;
		for (i=0; i<linebytes; i+= 1) {
			cli_debug(DBG_OBRING, "%02X ", (*ucp++ = *cp));
			cp += 1;
		}
		cli_debug(DBG_OBRING, "\r\n");
		nbytes -= linebytes;

	} while (nbytes > 0);

	cli_debug(DBG_OBRING, "\r\n");
}

/**************************************************************************
  * @brief  Flush FDB table
  * @param  none
  * @retval 0: success, 1: failed
  *************************************************************************/
int obring_mac_flush(void)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	if((status = gfdbFlush(dev, GT_FLUSH_ALL_UNLOCKED)) != GT_OK) {
		return 1;
	}

	return 0;
#elif SWITCH_CHIP_BCM53101
	int port;
	unsigned char reg_val;
	int i;

	for(port=0; port<MAX_PORT_NUM; port++) {
		/* Set fast-aging port control register */
		reg_val = port;
		robo_write(PAGE_CONTROL, REG_FAST_AGE_PORT, &reg_val, 1);	

		/* Start fast aging process */
		robo_read(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);	

		reg_val |= (MASK_EN_FAST_AGE_STATIC | MASK_EN_FAST_AGE_DYNAMIC | MASK_FAST_AGE_STR_DONE); 
		robo_write(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);

		/* Wait for complete */
		for(i=0; i<100; i++) {
			robo_read(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);	
			if((reg_val & 0x80) == 0)
				break;
		}

		/* Restore register value to 0x2, otherwise aging will fail	*/
		reg_val = MASK_EN_FAST_AGE_DYNAMIC;
		robo_write(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);
	}
	
	return 0;
    
#elif SWITCH_CHIP_BCM53286
    return 0;
#elif SWITCH_CHIP_BCM5396
    return 0;
#else 
	#error "switch chip type unknow"
#endif
	return 1;
}

unsigned char obring_get_ring_idx_by_port(unsigned char Lport)
{
	tRingInfo *pRingInfo = &RingInfo;
	unsigned char RingIndex;

	if(pRingInfo->GlobalConfig.ucGlobalEnable != 0x01)
		return 0xFF;
	if(pRingInfo->GlobalConfig.ucRecordNum > MAX_RING_NUM)
		return 0xFF;

	for(RingIndex=0; RingIndex<pRingInfo->GlobalConfig.ucRecordNum; RingIndex++) {
		if((pRingInfo->RingConfig[RingIndex].ucPrimaryPort == Lport) || (pRingInfo->RingConfig[RingIndex].ucSecondaryPort == Lport)) {
			break;
		}
	}
	if(RingIndex == pRingInfo->GlobalConfig.ucRecordNum)
		return 0xFF;

	return RingIndex;
}

unsigned char obring_get_port_idx_by_port(tRingConfigRec *pRingConfig, unsigned char Lport) 
{
	if(Lport == pRingConfig->ucPrimaryPort)
		return INDEX_PRIMARY;
	else
		return INDEX_SECONDARY;
}
/**************************************************************************
  * @brief  Add mac entry
  * @param  *pRingConfig, *Mac
  * @retval 0: success, 1: failed
  *************************************************************************/
int obring_mac_add(tRingConfigRec *pRingConfig, unsigned char *Mac)
{
#if 0
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;
	unsigned int PortMembers;
	unsigned char primary_hport, secondary_hport;
	unsigned short RingId;
	
	memset(&macEntry,0,sizeof(GT_ATU_ENTRY));

	RingId = pRingConfig->usRingId[0];
	RingId = (RingId << 8) | (unsigned short)(pRingConfig->usRingId[1]);
	primary_hport = hal_swif_lport_2_hport(pRingConfig->ucPrimaryPort);
	secondary_hport = hal_swif_lport_2_hport(pRingConfig->ucSecondaryPort);

	if(pRingConfig->ucNodeType == NODE_TYPE_MASTER) {
		PortMembers = (1 << CPU_PORT);
	} else {
		PortMembers = (1 << primary_hport);
		PortMembers |= (1 << secondary_hport);
		PortMembers |= (1 << CPU_PORT);
	}
	
	macEntry.macAddr.arEther[0] = Mac[0];
	macEntry.macAddr.arEther[1] = Mac[1];
	macEntry.macAddr.arEther[2] = Mac[2];
	macEntry.macAddr.arEther[3] = Mac[3];
	macEntry.macAddr.arEther[4] = Mac[4];
	macEntry.macAddr.arEther[5] = (unsigned char)(RingId & 0xFF);
	macEntry.DBNum = 0;
	macEntry.portVec = PortMembers;
	macEntry.prio = 7;

	if(pRingConfig->ucNodeType == NODE_TYPE_MASTER) 
		macEntry.entryState.ucEntryState = GT_UC_TO_CPU_STATIC;
	else 
		macEntry.entryState.ucEntryState = GT_UC_STATIC;

	if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK) {
		return 1;
	}

	return 0;
#endif
	return 0;
}

static char *obring_packet_parse(ePacketType PacketType)
{
	switch (PacketType) {
		case PACKET_AUTHENTICATION:			return "[Authentication]"; break;
		case PACKET_BALLOT:					return "[Ballot]"; break;
		case PACKET_HELLO: 					return "[Hello]"; break;
		case PACKET_COMPLETE: 				return "[Complete]"; break;
		case PACKET_COMMON:					return "[Common]"; break;		
		case PACKET_LINKDOWN:				return "[LinkDown]"; break;		
		default: 							return "[Unkown]"; break;
	}
}

/**************************************************************************
  * @brief  Prepare OBRing frame
  * @param  none
  * @retval none
  *************************************************************************/
void obring_prepare_tx_frame(tRingConfigRec *pRingConfig, unsigned char TxLport, ePacketType PacketType)
{
	tRingFrame *txFrame = (tRingFrame *)&RingTxBuf[0];
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;
	unsigned char hport;
	unsigned short HelloTime;
	unsigned short FailTime;
	unsigned int SysTickCount;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern unsigned char DevMac[];

	pRingState = &(pRingInfo->DevState[pRingConfig->ucRingIndex]);

	/* 802.3 LLC header*/
	memcpy(txFrame->dst_mac, RingMgmtMultiDA, 6);	/* Dst MAC */
	memcpy(txFrame->src_mac, DevMac, 6);			/* Src MAC */
	
#if SWITCH_CHIP_88E6095
	hport = hal_swif_lport_2_hport(TxLport);
	txFrame->switch_tag[0] = 0x40;
	txFrame->switch_tag[1] = hport << 3;
	txFrame->switch_tag[2] = 0xe0;
	txFrame->switch_tag[3] = 0x00;					/* DSA Tag for 88E6095 */
#elif SWITCH_CHIP_BCM53101
    hport = hal_swif_lport_2_hport(TxLport);   
	txFrame->switch_tag[0] = OP_BRCM_SND;
	txFrame->switch_tag[1] = 0x00;
	txFrame->switch_tag[2] = 0x00;
	txFrame->switch_tag[3] = 1 << hport;			/* BRCM Tag for BCM53101 */	
#elif SWITCH_CHIP_BCM5396

#elif SWITCH_CHIP_BCM53286
	txFrame->switch_tag[0] = 0x00;
	txFrame->switch_tag[1] = 0x00;
	txFrame->switch_tag[2] = 0x00;
	txFrame->switch_tag[3] = 0x00;

#else
	#error "switch chip type unkown"
	txFrame->switch_tag[0] = 0x00;
	txFrame->switch_tag[1] = 0x00;
	txFrame->switch_tag[2] = 0x00;
	txFrame->switch_tag[3] = 0x00;
#endif
	txFrame->ether_type[0] = 0x81;
	txFrame->ether_type[1] = 0x00;
	txFrame->vlan_tag[0] = 0xEF;
	txFrame->vlan_tag[1] = 0xFE;
	txFrame->frame_length[0] = 0x00;
	txFrame->frame_length[1] = 0x48;				/* Length */
	txFrame->dsap = 0xaa;
	txFrame->ssap = 0xaa;
	txFrame->llc = 0x03;							/* Control */
	txFrame->oui_code[0] = 0x0c;
	txFrame->oui_code[1] = 0xa4;
	txFrame->oui_code[2] = 0x2a;					/* OBTelecom Networks */
	txFrame->pid[0] = 0x00;
	txFrame->pid[1] = 0xbb;							/* OB-Ring */

	/* OB-Ring header, 28 bytes */
	txFrame->obr_res1[0]= 0x99;
	txFrame->obr_res1[1]= 0x0b;						/* Reservd, always 0x990b */
	txFrame->obr_length[0]= 0x00;
	txFrame->obr_length[1]= 0x40;					/* Always 64 bytes for OB-Ring Data */
	txFrame->obr_version = 0x01;
	txFrame->obr_type = (unsigned char)PacketType;	/* Packet Type */
	txFrame->obr_domain_id[0] = pRingConfig->usDomainId[0];
	txFrame->obr_domain_id[1] = pRingConfig->usDomainId[1];
	txFrame->obr_ring_id[0] = pRingConfig->usRingId[0];
	txFrame->obr_ring_id[1] = pRingConfig->usRingId[1];
	txFrame->obr_res2[0]= 0x00;
	txFrame->obr_res2[1]= 0x00;
	memcpy(txFrame->obr_sys_mac, DevMac, 6);		/* Bridge mac address */
	txFrame->obr_port_id = TxLport;					/* Bridge ring port */
	txFrame->obr_ring_level = 0;
	
	if((pRingState->NodeType == NODE_TYPE_MASTER) && (PacketType == PACKET_HELLO)) {
		pRingState->HelloSeq++;
		txFrame->obr_hello_seq[0]= (unsigned char)((pRingState->HelloSeq & 0xFF00) >> 8);
		txFrame->obr_hello_seq[1]= (unsigned char)(pRingState->HelloSeq & 0x00FF);
		txFrame->obr_hello_time[0]= pRingConfig->usHelloTime[0];
		txFrame->obr_hello_time[1]= pRingConfig->usHelloTime[1];
		txFrame->obr_fail_time[0]= pRingConfig->usFailTime[0];
		txFrame->obr_fail_time[1]= pRingConfig->usFailTime[1];
	} else {
		txFrame->obr_hello_seq[0]= 0x00;
		txFrame->obr_hello_seq[1]= 0x00;
		txFrame->obr_hello_time[0]= 0x00;
		txFrame->obr_hello_time[1]= 0x00;
		txFrame->obr_fail_time[0]= 0x00;
		txFrame->obr_fail_time[1]= 0x00;
	}
	txFrame->obr_res3[0]= 0x00;
	txFrame->obr_res3[1]= 0x00;
	
	/* OB-Ring data, 36 bytes */
	memset(pRingMsg, 0, MAX_RING_DATA_SIZE);	
}

/**************************************************************************
  * @brief  Prepare OBRing frame
  * @param  none
  * @retval none
  *************************************************************************/
void obring_prepare_tx_frame2(tRingConfigRec *pRingConfig, unsigned char *TxBuf, unsigned char TxLport, ePacketType PacketType)
{
	tRingFrame *txFrame = (tRingFrame *)TxBuf;
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;
	unsigned char hport;
	unsigned short HelloTime;
	unsigned short FailTime;
	unsigned int SysTickCount;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)(TxBuf + OFFSET_RING_DATA);
	extern unsigned char DevMac[];

	pRingState = &(pRingInfo->DevState[pRingConfig->ucRingIndex]);

	/* 802.3 LLC header*/
	memcpy(txFrame->dst_mac, RingMgmtMultiDA, 6);	/* Dst MAC */
	memcpy(txFrame->src_mac, DevMac, 6);			/* Src MAC */
	
#if SWITCH_CHIP_88E6095
	hport = hal_swif_lport_2_hport(TxLport);
	txFrame->switch_tag[0] = 0x40;
	txFrame->switch_tag[1] = hport << 3;
	txFrame->switch_tag[2] = 0xe0;
	txFrame->switch_tag[3] = 0x00;					/* DSA Tag for 88E6095 */
#elif SWITCH_CHIP_BCM53101
    hport = hal_swif_lport_2_hport(TxLport);   
	txFrame->switch_tag[0] = OP_BRCM_SND;
	txFrame->switch_tag[1] = 0x00;
	txFrame->switch_tag[2] = 0x00;
	txFrame->switch_tag[3] = 1 << hport;			/* BRCM Tag for BCM53101 */	
#elif SWITCH_CHIP_BCM53286
	txFrame->switch_tag[0] = 0x00;
	txFrame->switch_tag[1] = 0x00;
	txFrame->switch_tag[2] = 0x00;
	txFrame->switch_tag[3] = 0x00;

#elif SWITCH_CHIP_BCM5396
    
#else
	#error "switch chip type unkown"
	txFrame->switch_tag[0] = 0x00;
	txFrame->switch_tag[1] = 0x00;
	txFrame->switch_tag[2] = 0x00;
	txFrame->switch_tag[3] = 0x00;
#endif
	txFrame->ether_type[0] = 0x81;
	txFrame->ether_type[1] = 0x00;
	txFrame->vlan_tag[0] = 0xEF;
	txFrame->vlan_tag[1] = 0xFE;
	txFrame->frame_length[0] = 0x00;
	txFrame->frame_length[1] = 0x48;				/* Length */
	txFrame->dsap = 0xaa;
	txFrame->ssap = 0xaa;
	txFrame->llc = 0x03;							/* Control */
	txFrame->oui_code[0] = 0x0c;
	txFrame->oui_code[1] = 0xa4;
	txFrame->oui_code[2] = 0x2a;					/* OBTelecom Networks */
	txFrame->pid[0] = 0x00;
	txFrame->pid[1] = 0xbb;							/* OB-Ring */

	/* OB-Ring header, 28 bytes */
	txFrame->obr_res1[0]= 0x99;
	txFrame->obr_res1[1]= 0x0b;						/* Reservd, always 0x990b */
	txFrame->obr_length[0]= 0x00;
	txFrame->obr_length[1]= 0x40;					/* Always 64 bytes for OB-Ring Data */
	txFrame->obr_version = 0x01;
	txFrame->obr_type = (unsigned char)PacketType;	/* Packet Type */
	txFrame->obr_domain_id[0] = pRingConfig->usDomainId[0];
	txFrame->obr_domain_id[1] = pRingConfig->usDomainId[1];
	txFrame->obr_ring_id[0] = pRingConfig->usRingId[0];
	txFrame->obr_ring_id[1] = pRingConfig->usRingId[1];
	txFrame->obr_res2[0]= 0x00;
	txFrame->obr_res2[1]= 0x00;
	memcpy(txFrame->obr_sys_mac, DevMac, 6);		/* Bridge mac address */
	txFrame->obr_port_id = TxLport;					/* Bridge ring port */
	txFrame->obr_ring_level = 0;
	
	if((pRingState->NodeType == NODE_TYPE_MASTER) && (PacketType == PACKET_HELLO)) {
		pRingState->HelloSeq++;
		txFrame->obr_hello_seq[0]= (unsigned char)((pRingState->HelloSeq & 0xFF00) >> 8);
		txFrame->obr_hello_seq[1]= (unsigned char)(pRingState->HelloSeq & 0x00FF);
		txFrame->obr_hello_time[0]= pRingConfig->usHelloTime[0];
		txFrame->obr_hello_time[1]= pRingConfig->usHelloTime[1];
		txFrame->obr_fail_time[0]= pRingConfig->usFailTime[0];
		txFrame->obr_fail_time[1]= pRingConfig->usFailTime[1];
	} else {
		txFrame->obr_hello_seq[0]= 0x00;
		txFrame->obr_hello_seq[1]= 0x00;
		txFrame->obr_hello_time[0]= 0x00;
		txFrame->obr_hello_time[1]= 0x00;
		txFrame->obr_fail_time[0]= 0x00;
		txFrame->obr_fail_time[1]= 0x00;
	}
	txFrame->obr_res3[0]= 0x00;
	txFrame->obr_res3[1]= 0x00;
	
	/* OB-Ring data, 36 bytes */
	memset(pRingMsg, 0, MAX_RING_DATA_SIZE);	
}

void obring_prepare_fw_frame(tRingFrame *pFrame, unsigned char TxLport)
{
	tRingFrame *txFrame = (tRingFrame *)&RingTxBuf[0];
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	unsigned char RingIndex;
	unsigned char hport;
	extern unsigned char DevMac[];

	RingIndex = obring_get_ring_idx_by_port(TxLport);
	if(RingIndex == 0xFF)
		return;
	pRingConfig = &(pRingInfo->RingConfig[RingIndex]);
	pRingState = &(pRingInfo->DevState[RingIndex]);

	/* 802.3 LLC header*/
	memcpy(txFrame->dst_mac, pFrame->dst_mac, 6);	/* Dst MAC */
	memcpy(txFrame->src_mac, pFrame->src_mac, 6);	/* Src MAC */
#if SWITCH_CHIP_88E6095
	hport = hal_swif_lport_2_hport(TxLport);
	txFrame->switch_tag[0] = 0x40;
	txFrame->switch_tag[1] = hport << 3;
	txFrame->switch_tag[2] = 0xe0;
	txFrame->switch_tag[3] = 0x00;					/* DSA Tag for 88E6095 */
#elif SWITCH_CHIP_BCM53101
    hport = hal_swif_lport_2_hport(TxLport);   
	txFrame->switch_tag[0] = OP_BRCM_SND;
	txFrame->switch_tag[1] = 0x00;
	txFrame->switch_tag[2] = 0x00;
	txFrame->switch_tag[3] = 1 << hport;			/* BRCM Tag for BCM53101 */	
#elif SWITCH_CHIP_BCM53286
	txFrame->switch_tag[0] = 0x00;
	txFrame->switch_tag[1] = 0x00;
	txFrame->switch_tag[2] = 0x00;
	txFrame->switch_tag[3] = 0x00;

#elif SWITCH_CHIP_BCM5396

#else
	#error "switch chip type unkown"
	txFrame->switch_tag[0] = 0x00;
	txFrame->switch_tag[1] = 0x00;
	txFrame->switch_tag[2] = 0x00;
	txFrame->switch_tag[3] = 0x00;
#endif
	txFrame->ether_type[0] = 0x81;
	txFrame->ether_type[1] = 0x00;
	txFrame->vlan_tag[0] = 0xEF;
	txFrame->vlan_tag[1] = 0xFE;
	txFrame->frame_length[0] = 0x00;
	txFrame->frame_length[1] = 0x48;				/* Length */
	txFrame->dsap = 0xaa;
	txFrame->ssap = 0xaa;
	txFrame->llc = 0x03;							/* Control */
	txFrame->oui_code[0] = 0x0c;
	txFrame->oui_code[1] = 0xa4;
	txFrame->oui_code[2] = 0x2a;					/* OBTelecom Networks */
	txFrame->pid[0] = 0x00;
	txFrame->pid[1] = 0xbb;							/* OB-Ring */

	/* OB-Ring header, 28 bytes */
	txFrame->obr_res1[0]= 0x99;
	txFrame->obr_res1[1]= 0x0b;						/* Reservd, always 0x990b */
	txFrame->obr_length[0]= 0x00;
	txFrame->obr_length[1]= 0x40;					/* Always 64 bytes for OB-Ring Data */
	txFrame->obr_version = 0x01;
	txFrame->obr_type = pFrame->obr_type;			/* Packet Type */
	txFrame->obr_domain_id[0] = pFrame->obr_domain_id[0];
	txFrame->obr_domain_id[1] = pFrame->obr_domain_id[1];
	txFrame->obr_ring_id[0] = pFrame->obr_ring_id[0];
	txFrame->obr_ring_id[1] = pFrame->obr_ring_id[1];
	txFrame->obr_res2[0]= 0x00;
	txFrame->obr_res2[1]= 0x00;
	memcpy(txFrame->obr_sys_mac, DevMac, 6);
	txFrame->obr_hello_time[0]= pFrame->obr_hello_time[0];
	txFrame->obr_hello_time[1]= pFrame->obr_hello_time[1];
	txFrame->obr_fail_time[0]= pFrame->obr_fail_time[0];
	txFrame->obr_fail_time[1]= pFrame->obr_fail_time[1];
	txFrame->obr_port_id = TxLport;
	txFrame->obr_ring_level = 0;
	txFrame->obr_hello_seq[0]= pFrame->obr_hello_seq[0];
	txFrame->obr_hello_seq[1]= pFrame->obr_hello_seq[1];
	txFrame->obr_res3[0]= 0x00;
	txFrame->obr_res3[1]= 0x00;
}

/**************************************************************************
  * @brief  Send Authentication frame
  * @param  none
  * @retval none
  *************************************************************************/
void obring_auth_request(tRingConfigRec *pRingConfig, unsigned char TxLport)
{
	tRMsgAuth AuthReq;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern void EthSend(unsigned char *, unsigned short);
	
	obring_prepare_tx_frame(pRingConfig, TxLport, PACKET_AUTHENTICATION);
	/* Prepare ring message body */
	AuthReq.Type = MSG_AUTH_REQ;
	memset(&AuthReq.BallotId, 0, sizeof(tBallotId));
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &AuthReq, sizeof(tRMsgAuth));
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

void obring_auth_ack(tRingConfigRec *pRingConfig, unsigned char TxLport)
{
	tRMsgAuth AuthAck;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;
	unsigned char TxPeerLportIndex;
	extern void EthSend(unsigned char *, unsigned short);

	obring_prepare_tx_frame(pRingConfig, TxLport, PACKET_AUTHENTICATION);
	/* Prepare ring message body */
	pRingState = &pRingInfo->DevState[pRingConfig->ucRingIndex];
	AuthAck.Type = MSG_AUTH_ACK;
	TxPeerLportIndex = (TxLport == pRingConfig->ucPrimaryPort)? INDEX_SECONDARY : INDEX_PRIMARY;
	if(pRingState->PortState[TxPeerLportIndex].RunState == PORT_BALLOT_FINISH) {
		memcpy(&AuthAck.BallotId, &pRingState->PortState[TxPeerLportIndex].BallotId, sizeof(tBallotId));
	} else {
		memset(&AuthAck.BallotId, 0, sizeof(tBallotId));
	}
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &AuthAck, sizeof(tRMsgAuth));
	
	/* Send */	
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

#if BALLOT_MODE_1
/**************************************************************************
  * @brief  Send interface for ballot frame
  * @param  none
  * @retval none
  *************************************************************************/
void obring_ballot_active_send(tRingConfigRec *pRingConfig, unsigned char TxLport)
{
	tRMsgBallot MsgBallot;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA]; 
	extern unsigned char DevMac[];
	extern void EthSend(unsigned char *, unsigned short);
	
	obring_prepare_tx_frame(pRingConfig, TxLport, PACKET_BALLOT);
	/* Prepare ring message body */
	MsgBallot.Type = MSG_BALLOT_ACTIVE;
	MsgBallot.Port = TxLport;
	MsgBallot.Id.Prio = pRingConfig->ucNodePrio;
	memcpy(MsgBallot.Id.Mac, DevMac, MAC_LEN);
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &MsgBallot, sizeof(tRMsgBallot));
	
	/* Send */ 
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);	
}

void obring_ballot_passive_send(tRingConfigRec *pRingConfig, unsigned char TxLport)
{
	tRMsgBallot MsgBallot;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA]; 
	extern unsigned char DevMac[];
	extern void EthSend(unsigned char *, unsigned short);
	
	obring_prepare_tx_frame(pRingConfig, TxLport, PACKET_BALLOT);
	/* Prepare ring message body */
	MsgBallot.Type = MSG_BALLOT_PASSIVE;
	MsgBallot.Port = TxLport;
	MsgBallot.Id.Prio = pRingConfig->ucNodePrio;
	memcpy(MsgBallot.Id.Mac, DevMac, MAC_LEN);
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &MsgBallot, sizeof(tRMsgBallot));
	
	/* Send */ 
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

void obring_ballot_back(tRingFrame *pFrame, unsigned char TxLport, tBallotId *pBallotId)
{
	tRMsgBallot MsgBallot;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern unsigned char DevMac[];
	extern void EthSend(unsigned char *, unsigned short);
	
	obring_prepare_fw_frame(pFrame, TxLport);
	/* Prepare ring message body */
	MsgBallot.Type = MSG_BALLOT_LOOPBACK;
	MsgBallot.Port = pFrame->obr_msg.Ballot.Port;
	MsgBallot.Id.Prio = pBallotId->Prio;
	memcpy(MsgBallot.Id.Mac, pBallotId->Mac, MAC_LEN);
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &MsgBallot, sizeof(tRMsgBallot));
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

void obring_ballot_forward(tRingFrame *pFrame, unsigned char TxLport, tBallotId *pBallotId)
{
	tRMsgBallot MsgBallot;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern unsigned char DevMac[];
	extern void EthSend(unsigned char *, unsigned short);
	
	obring_prepare_fw_frame(pFrame, TxLport);
	/* Prepare ring message body */
	MsgBallot.Type = pFrame->obr_msg.Ballot.Type;
	MsgBallot.Port = pFrame->obr_msg.Ballot.Port;
	MsgBallot.Id.Prio = pBallotId->Prio;
	memcpy(MsgBallot.Id.Mac, pBallotId->Mac, MAC_LEN);
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &MsgBallot, sizeof(tRMsgBallot));
	
	/* Send */	
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

#else

/**************************************************************************
  * @brief  Send interface for ballot frame
  * @param  none
  * @retval none
  *************************************************************************/
void obring_ballot_active_send(tRingConfigRec *pRingConfig, unsigned char TxLport)
{
	tRMsgBallot MsgBallot;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA]; 
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;	
	unsigned char TxLportIndex, TxPeerLportIndex;
	extern unsigned char DevMac[];
	extern void EthSend(unsigned char *, unsigned short);

	pRingState = &pRingInfo->DevState[pRingConfig->ucRingIndex];
	TxLportIndex = obring_get_port_idx_by_port(pRingConfig, TxLport);
	TxPeerLportIndex = (TxLportIndex == INDEX_PRIMARY)? INDEX_SECONDARY: INDEX_PRIMARY;
	
	if(pRingState->PortState[TxLportIndex].RunState == PORT_BALLOT_ACTIVE) {
		obring_prepare_tx_frame(pRingConfig, TxLport, PACKET_BALLOT);
		/* Prepare ring message body */		
		MsgBallot.Type = MSG_BALLOT_ACTIVE;
		MsgBallot.Port = TxLport;
		#if 0
		memcpy(&MsgBallot.Id, &pRingState->PortState[TxPeerLportIndex].BallotId, sizeof(tBallotId));
		#else
		MsgBallot.Id.Prio = pRingConfig->ucNodePrio;
		memcpy(MsgBallot.Id.Mac, DevMac, MAC_LEN);
		#endif
		MsgBallot.State = pRingState->PortState[TxPeerLportIndex].RunState;
		memcpy(&RingTxBuf[OFFSET_RING_DATA], &MsgBallot, sizeof(tRMsgBallot));
		
		/* Send */ 
		EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
	}	
}

void obring_ballot_chain_back(tRingFrame *pFrame, unsigned char TxLport, tBallotId *pBallotId)
{
	tRMsgBallot MsgBallot;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern unsigned char DevMac[];
	extern void EthSend(unsigned char *, unsigned short);
	
	obring_prepare_fw_frame(pFrame, TxLport);
	/* Prepare ring message body */
	MsgBallot.Type = MSG_BALLOT_CHAIN_BACK;
	MsgBallot.Port = pFrame->obr_msg.Ballot.Port;
	MsgBallot.Id.Prio = pBallotId->Prio;
	memcpy(&MsgBallot.Id, pBallotId, sizeof(tBallotId));
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &MsgBallot, sizeof(tRMsgBallot));
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

void obring_ballot_ring_back(tRingFrame *pFrame, unsigned char TxLport, tBallotId *pBallotId)
{
	tRMsgBallot MsgBallot;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern unsigned char DevMac[];
	extern void EthSend(unsigned char *, unsigned short);
	
	obring_prepare_fw_frame(pFrame, TxLport);
	/* Prepare ring message body */
	MsgBallot.Type = MSG_BALLOT_RING_BACK;
	MsgBallot.Port = pFrame->obr_msg.Ballot.Port;
	MsgBallot.Id.Prio = pBallotId->Prio;
	memcpy(&MsgBallot.Id, pBallotId, sizeof(tBallotId));
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &MsgBallot, sizeof(tRMsgBallot));
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

void obring_ballot_forward(tRingFrame *pFrame, unsigned char TxLport, tBallotId *pBallotId)
{
	tRMsgBallot MsgBallot;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern unsigned char DevMac[];
	extern void EthSend(unsigned char *, unsigned short);
	
	obring_prepare_fw_frame(pFrame, TxLport);
	/* Prepare ring message body */
	MsgBallot.Type = pFrame->obr_msg.Ballot.Type;
	MsgBallot.Port = pFrame->obr_msg.Ballot.Port;
	MsgBallot.Id.Prio = pBallotId->Prio;
	memcpy(MsgBallot.Id.Mac, pBallotId->Mac, MAC_LEN);
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &MsgBallot, sizeof(tRMsgBallot));
	
	/* Send */	
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}
#endif

/**************************************************************************
  * @brief  Send interface for hello frame
  * @param  none
  * @retval none
  *************************************************************************/
void obring_hello_send(tRingConfigRec *pRingConfig, unsigned char TxLport)
{
	tRMsgHello MsgHello;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;	
	unsigned char TxLportIndex;
	unsigned int SysTickCount;
	extern unsigned char DevMac[];
	extern void EthSend(unsigned char *, unsigned short);
	
	obring_prepare_tx_frame(pRingConfig, TxLport, PACKET_HELLO);
	/* Prepare ring message body */
	pRingState = &pRingInfo->DevState[pRingConfig->ucRingIndex];	
	TxLportIndex = obring_get_port_idx_by_port(pRingConfig, TxLport);
	
	SysTickCount = (unsigned int)(xTaskGetTickCount());
	MsgHello.Tick[0]= (unsigned char)((SysTickCount & 0xff000000) >> 24);
	MsgHello.Tick[1]= (unsigned char)((SysTickCount & 0x00ff0000) >> 16);
	MsgHello.Tick[2]= (unsigned char)((SysTickCount & 0x0000ff00) >> 8);
	MsgHello.Tick[3]= (unsigned char)(SysTickCount & 0x000000ff);
	MsgHello.MasterSecondaryStp = pRingState->PortState[INDEX_SECONDARY].StpState;
	MsgHello.TxPortStpState = pRingState->PortState[TxLportIndex].StpState;
	MsgHello.BlockLineNum[0] = 0x00;
	MsgHello.BlockLineNum[1] = 0x00;
	MsgHello.RingState = pRingState->RingState;
	
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &MsgHello, sizeof(tRMsgHello));
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

void obring_hello_forward(tRingConfigRec *pRingConfig, tRingFrame *pFrame, unsigned char TxLport)
{
	tRMsgHello MsgHello;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;	
	unsigned char TxLportIndex, TxPeerLportIndex;	
	unsigned short BlockLineNum;
	extern unsigned char DevMac[];
	extern void EthSend(unsigned char *, unsigned short);
	
	obring_prepare_fw_frame(pFrame, TxLport);
	/* Prepare ring message body */
	pRingState = &pRingInfo->DevState[pRingConfig->ucRingIndex];
	TxLportIndex = obring_get_port_idx_by_port(pRingConfig, TxLport);
	TxPeerLportIndex = (TxLportIndex == INDEX_PRIMARY)? INDEX_SECONDARY: INDEX_PRIMARY;

	memcpy(MsgHello.Tick, pFrame->obr_msg.Hello.Tick, 4);
	MsgHello.MasterSecondaryStp = pFrame->obr_msg.Hello.MasterSecondaryStp;
	MsgHello.TxPortStpState = pRingState->PortState[TxLportIndex].StpState;
	if((pFrame->obr_msg.Hello.TxPortStpState == BLOCKING) || (pRingState->PortState[TxPeerLportIndex].StpState == BLOCKING)) {
		BlockLineNum = pFrame->obr_msg.Hello.BlockLineNum[0];
		BlockLineNum = (BlockLineNum << 8) | pFrame->obr_msg.Hello.BlockLineNum[1];
		BlockLineNum++;
		MsgHello.BlockLineNum[0] = (unsigned char)((BlockLineNum & 0xFF00) >> 8);
		MsgHello.BlockLineNum[1] = (unsigned char)(BlockLineNum & 0x00FF);		
	} else {
		MsgHello.BlockLineNum[0] = pFrame->obr_msg.Hello.BlockLineNum[0];
		MsgHello.BlockLineNum[1] = pFrame->obr_msg.Hello.BlockLineNum[1];
	}
	MsgHello.RingState = pFrame->obr_msg.Hello.RingState;
	
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &MsgHello, sizeof(tRMsgHello));
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

/**************************************************************************
  * @brief  Send interface for Complete frame
  * @param  none
  * @retval none
  *************************************************************************/
void obring_complete_send(tRingConfigRec *pRingConfig, unsigned char TxLport, tRMsgComplete *pMsg)
{
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern void EthSend(unsigned char *, unsigned short);

	obring_prepare_tx_frame(pRingConfig, TxLport, PACKET_COMPLETE);
	memcpy(&RingTxBuf[OFFSET_RING_DATA], pMsg, sizeof(tRMsgComplete));	
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

void obring_complete_forward(tRingFrame *pFrame, unsigned char TxLport)
{
	tRMsgComplete MsgComplete;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern void EthSend(unsigned char *, unsigned short);

	obring_prepare_fw_frame(pFrame, TxLport);
	memcpy(&RingTxBuf[OFFSET_RING_DATA], &(pFrame->obr_msg.Complete), sizeof(tRMsgComplete));
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

/**************************************************************************
  * @brief  Send interface for Common frame
  * @param  none
  * @retval none
  *************************************************************************/
void obring_common_send(tRingConfigRec *pRingConfig, unsigned char TxLport, tRMsgCommon *pMsg)
{
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern void EthSend(unsigned char *, unsigned short);

	obring_prepare_tx_frame(pRingConfig, TxLport, PACKET_COMMON);
	/* Prepare ring message body */
	pRingMsg->Common.Type = pMsg->Type;
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

void obring_common_forward(tRingFrame *pFrame, unsigned char TxLport)
{
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern void EthSend(unsigned char *, unsigned short);

	obring_prepare_fw_frame(pFrame, TxLport);
	/* Prepare ring message body */
	pRingMsg->Common.Type = pFrame->obr_msg.Common.Type;
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

/**************************************************************************
  * @brief  Send interface for LinkDown frame
  * @param  none
  * @retval none
  *************************************************************************/
void obring_linkdown_send(tRingConfigRec *pRingConfig, unsigned char TxLport, tRMsgLinkDown *pMsg)
{
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern void EthSend(unsigned char *, unsigned short);

	obring_prepare_tx_frame(pRingConfig, TxLport, PACKET_LINKDOWN);
	
	/* Prepare ring message body */
	pRingMsg->Complete.Type = pMsg->Type;
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

void obring_linkdown_forward(tRingFrame *pFrame, unsigned char TxLport)
{
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern void EthSend(unsigned char *, unsigned short);

	obring_prepare_fw_frame(pFrame, TxLport);
	/* Prepare ring message body */
	pRingMsg->LinkDown.Type = pFrame->obr_msg.LinkDown.Type;
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

/**************************************************************************
  * @brief  Send interface for Command frame
  * @param  none
  * @retval none
  *************************************************************************/
void obring_command_request(tRingConfigRec *pRingConfig, unsigned char TxLport, tRMsgCmd *pCmdRequest)
{
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBufTest[OFFSET_RING_DATA];
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;
	extern void EthSend(unsigned char *, unsigned short);

	obring_prepare_tx_frame2(pRingConfig, &RingTxBufTest[0], TxLport, PACKET_COMMAND);
	/* Prepare ring message body */
	pRingMsg->Command.Code = pCmdRequest->Code;
	switch(pCmdRequest->Code) {
		case CMD_GET_NODE_REQ:
		memcpy(&pRingMsg->Command.Action.ReqGetNode, &pCmdRequest->Action.ReqGetNode, sizeof(tCmdReqGetNode));
		break;
		
		case CMD_RING_DISABLE_REQ:
		memcpy(&pRingMsg->Command.Action.ReqRingDisable, &pCmdRequest->Action.ReqRingDisable, sizeof(tCmdReqRingDisable));
		break;

		case CMD_RING_ENABLE_REQ:
		memcpy(&pRingMsg->Command.Action.ReqRingEnable, &pCmdRequest->Action.ReqRingEnable, sizeof(tCmdReqRingEnable));
		break;

		case CMD_RING_REBOOT_REQ:
		memcpy(&pRingMsg->Command.Action.ReqRingReboot, &pCmdRequest->Action.ReqRingReboot, sizeof(tCmdReqRingReboot));
		break;
		
		default:
		break;
	}
	
	/* Send */
	EthSend((u8 *)&RingTxBufTest[0], MAX_RING_MSG_SIZE);
}

void obring_command_forward(tRingFrame *pFrame, unsigned char TxLport)
{
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	extern void EthSend(unsigned char *, unsigned short);

	obring_prepare_fw_frame(pFrame, TxLport);
	/* Prepare ring message body */
	pRingMsg->Command.Code = pFrame->obr_msg.Command.Code;
	switch(pFrame->obr_msg.Command.Code) {
		case CMD_GET_NODE_REQ:
		memcpy(&pRingMsg->Command.Action.ReqGetNode, &pFrame->obr_msg.Command.Action.ReqGetNode, sizeof(tCmdReqGetNode));
		pRingMsg->Command.Action.ReqGetNode.NodeIndexInc++;
		break;

		case CMD_GET_NODE_RSP:
		memcpy(&pRingMsg->Command.Action.RspGetNode, &pFrame->obr_msg.Command.Action.RspGetNode, sizeof(tCmdRspGetNode));
		break;
		
		case CMD_RING_DISABLE_REQ:
		memcpy(&pRingMsg->Command.Action.ReqRingDisable, &pFrame->obr_msg.Command.Action.ReqRingDisable, sizeof(tCmdReqRingDisable));
		pRingMsg->Command.Action.ReqRingDisable.NodeIndexInc++;
		break;

		case CMD_RING_DISABLE_RSP:
		memcpy(&pRingMsg->Command.Action.RspRingDisable, &pFrame->obr_msg.Command.Action.RspRingDisable, sizeof(tCmdRspRingDisable));
		break;

		case CMD_RING_ENABLE_REQ:
		memcpy(&pRingMsg->Command.Action.ReqRingEnable, &pFrame->obr_msg.Command.Action.ReqRingEnable, sizeof(tCmdReqRingEnable));
		pRingMsg->Command.Action.ReqRingEnable.NodeIndexInc++;
		break;

		case CMD_RING_ENABLE_RSP:
		memcpy(&pRingMsg->Command.Action.RspRingEnable, &pFrame->obr_msg.Command.Action.RspRingEnable, sizeof(tCmdRspRingEnable));
		break;

		case CMD_RING_REBOOT_REQ:
		memcpy(&pRingMsg->Command.Action.ReqRingReboot, &pFrame->obr_msg.Command.Action.ReqRingReboot, sizeof(tCmdReqRingReboot));
		pRingMsg->Command.Action.ReqRingReboot.NodeIndexInc++;
		break;

		case CMD_RING_REBOOT_RSP:
		memcpy(&pRingMsg->Command.Action.RspRingReboot, &pFrame->obr_msg.Command.Action.RspRingReboot, sizeof(tCmdRspRingReboot));
		break;
		
		default:
		break;
	}
	
	/* Send */
	EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
}

/**************************************************************************
  * @brief  obring disable
  * @param  none
  * @retval none
  *************************************************************************/
int obring_disable(unsigned char RingIndex)
{
	tRingConfigGlobal RingCfgGlobal;
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	int ret;
	
	if(conf_get_ring_global(&RingCfgGlobal) != CONF_ERR_NONE) {
		return -1;
	}

	pRingConfig = &(pRingInfo->RingConfig[RingIndex]);
	pRingState = &(pRingInfo->DevState[RingIndex]);
			
	if(conf_set_ring_disable(RingIndex) != CONF_ERR_NONE) {
		return -1;
	}
	
	Set_RingLED_BlinkMode(BLINK_1HZ);
	pRingConfig->ucEnable = 0x00;
	pRingState->PortState[INDEX_PRIMARY].RunState = PORT_IDLE;
	pRingState->PortState[INDEX_SECONDARY].RunState = PORT_IDLE;
	
	if(pRingState->PortState[INDEX_PRIMARY].StpState == BLOCKING) {
		hal_swif_port_set_stp_state(pRingConfig->ucPrimaryPort, FORWARDING);
		pRingState->PortState[INDEX_PRIMARY].StpState = FORWARDING;	
	}

	if(pRingState->PortState[INDEX_SECONDARY].StpState == BLOCKING) {
		hal_swif_port_set_stp_state(pRingConfig->ucSecondaryPort, FORWARDING);
		pRingState->PortState[INDEX_SECONDARY].StpState = FORWARDING;
	}

	return 0;
}

/**************************************************************************
  * @brief  obring enable
  * @param  none
  * @retval none
  *************************************************************************/
int obring_enable(unsigned char RingIndex)
{
	tRingConfigGlobal RingCfgGlobal;
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	int ret;
	
	if(conf_get_ring_global(&RingCfgGlobal) != CONF_ERR_NONE) {
		return -1;
	}

	pRingConfig = &(pRingInfo->RingConfig[RingIndex]);
	pRingState = &(pRingInfo->DevState[RingIndex]);
		
	if(conf_set_ring_enable(RingIndex) != CONF_ERR_NONE) {
		return -1;
	}

#if 0	
	if(pRingState->PortState[INDEX_PRIMARY].NeighborValid == HAL_FALSE) {
		//hal_swif_port_set_stp_state(pRingConfig->ucPrimaryPort, BLOCKING);
		//pRingState->PortState[INDEX_PRIMARY].StpState = BLOCKING;	
	}

	if(pRingState->PortState[INDEX_SECONDARY].NeighborValid == HAL_FALSE) {
		//hal_swif_port_set_stp_state(pRingConfig->ucSecondaryPort, BLOCKING);
		//pRingState->PortState[INDEX_SECONDARY].StpState = BLOCKING;
	}
#endif

	return 0;
}

int obring_reboot(unsigned char RingIndex)
{
	tRingConfigGlobal RingCfgGlobal;
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	int ret;
	
	if(conf_get_ring_global(&RingCfgGlobal) != CONF_ERR_NONE) {
		return -1;
	}

	RebootDelayMs(5000);
	
	return 0;
}

void obring_trap_ring_change(void)
{
#if (BOARD_FEATURE & LOCAL_TRAP)
	hal_trap_ring_status TrapRingStatus;
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	unsigned char RingIndex, RingPortIndex;
	unsigned char RingPortStatus;
	extern hal_trap_info_t gTrapInfo;
	

	if((gTrapInfo.FeatureEnable == HAL_TRUE) && (gTrapInfo.GateMask & TRAP_MASK_RING_STATUS)) {
		gTrapInfo.SendEnable[TRAP_INDEX_RING_STATUS] = HAL_TRUE;
		gTrapInfo.RequestID[TRAP_INDEX_RING_STATUS]++;
		
		if(gTrapInfo.SendEnable[TRAP_INDEX_PORT_STATUS] == HAL_TRUE) {
			memset(&TrapRingStatus, 0, sizeof(hal_trap_ring_status));
			TrapRingStatus.TrapIndex = TRAP_INDEX_RING_STATUS;
			TrapRingStatus.RingPairNum = RingInfo.GlobalConfig.ucRecordNum;

			for(RingIndex=0; RingIndex<TrapRingStatus.RingPairNum; RingIndex++) {
				pRingConfig = &(pRingInfo->RingConfig[RingIndex]);
				pRingState = &(pRingInfo->DevState[RingIndex]);

				for(RingPortIndex=0; RingPortIndex<2; RingPortIndex++) {
					if(pRingState->PortState[RingPortIndex].LinkState == LINK_DOWN) {
						RingPortStatus = 0x01;
					} else {
						if(pRingState->PortState[RingPortIndex].StpState == FORWARDING)
							RingPortStatus = 0x3;
						else if(pRingState->PortState[RingPortIndex].StpState == BLOCKING)
							RingPortStatus = 0x2;
						else
							RingPortStatus = 0x0;
					}
					
					if(pRingConfig->ucEnable == 0x01)
						RingPortStatus |= 0x80;
					if(pRingState->RingState == RING_HEALTH)
						RingPortStatus |= 0x40;	
					if(pRingState->NodeType == NODE_TYPE_MASTER)
						RingPortStatus |= 0x20;	
					
					TrapRingStatus.RingPairStatus[2*RingIndex + RingPortIndex].Flag = RingPortStatus;
					memcpy(TrapRingStatus.RingPairStatus[2*RingIndex + RingPortIndex].ExtNeighborMac, pRingState->PortState[RingPortIndex].NeighborMac, MAC_LEN);
					TrapRingStatus.RingPairStatus[2*RingIndex + RingPortIndex].ExtNeighborPortNo = pRingState->PortState[RingPortIndex].NeighborPortNo;
				}
			}
			hal_swif_trap_send(gTrapInfo.ServerMac, (uint8 *)&TrapRingStatus, sizeof(hal_trap_ring_status), gTrapInfo.RequestID[TRAP_INDEX_RING_STATUS]);
		}
	}
#endif
}

/**************************************************************************
  * @brief  OBRing timer start
  * @param  *Timer, value
  * @retval none
  *************************************************************************/
void obring_timer_start(tRingTimer *Timer, unsigned short value, eTimingUnit Timing)
{
	if(Timing == T_MS)
		Timer->Value = value;
	else
		Timer->Value = value * 1000;
	Timer->Active = 1;
}

/**************************************************************************
  * @brief  OBRing timer stop
  * @param  *Timer
  * @retval none
  *************************************************************************/
void obring_timer_stop(tRingTimer *Timer)
{
	Timer->Value = 0;
	Timer->Active = 0;
}

/**************************************************************************
  * @brief  OBRing hello timer expire handle
  * @param  *pstRing
  * @retval none
  *************************************************************************/
void obring_hello_timer_expire(tRingConfigRec *pRingConfig)
{
	unsigned short HelloTime;
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;
	
	pRingState = &(pRingInfo->DevState[pRingConfig->ucRingIndex]);
	HelloTime = pRingConfig->usHelloTime[0];
	HelloTime = (HelloTime << 8) | (unsigned short)(pRingConfig->usHelloTime[1]);
	if((pRingState->PortState[INDEX_PRIMARY].RunState == PORT_BALLOT_FINISH) && (pRingState->NodeType == NODE_TYPE_MASTER)) {
		OB_DEBUG(DBG_OBRING, "[Port1 Tx Timing Hello %d]\r\n", pRingState->HelloSeq + 1);
		obring_hello_send(pRingConfig, pRingConfig->ucPrimaryPort);
	}
	obring_timer_start(&(RingTimer[pRingConfig->ucRingIndex].Hello), HelloTime, T_SEC);	
}

/**************************************************************************
  * @brief  OBRing fail timer expire handle
  * @param  *pstRing
  * @retval none
  *************************************************************************/
void obring_fail_timer_expire(tRingConfigRec *pRingConfig)
{
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;
	unsigned char RingIndex;
	unsigned short AuthTime;
	tRMsgCommon MsgCommon;
	
	RingIndex = pRingConfig->ucRingIndex;
	pRingState = &(pRingInfo->DevState[RingIndex]);
	obring_timer_stop(&RingTimer[RingIndex].Fail);
	
	if(pRingState->NodeType == NODE_TYPE_MASTER) {
		if(pRingState->NodeState == NODE_STATE_FAIL) {
			Set_RingLED_BlinkMode(BLINK_2HZ);
			pRingState->RingState = RING_FAULT;

			if(pRingState->PortState[INDEX_PRIMARY].NeighborValid == HAL_TRUE) {
				if(pRingState->PortState[INDEX_PRIMARY].StpState == BLOCKING) {
        			hal_swif_port_set_stp_state(pRingConfig->ucPrimaryPort, FORWARDING);
					pRingState->PortState[INDEX_PRIMARY].StpState = FORWARDING;
				}
			}

			if(pRingState->PortState[INDEX_SECONDARY].NeighborValid == HAL_TRUE) {
				if(pRingState->PortState[INDEX_SECONDARY].StpState == BLOCKING) {
        			hal_swif_port_set_stp_state(pRingConfig->ucSecondaryPort, FORWARDING);
					pRingState->PortState[INDEX_SECONDARY].StpState = FORWARDING;
				}
			}

			obring_mac_flush();
		} else {
			Set_RingLED_BlinkMode(BLINK_2HZ);
			pRingState->NodeState = NODE_STATE_FAIL;
			pRingState->RingState = RING_FAULT;
			/* what to do next ? */
		}
	} else if(pRingState->NodeType == NODE_TYPE_TRANSIT) {
		if(pRingState->RingState == RING_FAULT) {
			if(pRingState->PortState[INDEX_PRIMARY].NeighborValid == HAL_TRUE) {
				if(pRingState->PortState[INDEX_PRIMARY].StpState == BLOCKING) {
        			hal_swif_port_set_stp_state(pRingConfig->ucPrimaryPort, FORWARDING);
					pRingState->PortState[INDEX_PRIMARY].StpState = FORWARDING;
				}
			}

			if(pRingState->PortState[INDEX_SECONDARY].NeighborValid == HAL_TRUE) {
				if(pRingState->PortState[INDEX_SECONDARY].StpState == BLOCKING) {
        			hal_swif_port_set_stp_state(pRingConfig->ucSecondaryPort, FORWARDING);
					pRingState->PortState[INDEX_SECONDARY].StpState = FORWARDING;
				}
			}

			obring_mac_flush();
		} else {
			Set_RingLED_BlinkMode(BLINK_2HZ);
			pRingState->RingState = RING_FAULT;
			#if 0
			AuthTime = pRingConfig->usAuthTime[0];
			AuthTime = (AuthTime << 8) | (unsigned short)(pRingConfig->usAuthTime[1]);
			obring_timer_start(&RingTimer[RingIndex].AuthP, AuthTime, T_SEC);
			obring_timer_start(&RingTimer[RingIndex].AuthS, AuthTime, T_SEC);
			hal_swif_port_set_stp_state(pRingConfig->ucPrimaryPort, BLOCKING);
			pRingState->PortState[INDEX_PRIMARY].StpState = BLOCKING;
    		hal_swif_port_set_stp_state(pRingConfig->ucSecondaryPort, BLOCKING);
			pRingState->PortState[INDEX_SECONDARY].StpState = BLOCKING;	
			obring_mac_flush();
			#endif
		}
	}
}

/**************************************************************************
  * @brief  Primary port ballot timer expire handle
  * @param  *pstRing
  * @retval none
  *************************************************************************/
void obring_ballotp_timer_expire(tRingConfigRec *pRingConfig)
{
	unsigned short BallotTime;

	OB_DEBUG(DBG_OBRING, "[Port1 Tx Timing Ballot Active]\r\n");
	obring_ballot_active_send(pRingConfig, pRingConfig->ucPrimaryPort);
	BallotTime = pRingConfig->usBallotTime[0];
	BallotTime = (BallotTime << 8) | (unsigned short)(pRingConfig->usBallotTime[1]);
	obring_timer_start(&(RingTimer[pRingConfig->ucRingIndex].BallotP), BallotTime, T_SEC);
}

/**************************************************************************
  * @brief  Secondary port ballot timer expire handle
  * @param  *pRingConfig
  * @retval none
  *************************************************************************/
void obring_ballots_timer_expire(tRingConfigRec *pRingConfig)
{
	unsigned short BallotTime;

	OB_DEBUG(DBG_OBRING, "[Port2 Tx Timing Ballot Active]\r\n");
	obring_ballot_active_send(pRingConfig, pRingConfig->ucSecondaryPort);
	BallotTime = pRingConfig->usBallotTime[0];
	BallotTime = (BallotTime << 8) | (unsigned short)(pRingConfig->usBallotTime[1]);
	obring_timer_start(&(RingTimer[pRingConfig->ucRingIndex].BallotS), BallotTime, T_SEC);
}

/**************************************************************************
  * @brief  Primary port auth timer expire handle
  * @param  *pRingConfig
  * @retval none
  *************************************************************************/
void obring_authp_timer_expire(tRingConfigRec *pRingConfig)
{
	unsigned short AuthTime;
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;

	pRingState = &(pRingInfo->DevState[pRingConfig->ucRingIndex]);
	if(pRingState->PortState[INDEX_PRIMARY].LinkState == LINK_UP) {
		OB_DEBUG(DBG_OBRING, "[Port%d Tx Timing Authentication Request]\r\n", pRingConfig->ucPrimaryPort);
		obring_auth_request(pRingConfig, pRingConfig->ucPrimaryPort);
	}
	AuthTime = pRingConfig->usAuthTime[0];
	AuthTime = (AuthTime << 8) | (unsigned short)(pRingConfig->usAuthTime[1]);	
	obring_timer_start(&(RingTimer[pRingConfig->ucRingIndex].AuthP), AuthTime, T_SEC);
	
}

/**************************************************************************
  * @brief  Secondary port auth timer expire handle
  * @param  *pRingConfig
  * @retval none
  *************************************************************************/
void obring_auths_timer_expire(tRingConfigRec *pRingConfig)
{
	unsigned short AuthTime;
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;

	pRingState = &(pRingInfo->DevState[pRingConfig->ucRingIndex]);
	if(pRingState->PortState[INDEX_SECONDARY].LinkState == LINK_UP) {
		OB_DEBUG(DBG_OBRING, "[Port%d Tx Timing Authentication Request]\r\n", pRingConfig->ucSecondaryPort);
		obring_auth_request(pRingConfig, pRingConfig->ucSecondaryPort);
	}
	AuthTime = pRingConfig->usAuthTime[0];
	AuthTime = (AuthTime << 8) | (unsigned short)(pRingConfig->usAuthTime[1]);		
	obring_timer_start(&(RingTimer[pRingConfig->ucRingIndex].AuthS), AuthTime, T_SEC);
}

/**************************************************************************
  * @brief  Is timer expired
  * @param  *Timer
  * @retval 0: ture, 1: false
  *************************************************************************/
int obring_timer_expired(tRingTimer *Timer)
{
	if(Timer->Active == 0)
		return 0;
	Timer->Value -= RING_TICK_VAL;
	if(Timer->Value <= 0) {
		Timer->Active = 0;
		Timer->Value = 0;
		return 1;
	}
	return 0;
}

/**************************************************************************
  * @brief  Timer tick handle
  * @param  *Timer
  * @retval none
  *************************************************************************/
void obring_timer_tick(void)
{
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	unsigned char RingIndex;
	extern unsigned char DevMac[];
    
	for(RingIndex = 0; RingIndex < pRingInfo->GlobalConfig.ucRecordNum; RingIndex++) {
		pRingConfig = &(pRingInfo->RingConfig[RingIndex]);
		pRingState = &(pRingInfo->DevState[RingIndex]);

		/* Check PrimaryPort authentication timer is expire */
		if(obring_timer_expired(&(RingTimer[RingIndex].AuthP))) {
			pRingState->PortState[INDEX_PRIMARY].AuthTimoutCount++;
			if(pRingState->PortState[INDEX_PRIMARY].AuthTimoutCount >= MAX_AUTH_TIMEOUT_COUNT) {
				obring_timer_stop(&(RingTimer[RingIndex].AuthP));
				pRingState->PortState[INDEX_PRIMARY].AuthTimoutCount = 0;
				pRingState->PortState[INDEX_PRIMARY].RunState = PORT_AUTH_FAIL;
				if(pRingState->PortState[INDEX_PRIMARY].StpState == FORWARDING) {
        			hal_swif_port_set_stp_state(pRingConfig->ucPrimaryPort, BLOCKING);
					pRingState->PortState[INDEX_PRIMARY].StpState = BLOCKING;
				}
				memcpy(pRingState->PortState[INDEX_PRIMARY].BallotId.Mac, DevMac, MAC_LEN);
				pRingState->PortState[INDEX_PRIMARY].NeighborValid = HAL_FALSE;
				memset(pRingState->PortState[INDEX_PRIMARY].NeighborMac, 0, MAC_LEN);
				pRingState->PortState[INDEX_PRIMARY].NeighborPortNo = 0;
			} else {			
				obring_authp_timer_expire(&(pRingInfo->RingConfig[RingIndex]));
			}
		}

		/* Check SecondaryPort authentication timer is expire */
		if(obring_timer_expired(&(RingTimer[RingIndex].AuthS))) {
			pRingState->PortState[INDEX_SECONDARY].AuthTimoutCount++;
			if(pRingState->PortState[INDEX_SECONDARY].AuthTimoutCount >= MAX_AUTH_TIMEOUT_COUNT) {
				obring_timer_stop(&(RingTimer[RingIndex].AuthS));
				pRingState->PortState[INDEX_SECONDARY].AuthTimoutCount = 0;
				pRingState->PortState[INDEX_SECONDARY].RunState = PORT_AUTH_FAIL;
				if(pRingState->PortState[INDEX_SECONDARY].StpState == FORWARDING) {
        			hal_swif_port_set_stp_state(pRingConfig->ucSecondaryPort, BLOCKING);
					pRingState->PortState[INDEX_SECONDARY].StpState = BLOCKING;
				}
				memcpy(pRingState->PortState[INDEX_SECONDARY].BallotId.Mac, DevMac, MAC_LEN);
				pRingState->PortState[INDEX_SECONDARY].NeighborValid = HAL_FALSE;
				memset(pRingState->PortState[INDEX_SECONDARY].NeighborMac, 0, MAC_LEN);
				pRingState->PortState[INDEX_SECONDARY].NeighborPortNo = 0;				
			} else {			
				obring_auths_timer_expire(&(pRingInfo->RingConfig[RingIndex]));
			}
		}

		/* Check PrimaryPort ballot timer is expire */
		if(obring_timer_expired(&(RingTimer[RingIndex].BallotP))) {	
			obring_ballotp_timer_expire(&(pRingInfo->RingConfig[RingIndex]));
		}

		/* Check SecondaryPort ballot timer is expire */
		if(obring_timer_expired(&(RingTimer[RingIndex].BallotS))) {	
			obring_ballots_timer_expire(&(pRingInfo->RingConfig[RingIndex]));
		}
		
		/* Check Hello timer is expire */
		if(obring_timer_expired(&(RingTimer[RingIndex].Hello))) {	
			obring_hello_timer_expire(&(pRingInfo->RingConfig[RingIndex]));
		}

		/* Check Fail timer is expire */
		if(obring_timer_expired(&(RingTimer[RingIndex].Fail))) {
			obring_fail_timer_expire(&(pRingInfo->RingConfig[RingIndex]));
		}
	}
}

void obring_ballot_master_handle(tRingConfigRec *pRingConfig, HAL_BOOL IsRing)
{
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;	
	tBallotId DevSelfBallotId;
	unsigned short HelloTime, FailTime;
	unsigned char RingIndex;
	
	HelloTime = pRingConfig->usHelloTime[0];
	HelloTime = (HelloTime << 8) | (unsigned short)(pRingConfig->usHelloTime[1]);	
	FailTime = pRingConfig->usFailTime[0];
	FailTime = (FailTime << 8) | (unsigned short)(pRingConfig->usFailTime[1]);
	RingIndex = pRingConfig->ucRingIndex;
	pRingState = &pRingInfo->DevState[RingIndex];
	
	if(pRingState->NodeType == NODE_TYPE_TRANSIT) {
		pRingState->SwitchTimes = 0;
		pRingState->StormCount = 0;
		pRingState->HelloSeq = 0;
		pRingState->NodeType = NODE_TYPE_MASTER;
		pRingState->NodeState = NODE_STATE_FAIL;
		pRingState->RingState = RING_FAULT;
		
		hal_swif_port_set_stp_state(pRingConfig->ucSecondaryPort, BLOCKING);
		pRingState->PortState[INDEX_SECONDARY].StpState = BLOCKING;
		
		obring_timer_start(&RingTimer[RingIndex].Hello, HelloTime, T_SEC);
		if(IsRing == HAL_FALSE) {
			obring_timer_start(&RingTimer[RingIndex].Fail, FailTime, T_SEC);
		}
	} else {	
		if(pRingState->NodeState == NODE_STATE_IDLE)
			pRingState->NodeState = NODE_STATE_FAIL;
		
		if(pRingState->NodeState == NODE_STATE_FAIL) {
			if(IsRing == HAL_TRUE) {
				OB_DEBUG(DBG_OBRING, "[Start HelloTimer and FailTimer]\r\n");	
				if(RingTimer[RingIndex].Hello.Active == 0) {
					obring_timer_start(&RingTimer[RingIndex].Hello, HelloTime, T_SEC);
				}
			} else {
				obring_timer_stop(&RingTimer[RingIndex].Hello);
				obring_timer_start(&RingTimer[RingIndex].Fail, FailTime, T_SEC);
			} 
		}
	}
}

void obring_ballot_transit_handle(tRingConfigRec *pRingConfig, HAL_BOOL IsRing)
{
	tRingInfo *pRingInfo = &RingInfo;
	tRingState *pRingState;	
	unsigned short FailTime;
	unsigned char RingIndex;
	
	FailTime = pRingConfig->usFailTime[0];
	FailTime = (FailTime << 8) | (unsigned short)(pRingConfig->usFailTime[1]);
	RingIndex = pRingConfig->ucRingIndex;
	pRingState = &pRingInfo->DevState[RingIndex];
	
	/* Transit node */
	if(pRingState->NodeType == NODE_TYPE_MASTER) {
		obring_timer_stop(&RingTimer[RingIndex].Hello);
		pRingState->SwitchTimes = 0;
		pRingState->HelloSeq = 0;
		pRingState->NodeType = NODE_TYPE_TRANSIT;
	}
	
	if(IsRing == HAL_FALSE)
		obring_timer_start(&RingTimer[RingIndex].Fail, FailTime, T_SEC);
}

/**************************************************************************
  * @brief  OBRing frame receive and handle
  * @param  *pRxFrame, rxLen
  * @retval none
  *************************************************************************/
void obring_frame_handle(unsigned char *pRxBuf, unsigned short rxLen)
{
	tRingFrame *pFrame = (tRingFrame *)pRxBuf;
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	unsigned short HelloTime, FailTime, BallotTime, AuthTime;
	unsigned char RingIndex, RxLportIndex, RxPeerLportIndex;
	unsigned char BallotLportIndex, BallotLportPeerIndex;
	unsigned char DevBallotEndFlag=0;
	unsigned char PacketType;
	unsigned short BlockLineNum;
	unsigned short HelloSeq;
	tRMsgComplete MsgComplete;
	tRMsgCommon MsgCommon;
	tRMsgCmd MsgCmd;
	tBallotId DevSelfBallotId, TempBallotId;
	unsigned int CurrSysTick, PrevSysTick;
	unsigned short RingId, SeqNum;
	unsigned char RxHport, RxLport, RxPeerLport;
	tRingMsgBody *pRingMsg;
	tRingMsgBody *pRingTxMsg = (tRingMsgBody *)&RingTxBuf[OFFSET_RING_DATA];
	int ret;
    extern unsigned char DevMac[];
	extern void EthSend(unsigned char *, unsigned short);
	
#if SWITCH_CHIP_88E6095
	RxHport = (pFrame->switch_tag[1] & 0xF8) >> 3;
	RxLport = hal_swif_hport_2_lport(RxHport);
#elif SWITCH_CHIP_BCM53101
    RxHport = (pFrame->switch_tag[3] & 0x3F);
    RxLport = hal_swif_hport_2_lport(RxHport);
#elif SWITCH_CHIP_BCM53286
#elif SWITCH_CHIP_BCM5396

#else
	#error "switch chip type unkonw"	
#endif

	RingIndex = obring_get_ring_idx_by_port(RxLport);
	if(RingIndex == 0xFF)
		return;
		
	pRingConfig = &(pRingInfo->RingConfig[RingIndex]);
	pRingState = &(pRingInfo->DevState[RingIndex]);
	RxLportIndex = obring_get_port_idx_by_port(pRingConfig, RxLport);
			
	if(pRingState->PortState[RxLportIndex].LinkState == LINK_DOWN)
		return;
	
	RxPeerLport = (RxLport == pRingConfig->ucPrimaryPort)? pRingConfig->ucSecondaryPort : pRingConfig->ucPrimaryPort;
	RxPeerLportIndex = (RxLportIndex == INDEX_PRIMARY)? INDEX_SECONDARY: INDEX_PRIMARY;
	DevSelfBallotId.Prio = pRingConfig->ucNodePrio;
	memcpy(DevSelfBallotId.Mac, DevMac, MAC_LEN);
	AuthTime = pRingConfig->usAuthTime[0];
	AuthTime = (AuthTime << 8) | (unsigned short)(pRingConfig->usAuthTime[1]);	
	BallotTime = pRingConfig->usBallotTime[0];
	BallotTime = (BallotTime << 8) | (unsigned short)(pRingConfig->usBallotTime[1]);
	HelloTime = pRingConfig->usHelloTime[0];
	HelloTime = (HelloTime << 8) | (unsigned short)(pRingConfig->usHelloTime[1]);	
	FailTime = pRingConfig->usFailTime[0];
	FailTime = (FailTime << 8) | (unsigned short)(pRingConfig->usFailTime[1]);
	pRingMsg = &pFrame->obr_msg;
	
	switch(pFrame->obr_type) {
		case PACKET_AUTHENTICATION:
		if(pRingConfig->ucEnable == 0x01) {
			switch(pRingMsg->Auth.Type) {
	        	case MSG_AUTH_REQ:
				OB_DEBUG(DBG_OBRING, "[Port%d Rx Authentication Request, and Tx Ack]\r\n", RxLport);
				obring_auth_ack(pRingConfig, RxLport);
				break;

	        	case MSG_AUTH_ACK:
				OB_DEBUG(DBG_OBRING, "[Port%d Rx Authentication Ack]", RxLport);
				/* Stop Authentication timer */
				if(RxLport == pRingConfig->ucPrimaryPort) {
					obring_timer_stop(&(RingTimer[RingIndex].AuthP));
					pRingState->PortState[RxLportIndex].AuthTimoutCount = 0;
				} else {
					obring_timer_stop(&(RingTimer[RingIndex].AuthS));
					pRingState->PortState[RxLportIndex].AuthTimoutCount = 0;
				}
				
				/* Update neigbor inforamtion */
				memcpy(pRingState->PortState[RxLportIndex].NeighborMac, pFrame->obr_sys_mac, MAC_LEN);
				pRingState->PortState[RxLportIndex].NeighborPortNo = pFrame->obr_port_id;
				pRingState->PortState[RxLportIndex].NeighborValid = HAL_TRUE;
				pRingState->PortState[RxLportIndex].RunState = PORT_AUTH_PASS;

				#if 1
				pRingState->PortState[RxLportIndex].RunState = PORT_BALLOT_ACTIVE;
				#else
				if((pRingState->PortState[RxPeerLportIndex].RunState == LINK_DOWN) || \
					(pRingState->PortState[RxPeerLportIndex].RunState == PORT_AUTH_FAIL) || \
					(pRingState->PortState[RxPeerLportIndex].RunState == PORT_AUTH_PASS) || \
					(pRingState->PortState[RxPeerLportIndex].RunState == PORT_BALLOT_FINISH)) {
					pRingState->PortState[RxLportIndex].RunState = PORT_BALLOT_ACTIVE;
				}
				#endif
				
				if(pRingState->PortState[RxLportIndex].RunState == PORT_BALLOT_ACTIVE) {
					OB_DEBUG(DBG_OBRING, ", [Port%d Start BallotTimer]\r\n", RxLport);
					if(RxLport == pRingConfig->ucPrimaryPort)
						obring_timer_start(&(RingTimer[RingIndex].BallotP), BallotTime, T_SEC);
					else
						obring_timer_start(&(RingTimer[RingIndex].BallotS), BallotTime, T_SEC);	
				}
				
				break;

				default:
				break;
			}
		}
		break;

		case PACKET_BALLOT:
		if((pRingConfig->ucEnable == 0x01) && (pRingState->PortState[RxLportIndex].NeighborValid == HAL_TRUE)) {
#if BALLOT_MODE_1
			OB_DEBUG(DBG_OBRING, "[Port%d Rx Ballot-%s]", RxLport, 
								(pRingMsg->Ballot.Type == MSG_BALLOT_ACTIVE)? "Active": \
								(pRingMsg->Ballot.Type == MSG_BALLOT_PASSIVE)? "Passive": \
								(pRingMsg->Ballot.Type == MSG_BALLOT_LOOPBACK)? "Loopback": "Unkown");
#else
			OB_DEBUG(DBG_OBRING, "[Port%d Rx Ballot-%s]", RxLport, 
								(pRingMsg->Ballot.Type == MSG_BALLOT_ACTIVE)? "Active": \
								(pRingMsg->Ballot.Type == MSG_BALLOT_CHAIN_BACK)? "ChainBack": \
								(pRingMsg->Ballot.Type == MSG_BALLOT_RING_BACK)? "RingBack": "Unkown");
#endif
			/********************************************************************************************************************
														Packet's SourceMac == DeviceMac 
			 ********************************************************************************************************************/			
			if(memcmp(pFrame->src_mac, DevMac, 6) == 0) {

#if BALLOT_MODE_1
				BallotLportIndex = obring_get_port_idx_by_port(pRingConfig, pFrame->obr_msg.Ballot.Port);
				BallotLportPeerIndex = (BallotLportIndex == INDEX_PRIMARY)? INDEX_SECONDARY: INDEX_PRIMARY;
				OB_DEBUG(DBG_OBRING, ", [Port%d Ballot Finish]", pFrame->obr_msg.Ballot.Port); 
				
				if(BallotLportIndex == INDEX_PRIMARY)
					obring_timer_stop(&(RingTimer[RingIndex].BallotP));
				else
					obring_timer_stop(&(RingTimer[RingIndex].BallotS));
				
				pRingState->PortState[BallotLportIndex].RunState = PORT_BALLOT_FINISH;
				memcpy(&pRingState->PortState[BallotLportIndex].BallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId));

				DevBallotEndFlag = 0;
				if(pRingState->PortState[BallotLportPeerIndex].NeighborValid == HAL_TRUE) {
					if(pRingState->PortState[BallotLportPeerIndex].RunState == PORT_BALLOT_FINISH) {
						if( (memcmp(&pRingState->PortState[BallotLportIndex].BallotId, &DevSelfBallotId, sizeof(tBallotId)) == 0) && \
							(memcmp(&pRingState->PortState[BallotLportPeerIndex].BallotId, &DevSelfBallotId, sizeof(tBallotId)) == 0)) {
							DevBallotEndFlag = 1;
						} else {
							DevBallotEndFlag = 2;
						}
					}
				} else {
					if((memcmp(&pRingState->PortState[BallotLportIndex].BallotId, &DevSelfBallotId, sizeof(tBallotId)) == 0)) {
						DevBallotEndFlag = 1;
					} else {
						DevBallotEndFlag = 2;
					}
				}

				switch(DevBallotEndFlag) {
					case 1:
					if(pRingState->NodeType != NODE_TYPE_MASTER) {
						pRingState->SwitchTimes = 0;
						pRingState->HelloSeq = 0;
						pRingState->NodeType = NODE_TYPE_MASTER;
						pRingState->NodeState = NODE_STATE_FAIL;
						OB_DEBUG(DBG_OBRING, ", [Start HelloTimer and FailTimer]");	
						obring_timer_start(&RingTimer[RingIndex].Hello, HelloTime, T_SEC);
						obring_timer_start(&RingTimer[RingIndex].Fail, FailTime, T_SEC);
					} else {
						if(pRingState->NodeState == NODE_STATE_IDLE)
							pRingState->NodeState = NODE_STATE_FAIL;
						
						if(pRingState->NodeState == NODE_STATE_FAIL) {
							OB_DEBUG(DBG_OBRING, ", [Start HelloTimer and FailTimer]");	
							if(RingTimer[RingIndex].Hello.Active == 0) {
								obring_timer_start(&RingTimer[RingIndex].Hello, HelloTime, T_SEC);
							}
							obring_timer_start(&RingTimer[RingIndex].Fail, FailTime, T_SEC);
						}
					}
					break;

					case 2:
					if(pRingState->NodeType == NODE_TYPE_MASTER) {
						obring_timer_stop(&RingTimer[RingIndex].Hello);
						pRingState->SwitchTimes = 0;
						pRingState->HelloSeq = 0;
						pRingState->NodeType = NODE_TYPE_TRANSIT;
					}
					obring_timer_start(&RingTimer[RingIndex].Fail, FailTime, T_SEC);
					break;

					default:
					break;
				}
				OB_DEBUG(DBG_OBRING, "\r\n");
				
#else
				BallotLportIndex = obring_get_port_idx_by_port(pRingConfig, pFrame->obr_msg.Ballot.Port);
				BallotLportPeerIndex = (BallotLportIndex == INDEX_PRIMARY)? INDEX_SECONDARY: INDEX_PRIMARY;
				
				if(RxLport == pFrame->obr_msg.Ballot.Port) {
					switch(pFrame->obr_msg.Ballot.Type) {
						case MSG_BALLOT_CHAIN_BACK:
						/* Ballot Finish */
						if(BallotLportIndex == INDEX_PRIMARY)
							obring_timer_stop(&(RingTimer[RingIndex].BallotP));
						else
							obring_timer_stop(&(RingTimer[RingIndex].BallotS));

						/* Update Port Ballot ID */
						memcpy(&pRingState->PortState[RxLportIndex].BallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId));
						pRingState->PortState[RxLportIndex].RunState = PORT_BALLOT_FINISH;

						#if 0
						if(pRingState->PortState[RxPeerLportIndex].RunState == PORT_AUTH_PASS) {
							pRingState->PortState[RxPeerLportIndex].RunState = PORT_BALLOT_ACTIVE;
							obring_ballot_active_send(pRingConfig, RxPeerLport);
						} else if(pRingState->PortState[RxPeerLportIndex].RunState == PORT_BALLOT_FINISH) {
							//obring_ballot_feedback(pRingConfig, RxPeerLport);
						}
						#endif

						
						/* Update NodeType, NodeState, and others */
						if(pRingState->PortState[BallotLportPeerIndex].NeighborValid == HAL_TRUE) {
							if(pRingState->PortState[BallotLportPeerIndex].RunState == PORT_BALLOT_FINISH) {
								if( (memcmp(&pRingState->PortState[BallotLportIndex].BallotId, &DevSelfBallotId, sizeof(tBallotId)) == 0) && \
									(memcmp(&pRingState->PortState[BallotLportPeerIndex].BallotId, &DevSelfBallotId, sizeof(tBallotId)) == 0)) {
									
									if(pRingState->NodeType != NODE_TYPE_MASTER) {
										pRingState->SwitchTimes = 0;
										pRingState->HelloSeq = 0;
										pRingState->NodeType = NODE_TYPE_MASTER;
										pRingState->NodeState = NODE_STATE_FAIL;
									} else {
										//if(pRingState->NodeState == NODE_STATE_IDLE)
										pRingState->NodeState = NODE_STATE_FAIL;
									}
									//DevBallotEndFlag = 1;
								} else {
									if(pRingState->NodeType == NODE_TYPE_MASTER) {
										if(RingTimer[RingIndex].Hello.Active == 1) {
											obring_timer_stop(&RingTimer[RingIndex].Hello);
										}
										pRingState->SwitchTimes = 0;
										pRingState->HelloSeq = 0;
										pRingState->NodeType = NODE_TYPE_TRANSIT;
									}								
									//DevBallotEndFlag = 2;
								}
							}
						} else {
							if((memcmp(&pRingState->PortState[BallotLportIndex].BallotId, &DevSelfBallotId, sizeof(tBallotId)) == 0)) {
								if(pRingState->NodeType != NODE_TYPE_MASTER) {
									pRingState->SwitchTimes = 0;
									pRingState->HelloSeq = 0;
									pRingState->NodeType = NODE_TYPE_MASTER;
									pRingState->NodeState = NODE_STATE_FAIL;
								} else {
									//if(pRingState->NodeState == NODE_STATE_IDLE)
										pRingState->NodeState = NODE_STATE_FAIL;
								}								
								//DevBallotEndFlag = 1;
							} else {
								//DevBallotEndFlag = 2;
								if(pRingState->NodeType == NODE_TYPE_MASTER) {
									if(RingTimer[RingIndex].Hello.Active == 1) {
										obring_timer_stop(&RingTimer[RingIndex].Hello);
									}
									pRingState->SwitchTimes = 0;
									pRingState->HelloSeq = 0;
									pRingState->NodeType = NODE_TYPE_TRANSIT;
								}
							}
						}
						obring_timer_start(&RingTimer[RingIndex].Fail, FailTime, T_SEC);
						break;

						case MSG_BALLOT_RING_BACK:
						if(BallotLportIndex == INDEX_PRIMARY)
							obring_timer_stop(&(RingTimer[RingIndex].BallotP));
						else
							obring_timer_stop(&(RingTimer[RingIndex].BallotS));
						pRingState->PortState[BallotLportIndex].RunState = PORT_BALLOT_FINISH;
						memcpy(&pRingState->PortState[BallotLportIndex].BallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId));	
						if(memcmp(&pRingMsg->Ballot.Id, &DevSelfBallotId, sizeof(tBallotId)) == 0) {
							/* Master node */
							obring_ballot_master_handle(pRingConfig, HAL_TRUE);
						} else {
							/* Transit node */
							obring_ballot_transit_handle(pRingConfig, HAL_TRUE);
						}
						//obring_timer_start(&RingTimer[RingIndex].Fail, FailTime, T_SEC);
						break;

						default:
						break;
					}

				} else {
					if(pFrame->obr_msg.Ballot.Type == MSG_BALLOT_ACTIVE) {
						/* Ring back ballot packet */
						obring_ballot_ring_back(pFrame, RxLport, &pRingMsg->Ballot.Id);
						#if 0
						if(memcmp(&pRingMsg->Ballot.Id, &DevSelfBallotId, sizeof(tBallotId)) == 0) {
							/* Master node */
							obring_ballot_master_handle(pRingConfig, &pRingMsg->Ballot.Id, HAL_TRUE);
						} else {
							/* Transit node */
							obring_ballot_transit_handle(pRingConfig, &pRingMsg->Ballot.Id, HAL_TRUE);
						}
						#endif
					}
				}
				OB_DEBUG(DBG_OBRING, "\r\n");
#endif				

			} else {	
			/********************************************************************************************************************
														Packet's SourceMac != DeviceMac 
			 ********************************************************************************************************************/		
			 	switch(pFrame->obr_msg.Ballot.Type) {
#if BALLOT_MODE_1
	            	case MSG_BALLOT_ACTIVE:
					if(memcmp(&DevSelfBallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId)) < 0) {
						memcpy(&TempBallotId, &DevSelfBallotId, sizeof(tBallotId));
					} else {
						memcpy(&TempBallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId));
					}
					/* Forward or loopback ballot packet */
					if(pRingState->PortState[RxPeerLportIndex].NeighborValid == HAL_TRUE) {
						OB_DEBUG(DBG_OBRING, ", [Port%d Forward Ballot-%s]", RxPeerLport,
							(pRingMsg->Ballot.Type == MSG_BALLOT_ACTIVE)? "Active": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_PASSIVE)? "Passive": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_LOOPBACK)? "Loopback": "Unkown");
						obring_ballot_forward(pFrame, RxPeerLport, &TempBallotId);
					} else {
						OB_DEBUG(DBG_OBRING, ", [Port%d Loopback Ballot-%s]", RxLport,
							(pRingMsg->Ballot.Type == MSG_BALLOT_ACTIVE)? "Active": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_PASSIVE)? "Passive": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_LOOPBACK)? "Loopback": "Unkown");	
						obring_ballot_back(pFrame, RxLport, &TempBallotId);	
					}
					
					/* Send passive ballot packet */
					if(pRingState->PortState[RxLportIndex].RunState == PORT_BALLOT_FINISH) {
						if((pRingState->PortState[RxLportIndex].NeighborValid == HAL_TRUE) && \
							(memcmp(pRingState->PortState[RxLportIndex].NeighborMac, pFrame->src_mac, MAC_LEN) != 0)) {
							OB_DEBUG(DBG_OBRING, ", [Port%d Tx Ballot-Passive]", RxLport); 
							obring_ballot_passive_send(pRingConfig, RxLport);
						}
					}	
					OB_DEBUG(DBG_OBRING, "\r\n"); 
					break;
					
	            	case MSG_BALLOT_PASSIVE:
					if(memcmp(&DevSelfBallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId)) < 0) {
						memcpy(&TempBallotId, &DevSelfBallotId, sizeof(tBallotId));
					} else {
						memcpy(&TempBallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId));
					}
					/* Forward or loopback ballot packet */
					if(pRingState->PortState[RxPeerLportIndex].NeighborValid == HAL_TRUE) {
						OB_DEBUG(DBG_OBRING, ", [Port%d Forward Ballot-%s]\r\n", RxPeerLport,
							(pRingMsg->Ballot.Type == MSG_BALLOT_ACTIVE)? "Active": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_PASSIVE)? "Passive": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_LOOPBACK)? "Loopback": "Unkown");
						obring_ballot_forward(pFrame, RxPeerLport, &TempBallotId);
					} else {
						OB_DEBUG(DBG_OBRING, ", [Port%d Loopback Ballot-%s]\r\n", RxPeerLport,
							(pRingMsg->Ballot.Type == MSG_BALLOT_ACTIVE)? "Active": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_PASSIVE)? "Passive": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_LOOPBACK)? "Loopback": "Unkown");	
						obring_ballot_back(pFrame, RxLport, &TempBallotId);	
					}
					break;

	            	case MSG_BALLOT_LOOPBACK:
					/* Forward or loopback ballot packet */
					if(pRingState->PortState[RxPeerLportIndex].NeighborValid == HAL_TRUE) {
						OB_DEBUG(DBG_OBRING, ", [Port%d Forward Ballot-%s]\r\n", RxPeerLport,
							(pRingMsg->Ballot.Type == MSG_BALLOT_ACTIVE)? "Active": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_PASSIVE)? "Passive": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_LOOPBACK)? "Loopback": "Unkown");	
						obring_ballot_forward(pFrame, RxPeerLport, &pRingMsg->Ballot.Id);
					}
					break;
#else	
	            	case MSG_BALLOT_ACTIVE:
					if(memcmp(&DevSelfBallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId)) < 0) {
						memcpy(&TempBallotId, &DevSelfBallotId, sizeof(tBallotId));
					} else {
						memcpy(&TempBallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId));
					}
					/* Forward or loopback ballot packet */
					if(pRingState->PortState[RxPeerLportIndex].NeighborValid == HAL_TRUE) {
						OB_DEBUG(DBG_OBRING, ", [Port%d Forward Ballot-%s]", RxPeerLport,
							(pRingMsg->Ballot.Type == MSG_BALLOT_ACTIVE)? "Active": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_CHAIN_BACK)? "ChainBack": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_RING_BACK)? "RingBack": "Unkown");
						obring_ballot_forward(pFrame, RxPeerLport, &TempBallotId);
					} else {
						OB_DEBUG(DBG_OBRING, ", [Port%d Loopback Ballot-%s]", RxLport,
							(pRingMsg->Ballot.Type == MSG_BALLOT_ACTIVE)? "Active": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_CHAIN_BACK)? "ChainBack": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_RING_BACK)? "RingBack": "Unkown");
						obring_ballot_chain_back(pFrame, RxLport, &TempBallotId);	
					}
					OB_DEBUG(DBG_OBRING, "\r\n"); 
					break;

					case MSG_BALLOT_CHAIN_BACK:
					/* Update the Port's BallotId */
					memcpy(&pRingState->PortState[BallotLportIndex].BallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId));

					/* Forward the frame */
					if(pRingState->PortState[RxPeerLportIndex].NeighborValid == HAL_TRUE) {
						OB_DEBUG(DBG_OBRING, ", [Port%d Forward Ballot-%s]\r\n", RxPeerLport,
							(pRingMsg->Ballot.Type == MSG_BALLOT_ACTIVE)? "Active": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_CHAIN_BACK)? "ChainBack": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_RING_BACK)? "RingBack": "Unkown");
						obring_ballot_forward(pFrame, RxPeerLport, &pRingMsg->Ballot.Id);
					}
					break;

					case MSG_BALLOT_RING_BACK:
					/* Update the Port's BallotId */
					memcpy(&pRingState->PortState[BallotLportIndex].BallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId));
					/* Forward the frame */
					if(pRingState->PortState[RxPeerLportIndex].NeighborValid == HAL_TRUE) {
						OB_DEBUG(DBG_OBRING, ", [Port%d Forward Ballot-%s]\r\n", RxPeerLport,
							(pRingMsg->Ballot.Type == MSG_BALLOT_ACTIVE)? "Active": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_CHAIN_BACK)? "ChainBack": \
							(pRingMsg->Ballot.Type == MSG_BALLOT_RING_BACK)? "RingBack": "Unkown");
						obring_ballot_forward(pFrame, RxPeerLport, &pRingMsg->Ballot.Id);
					}
					
					if(memcmp(&DevSelfBallotId, &pRingMsg->Ballot.Id, sizeof(tBallotId)) == 0) {
						/* Master node */
						obring_ballot_master_handle(pRingConfig, HAL_TRUE);
					} else {
						/* Transit node */
						obring_ballot_transit_handle(pRingConfig, HAL_TRUE);
					}
					break;
#endif					
					default:
					break;
				}
			}
		}
		break;

		case PACKET_HELLO:
		if((pRingConfig->ucEnable == 0x01) && (pRingState->PortState[RxLportIndex].RunState == PORT_BALLOT_FINISH)) {
			HelloSeq = pFrame->obr_hello_seq[0];
			HelloSeq = (HelloSeq << 8) | (unsigned short)(pFrame->obr_hello_seq[1]);
			OB_DEBUG(DBG_OBRING, "[Port%d Rx Hello %d]", RxLport, HelloSeq);
			
			BlockLineNum = pFrame->obr_msg.Hello.BlockLineNum[0];
			BlockLineNum = (BlockLineNum << 8) | pFrame->obr_msg.Hello.BlockLineNum[1];
			if((pFrame->obr_msg.Hello.TxPortStpState == BLOCKING) || (pRingState->PortState[RxLportIndex].StpState == BLOCKING)) {
				BlockLineNum++;
			} 
			
			if(pRingState->NodeType == NODE_TYPE_MASTER) {
				if((memcmp(pFrame->src_mac, DevMac, 6) == 0) && (HelloSeq == pRingState->HelloSeq)) {
					CurrSysTick = (unsigned int)(xTaskGetTickCount());
					PrevSysTick = cli_ntohl(*(unsigned int *)(&pFrame->obr_msg.Hello.Tick[0]));
					pRingState->HelloElapsed = CurrSysTick - PrevSysTick;
		
					obring_timer_start(&RingTimer[RingIndex].Fail, FailTime, T_SEC);
				
					if(BlockLineNum > 1) {
						if(pRingState->NodeState == NODE_STATE_FAIL) {
							Set_RingLED_BlinkMode(BLINK_10HZ);
							pRingState->NodeState = NODE_STATE_COMPLETE;
							pRingState->RingState = RING_HEALTH;
						}
						hal_swif_port_set_stp_state(pRingConfig->ucPrimaryPort, FORWARDING);	
						hal_swif_port_set_stp_state(pRingConfig->ucSecondaryPort, BLOCKING);
						pRingState->PortState[INDEX_PRIMARY].StpState = FORWARDING;
						pRingState->PortState[INDEX_SECONDARY].StpState = BLOCKING;
						
						MsgComplete.Type = MSG_COMPLETE_WITH_FLUSH_FDB;
						OB_DEBUG(DBG_OBRING, ", [BlockLineNum = %d, Port%d Tx Complete-Flush]", BlockLineNum, pRingConfig->ucPrimaryPort);
						obring_complete_send(pRingConfig, pRingConfig->ucPrimaryPort, &MsgComplete);
						obring_mac_flush();
					} else if(BlockLineNum == 1) {
						if(pRingState->NodeState == NODE_STATE_FAIL) {
							Set_RingLED_BlinkMode(BLINK_10HZ);
							pRingState->NodeState = NODE_STATE_COMPLETE;
							pRingState->RingState = RING_HEALTH;
							MsgComplete.Type = MSG_COMPLETE_UPDATE_NODE;
							OB_DEBUG(DBG_OBRING, ", [Port%d Tx Complete-Update]", pRingConfig->ucPrimaryPort);
							obring_complete_send(pRingConfig, pRingConfig->ucPrimaryPort, &MsgComplete);
						}
					} else {
						pRingState->StormCount++;
						if(pRingState->NodeState == NODE_STATE_FAIL) {
							Set_RingLED_BlinkMode(BLINK_10HZ);
							pRingState->NodeState = NODE_STATE_COMPLETE;
							pRingState->RingState = RING_HEALTH;
						}				
						hal_swif_port_set_stp_state(pRingConfig->ucPrimaryPort, FORWARDING);	
						hal_swif_port_set_stp_state(pRingConfig->ucSecondaryPort, BLOCKING);
						pRingState->PortState[INDEX_PRIMARY].StpState = FORWARDING;
						pRingState->PortState[INDEX_SECONDARY].StpState = BLOCKING;
						MsgComplete.Type = MSG_COMPLETE_WITH_FLUSH_FDB;
						OB_DEBUG(DBG_OBRING, ", [Port%d Tx Complete-Flush]", pRingConfig->ucPrimaryPort);
						obring_complete_send(pRingConfig, pRingConfig->ucPrimaryPort, &MsgComplete);
						obring_mac_flush();
					}
				}
			} else {
				/* Transmit transparently for the hello frame */
				OB_DEBUG(DBG_OBRING, ", [Port%d Forward Hello]", RxPeerLport);
				if(pRingState->PortState[RxPeerLportIndex].RunState == PORT_BALLOT_FINISH) {
					obring_hello_forward(pRingConfig, pFrame, RxPeerLport);
				}
				
				if(pFrame->obr_msg.Hello.RingState == RING_HEALTH) {
					if(pRingState->RingState != RING_HEALTH) {
						pRingState->RingState = RING_HEALTH;
						Set_RingLED_BlinkMode(BLINK_10HZ);
					}
					if(pFrame->obr_msg.Hello.MasterSecondaryStp == BLOCKING) {
						if(memcmp(pRingState->PortState[RxPeerLportIndex].NeighborMac, pFrame->src_mac, MAC_LEN) == 0) {
							if(pRingState->PortState[RxPeerLportIndex].StpState == FORWARDING) {
								hal_swif_port_set_stp_state(RxPeerLport, BLOCKING);
								pRingState->PortState[RxPeerLportIndex].StpState = BLOCKING;
							}
							pRingState->NodeState = NODE_STATE_PRE_FORWARDING;
						}
					}
				} else {
					if(pRingState->RingState == RING_HEALTH) {
						pRingState->RingState = RING_FAULT;
						Set_RingLED_BlinkMode(BLINK_2HZ);
					}
				}
				
				if(pRingState->RingState == RING_HEALTH) {
					obring_timer_start(&RingTimer[RingIndex].Fail, FailTime, T_SEC);
				} 
			}
			OB_DEBUG(DBG_OBRING, "\r\n"); 
		}
		break;

		case PACKET_COMPLETE:
		if(pRingConfig->ucEnable == 0x01) {
			if(pRingState->NodeType == NODE_TYPE_TRANSIT) {
				OB_DEBUG(DBG_OBRING, "[Port%d Rx Complete-%s]", RxLport,
					(pFrame->obr_msg.Complete.Type == MSG_COMPLETE_UPDATE_NODE)? "Update" : \
					(pFrame->obr_msg.Complete.Type == MSG_COMPLETE_WITH_FLUSH_FDB)? "Flush" : "Unkown");
				
				OB_DEBUG(DBG_OBRING, ", [Port%d Forward Complete-%s]", RxPeerLport,
					(pFrame->obr_msg.Complete.Type == MSG_COMPLETE_UPDATE_NODE)? "Update" : \
					(pFrame->obr_msg.Complete.Type == MSG_COMPLETE_WITH_FLUSH_FDB)? "Flush" : "Unkown");
				
				if(RxLport == pRingConfig->ucPrimaryPort) {
					obring_complete_forward(pFrame, pRingConfig->ucSecondaryPort);
				} else {
					obring_complete_forward(pFrame, pRingConfig->ucPrimaryPort);
				}

				Set_RingLED_BlinkMode(BLINK_10HZ);
				pRingState->RingState = RING_HEALTH;
				
				switch(pFrame->obr_msg.Complete.Type) {
					case MSG_COMPLETE_UPDATE_NODE:
					if((pRingState->PortState[INDEX_PRIMARY].StpState == BLOCKING) || (pRingState->PortState[INDEX_SECONDARY].StpState == BLOCKING))
						pRingState->NodeState = NODE_STATE_PRE_FORWARDING;
					else
						pRingState->NodeState = NODE_STATE_LINK_UP;
					break;

					case MSG_COMPLETE_WITH_FLUSH_FDB:
					if(pRingState->PortState[RxLportIndex].StpState == BLOCKING) {
						hal_swif_port_set_stp_state(RxLport, FORWARDING);
						pRingState->PortState[RxLportIndex].StpState = FORWARDING;
					}

					if(memcmp(pRingState->PortState[RxPeerLportIndex].NeighborMac, pFrame->src_mac, MAC_LEN) == 0) {
						if(pRingState->PortState[RxPeerLportIndex].StpState == FORWARDING) {
							hal_swif_port_set_stp_state(RxPeerLport, BLOCKING);
							pRingState->PortState[RxPeerLportIndex].StpState = BLOCKING;
						}
						pRingState->NodeState = NODE_STATE_PRE_FORWARDING;
					} else {
						if(pRingState->PortState[RxPeerLportIndex].StpState == BLOCKING) {
							hal_swif_port_set_stp_state(RxPeerLport, FORWARDING);
							pRingState->PortState[RxPeerLportIndex].StpState = FORWARDING;
						}
						pRingState->NodeState = NODE_STATE_LINK_UP;
					}

					obring_mac_flush();
					break;

					default:
					break;
				}
				OB_DEBUG(DBG_OBRING, "\r\n"); 
			}
		}
		break;	

		case PACKET_LINKDOWN:
		if(pRingConfig->ucEnable == 0x01) {
			OB_DEBUG(DBG_OBRING, "[Port%d Rx LinkDown-%s]", RxLport, 
					(pFrame->obr_msg.LinkDown.Type == MSG_LINKDOWN_REQ_UPDATE_NODE)? "Update" : \
					(pFrame->obr_msg.LinkDown.Type == MSG_LINKDOWN_REQ_FLUSH_FDB)? "Flush" : "Unkown");
			
			if(pRingState->RingState == RING_HEALTH) {
				if(pRingState->NodeType == NODE_TYPE_MASTER) {
					if(memcmp(pFrame->obr_msg.LinkDown.ExtNeighborMac, DevMac, MAC_LEN) == 0) {
						/* do nothing */
					} else {
						obring_timer_stop(&RingTimer[RingIndex].Fail);
						pRingState->RingState = RING_FAULT;
						pRingState->NodeState = NODE_STATE_FAIL;
						Set_RingLED_BlinkMode(BLINK_2HZ);

						if(pRingState->PortState[RxLportIndex].StpState == BLOCKING) {
							hal_swif_port_set_stp_state(RxLport, FORWARDING);
							pRingState->PortState[RxLportIndex].StpState = FORWARDING;
						}
						if(pRingState->PortState[RxPeerLportIndex].StpState == BLOCKING) {
							hal_swif_port_set_stp_state(RxPeerLport, FORWARDING);
							pRingState->PortState[RxPeerLportIndex].StpState = FORWARDING;
						}
						
						switch(pFrame->obr_msg.LinkDown.Type) {
							case MSG_LINKDOWN_REQ_UPDATE_NODE:
							/* do nothing */
							break;

							case MSG_LINKDOWN_REQ_FLUSH_FDB:
							obring_mac_flush();
							pRingState->SwitchTimes++;
							break;

							default:
							break;
						}
					}
				} else {
					obring_timer_stop(&RingTimer[RingIndex].Fail);
					pRingState->RingState = RING_FAULT;
					Set_RingLED_BlinkMode(BLINK_2HZ);
					
					OB_DEBUG(DBG_OBRING, ", [Port%d Forward LinkDown-%s]", RxPeerLport,
						(pFrame->obr_msg.LinkDown.Type == MSG_LINKDOWN_REQ_UPDATE_NODE)? "Update" : \
						(pFrame->obr_msg.LinkDown.Type == MSG_LINKDOWN_REQ_FLUSH_FDB)? "Flush" : "Unkown");

					if(pRingState->PortState[RxPeerLportIndex].NeighborValid == HAL_TRUE) {
						obring_linkdown_forward(pFrame, RxPeerLport);
					}
					
					if(pRingState->PortState[RxLportIndex].StpState == BLOCKING) {
						hal_swif_port_set_stp_state(RxLport, FORWARDING);
						pRingState->PortState[RxLportIndex].StpState = FORWARDING;
					}
					if(pRingState->PortState[RxPeerLportIndex].StpState == BLOCKING) {
						hal_swif_port_set_stp_state(RxPeerLport, FORWARDING);
						pRingState->PortState[RxPeerLportIndex].StpState = FORWARDING;
					}
					switch(pFrame->obr_msg.LinkDown.Type) {
						case MSG_LINKDOWN_REQ_UPDATE_NODE:
						/* do nothing */
						break;

						case MSG_LINKDOWN_REQ_FLUSH_FDB:
						obring_mac_flush();
						pRingState->SwitchTimes++;
						break;

						default:
						break;
					}
				}
			}
			OB_DEBUG(DBG_OBRING, "\r\n"); 
		}
		break;

		case PACKET_COMMON: 
		if(pRingConfig->ucEnable == 0x01) {
			if(pRingState->NodeType == NODE_TYPE_TRANSIT) {
				OB_DEBUG(DBG_OBRING, "[Port%d Rx Common-%s]", RxLport, 
					(pFrame->obr_msg.Common.Type == MSG_COMMON_UPDATE_NODE)? "Update" : \
					(pFrame->obr_msg.Common.Type == MSG_COMMON_WITH_FLUSH_FDB)? "Flush" : "Unkown");
				
				pRingState->RingState = RING_FAULT;
				Set_RingLED_BlinkMode(BLINK_2HZ);
				
				if(pRingState->PortState[RxPeerLportIndex].NeighborValid == HAL_TRUE) {
					OB_DEBUG(DBG_OBRING, ", [Port%d Forward Common-%s]", RxPeerLport, 
						(pFrame->obr_msg.Common.Type == MSG_COMMON_UPDATE_NODE)? "Update" : \
						(pFrame->obr_msg.Common.Type == MSG_COMMON_WITH_FLUSH_FDB)? "Flush" : "Unkown");
					obring_common_forward(pFrame, RxPeerLport);
				} 
				
				switch(pFrame->obr_msg.Common.Type) {
					case MSG_COMMON_UPDATE_NODE:
					
					break;
#if 0
					case MSG_COMMON_WITH_FORWARDING:
					if(pRingState->PortState[RxLportIndex].StpState == BLOCKING) {
						hal_swif_port_set_stp_state(RxLport, FORWARDING);
						pRingState->PortState[RxLportIndex].StpState = FORWARDING;
					}

					if(pRingState->PortState[RxPeerLportIndex].NeighborValid == HAL_TRUE) {
						if(pRingState->PortState[RxPeerLportIndex].StpState == BLOCKING) {
							hal_swif_port_set_stp_state(RxPeerLport, FORWARDING);
							pRingState->PortState[RxPeerLportIndex].StpState = FORWARDING;
						}
					}					
					break;
#endif					
					case MSG_COMMON_WITH_FLUSH_FDB:
					if(pRingState->PortState[RxLportIndex].StpState == BLOCKING) {
						hal_swif_port_set_stp_state(RxLport, FORWARDING);
						pRingState->PortState[RxLportIndex].StpState = FORWARDING;
					}
					
					if(pRingState->PortState[RxPeerLportIndex].NeighborValid == HAL_TRUE) {
						if(pRingState->PortState[RxPeerLportIndex].StpState == BLOCKING) {
							hal_swif_port_set_stp_state(RxPeerLport, FORWARDING);
							pRingState->PortState[RxPeerLportIndex].StpState = FORWARDING;
						}
					}				
					obring_mac_flush();
					break;

					default:
					break;
				}
			}
			OB_DEBUG(DBG_OBRING, "\r\n"); 
		}
		break;

		case PACKET_COMMAND:
		switch(pFrame->obr_msg.Command.Code) {
			case CMD_GET_NODE_REQ:
			OB_DEBUG(DBG_OBRING, "[Port%d Rx Command-Get-Node-Request]", RxLport);
			if(pFrame->obr_msg.Command.Action.ReqGetNode.DstNodeIndex == \
				pFrame->obr_msg.Command.Action.ReqGetNode.SrcNodeIndex + pFrame->obr_msg.Command.Action.ReqGetNode.NodeIndexInc) {
				OB_DEBUG(DBG_OBRING, ", [Port%d Tx Command-Get-Node-Response]", RxLport);
				/* Prepare message body and send */
				obring_prepare_tx_frame(pRingConfig, RxLport, PACKET_COMMAND);
				pRingTxMsg->Command.Code = CMD_GET_NODE_RSP;
				memcpy(pRingTxMsg->Command.Action.RspGetNode.DstMac, pFrame->src_mac, MAC_LEN);
				pRingTxMsg->Command.Action.RspGetNode.ReqestId = pFrame->obr_msg.Command.Action.ReqGetNode.ReqestId;
				pRingTxMsg->Command.Action.RspGetNode.RetCode= 0;
				memcpy(pRingTxMsg->Command.Action.RspGetNode.NodeMac, DevMac, MAC_LEN);
				pRingTxMsg->Command.Action.RspGetNode.NodeVersion[0] = (unsigned char)((FIRMWARE_VERSION & 0xF000)>>12);
				pRingTxMsg->Command.Action.RspGetNode.NodeVersion[1] = (unsigned char)((FIRMWARE_VERSION & 0x0F00)>>8);
				pRingTxMsg->Command.Action.RspGetNode.NodeVersion[2] = (unsigned char)((FIRMWARE_VERSION & 0x00F0)>>4);
				pRingTxMsg->Command.Action.RspGetNode.NodeVersion[3] = (unsigned char)(FIRMWARE_VERSION & 0x000F);
				pRingTxMsg->Command.Action.RspGetNode.NodeType = pRingState->NodeType;
				pRingTxMsg->Command.Action.RspGetNode.RingEnable = pRingConfig->ucEnable;
				pRingTxMsg->Command.Action.RspGetNode.RingState = pRingState->RingState;
				pRingTxMsg->Command.Action.RspGetNode.WestPortNo = RxLport;
				pRingTxMsg->Command.Action.RspGetNode.WestPortLink = pRingState->PortState[RxLportIndex].LinkState;
				pRingTxMsg->Command.Action.RspGetNode.WestPortStp = pRingState->PortState[RxLportIndex].StpState;
				pRingTxMsg->Command.Action.RspGetNode.EastPortNo = RxPeerLport;
				pRingTxMsg->Command.Action.RspGetNode.EastPortLink = pRingState->PortState[RxPeerLportIndex].LinkState;
				pRingTxMsg->Command.Action.RspGetNode.EastPortStp = pRingState->PortState[RxPeerLportIndex].StpState;
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_DOWN)
					pRingTxMsg->Command.Action.RspGetNode.LastNodeFlag = CMD_RET_LAST_NODE_CHAIN;
				else if(memcmp(pFrame->src_mac, DevMac, MAC_LEN) == 0)
					pRingTxMsg->Command.Action.RspGetNode.LastNodeFlag = CMD_RET_LAST_NODE_RING;
				else
					pRingTxMsg->Command.Action.RspGetNode.LastNodeFlag = CMD_RET_NODE_NOT_LAST;
				/* Send */
				EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
			} else {
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_UP) {
					OB_DEBUG(DBG_OBRING, ", [Port%d Forward Command-Get-Node-Request]", RxPeerLport);
					obring_command_forward(pFrame, RxPeerLport);
				}
			}
			break;


			case CMD_GET_NODE_RSP:
			OB_DEBUG(DBG_OBRING, "[Port%d Rx Command-Get-Node-Response]", RxLport);
			if(memcmp(pFrame->obr_msg.Command.Action.RspGetNode.DstMac, DevMac, MAC_LEN) == 0) {
				if(ObrMsgRxQueue != NULL)
					xQueueSendToBack(ObrMsgRxQueue, &pFrame->obr_msg, 1000);
			} else {
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_UP) {
					OB_DEBUG(DBG_OBRING, ", [Port%d Forward Command-Ring-Disable-Response]", RxPeerLport);
					obring_command_forward(pFrame, RxPeerLport);
				}
			}				
			break;
	
			case CMD_RING_DISABLE_REQ:
			OB_DEBUG(DBG_OBRING, "[Port%d Rx Command-Ring-Disable-Request]", RxLport);
			if(pFrame->obr_msg.Command.Action.ReqRingDisable.DstNodeIndex == \
				pFrame->obr_msg.Command.Action.ReqRingDisable.SrcNodeIndex + pFrame->obr_msg.Command.Action.ReqRingDisable.NodeIndexInc) {
				OB_DEBUG(DBG_OBRING, ", [Port%d Tx Command-Ring-Disable-Response]", RxLport);
				/* Prepare message body and send */
				obring_prepare_tx_frame(pRingConfig, RxLport, PACKET_COMMAND);
				pRingTxMsg->Command.Code = CMD_RING_DISABLE_RSP;
				memcpy(pRingTxMsg->Command.Action.RspRingDisable.DstMac, pFrame->src_mac, MAC_LEN);
				pRingTxMsg->Command.Action.RspRingDisable.ReqestId = pFrame->obr_msg.Command.Action.ReqRingDisable.ReqestId;
				
				memcpy(pRingTxMsg->Command.Action.RspRingDisable.NodeMac, DevMac, MAC_LEN);
				
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_DOWN)
					pRingTxMsg->Command.Action.RspRingDisable.LastNodeFlag = CMD_RET_LAST_NODE_CHAIN;
				else if(memcmp(pFrame->src_mac, DevMac, MAC_LEN) == 0)
					pRingTxMsg->Command.Action.RspRingDisable.LastNodeFlag = CMD_RET_LAST_NODE_RING;
				else
					pRingTxMsg->Command.Action.RspRingDisable.LastNodeFlag = CMD_RET_NODE_NOT_LAST;

				if(obring_disable(RingIndex) == 0) {
					pRingTxMsg->Command.Action.RspRingDisable.RetCode = 0;
				} else {
					pRingTxMsg->Command.Action.RspRingDisable.RetCode = 1;
				}
				
				/* Send */
				EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
			} else {
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_UP) {
					OB_DEBUG(DBG_OBRING, ", [Port%d Forward Command-Ring-Disable-Request]", RxPeerLport);
					obring_command_forward(pFrame, RxPeerLport);
				}
			}			
			break;

			case CMD_RING_DISABLE_RSP:
			OB_DEBUG(DBG_OBRING, "[Port%d Rx Command-Ring-Disable-Response]", RxLport);
			if(memcmp(pFrame->obr_msg.Command.Action.RspRingDisable.DstMac, DevMac, MAC_LEN) == 0) {
				if(ObrMsgRxQueue != NULL)
					xQueueSendToBack(ObrMsgRxQueue, &pFrame->obr_msg, 1000);
			} else {
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_UP) {
					OB_DEBUG(DBG_OBRING, ", [Port%d Forward Command-Ring-Disable-Response]", RxPeerLport);
					obring_command_forward(pFrame, RxPeerLport);
				}
			}
			break;

			case CMD_RING_ENABLE_REQ:
			OB_DEBUG(DBG_OBRING, "[Port%d Rx Command-Ring-Enable-Request]", RxLport);
			if(pFrame->obr_msg.Command.Action.ReqRingEnable.DstNodeIndex == \
				pFrame->obr_msg.Command.Action.ReqRingEnable.SrcNodeIndex + pFrame->obr_msg.Command.Action.ReqRingEnable.NodeIndexInc) {
				OB_DEBUG(DBG_OBRING, ", [Port%d Tx Command-Ring-Enable-Response]", RxLport);
				/* Prepare message body and send */
				obring_prepare_tx_frame(pRingConfig, RxLport, PACKET_COMMAND);
				pRingTxMsg->Command.Code = CMD_RING_ENABLE_RSP;
				memcpy(pRingTxMsg->Command.Action.RspRingEnable.DstMac, pFrame->src_mac, MAC_LEN);
				pRingTxMsg->Command.Action.RspRingEnable.ReqestId = pFrame->obr_msg.Command.Action.ReqRingEnable.ReqestId;	
				memcpy(pRingTxMsg->Command.Action.RspRingEnable.NodeMac, DevMac, MAC_LEN);
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_DOWN)
					pRingTxMsg->Command.Action.RspRingEnable.LastNodeFlag = CMD_RET_LAST_NODE_CHAIN;
				else if(memcmp(pFrame->src_mac, DevMac, MAC_LEN) == 0)
					pRingTxMsg->Command.Action.RspRingEnable.LastNodeFlag = CMD_RET_LAST_NODE_RING;
				else
					pRingTxMsg->Command.Action.RspRingEnable.LastNodeFlag = CMD_RET_NODE_NOT_LAST;

				if(obring_enable(RingIndex) == 0) {
					pRingTxMsg->Command.Action.RspRingEnable.RetCode = 0;
				} else {
					pRingTxMsg->Command.Action.RspRingEnable.RetCode = 1;
				}
				
				/* Send */
				EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
			} else {
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_UP) {
					OB_DEBUG(DBG_OBRING, ", [Port%d Forward Command-Ring-Enable-Request]", RxPeerLport);
					obring_command_forward(pFrame, RxPeerLport);
				}
			}
			break;

			case CMD_RING_ENABLE_RSP:
			OB_DEBUG(DBG_OBRING, "[Port%d Rx Command-Ring-Enable-Response]", RxLport);
			if(memcmp(pFrame->obr_msg.Command.Action.RspRingEnable.DstMac, DevMac, MAC_LEN) == 0) {
				if(ObrMsgRxQueue != NULL)
					xQueueSendToBack(ObrMsgRxQueue, &pFrame->obr_msg, 1000);
			} else {
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_UP) {
					OB_DEBUG(DBG_OBRING, ", [Port%d Forward Command-Ring-Enable-Response]", RxPeerLport);
					obring_command_forward(pFrame, RxPeerLport);
				}
			}
			break;

			case CMD_RING_REBOOT_REQ:
			OB_DEBUG(DBG_OBRING, "[Port%d Rx Command-Ring-Reboot-Request]", RxLport);
			if(pFrame->obr_msg.Command.Action.ReqRingReboot.DstNodeIndex == \
				pFrame->obr_msg.Command.Action.ReqRingReboot.SrcNodeIndex + pFrame->obr_msg.Command.Action.ReqRingReboot.NodeIndexInc) {
				OB_DEBUG(DBG_OBRING, ", [Port%d Tx Command-Ring-Reboot-Response]", RxLport);
				/* Prepare message body and send */
				obring_prepare_tx_frame(pRingConfig, RxLport, PACKET_COMMAND);
				pRingTxMsg->Command.Code = CMD_RING_REBOOT_RSP;
				memcpy(pRingTxMsg->Command.Action.RspRingReboot.DstMac, pFrame->src_mac, MAC_LEN);
				pRingTxMsg->Command.Action.RspRingReboot.ReqestId = pFrame->obr_msg.Command.Action.ReqRingReboot.ReqestId;
				memcpy(pRingTxMsg->Command.Action.RspRingReboot.NodeMac, DevMac, MAC_LEN);
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_DOWN)
					pRingTxMsg->Command.Action.RspRingReboot.LastNodeFlag = CMD_RET_LAST_NODE_CHAIN;
				else if(memcmp(pFrame->src_mac, DevMac, MAC_LEN) == 0)
					pRingTxMsg->Command.Action.RspRingReboot.LastNodeFlag = CMD_RET_LAST_NODE_RING;
				else
					pRingTxMsg->Command.Action.RspRingReboot.LastNodeFlag = CMD_RET_NODE_NOT_LAST;

				if(obring_reboot(RingIndex) == 0) {
					pRingTxMsg->Command.Action.RspRingReboot.RetCode = 0;
				} else {
					pRingTxMsg->Command.Action.RspRingReboot.RetCode = 1;
				}
				
				/* Send */
				EthSend((u8 *)&RingTxBuf[0], MAX_RING_MSG_SIZE);
			} else {
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_UP) {
					OB_DEBUG(DBG_OBRING, ", [Port%d Forward Command-Ring-Reboot-Request]", RxPeerLport);
					obring_command_forward(pFrame, RxPeerLport);
				}
			}
			break;

			case CMD_RING_REBOOT_RSP:
			OB_DEBUG(DBG_OBRING, "[Port%d Rx Command-Ring-Reboot-Response]", RxLport);
			if(memcmp(pFrame->obr_msg.Command.Action.RspRingReboot.DstMac, DevMac, MAC_LEN) == 0) {
				if(ObrMsgRxQueue != NULL)
					xQueueSendToBack(ObrMsgRxQueue, &pFrame->obr_msg, 1000);
			} else {
				if(pRingState->PortState[RxPeerLportIndex].LinkState == LINK_UP) {
					OB_DEBUG(DBG_OBRING, ", [Port%d Forward Command-Ring-Reboot-Response]", RxPeerLport);
					obring_command_forward(pFrame, RxPeerLport);
				}
			}
			break;
			
			default:
			break;
		}
		OB_DEBUG(DBG_OBRING, "\r\n"); 
		break;
		
		default:
		break;
	}
}

/**************************************************************************
  * @brief  OBRing frame receive
  * @param  *rxBuf, rxLen
  * @retval none
  *************************************************************************/
void obring_frame_receive(unsigned char *rxBuf, u16 rxLen)
{
	if(rxLen <= MAX_RING_MSG_SIZE) {
		RingTxMsg.BufLen = rxLen;
		memcpy((u8 *)&(RingTxMsg.Buf[0]), rxBuf, rxLen);

		if(RingMsgQueue != NULL) {
			while(xQueueSendToBack(RingMsgQueue, &RingTxMsg, portMAX_DELAY) != pdTRUE) {
				//vTaskDelay(10);
			}
		}
	}
}

/**************************************************************************
  * @brief  OBRing port link state change handle
  * @param  usRingId, ucPort, ucLinkValue
  * @retval none
  *************************************************************************/
void obring_link_change_handle(unsigned char RingIndex, unsigned char Lport, HAL_PORT_LINK_STATE ucLinkValue)
{
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigRec *pRingConfig = &(pRingInfo->RingConfig[RingIndex]);
	tRingState *pRingState = &(pRingInfo->DevState[RingIndex]);
	unsigned short HelloTime, FailTime, BallotTime, AuthTime;
	unsigned short LportIndex, LportPeerIndex;
	unsigned char LportPeerPort;
	tRMsgLinkDown MsgLinkDown;
    extern unsigned char DevMac[];

	LportIndex = obring_get_port_idx_by_port(pRingConfig, Lport);
	LportPeerIndex = (LportIndex == INDEX_PRIMARY)? INDEX_SECONDARY: INDEX_PRIMARY;
	LportPeerPort = (LportPeerIndex == INDEX_PRIMARY)? pRingConfig->ucPrimaryPort : pRingConfig->ucSecondaryPort;
	AuthTime = pRingConfig->usAuthTime[0];
	AuthTime = (AuthTime << 8) | (unsigned short)(pRingConfig->usAuthTime[1]);		
	BallotTime = pRingConfig->usBallotTime[0];
	BallotTime = (BallotTime << 8) | (unsigned short)(pRingConfig->usBallotTime[1]);
	
	/*************************************************************************************************
											LINK_UP
	 *************************************************************************************************/
	if(ucLinkValue == LINK_UP) {
		if(Lport == pRingConfig->ucPrimaryPort) {
			if(pRingConfig->ucEnable == 0x01) {
				pRingState->PortState[INDEX_PRIMARY].RunState = PORT_AUTH_REQ;
			} else {
				pRingState->PortState[INDEX_PRIMARY].RunState = PORT_IDLE;
			}
	    	obring_timer_start(&(RingTimer[RingIndex].AuthP), AuthTime, T_SEC);
		} else {
			if(pRingConfig->ucEnable == 0x01) {
				pRingState->PortState[INDEX_SECONDARY].RunState = PORT_AUTH_REQ;
			} else {
				pRingState->PortState[INDEX_SECONDARY].RunState = PORT_IDLE;
			}
	    	obring_timer_start(&(RingTimer[RingIndex].AuthS), AuthTime, T_SEC);
		}
      
	/*************************************************************************************************
											LINK_DOWN
	 *************************************************************************************************/		
	} else {
		if(pRingConfig->ucEnable == 0x01) {
						
			if(pRingState->RingState == RING_HEALTH) {
				if(pRingState->NodeType == NODE_TYPE_MASTER) {
					obring_timer_stop(&RingTimer[RingIndex].Fail);	
					pRingState->NodeState = NODE_STATE_FAIL;
					pRingState->RingState = RING_FAULT;
					Set_RingLED_BlinkMode(BLINK_2HZ);
				
					if(pRingState->PortState[LportIndex].StpState == BLOCKING) {
						/* do nothing */
					} else {
						hal_swif_port_set_stp_state(Lport, BLOCKING);
						pRingState->PortState[LportIndex].StpState = BLOCKING;
						if(pRingState->PortState[LportPeerIndex].StpState == BLOCKING) {
							hal_swif_port_set_stp_state(LportPeerPort, FORWARDING);
							pRingState->PortState[LportPeerIndex].StpState = FORWARDING;
						}
						obring_mac_flush();
						pRingState->SwitchTimes++;
					}
					
					memset(pRingState->PortState[LportIndex].NeighborMac, 0, MAC_LEN);
					pRingState->PortState[LportIndex].NeighborPortNo = 0;
					pRingState->PortState[LportIndex].NeighborValid = HAL_FALSE;
					
				} else {
					obring_timer_stop(&RingTimer[RingIndex].Fail);	
					pRingState->NodeState = NODE_STATE_LINK_DOWN;
					pRingState->RingState = RING_FAULT;
					Set_RingLED_BlinkMode(BLINK_2HZ);
					
					memcpy(MsgLinkDown.ExtNeighborMac, pRingState->PortState[LportIndex].NeighborMac, MAC_LEN);
					MsgLinkDown.ExtNeighborPortNo = pRingState->PortState[LportIndex].NeighborPortNo;
					memset(pRingState->PortState[LportIndex].NeighborMac, 0, MAC_LEN);
					pRingState->PortState[LportIndex].NeighborPortNo = 0;
					pRingState->PortState[LportIndex].NeighborValid = HAL_FALSE;
					
					if(pRingState->PortState[LportIndex].StpState == BLOCKING) {
						MsgLinkDown.Type = MSG_LINKDOWN_REQ_UPDATE_NODE;
						OB_DEBUG(DBG_OBRING, "[Port%d Tx LinkDown-Update]\r\n", LportPeerPort);
						obring_linkdown_send(pRingConfig, LportPeerPort, &MsgLinkDown);
						if(pRingState->PortState[LportPeerIndex].StpState == BLOCKING) {
							hal_swif_port_set_stp_state(LportPeerPort, FORWARDING);
							pRingState->PortState[LportPeerIndex].StpState = FORWARDING;
						}
					} else {
						MsgLinkDown.Type = MSG_LINKDOWN_REQ_FLUSH_FDB;
						OB_DEBUG(DBG_OBRING, "[Port%d Tx LinkDown-Flush]\r\n", LportPeerPort);
						obring_linkdown_send(pRingConfig, LportPeerPort, &MsgLinkDown);
						hal_swif_port_set_stp_state(Lport, BLOCKING);
						pRingState->PortState[LportIndex].StpState = BLOCKING;
						if(pRingState->PortState[LportPeerIndex].StpState == BLOCKING) {
							hal_swif_port_set_stp_state(LportPeerPort, FORWARDING);
							pRingState->PortState[LportPeerIndex].StpState = FORWARDING;
						}						
						obring_mac_flush();
					}
				}
			} else {
				if(pRingState->NodeType == NODE_TYPE_TRANSIT) {
					pRingState->NodeState = NODE_STATE_LINK_DOWN;
				}

				if(pRingState->PortState[LportIndex].StpState == FORWARDING) {
					hal_swif_port_set_stp_state(Lport, BLOCKING);
					pRingState->PortState[LportIndex].StpState = BLOCKING;
				}

				memset(pRingState->PortState[LportIndex].NeighborMac, 0, MAC_LEN);
				pRingState->PortState[LportIndex].NeighborPortNo = 0;
				pRingState->PortState[LportIndex].NeighborValid = HAL_FALSE;

				if(pRingState->PortState[LportPeerIndex].NeighborValid == HAL_TRUE) {
					pRingState->PortState[LportPeerIndex].RunState = PORT_BALLOT_ACTIVE;
					OB_DEBUG(DBG_OBRING, "[Port%d Start BallotTimer]\r\n", Lport);
					if(Lport == pRingConfig->ucPrimaryPort)
						obring_timer_start(&(RingTimer[RingIndex].BallotS), BallotTime, T_SEC);
					else
						obring_timer_start(&(RingTimer[RingIndex].BallotP), BallotTime, T_SEC);
				}
			}
		} else {
			memset(pRingState->PortState[LportIndex].NeighborMac, 0, MAC_LEN);
			pRingState->PortState[LportIndex].NeighborPortNo = 0;
			pRingState->PortState[LportIndex].NeighborValid = HAL_FALSE;
		}
	}
}

/**************************************************************************
  * @brief  OBRing command send
  * @param  arg
  * @retval none
  *************************************************************************/
int obring_command_send(void *pCliEnv, unsigned char RingIndex, unsigned char TxLport, tRMsgCmd *pCmdRequest, tRMsgCmd *pCmdResponse)
{
	tRMsgCmd CmdResponse;
	tRingMsgBody *pRingMsg = (tRingMsgBody *)&RingTxBufTest[OFFSET_RING_DATA];
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigRec *pRingConfig;
	int ret = 0;
	extern void EthSend(unsigned char *, unsigned short);

	pRingConfig = &(pRingInfo->RingConfig[RingIndex]);	
	obring_prepare_tx_frame2(pRingConfig, &RingTxBufTest[0], TxLport, PACKET_COMMAND);
	memcpy(&pRingMsg->Command, pCmdRequest, sizeof(tRMsgCmd));
	EthSend((u8 *)&RingTxBufTest[0], MAX_RING_MSG_SIZE);

	if(ObrMsgRxQueue != NULL) { 
		while(1) {
			if(xQueueReceive(ObrMsgRxQueue, &CmdResponse, 3000) == pdTRUE) {
				if(CmdResponse.Code == CMD_GET_NODE_RSP) {
					if(CmdResponse.Action.RspGetNode.ReqestId != pCmdRequest->Action.ReqGetNode.ReqestId)
						continue;
					memcpy(pCmdResponse, &CmdResponse, sizeof(tRMsgCmd));
					break;
				} else if(CmdResponse.Code == CMD_RING_DISABLE_RSP) {
					if(CmdResponse.Action.RspRingDisable.ReqestId != pCmdRequest->Action.ReqRingDisable.ReqestId)
						continue;
					memcpy(pCmdResponse, &CmdResponse, sizeof(tRMsgCmd));
					break;
				} else if(CmdResponse.Code == CMD_RING_ENABLE_RSP) {
					if(CmdResponse.Action.RspRingEnable.ReqestId != pCmdRequest->Action.ReqRingEnable.ReqestId)
						continue;
					memcpy(pCmdResponse, &CmdResponse, sizeof(tRMsgCmd));
					break;
				} else if(CmdResponse.Code == CMD_RING_REBOOT_RSP) {
					if(CmdResponse.Action.RspRingReboot.ReqestId != pCmdRequest->Action.ReqRingReboot.ReqestId)
						continue;
					memcpy(pCmdResponse, &CmdResponse, sizeof(tRMsgCmd));
					break;					
				} else {
					break;
				}
			} else {
				ret = CMD_RET_CODE_TIMEOUT;
				break;
			}
		}
	} 
	
	return ret;
}

/**************************************************************************
  * @brief  OBRing frame read task
  * @param  arg
  * @retval none
  *************************************************************************/
void obring_read_task(void *arg)
{
	RingMsgQueue = xQueueCreate(MAX_RING_MSG_QUEUE_LEN, sizeof(tRingMessage));
	if(RingMsgQueue == NULL) {
		printf("Ring message queue create error\r\n");
		vTaskDelete(NULL);
		return;
	}

	for(;;) {
		if(RingMsgQueue != NULL) {
			if(xQueueReceive(RingMsgQueue, (void *)&RingRxMsg, portMAX_DELAY) == pdTRUE) {
				os_mutex_lock(&RingMutex, OS_MUTEX_WAIT_FOREVER);
				obring_frame_handle(&RingRxMsg.Buf[0], RingRxMsg.BufLen);
				os_mutex_unlock(&RingMutex);
			}
		} else {
			vTaskDelay(100);
		}
	}
}

/**************************************************************************
  * @brief  OBRing port poll task
  * @param  arg
  * @retval none
  *************************************************************************/
void obring_poll_task(void *arg)
{
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	unsigned char RingIndex;	
	unsigned char CurrRingLinkMap[MAX_RING_NUM] = {0};
	unsigned char PrevRingLinkMap[MAX_RING_NUM] = {0};
	unsigned char LinkChangeMap;
	HAL_PORT_LINK_STATE PrimaryPortLinkState, SecondaryPortLinkState;
	int count = 0;
	extern unsigned char DevMac[];
	extern void ETH_FlushTransmitFIFO(void);
	
	vTaskDelay(1000);
	
	while(1) {
		os_mutex_lock(&RingMutex, OS_MUTEX_WAIT_FOREVER);
		
		for(RingIndex = 0; RingIndex < pRingInfo->GlobalConfig.ucRecordNum; RingIndex++) {
			pRingConfig = &(pRingInfo->RingConfig[RingIndex]);
			pRingState = &(pRingInfo->DevState[RingIndex]);
			
			if(hal_swif_port_get_link_state(pRingConfig->ucPrimaryPort, &PrimaryPortLinkState) == HAL_SWIF_SUCCESS) {
				if(PrimaryPortLinkState == LINK_UP) {
					CurrRingLinkMap[RingIndex] |= 0x01;
				} else {
					CurrRingLinkMap[RingIndex] &= 0xFE;
				}
			} 

			if(hal_swif_port_get_link_state(pRingConfig->ucSecondaryPort, &SecondaryPortLinkState) == HAL_SWIF_SUCCESS) {
				if(SecondaryPortLinkState == LINK_UP) {
					CurrRingLinkMap[RingIndex] |= 0x02;
				} else {
					CurrRingLinkMap[RingIndex] &= 0xFD;
				}
			}

			LinkChangeMap = CurrRingLinkMap[RingIndex] ^ PrevRingLinkMap[RingIndex];
			if(LinkChangeMap) {
				if(LinkChangeMap & 0x01) {
					OB_DEBUG(DBG_OBRING, "[PrimaryPort %d %s]\r\n", pRingConfig->ucPrimaryPort, (PrimaryPortLinkState == LINK_UP)? "LinkUp":"LinkDown");
					if(PrimaryPortLinkState == LINK_DOWN) {
						ETH_FlushTransmitFIFO();
						/* PrimaryPort state initialize */
						pRingState->PortState[INDEX_PRIMARY].LinkState = LINK_DOWN;
						if(pRingConfig->ucEnable == 0x01) {
							pRingState->PortState[INDEX_PRIMARY].RunState = PORT_DOWN;
							pRingState->PortState[INDEX_PRIMARY].AuthTimoutCount = 0;
							pRingState->PortState[INDEX_PRIMARY].BallotId.Prio = pRingConfig->ucNodePrio;
							memcpy(pRingState->PortState[INDEX_PRIMARY].BallotId.Mac, DevMac, MAC_LEN);
							/* All timer stop */
							obring_timer_stop(&(RingTimer[RingIndex].AuthP));
							obring_timer_stop(&(RingTimer[RingIndex].BallotP));
						} else {
							pRingState->PortState[INDEX_PRIMARY].RunState = PORT_IDLE;
						}
					} else {
						pRingState->PortState[INDEX_PRIMARY].LinkState = LINK_UP;
					}
				}
				
				if(LinkChangeMap & 0x02) {
					OB_DEBUG(DBG_OBRING, "[SecondaryPort %d %s]\r\n", pRingConfig->ucSecondaryPort, (SecondaryPortLinkState == LINK_UP)? "LinkUp":"LinkDown");
					if(SecondaryPortLinkState == LINK_DOWN) {
						ETH_FlushTransmitFIFO();
						/* SecondaryPort state initialize */
						pRingState->PortState[INDEX_SECONDARY].LinkState = LINK_DOWN;
						if(pRingConfig->ucEnable == 0x01) {
							pRingState->PortState[INDEX_SECONDARY].RunState = PORT_DOWN;
							pRingState->PortState[INDEX_SECONDARY].AuthTimoutCount = 0;
							pRingState->PortState[INDEX_SECONDARY].BallotId.Prio = pRingConfig->ucNodePrio;
							memcpy(pRingState->PortState[INDEX_SECONDARY].BallotId.Mac, DevMac, MAC_LEN);
							/* All timer stop */
							obring_timer_stop(&(RingTimer[RingIndex].AuthS));
							obring_timer_stop(&(RingTimer[RingIndex].BallotS));
						} else {
							pRingState->PortState[INDEX_SECONDARY].RunState = PORT_IDLE;
						}
					} else {
						pRingState->PortState[INDEX_SECONDARY].LinkState = LINK_UP;
					}
				}

				if((LinkChangeMap & 0x01) && (pRingConfig->ucEnable == 0x01)) {
					obring_link_change_handle(RingIndex, pRingConfig->ucPrimaryPort, PrimaryPortLinkState);
				}
				if((LinkChangeMap & 0x02) && (pRingConfig->ucEnable == 0x01)) {
					obring_link_change_handle(RingIndex, pRingConfig->ucSecondaryPort, SecondaryPortLinkState);
				}	

				PrevRingLinkMap[RingIndex] = CurrRingLinkMap[RingIndex];
			}
		}
		
		obring_timer_tick();
		
		os_mutex_unlock(&RingMutex);
		
		vTaskDelay(LINK_POLL_DELAY);
	}
}


void obring_switch_init(void)
{
#if SWITCH_CHIP_88E6095
	GT_ATU_ENTRY macEntry;
	GT_STATUS status;
	
	if(RingGlobalEnable) {
#if USE_OWN_MULTI_ADDR
		memset(&macEntry,0,sizeof(GT_ATU_ENTRY));
		memcpy(macEntry.macAddr.arEther, RingMgmtMultiDA, 6);
		macEntry.DBNum = 0;
		macEntry.portVec = (1<<dev->cpuPortNum);
		macEntry.prio = 7;
		macEntry.entryState.mcEntryState = GT_MC_PRIO_MGM_STATIC;

		if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK) {
			printf("gfdbAddMacEntry return failed, ret=%d\r\n", status);
			return;
		}
#endif /* USE_OWN_MULTI_ADDR */
	} 
	
#elif SWITCH_CHIP_BCM53101
	if(RingGlobalEnable) {
#if USE_OWN_MULTI_ADDR		
        unsigned char RingMgmtMultiDATmp[6];
        RingMgmtMultiDATmp[5] = RingMgmtMultiDA[0];
        RingMgmtMultiDATmp[4] = RingMgmtMultiDA[1];
        RingMgmtMultiDATmp[3] = RingMgmtMultiDA[2];
        RingMgmtMultiDATmp[2] = RingMgmtMultiDA[3];
        RingMgmtMultiDATmp[1] = RingMgmtMultiDA[4];
        RingMgmtMultiDATmp[0] = RingMgmtMultiDA[5];
		robo_write(PAGE_ARL_CONTROL, REG_BPDU_MC_ADDR, RingMgmtMultiDATmp, 6);
#endif /* USE_OWN_MULTI_ADDR */		
	}
#endif
}

/**************************************************************************
  * @brief  OBRing initialize
  * @param  none
  * @retval none
  *************************************************************************/
void obring_initialize(void)
{
	tRingInfo *pRingInfo = &RingInfo;
	tRingConfigGlobal RingCfgGlobal;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	unsigned char RingIndex;
	unsigned short HelloTime, FailTime, BallotTime;
	unsigned char PrimaryHport, SecondaryHport;
	extern unsigned char DevMac[];
	
	memset(RingTxBuf, 0, MAX_RING_MSG_SIZE);
	memset(&RingInfo, 0, sizeof(tRingInfo));
	
	if(conf_get_ring_global(&RingCfgGlobal) != CONF_ERR_NONE) {
		printf("Error: eeprom read failed\r\n");
		goto RingInitError;
	}
	if(RingCfgGlobal.ucGlobalEnable == 0x01)
		RingGlobalEnable = 0x01;
	else
		RingGlobalEnable = 0x00;
	
	if((RingCfgGlobal.ucGlobalEnable == 0x01) && (RingCfgGlobal.ucRecordNum > 0) && (RingCfgGlobal.ucRecordNum <= MAX_RING_NUM)) {
		pRingInfo->GlobalConfig.ucGlobalEnable = 0x01;
		pRingInfo->GlobalConfig.ucRecordNum = RingCfgGlobal.ucRecordNum;
		
		for(RingIndex = 0; RingIndex < RingCfgGlobal.ucRecordNum; RingIndex++) {
			pRingConfig = &(pRingInfo->RingConfig[RingIndex]);
			pRingState = &(pRingInfo->DevState[RingIndex]);
			
			if(conf_get_ring_record(RingIndex, pRingConfig) != CONF_ERR_NONE) {
				printf("Error: eeprom read failed\r\n");
				goto RingInitError;
			}

			if((pRingConfig->ucPrimaryPort > MAX_PORT_NUM) || (pRingConfig->ucSecondaryPort > MAX_PORT_NUM) || (pRingConfig->ucPrimaryPort == pRingConfig->ucSecondaryPort)) {
				printf("Error: configuration data invalid\r\n");
				goto RingInitError;
			}
			
			if(pRingConfig->ucEnable == 0x01) {
				Set_RingLED_BlinkMode(BLINK_2HZ);
				PrimaryHport = hal_swif_lport_2_hport(pRingConfig->ucPrimaryPort);
				SecondaryHport = hal_swif_lport_2_hport(pRingConfig->ucSecondaryPort);
				RingHportVec |= 1<<PrimaryHport;
				RingHportVec |= 1<<SecondaryHport;
			} else {
				Set_RingLED_BlinkMode(BLINK_1HZ);
			}
			
			/* All timer stop */
			obring_timer_stop(&(RingTimer[RingIndex].AuthP));
			obring_timer_stop(&(RingTimer[RingIndex].AuthS));
			obring_timer_stop(&(RingTimer[RingIndex].BallotP));
			obring_timer_stop(&(RingTimer[RingIndex].BallotS));
			obring_timer_stop(&(RingTimer[RingIndex].Hello));
			obring_timer_stop(&(RingTimer[RingIndex].Fail));

			
			/* Node state initialize */
			pRingState->SwitchTimes = 0;
			pRingState->StormCount = 0;
			pRingState->HelloSeq = 0;
			pRingState->NodeType = NODE_TYPE_MASTER;
			pRingState->NodeState = NODE_STATE_IDLE;
			pRingState->RingState = RING_FAULT;

			/* PrimaryPort state initialize */
			pRingState->PortState[INDEX_PRIMARY].LinkState = LINK_DOWN;
			if(pRingConfig->ucEnable == 0x01) {
				hal_swif_port_set_stp_state(pRingConfig->ucPrimaryPort, BLOCKING);
				pRingState->PortState[INDEX_PRIMARY].StpState = BLOCKING;
				pRingState->PortState[INDEX_PRIMARY].RunState = PORT_DOWN;
			} else {
				hal_swif_port_set_stp_state(pRingConfig->ucPrimaryPort, FORWARDING);
				pRingState->PortState[INDEX_PRIMARY].StpState = FORWARDING;	
				pRingState->PortState[INDEX_PRIMARY].RunState = PORT_IDLE;
			}
			
			pRingState->PortState[INDEX_PRIMARY].AuthTimoutCount = 0;
			pRingState->PortState[INDEX_PRIMARY].BallotId.Prio = pRingConfig->ucNodePrio;
			memcpy(pRingState->PortState[INDEX_PRIMARY].BallotId.Mac, DevMac, MAC_LEN);
			pRingState->PortState[INDEX_PRIMARY].NeighborValid = HAL_FALSE;
			memset(pRingState->PortState[INDEX_PRIMARY].NeighborMac, 0, MAC_LEN);
			pRingState->PortState[INDEX_PRIMARY].NeighborPortNo = 0;

			/* SecondaryPort state initialize */
			pRingState->PortState[INDEX_SECONDARY].LinkState = LINK_DOWN;
			if(pRingConfig->ucEnable == 0x01) {
				hal_swif_port_set_stp_state(pRingConfig->ucSecondaryPort, BLOCKING);
				pRingState->PortState[INDEX_SECONDARY].StpState = BLOCKING;
				pRingState->PortState[INDEX_SECONDARY].RunState = PORT_DOWN;
			} else {
				hal_swif_port_set_stp_state(pRingConfig->ucSecondaryPort, FORWARDING);
				pRingState->PortState[INDEX_SECONDARY].StpState = FORWARDING;
				pRingState->PortState[INDEX_SECONDARY].RunState = PORT_IDLE;
			}
			pRingState->PortState[INDEX_SECONDARY].AuthTimoutCount = 0;
			pRingState->PortState[INDEX_SECONDARY].BallotId.Prio = pRingConfig->ucNodePrio;
			memcpy(pRingState->PortState[INDEX_SECONDARY].BallotId.Mac, DevMac, MAC_LEN);
			pRingState->PortState[INDEX_SECONDARY].NeighborValid = HAL_FALSE;
			memset(pRingState->PortState[INDEX_SECONDARY].NeighborMac, 0, MAC_LEN);
			pRingState->PortState[INDEX_SECONDARY].NeighborPortNo = 0;	
		}

		os_mutex_init(&RingMutex);

		ObrMsgRxQueue = xQueueCreate(5, MAX_RING_DATA_SIZE);
		if(ObrMsgRxQueue == NULL) {
			printf("ObrMsgRxQueue create error\r\n");
		}
		
		xTaskCreate(obring_read_task, "tRingRead", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 7, NULL);
		xTaskCreate(obring_poll_task, "tRingPoll", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 6, NULL);
	}
	
RingInitError:
	obring_switch_init();
}

/**************************************************************************
  * @brief  check OBRing enable,
  * @param  Ring group index,first ring index is 0
  * @retval return 1 if OBRing enable,return 0 if OBRing disable
  *************************************************************************/
u8 obring_check_enable(unsigned char RingIndex)
{
    tRingInfo *pRingInfo = &RingInfo;
    tRingConfigRec *pRingConfig;
    
    pRingConfig = &(pRingInfo->RingConfig[RingIndex]);

    if( pRingConfig->ucEnable == 0x01){
        return 1;
    }else{
        return 0;
    }
}

#endif

