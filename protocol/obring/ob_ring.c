
/*************************************************************
 * Filename     : ob_ring.c
 * Description  : OB-Ring protocol
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

#include "mconfig.h"

#if MODULE_RING
/* Standard includes. */
#include "stdio.h"
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "os_mutex.h"

/* LwIP includes */
#include "lwip/inet.h"

/* BSP include */
#include "stm32f2xx.h"
#if ROBO_SWITCH
#include "robo_drv.h"
#endif
#include "hal_swif_port.h"
#include "hal_swif_mac.h"
#include "hal_swif_message.h"

#include "led_drv.h"
#include "ob_ring.h"
#include "conf_comm.h"
#include "conf_ring.h"
#include "conf_global.h"
#include "nms_global.h"
#include "cli_util.h"

static OS_MUTEX_T		RingMutex; 
static OBringFrame_t	RingTxFrame;
static DevNode_t		DevNode = {0};
static DevNode_t		*pDevNode = &DevNode;
static HAL_NODE_LINK_STATE	PreviousLinkState;
TimerGroup_t			Tim;
static u8				RingPortAlarmEnable;
extern u32				gTrapEnableMember;

static u8 ringPhyPortA;
static u8 ringPhyPortB;

void Ring_MsgSend(u8 MsgType, u8 *Mac, u8 Port)
{
	RingMsg_t *RingMsg = (RingMsg_t *)&(RingTxFrame.MsgData[0]);
	extern void EthSend(u8 *txBuffer, u16 len);
	
	memset(RingMsg, 0, sizeof(RingMsg_t));
#if SWITCH_CHIP_88E6095
	RingTxFrame.SwitchTag[1] = (Port << 3);
#elif SWITCH_CHIP_BCM53101
	RingTxFrame.SwitchTag[3] = 1 << Port;
#endif
	RingMsg->MsgType = MsgType;
	RingMsg->MsgLength = 64;
	if((MsgType == MsgRealBreak) || (MsgType == MsgChainFlushTbl))
		memcpy(RingMsg->RBMac, Mac, MAC_LEN);
	else
		memcpy(RingMsg->VBMac, Mac, MAC_LEN);
	RingMsg->TxPort = hal_swif_hport_2_lport(Port);
	EthSend((u8 *)&RingTxFrame, 64);		
}

void Ring_Frame_Process(u8 *rxBuf, u16 len)
{
	OBringFrame_t *pFrame = (OBringFrame_t *)rxBuf;
	RingMsg_t *RingMsg = (RingMsg_t *)&(RingTxFrame.MsgData[0]);
	RingMsg_t *pRxMsg;
	u16 Protocol;
	u8 rxRingPort;
	u8 ring_porta_lprx_count, ring_portb_lprx_count;
	
	if(pDevNode->RingEnable == 0)
		return;
	
	Protocol = pFrame->ProtocolID[0];
	Protocol = (Protocol << 8) | pFrame->ProtocolID[1];
	if(Protocol != OB_RING_PROTOCOL)
		return;
	
	os_mutex_lock(&RingMutex, OS_MUTEX_WAIT_FOREVER);
	
#if SWITCH_CHIP_88E6095
	rxRingPort = (pFrame->SwitchTag[1] & 0xF8) >> 3;
#elif SWITCH_CHIP_BCM53101
	rxRingPort = pFrame->SwitchTag[3] & 0x3F;
#endif
	pRxMsg = (RingMsg_t *)&(pFrame->MsgData[0]);
	switch(pRxMsg->MsgType) {
		case MsgRealBreak:
			#if 1
			OB_DEBUG(DBG_OBRING, "Port%s rx real-break packet, rb-mac: %02x-%02x-%02x-%02x-%02x-%02x\r\n", (rxRingPort == ringPhyPortA)? "A":"B", \
					pRxMsg->RBMac[0], pRxMsg->RBMac[1], pRxMsg->RBMac[2], pRxMsg->RBMac[3], pRxMsg->RBMac[4], pRxMsg->RBMac[5]);	
			#endif
			if(memcmp(pDevNode->NodeMac, pRxMsg->RBMac, MAC_LEN) != 0) {
				
				if(PreviousLinkState == STATE_11) {
					#if 0
					if(pDevNode->NodeType == RING_MASTER_NODE) {
						OB_DEBUG(DBG_OBRING, "Ring-Master-Node --> Chain-Transfer-Node\r\n");
						swif_SetPortStpState(ringPhyPortA, FORWARDING);
						swif_SetPortStpState(ringPhyPortB, FORWARDING);
						pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState = FORWARDING;
						pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState = FORWARDING;					
						PortA_LED_On();
						PortB_LED_On();
						Set_RingLED_BlinkMode(BLINK_2HZ);
						pDevNode->NodeType = SINGLE_NODE;
					}

					if(pDevNode->NodeType == RING_TRANSFER_NODE) {
						//OB_DEBUG(DBG_OBRING, "Ring-Transfer-Node --> Chain-Transfer-Node\r\n");
						pDevNode->NodeType = SINGLE_NODE;
					}
					#endif
					if(rxRingPort == ringPhyPortA) 
						Ring_MsgSend(MsgRealBreak, pRxMsg->RBMac, ringPhyPortB);
					else
						Ring_MsgSend(MsgRealBreak, pRxMsg->RBMac, ringPhyPortA);

				} else {
					if(memcmp(pDevNode->NodeMac, pRxMsg->RBMac, MAC_LEN) < 0) {					/* Device MAC  <  Received MAC */

						//if(pDevNode->NodeType == CHAIN_MASTER_NODE)
							//break;
						OB_DEBUG(DBG_OBRING, "Single-Node --> Chain-Master-Node, and Chain Flushing\r\n");
						pDevNode->NodeType = CHAIN_MASTER_NODE;
						pDevNode->FlushState = CHAIN_FLUSH_START;

						Set_RingLED_BlinkMode(BLINK_10HZ);
					
						//if(rxRingPort == ringPhyPortA) 
						if((PreviousLinkState == STATE_10) && (rxRingPort == ringPhyPortA))
							Ring_MsgSend(MsgChainFlushTbl, pDevNode->NodeMac, ringPhyPortA);
						else if((PreviousLinkState == STATE_01) && (rxRingPort == ringPhyPortB))
							Ring_MsgSend(MsgChainFlushTbl, pDevNode->NodeMac, ringPhyPortB);
						
					} else if(memcmp(pDevNode->NodeMac, pRxMsg->RBMac, MAC_LEN) > 0) {			/* Device MAC  >  Received MAC */
					
						//if(pDevNode->NodeType == CHAIN_SLAVER_NODE)
						//	break;
						OB_DEBUG(DBG_OBRING, "Send real-break message to Port%s\r\n",(rxRingPort == ringPhyPortA)? "A":"B");
						if((PreviousLinkState == STATE_10) && (rxRingPort == ringPhyPortA))
							Ring_MsgSend(MsgRealBreak, pDevNode->NodeMac, ringPhyPortA);
						else if((PreviousLinkState == STATE_01) && (rxRingPort == ringPhyPortB))
							Ring_MsgSend(MsgRealBreak, pDevNode->NodeMac, ringPhyPortB);
						
					} else {
																								/* Device MAC  =  Received MAC */
					}
				}
			}
		break;
		
		case MsgVirtualBreak:
			if(PreviousLinkState == STATE_11) {
				if(memcmp(pDevNode->NodeMac, pRxMsg->VBMac, MAC_LEN) > 0) {						/* Device MAC  >  Received MAC */
					#if 1
					OB_DEBUG(DBG_OBRING, "PortA rx virtual-break packet, dev-mac(%02x-%02x-%02x-%02x-%02x-%02x) > vb-mac(%02x-%02x-%02x-%02x-%02x-%02x)\r\n", \
							pDevNode->NodeMac[0],pDevNode->NodeMac[1],pDevNode->NodeMac[2],pDevNode->NodeMac[3],pDevNode->NodeMac[4],pDevNode->NodeMac[5],\
							pRxMsg->VBMac[0], pRxMsg->VBMac[1], pRxMsg->VBMac[2], pRxMsg->VBMac[3], pRxMsg->VBMac[4], pRxMsg->VBMac[5]);
					#endif
					#if 0
					swif_SetPortStpState(ringPhyPortA, FORWARDING);
					swif_SetPortStpState(ringPhyPortB, FORWARDING);
					pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState = FORWARDING;
					pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState = FORWARDING;
					PortA_LED_On();
					PortB_LED_On();
					#endif
					
					if(rxRingPort == ringPhyPortA)
						Ring_MsgSend(MsgVirtualBreak, pRxMsg->VBMac, ringPhyPortB);
					else
						Ring_MsgSend(MsgVirtualBreak, pRxMsg->VBMac, ringPhyPortA);
					
				} else {
					if(memcmp(pDevNode->NodeMac, pRxMsg->VBMac, MAC_LEN) != 0) {				/* Device MAC  <  Received MAC */
						#if 1
						OB_DEBUG(DBG_OBRING, "PortA rx virtual-break packet, dev-mac(%02x-%02x-%02x-%02x-%02x-%02x) < vb-mac(%02x-%02x-%02x-%02x-%02x-%02x)\r\n", \
								pDevNode->NodeMac[0],pDevNode->NodeMac[1],pDevNode->NodeMac[2],pDevNode->NodeMac[3],pDevNode->NodeMac[4],pDevNode->NodeMac[5],\
								pRxMsg->VBMac[0], pRxMsg->VBMac[1], pRxMsg->VBMac[2], pRxMsg->VBMac[3], pRxMsg->VBMac[4], pRxMsg->VBMac[5]);
						#endif
						#if 0
						swif_SetPortStpState(ringPhyPortB, BLOCKING);
						swif_SetPortStpState(ringPhyPortA, FORWARDING);
						PortA_LED_On();
						PortB_LED_Blink();
						#endif
						
						if(rxRingPort == ringPhyPortA)
							Ring_MsgSend(MsgVirtualBreak, pDevNode->NodeMac, ringPhyPortB);
						else
							Ring_MsgSend(MsgVirtualBreak, pDevNode->NodeMac, ringPhyPortA);	
					} else {																	/* Device MAC  =  Received MAC */
						HAL_PORT_STP_STATE PortA_StpState;
						HAL_PORT_STP_STATE PortB_StpState;
						#if 1						
						OB_DEBUG(DBG_OBRING, "PortA rx virtual-break packet, dev-mac(%02x-%02x-%02x-%02x-%02x-%02x) = vb-mac(%02x-%02x-%02x-%02x-%02x-%02x)\r\n", \
								pDevNode->NodeMac[0],pDevNode->NodeMac[1],pDevNode->NodeMac[2],pDevNode->NodeMac[3],pDevNode->NodeMac[4],pDevNode->NodeMac[5],\
								pRxMsg->VBMac[0], pRxMsg->VBMac[1], pRxMsg->VBMac[2], pRxMsg->VBMac[3], pRxMsg->VBMac[4], pRxMsg->VBMac[5]);
						#endif
						//if(pDevNode->NodeType == RING_MASTER_NODE)
							//break;
						OB_DEBUG(DBG_OBRING, "Node --> Ring-Master-Node, and Start Ring Flushing\r\n");

						ring_porta_lprx_count = 0;
						ring_portb_lprx_count = 0;
						pDevNode->NodeType = RING_MASTER_NODE;
						pDevNode->FlushState = RING_FLUSH_START;
						
						hal_swif_port_set_stp_state(RING_LOGIC_PORT_B + 1, BLOCKING);
						hal_swif_port_set_stp_state(RING_LOGIC_PORT_A + 1, FORWARDING);
						pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState = BLOCKING;	
						pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState = FORWARDING;
						PortA_LED_On();
						PortB_LED_Blink();
						
						Set_RingLED_BlinkMode(BLINK_10HZ);

						Ring_MsgSend(MsgRingFlushTbl, pDevNode->NodeMac, ringPhyPortA);
						Ring_MsgSend(MsgRingFlushTbl, pDevNode->NodeMac, ringPhyPortB);

						//hal_swif_mac_flush_all();
					}										
				}
			} else {
				if((PreviousLinkState == STATE_10) && (rxRingPort == ringPhyPortA))
					Ring_MsgSend(MsgRealBreak, pDevNode->NodeMac, ringPhyPortA);
				else if((PreviousLinkState == STATE_01) && (rxRingPort == ringPhyPortB))
					Ring_MsgSend(MsgRealBreak, pDevNode->NodeMac, ringPhyPortB);
			}
		break;
		
		case MsgChainFlushTbl:			/* PortA received chain flush table packet */
			
			if(memcmp(pDevNode->NodeMac, pRxMsg->RBMac, MAC_LEN) != 0) {
				if(PreviousLinkState == STATE_11) {
					if(rxRingPort == ringPhyPortA) {
						memcpy(pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborMac, pFrame->SrcMAC, MAC_LEN);
						pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborPortNum = pRxMsg->TxPort;
						pDevNode->PortInfo[RING_LOGIC_PORT_A].RingFlag = 0;
						Ring_MsgSend(MsgChainFlushTbl, pRxMsg->RBMac, ringPhyPortB);
					} else {
						memcpy(pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborMac, pFrame->SrcMAC, MAC_LEN);
						pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborPortNum = pRxMsg->TxPort;		
						pDevNode->PortInfo[RING_LOGIC_PORT_B].RingFlag = 0;
						Ring_MsgSend(MsgChainFlushTbl, pRxMsg->RBMac, ringPhyPortA);
					}

					if(pDevNode->NodeType != CHAIN_TRANSFER_NODE) {
						if(pDevNode->NodeType == RING_MASTER_NODE) {
							pDevNode->FlushState = FLUSH_IDLE;
						}
						OB_DEBUG(DBG_OBRING, "Port%s rx MsgChainFlushTbl, NodeType = Chain-Transfer-Node\r\n", (rxRingPort == ringPhyPortA)? "A":"B");	
						pDevNode->NodeType = CHAIN_TRANSFER_NODE;
						Set_RingLED_BlinkMode(BLINK_5HZ);
						hal_swif_port_set_stp_state(RING_LOGIC_PORT_A + 1, FORWARDING);
						hal_swif_port_set_stp_state(RING_LOGIC_PORT_B + 1, FORWARDING);
						pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState = FORWARDING;
						pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState = FORWARDING;						
						PortA_LED_On();	
						PortB_LED_On();	
						hal_swif_mac_flush_all();
					}
				} else {
					//if(rxRingPort == ringPhyPortA) {
					if((PreviousLinkState == STATE_01) && (rxRingPort == ringPhyPortB)){
						memcpy(pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborMac, pFrame->SrcMAC, MAC_LEN);
						pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborPortNum = pRxMsg->TxPort;
						pDevNode->PortInfo[RING_LOGIC_PORT_B].RingFlag = 0;
						Ring_MsgSend(MsgChainFlushTbl, pRxMsg->RBMac, ringPhyPortB);
					} else if((PreviousLinkState == STATE_10) && (rxRingPort == ringPhyPortA)){
						memcpy(pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborMac, pFrame->SrcMAC, MAC_LEN);
						pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborPortNum = pRxMsg->TxPort;		
						pDevNode->PortInfo[RING_LOGIC_PORT_A].RingFlag = 0;
						Ring_MsgSend(MsgChainFlushTbl, pRxMsg->RBMac, ringPhyPortA);
					}
					
					if(pDevNode->NodeType != CHAIN_SLAVER_NODE) {
						OB_DEBUG(DBG_OBRING, "Port%s rx MsgChainFlushTbl, NodeType = Chain-Slaver-Node\r\n", (rxRingPort == ringPhyPortA)? "A":"B");	
						pDevNode->NodeType = CHAIN_SLAVER_NODE;
						Set_RingLED_BlinkMode(BLINK_5HZ);
						if(RingPortAlarmEnable) {
							OB_DEBUG(DBG_OBRING, "Report trap alarm of ring port\r\n");	
							//TrapSend_RingChange(TRAP_RINGPORT_NO);
						}
						hal_swif_mac_flush_all();
					}
				}
			}  else {
				if(pDevNode->FlushState == CHAIN_FLUSH_START) {
					OB_DEBUG(DBG_OBRING, "Port%s rx MsgChainFlushTbl, Chain Flush Done\r\n", (rxRingPort == ringPhyPortA)? "A":"B");

					if(rxRingPort == ringPhyPortA) {
						memcpy(pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborMac, pFrame->SrcMAC, MAC_LEN);
						pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborPortNum = pRxMsg->TxPort;
						pDevNode->PortInfo[RING_LOGIC_PORT_A].RingFlag = 0;
						
					} else {
						memcpy(pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborMac, pFrame->SrcMAC, MAC_LEN);
						pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborPortNum = pRxMsg->TxPort;		
						pDevNode->PortInfo[RING_LOGIC_PORT_B].RingFlag = 0;
						
					}

					if(RingPortAlarmEnable){
						OB_DEBUG(DBG_OBRING, "Report trap alarm of ring port\r\n");	
						//TrapSend_RingChange(TRAP_RINGPORT_NO);
					}
					pDevNode->FlushState = CHAIN_FLUSH_COMPLETE;
					hal_swif_mac_flush_all();
				}							
			}
			

		break;

		case MsgRingFlushTbl:
			if(memcmp(pDevNode->NodeMac, pRxMsg->VBMac, MAC_LEN) != 0) {
				if(PreviousLinkState == STATE_11) {

					if(rxRingPort == ringPhyPortA) {
						memcpy(pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborMac, pFrame->SrcMAC, MAC_LEN);
						pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborPortNum = pRxMsg->TxPort;
						pDevNode->PortInfo[RING_LOGIC_PORT_A].RingFlag = 1;
						Ring_MsgSend(MsgRingFlushTbl, pRxMsg->VBMac, ringPhyPortB);
					} else {
						memcpy(pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborMac, pFrame->SrcMAC, MAC_LEN);
						pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborPortNum = pRxMsg->TxPort;		
						pDevNode->PortInfo[RING_LOGIC_PORT_B].RingFlag = 1;
						Ring_MsgSend(MsgRingFlushTbl, pRxMsg->VBMac, ringPhyPortA);
					}
					
					if(pDevNode->NodeType != RING_TRANSFER_NODE) {
						OB_DEBUG(DBG_OBRING, "Port%s rx MsgRingFlushTbl, NodeType = Ring-Transfer-Node\r\n", (rxRingPort == ringPhyPortA)? "A":"B");
						pDevNode->NodeType = RING_TRANSFER_NODE;
						Set_RingLED_BlinkMode(BLINK_5HZ);
						hal_swif_port_set_stp_state(RING_LOGIC_PORT_A + 1, FORWARDING);
						hal_swif_port_set_stp_state(RING_LOGIC_PORT_B + 1, FORWARDING);
						pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState = FORWARDING;
						pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState = FORWARDING;					
						PortA_LED_On();	
						PortB_LED_On();	
						hal_swif_mac_flush_all();
					}
				}
			} else {
				if(PreviousLinkState == STATE_11) {
					if(pDevNode->FlushState == RING_FLUSH_START) {
						
						OB_DEBUG(DBG_OBRING, "Port%s rx MsgRingFlushTbl, Ring Flush Done\r\n", (rxRingPort == ringPhyPortA)? "A":"B");
						
						if(rxRingPort == ringPhyPortA) {
							memcpy(pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborMac, pFrame->SrcMAC, MAC_LEN);
							pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborPortNum = pRxMsg->TxPort;
							pDevNode->PortInfo[RING_LOGIC_PORT_A].RingFlag = 1;
							ring_porta_lprx_count++;
						} else {
							memcpy(pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborMac, pFrame->SrcMAC, MAC_LEN);
							pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborPortNum = pRxMsg->TxPort;		
							pDevNode->PortInfo[RING_LOGIC_PORT_B].RingFlag = 1;
							ring_portb_lprx_count++;
						}

						if((ring_porta_lprx_count > 0) && (ring_portb_lprx_count > 0)) {
							pDevNode->FlushState = RING_FLUSH_COMPLETE;
							if(RingPortAlarmEnable) {
								OB_DEBUG(DBG_OBRING, "Report trap alarm of ring port\r\n");	
								//TrapSend_RingChange(TRAP_RINGPORT_OK);
							}
							hal_swif_mac_flush_all();
						}
					}
				}
			}
		break;
		
		default:
		break;
	}
	
	os_mutex_unlock(&RingMutex);
}		


void Ring_LinkChangeProcess(HAL_NODE_LINK_STATE PreviousState, HAL_NODE_LINK_STATE CurrentState)
{
	RingMsg_t *RingMsg = (RingMsg_t *)&(RingTxFrame.MsgData[0]);
	HAL_PORT_STP_STATE PortA_StpState, PortB_StpState;

	OB_DEBUG(DBG_OBRING, "Link State %02x -> %02x\r\n", PreviousState, CurrentState);

	Set_RingLED_BlinkMode(BLINK_2HZ);
	
	switch(CurrentState) {
		case STATE_00:
			hal_swif_port_set_stp_state(RING_LOGIC_PORT_A + 1, BLOCKING);
			hal_swif_port_set_stp_state(RING_LOGIC_PORT_B + 1, BLOCKING);
			
			/* Record the current state */
			pDevNode->PortInfo[RING_LOGIC_PORT_A].LinkState= LINK_DOWN;
			pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState = BLOCKING;
			pDevNode->PortInfo[RING_LOGIC_PORT_B].LinkState= LINK_DOWN;
			pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState = BLOCKING;
			
			memset(pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborMac, 0, MAC_LEN);
			pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborPortNum = 0;
			pDevNode->PortInfo[RING_LOGIC_PORT_A].RingFlag = 0;

			memset(pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborMac, 0, MAC_LEN);
			pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborPortNum = 0;
			pDevNode->PortInfo[RING_LOGIC_PORT_B].RingFlag = 0;
			
			pDevNode->NodeType = SINGLE_NODE;
			PortA_LED_Off();
			PortB_LED_Off();
		break;
		
		case STATE_10:
			hal_swif_port_set_stp_state(RING_LOGIC_PORT_A + 1, FORWARDING);
			hal_swif_port_set_stp_state(RING_LOGIC_PORT_B + 1, BLOCKING);

			/* Record the current state  */
			pDevNode->PortInfo[RING_LOGIC_PORT_A].LinkState= LINK_UP;
			pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState = FORWARDING;
			pDevNode->PortInfo[RING_LOGIC_PORT_B].LinkState= LINK_DOWN;
			pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState = BLOCKING;
			
			memset(pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborMac, 0, MAC_LEN);
			pDevNode->PortInfo[RING_LOGIC_PORT_B].ExtNeighborPortNum = 0;
			pDevNode->PortInfo[RING_LOGIC_PORT_B].RingFlag = 0;

			pDevNode->NodeType = SINGLE_NODE;
			PortA_LED_On();
			PortB_LED_Off();
			
			/* Send the real break message */
			Ring_MsgSend(MsgRealBreak, pDevNode->NodeMac, ringPhyPortA);
			Tim.TimTxPortA = TIM_TX_FRAME;
			
			OB_DEBUG(DBG_OBRING, "Send real-break message to PortA\r\n");
		break;
		
		case STATE_01:
			hal_swif_port_set_stp_state(RING_LOGIC_PORT_A + 1, BLOCKING);
			hal_swif_port_set_stp_state(RING_LOGIC_PORT_B + 1, FORWARDING);
			
			/* Record the current state */
			pDevNode->PortInfo[RING_LOGIC_PORT_A].LinkState= LINK_DOWN;
			pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState = BLOCKING;
			pDevNode->PortInfo[RING_LOGIC_PORT_B].LinkState= LINK_UP;
			pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState = FORWARDING;

			memset(pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborMac, 0, MAC_LEN);
			pDevNode->PortInfo[RING_LOGIC_PORT_A].ExtNeighborPortNum = 0;
			pDevNode->PortInfo[RING_LOGIC_PORT_A].RingFlag = 0;
			
			pDevNode->NodeType = SINGLE_NODE;

			PortA_LED_Off();
			PortB_LED_On();
			
			/* Send the real break message */
			Ring_MsgSend(MsgRealBreak, pDevNode->NodeMac, ringPhyPortB);
			Tim.TimTxPortB = TIM_TX_FRAME;
			
			OB_DEBUG(DBG_OBRING, "Send real-break message to PortB\r\n");
		break;
		
		case STATE_11:
			switch(PreviousState) {
				case STATE_01:
					hal_swif_port_set_stp_state(RING_LOGIC_PORT_A + 1, BLOCKING);

					/* Record the current state */
					pDevNode->PortInfo[RING_LOGIC_PORT_A].LinkState= LINK_UP;
					pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState = BLOCKING;
					pDevNode->NodeType = SINGLE_NODE;
					PortA_LED_Blink();
					
					/* Send the vistual break message */
					Ring_MsgSend(MsgVirtualBreak, pDevNode->NodeMac, ringPhyPortB);
					Tim.TimTxPortB = TIM_TX_FRAME;
					
					OB_DEBUG(DBG_OBRING, "Send virtual-break message to PortB\r\n");
				break;

				case STATE_10:
					hal_swif_port_set_stp_state(RING_LOGIC_PORT_B + 1, BLOCKING);
					
					/* Record the current state */
					pDevNode->PortInfo[RING_LOGIC_PORT_B].LinkState= LINK_UP;
					pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState = BLOCKING;
					pDevNode->NodeType = SINGLE_NODE;
					PortB_LED_Blink();
					
					/* Send the vistual break message */
					Ring_MsgSend(MsgVirtualBreak, pDevNode->NodeMac, ringPhyPortA);
					Tim.TimTxPortA = TIM_TX_FRAME;
	
					OB_DEBUG(DBG_OBRING, "Send virtual-break message to PortA\r\n");
				break;

				case STATE_00:	
					hal_swif_port_set_stp_state(RING_LOGIC_PORT_B + 1, BLOCKING);
					hal_swif_port_set_stp_state(RING_LOGIC_PORT_A + 1, FORWARDING);

					/* Record the current state */
					pDevNode->PortInfo[RING_LOGIC_PORT_A].LinkState= LINK_UP;
					pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState = FORWARDING;
					pDevNode->PortInfo[RING_LOGIC_PORT_B].LinkState= LINK_UP;
					pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState = BLOCKING;
					pDevNode->NodeType = SINGLE_NODE;
					PortA_LED_On();
					PortB_LED_Blink();
					
					/* Send the vistual break message */
					Ring_MsgSend(MsgVirtualBreak, pDevNode->NodeMac, ringPhyPortB);
					Tim.TimTxPortB = TIM_TX_FRAME;
					Ring_MsgSend(MsgVirtualBreak, pDevNode->NodeMac, ringPhyPortA);
					Tim.TimTxPortA = TIM_TX_FRAME;
					
					OB_DEBUG(DBG_OBRING, "Send virtual-break message to PortB\r\n");
				break;	
			}
		break;		
	}
}

int Ring_GetNodeLinkState(HAL_NODE_LINK_STATE *nodeLinkState)
{
	int ret = 0;
	u16 regval;
	
	HAL_PORT_LINK_STATE	portA_linkState = LINK_DOWN;
	HAL_PORT_LINK_STATE	portB_linkState = LINK_DOWN;

//#if BOARD_GE22103MA
	ret = hal_swif_port_get_link_state(RING_LOGIC_PORT_A + 1, &portA_linkState);
	ret = hal_swif_port_get_link_state(RING_LOGIC_PORT_B + 1, &portB_linkState);	
//#elif BOARD_GE20023MA
//	ret = robo_read(0x01, 0x00, (u8 *)&regval, 2);
//	portA_linkState = (HAL_PORT_LINK_STATE)((regval & (1 << RING_LOGIC_PORT_A)) >> RING_LOGIC_PORT_A);
//	portB_linkState = (HAL_PORT_LINK_STATE)((regval & (1 << RING_LOGIC_PORT_B)) >> RING_LOGIC_PORT_B);
//#endif
	
	if((portA_linkState == LINK_UP) && (portB_linkState == LINK_DOWN)) {
		*nodeLinkState = STATE_10;
	} else if((portA_linkState == LINK_DOWN) && (portB_linkState == LINK_UP)) {
		*nodeLinkState = STATE_01;
	} else if((portA_linkState == LINK_UP) && (portB_linkState == LINK_UP)) {
		*nodeLinkState = STATE_11;
	} else if((portA_linkState == LINK_DOWN) && (portB_linkState == LINK_DOWN)) {
		*nodeLinkState = STATE_00;
	}

	return ret;
}

DevNode_t *Ring_GetNode(void)
{
	return pDevNode;
}

void Ring_Initialize(void)
{
	extern unsigned char DevMac[];
	extern unsigned char MultiAddress[];
	extern hal_trap_info_t gTrapInfo;
	ring_conf_t	ringcfg;
	int ret, i, j;

	/*******************************************************************************************
	 *                            Read Ring Configuration 
	 *******************************************************************************************/
	if((ret = conf_get_ring_config(&ringcfg)) != CONF_ERR_NONE) {
		memset(&ringcfg, 0, sizeof(ring_conf_t));
	}

	/* Initialize node data */
	memset(pDevNode, 0, sizeof(DevNode_t));
	memcpy(pDevNode->NodeMac, DevMac, MAC_LEN);
	pDevNode->RingEnable = 1;
	pDevNode->NodeType = SINGLE_NODE;
	pDevNode->FlushState = FLUSH_IDLE;
	
	/* Check ring pair number */
	if((ringcfg.ring_num != RING_PAIR_NUM) || !(ringcfg.ring_gate[3] & 0x1)) {
		pDevNode->RingEnable = 0;
	} 

	/* Check ring port number */
	pDevNode->PortInfo[0].DevPortNum = ringcfg.config[0].logic_port_num;
	pDevNode->PortInfo[1].DevPortNum = ringcfg.config[1].logic_port_num;
	if((pDevNode->PortInfo[0].DevPortNum != RING_LOGIC_PORT_A + 1) || (pDevNode->PortInfo[1].DevPortNum != RING_LOGIC_PORT_B + 1)) {
		pDevNode->RingEnable = 0;
	} 

	/* Check ring mode */
	if((ringcfg.config[RING_LOGIC_PORT_A].ring_mode != FAST_MODE) || (ringcfg.config[RING_LOGIC_PORT_B].ring_mode != FAST_MODE)) {
		pDevNode->RingEnable = 0;
	}
        
	/*******************************************************************************************
	 *                            Read Ring Port Alarm configuration 
	 *******************************************************************************************/
#if (BOARD_FEATURE & LOCAL_TRAP)
	if(gTrapInfo.GateMask & TRAP_MASK_RING_STATUS)
		RingPortAlarmEnable = 1;
	else
		RingPortAlarmEnable = 0;
#else
	RingPortAlarmEnable = 0;
#endif

	/*******************************************************************************************
	 *                            Ring initialize 
	 *******************************************************************************************/	
	if(pDevNode->RingEnable) {
		os_mutex_init(&RingMutex);

		memset((u8 *)&RingTxFrame, 0, sizeof(OBringFrame_t));
		
		/* fill the destination mac and source mac */
		memcpy(RingTxFrame.DstMAC, MultiAddress, 6);
		memcpy(RingTxFrame.SrcMAC, DevMac, 6);

#if SWITCH_CHIP_88E6095
		/* fill the Switch tag header */
		RingTxFrame.SwitchTag[0] = 0x40;
		RingTxFrame.SwitchTag[1] = 0x00;
		RingTxFrame.SwitchTag[2] = 0xe0;
		RingTxFrame.SwitchTag[3] = 0x00;
#elif SWITCH_CHIP_BCM53101
		/* fill the Switch tag header */
		RingTxFrame.SwitchTag[0] = OP_BRCM_SND;
		RingTxFrame.SwitchTag[1] = 0x00;
		RingTxFrame.SwitchTag[2] = 0x00;
		RingTxFrame.SwitchTag[3] = 0x00;
#endif
		/* fill the ethernet type and OUI code */
		RingTxFrame.OUIExtEtherType[0] = 0x88;
		RingTxFrame.OUIExtEtherType[1] = 0xB7;
		memcpy(RingTxFrame.OUICode, DevMac, 3);
		RingTxFrame.ProtocolID[0] = (u8)(OB_RING_PROTOCOL >> 8);
		RingTxFrame.ProtocolID[1] = (u8)(OB_RING_PROTOCOL & 0xFF);
		RingTxFrame.Version = 0x00;
	}
}

void Ring_Task(void *arg)
{
	RingMsg_t *RingMsg = (RingMsg_t *)&(RingTxFrame.MsgData[0]);
	HAL_NODE_LINK_STATE CurrentLinkState;
	
	PreviousLinkState = STATE_00;
    
    /* logic port to phy port */
    ringPhyPortA =  hal_swif_lport_2_hport(RING_LOGIC_PORT_A + 1);
    ringPhyPortB =  hal_swif_lport_2_hport(RING_LOGIC_PORT_B + 1);
	
	while(1) {
		Ring_GetNodeLinkState(&CurrentLinkState);

		if(CurrentLinkState != PreviousLinkState) {
			pDevNode->FlushState = FLUSH_IDLE;
			os_mutex_lock(&RingMutex, OS_MUTEX_WAIT_FOREVER);
			Ring_LinkChangeProcess(PreviousLinkState, CurrentLinkState); 
			PreviousLinkState = CurrentLinkState;
			os_mutex_unlock(&RingMutex);
		} else {
			switch(CurrentLinkState) {
				case STATE_01:
				if(Tim.TimTxPortB == 0) {	/* Send the real break message */
					if(pDevNode->NodeType == SINGLE_NODE) {
						Ring_MsgSend(MsgRealBreak, pDevNode->NodeMac, ringPhyPortB);
					}
					if((pDevNode->NodeType == CHAIN_MASTER_NODE) && (pDevNode->FlushState != CHAIN_FLUSH_COMPLETE)) {
						Ring_MsgSend(MsgChainFlushTbl, pDevNode->NodeMac, ringPhyPortB);
					}
					
					Tim.TimTxPortB = TIM_TX_FRAME;
				}					
				break;

				case STATE_10:
				if(Tim.TimTxPortA == 0) {	/* Send the real break message */
					if(pDevNode->NodeType == SINGLE_NODE) {
						Ring_MsgSend(MsgRealBreak, pDevNode->NodeMac, ringPhyPortA);
					}
					if((pDevNode->NodeType == CHAIN_MASTER_NODE) && (pDevNode->FlushState != CHAIN_FLUSH_COMPLETE)) {
						Ring_MsgSend(MsgChainFlushTbl, pDevNode->NodeMac, ringPhyPortA);
					}

					Tim.TimTxPortA = TIM_TX_FRAME;
				}
				break;

				case STATE_11:
				if(Tim.TimTxPortA == 0) {
					if(pDevNode->NodeType == SINGLE_NODE){
						if(((pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState == BLOCKING) && (pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState == FORWARDING)) || 
							((pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState == FORWARDING) && (pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState == BLOCKING))) {
							Ring_MsgSend(MsgVirtualBreak, pDevNode->NodeMac, ringPhyPortA);	
						}
					}
					
					if(pDevNode->FlushState == RING_FLUSH_START) {
						Ring_MsgSend(MsgRingFlushTbl, pDevNode->NodeMac, ringPhyPortA);
					}	

					Tim.TimTxPortA = TIM_TX_FRAME;
				}

				if(Tim.TimTxPortB == 0) {
					if(pDevNode->NodeType == SINGLE_NODE){
						if(((pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState == BLOCKING) && (pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState == FORWARDING)) || 
							((pDevNode->PortInfo[RING_LOGIC_PORT_A].StpState == FORWARDING) && (pDevNode->PortInfo[RING_LOGIC_PORT_B].StpState == BLOCKING))) {
							Ring_MsgSend(MsgVirtualBreak, pDevNode->NodeMac, ringPhyPortB);	
						}
					}
						
					if(pDevNode->FlushState == RING_FLUSH_START) {
						Ring_MsgSend(MsgRingFlushTbl, pDevNode->NodeMac, ringPhyPortB);
					}	

					Tim.TimTxPortB = TIM_TX_FRAME;
				}				
				break;
			}
		}
		
		vTaskDelay(1);
	}
}

u8 Check_Ring_Enable(void)
{
    return pDevNode->RingEnable;
}

void Ring_Start(void) 
{
	if(pDevNode->RingEnable) {
		Set_RingLED_BlinkMode(BLINK_2HZ);
		hal_swif_port_set_stp_state(RING_LOGIC_PORT_A + 1, BLOCKING);
		hal_swif_port_set_stp_state(RING_LOGIC_PORT_B + 1, BLOCKING);

		xTaskCreate(Ring_Task, "tRing", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 6, NULL);
	} else {
		Set_RingLED_BlinkMode(BLINK_1HZ);
		hal_swif_port_set_stp_state(RING_LOGIC_PORT_A + 1, FORWARDING);
		hal_swif_port_set_stp_state(RING_LOGIC_PORT_B + 1, FORWARDING);
	}
}

#endif

