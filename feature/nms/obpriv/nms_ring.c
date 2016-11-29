

/*************************************************************
 * Filename     : nms_ring.c
 * Description  : API for NMS interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

#include "mconfig.h"

/* Standard includes */
#include "string.h"

/* Kernel includes */

/* LwIP includes */
#include "lwip/inet.h"

/* BSP includes */
#include "stm32f2xx.h"
#include "soft_i2c.h"
#include "misc_drv.h"

/* Other includes */
#include "nms_comm.h"
#include "nms_if.h"
#include "nms_ring.h"
#include "conf_comm.h"
#include "conf_map.h"
#include "conf_ring.h"
#include "obring.h"

#include "cli_util.h"

extern u8 NMS_TxBuffer[];

void Rsp_SetRingConfig(u8 *DMA, u8 *RequestID,u8 *RingConfig)
{
	POBNET_SET_RING_CONFIG pRingCfg = (POBNET_SET_RING_CONFIG)RingConfig;
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	tRingNmsConfigRec *pstRingNmsCfgRec;
	tRingConfigGlobal RingGlobalCfg;
	tRingConfigRec RingRecCfg;
	unsigned int RingGate;
	unsigned char RecIndex;
	char DomainName[8];
	int ret;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/**************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_RING_CFG;
	
#if MODULE_RING
	if((pRingCfg->RingNum > 0) && (pRingCfg->RingNum <= MAX_RING_NUM)) {
		
		RingGlobalCfg.ucGlobalEnable = 0x01;
		RingGlobalCfg.ucRecordNum = pRingCfg->RingNum;
		ret = conf_set_ring_global(&RingGlobalCfg);
		if(ret != CONF_ERR_NONE) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;
			goto ErrorSetRing;
		} 

		RingGate = ntohl(*(unsigned int *)(&(pRingCfg->RingGate[0])));
		for(RecIndex = 0; RecIndex < pRingCfg->RingNum; RecIndex++) {
			memset(&RingRecCfg, 0, sizeof(tRingConfigRec));
			pstRingNmsCfgRec = (tRingNmsConfigRec *)&(pRingCfg->RingConfig[RecIndex * 16]);
			RingRecCfg.ucRingIndex = RecIndex;
			RingRecCfg.ucEnable = (unsigned char)((RingGate >> RecIndex) & 0x01);
			RingRecCfg.usDomainId[0] = (unsigned char)(((RecIndex + 1) & 0xFF00) >> 8);
			RingRecCfg.usDomainId[1] = (unsigned char)((RecIndex + 1) & 0x00FF);	
			sprintf(DomainName,"DN%05d", RecIndex + 1);
			memcpy(RingRecCfg.ucDomainName, DomainName, 8);
			RingRecCfg.usRingId[0] = (unsigned char)(((RecIndex + 1) & 0xFF00) >> 8);
			RingRecCfg.usRingId[1] = (unsigned char)((RecIndex + 1) & 0x00FF);	
			RingRecCfg.ucPrimaryPort = pstRingNmsCfgRec->stPrimaryPortConfig.ucRingPort;
			RingRecCfg.ucSecondaryPort = pstRingNmsCfgRec->stSecondaryPortConfig.ucRingPort;
			RingRecCfg.ucRingMode = pstRingNmsCfgRec->stPrimaryPortConfig.ucRingMode;
			if(RingRecCfg.ucRingMode == PORT_FAST_MODE) {
				RingRecCfg.ucNodePrio = PRIO_LOW;
				RingRecCfg.usAuthTime[0] = (unsigned char)((DEFAULT_AUTH_TIME  & 0xFF00) >> 8);
				RingRecCfg.usAuthTime[1] = (unsigned char)(DEFAULT_AUTH_TIME & 0x00FF);					
				RingRecCfg.usBallotTime[0] = (unsigned char)((DEFAULT_BALLOT_TIME  & 0xFF00) >> 8);
				RingRecCfg.usBallotTime[1] = (unsigned char)(DEFAULT_BALLOT_TIME & 0x00FF);	
				RingRecCfg.usHelloTime[0] = (unsigned char)((DEFAULT_HELLO_TIME  & 0xFF00) >> 8);
				RingRecCfg.usHelloTime[1] = (unsigned char)(DEFAULT_HELLO_TIME & 0x00FF);	
				RingRecCfg.usFailTime[0] = (unsigned char)((DEFAULT_FAIL_TIME & 0xFF00) >> 8);
				RingRecCfg.usFailTime[1] = (unsigned char)(DEFAULT_FAIL_TIME & 0x00FF);	
			} else {
				RingRecCfg.ucNodePrio = pstRingNmsCfgRec->stPrimaryPortConfig.ucNodePrio;
				RingRecCfg.usAuthTime[0] = (unsigned char)((DEFAULT_AUTH_TIME  & 0xFF00) >> 8);
				RingRecCfg.usAuthTime[1] = (unsigned char)(DEFAULT_AUTH_TIME & 0x00FF);	
				
				if(pstRingNmsCfgRec->stPrimaryPortConfig.ucBallotTime < DEFAULT_BALLOT_TIME) {
					RingRecCfg.usBallotTime[0] = (unsigned char)((DEFAULT_BALLOT_TIME  & 0xFF00) >> 8);
					RingRecCfg.usBallotTime[1] = (unsigned char)(DEFAULT_BALLOT_TIME & 0x00FF);	
				} else {
					RingRecCfg.usBallotTime[0] = 0x00;
					RingRecCfg.usBallotTime[1] = pstRingNmsCfgRec->stPrimaryPortConfig.ucBallotTime;
				}
				
				if(pstRingNmsCfgRec->stPrimaryPortConfig.ucHelloTime < DEFAULT_HELLO_TIME) {
					RingRecCfg.usHelloTime[0] = (unsigned char)((DEFAULT_HELLO_TIME  & 0xFF00) >> 8);
					RingRecCfg.usHelloTime[1] = (unsigned char)(DEFAULT_HELLO_TIME & 0x00FF);		
				} else {
					RingRecCfg.usHelloTime[0] = 0x00;
					RingRecCfg.usHelloTime[1] = pstRingNmsCfgRec->stPrimaryPortConfig.ucHelloTime;
				}

				if(pstRingNmsCfgRec->stPrimaryPortConfig.ucFailTime < DEFAULT_FAIL_TIME) {
					RingRecCfg.usFailTime[0] = (unsigned char)((DEFAULT_FAIL_TIME & 0xFF00) >> 8);
					RingRecCfg.usFailTime[1] = (unsigned char)(DEFAULT_FAIL_TIME & 0x00FF);	
				} else {
					RingRecCfg.usFailTime[0] = 0x00;
					RingRecCfg.usFailTime[1] = pstRingNmsCfgRec->stPrimaryPortConfig.ucFailTime;
				}
			} 
			ret = conf_set_ring_record(RecIndex, &RingRecCfg);
			if(ret != CONF_ERR_NONE) {
				RspSet.RetCode = 0x01;
				RspSet.Res = RSP_ERR_EEPROM_OPERATION;
				goto ErrorSetRing;
			}
		}
		RspSet.RetCode = 0x00;
		RspSet.Res = 0x00;
	} else {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_FEATURE_NOT_SUPPORT;
	}
#else
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_FEATURE_NOT_SUPPORT;
#endif

ErrorSetRing:
	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}

void Rsp_GetRingConfig(u8 *DMA, u8 *RequestID)
{
	OBNET_RSP_RING_CONFIG RspRingConfig;	
	u16 RspLength;
	tRingNmsConfigInfo	RingNmsCfg;
	tRingNmsConfigRec *pstRingNmsConfigRec;
	tRingConfigGlobal RingGlobalCfg;
	tRingConfigRec RingRecCfg;
	unsigned int RingGate;
	unsigned char RecIndex;	
	unsigned char RingNum;
	int ret;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	RingNum = 1;
	memset(&RspRingConfig, 0, sizeof(OBNET_RSP_RING_CONFIG));
	RspRingConfig.GetCode = CODE_RING_CONFIG;

#if MODULE_RING
	ret = conf_get_ring_global(&RingGlobalCfg);
	if(ret != CONF_ERR_NONE) {
		RspRingConfig.RetCode = 0x01;
		goto ErrorGetRing;
	} else {
		if((RingGlobalCfg.ucGlobalEnable != 0x01) || (RingGlobalCfg.ucRecordNum == 0) || (RingGlobalCfg.ucRecordNum > MAX_RING_NUM)) {
			RspRingConfig.RetCode = 0x01;
			goto ErrorGetRing;
		} else {
			RspRingConfig.RingNum = RingGlobalCfg.ucRecordNum;
			RingNum = RingGlobalCfg.ucRecordNum;
			RingGate = 0;
			for(RecIndex = 0; RecIndex < RingGlobalCfg.ucRecordNum; RecIndex++) {
				pstRingNmsConfigRec = (tRingNmsConfigRec *)&(RspRingConfig.RingConfig[RecIndex*16]);
				ret = conf_get_ring_record(RecIndex, &RingRecCfg);
				if(ret != CONF_ERR_NONE) {
					memset(&RspRingConfig, 0, sizeof(OBNET_RSP_RING_CONFIG));
					RspRingConfig.GetCode = CODE_RING_CONFIG;					
					RspRingConfig.RetCode = 0x01;
					goto ErrorGetRing;
				}
				if(RingRecCfg.ucEnable == 0x01)
					RingGate |= 1<<RecIndex;
				
				pstRingNmsConfigRec->stPrimaryPortConfig.ucRingPort = RingRecCfg.ucPrimaryPort;
				pstRingNmsConfigRec->stPrimaryPortConfig.ucRingMode = RingRecCfg.ucRingMode;
				pstRingNmsConfigRec->stPrimaryPortConfig.ucNodePrio = RingRecCfg.ucNodePrio;
				pstRingNmsConfigRec->stPrimaryPortConfig.ucRes2 = 0x00;
				pstRingNmsConfigRec->stPrimaryPortConfig.ucHelloTime = RingRecCfg.usHelloTime[1];
				pstRingNmsConfigRec->stPrimaryPortConfig.ucBallotTime = RingRecCfg.usBallotTime[1];
				pstRingNmsConfigRec->stPrimaryPortConfig.ucFailTime = RingRecCfg.usFailTime[1];
				pstRingNmsConfigRec->stPrimaryPortConfig.ucRes3 = 0x00;
				pstRingNmsConfigRec->stSecondaryPortConfig.ucRingPort = RingRecCfg.ucSecondaryPort;
				pstRingNmsConfigRec->stSecondaryPortConfig.ucRingMode = RingRecCfg.ucRingMode;
				pstRingNmsConfigRec->stSecondaryPortConfig.ucNodePrio = RingRecCfg.ucNodePrio;
				pstRingNmsConfigRec->stSecondaryPortConfig.ucRes2 = 0x00;
				pstRingNmsConfigRec->stSecondaryPortConfig.ucHelloTime = RingRecCfg.usHelloTime[1];
				pstRingNmsConfigRec->stSecondaryPortConfig.ucBallotTime = RingRecCfg.usBallotTime[1];
				pstRingNmsConfigRec->stSecondaryPortConfig.ucFailTime = RingRecCfg.usFailTime[1];
				pstRingNmsConfigRec->stSecondaryPortConfig.ucRes3 = 0x00;		
			}
			RspRingConfig.RingGate[0] = (unsigned char)((RingGate & 0xFF000000) >> 24);
			RspRingConfig.RingGate[1] = (unsigned char)((RingGate & 0x00FF0000) >> 16);
			RspRingConfig.RingGate[2] = (unsigned char)((RingGate & 0x0000FF00) >> 8);
			RspRingConfig.RingGate[3] = (unsigned char)(RingGate & 0x000000FF);
			RspRingConfig.RetCode = 0x00;
		}
	}
#else
		RspRingConfig.RetCode = 0x01;
#endif

ErrorGetRing:
	/************************************************/
	/* prepare the data to send */	
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspRingConfig, 7 + 16 * RingNum);
	RspLength = PAYLOAD_OFFSET + 7 + 16 * RingNum;//sizeof(OBNET_RSP_RING_CONFIG);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);
}


void GetRingPortStatus(u8 RingLogicPort, POBNET_RING_PORT_INFO pRingPortInfo)
{
#if 0
	DevNode_t *pNode = Ring_GetNode();
	u8 port_status;

	pRingPortInfo->DevPortNum = pNode->PortInfo[RingLogicPort].DevPortNum;
	memcpy(pRingPortInfo->ExtNeighborMac, pNode->PortInfo[RingLogicPort].ExtNeighborMac, MAC_LEN);
	pRingPortInfo->ExtNeighborPortNum = pNode->PortInfo[RingLogicPort].ExtNeighborPortNum;

	if(pNode->PortInfo[RingLogicPort].LinkState == LINK_DOWN) {
		port_status = 0x1;
	} else {
		if(pNode->PortInfo[RingLogicPort].StpState == FORWARDING)
			port_status = 0x3;
		else if(pNode->PortInfo[RingLogicPort].StpState == BLOCKING)
			port_status = 0x2;
		else
			port_status = 0x0;
	}

	pRingPortInfo->Flag = (pNode->RingEnable << 7) | (pNode->PortInfo[RingLogicPort].RingFlag << 6) | port_status;
#else
	extern tRingInfo RingInfo;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	unsigned char RingIndex;
	unsigned char LportIndex;
	u8 PortStatus;
	
	RingIndex = obring_get_ring_idx_by_port(RingLogicPort);
	if(RingIndex == 0xFF)
		return;
	
	pRingConfig = &RingInfo.RingConfig[RingIndex];
	pRingState = &RingInfo.DevState[RingIndex];

	LportIndex = obring_get_port_idx_by_port(pRingConfig, RingLogicPort);
	pRingPortInfo->DevPortNum = RingLogicPort;
	memcpy(pRingPortInfo->ExtNeighborMac, pRingState->PortState[LportIndex].NeighborMac, MAC_LEN);
	pRingPortInfo->ExtNeighborPortNum = pRingState->PortState[LportIndex].NeighborPortNo;

	if(pRingState->PortState[LportIndex].LinkState == LINK_DOWN) {
		PortStatus = 0x01;
	} else {
		if(pRingState->PortState[LportIndex].StpState == FORWARDING)
			PortStatus = 0x3;
		else if(pRingState->PortState[LportIndex].StpState == BLOCKING)
			PortStatus = 0x2;
		else
			PortStatus = 0x0;
	}
	if(pRingConfig->ucEnable == 0x01)
		PortStatus |= 0x80;

	if(pRingState->RingState == RING_HEALTH)
		PortStatus |= 0x40;	

	if(pRingState->NodeType == NODE_TYPE_MASTER)
		PortStatus |= 0x20;	
	
	pRingPortInfo->Flag = PortStatus;
	
#endif
}

void Rsp_Topo(u8 *DMA, u8 *RequestID)
{
	extern dev_base_info_t	DeviceBaseInfo;

	OBNET_RSP_TOPO RspTopo;
	u16 RspLength;
	u8 ring_num;
	u8 i;
#if MODULE_RING	
	extern tRingInfo RingInfo;
	tRingConfigRec *pRingConfig;
	tRingState *pRingState;
	unsigned char RingIndex;
	unsigned char LportIndex;
	unsigned char PrimaryPort, SecondaryPort;
#endif

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	memset(&RspTopo, 0, sizeof(OBNET_RSP_TOPO));
	RspTopo.GetCode = CODE_TOPO;
	RspTopo.RetCode = 0x00;
	RspTopo.Res = 0x00;
	RspTopo.SwitchType[0] = DeviceBaseInfo.BoardType[0];
	RspTopo.SwitchType[1] = DeviceBaseInfo.BoardType[1];
	RspTopo.SwitchType[2] = DeviceBaseInfo.BoardType[2];
	RspTopo.SwitchType[3] = DeviceBaseInfo.BoardType[3];
	RspTopo.SwitchType[4] = DeviceBaseInfo.BoardType[4];
	RspTopo.SwitchType[5] = DeviceBaseInfo.BoardType[5];
	RspTopo.SwitchType[6] = DeviceBaseInfo.BoardType[6];
	RspTopo.SwitchType[7] = DeviceBaseInfo.BoardType[7];
    RspTopo.PowerNum = 1;
    RspTopo.PowerVol[0] = 5;
    RspTopo.PowerVol[1] = 0;
    RspTopo.PowerVol[2] = 0;
    RspTopo.PowerVol[3] = 0;

#if MODULE_RING	
	if(conf_get_ring_num(&ring_num) != CONF_ERR_NONE) {
		RspTopo.RingPairNum = 0;
		RspTopo.RetCode = 0x01;
		RspTopo.Res = RSP_ERR_EEPROM_OPERATION;		/* I2C read error */
	} else {
		if(ring_num > 1) {
			RspTopo.RetCode = 0x01;
			RspTopo.Res = 0x0F;
		} else {
			RspTopo.RingPairNum = ring_num;
			for(i=0; i<ring_num; i++) {
				pRingConfig = &RingInfo.RingConfig[i];
				pRingState = &RingInfo.DevState[i];	
				
				PrimaryPort = pRingConfig->ucPrimaryPort;
				SecondaryPort = pRingConfig->ucSecondaryPort;
				GetRingPortStatus(PrimaryPort, &(RspTopo.Info[0]));
				GetRingPortStatus(SecondaryPort, &(RspTopo.Info[1]));
			}
		}
	}
#else
	RspTopo.GetCode = CODE_TOPO;
	RspTopo.RetCode = 0x00;
	RspTopo.Res = 0x00;
	RspTopo.RingPairNum = 0;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspTopo, sizeof(OBNET_RSP_TOPO));
	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_TOPO);
	if(RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE + SWITCH_TAG_LEN;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	RspSend(NMS_TxBuffer, RspLength);	
}


