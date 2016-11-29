
/*************************************************************
 * Filename     : oam_port.c
 * Description  : API for OAM port
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

#include "mconfig.h"

/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* LwIP includes */
#include "lwip/inet.h"

#include "os_mutex.h"
#include "ssi_list.h"

#include "m88e6095_if.h"

#include "msApi.h"
#include "oam_port.h"
#include "conf_port.h"

extern GT_QD_DEV *dev;

int gMacLimitCount[MAX_PORT_NUM] = {0};
OS_MUTEX_T MacMutex; 
SSI_LIST_HEAD MacListHeader;



GT_BOOL MacIsEqual(GT_U8 *MacAddr1,GT_U8 *MacAddr2)
{
	int i;
	GT_BOOL ret = GT_TRUE;

	for(i=0;i<6;i++) {
		if(MacAddr1[i] != MacAddr2[i]) {
			ret = GT_FALSE;
			break;
		}
	}
	return ret;
}

GT_BOOL MacIsGreater(GT_U8 *MacAddr1, GT_U8 *MacAddr2)
{
	int i;
	GT_BOOL bret = GT_TRUE;
	
	for(i=0;i<6;i++) {
		if(MacAddr1[i] > MacAddr2[i]) {
			bret = GT_TRUE;
			break;
		} else if( MacAddr1[i] == MacAddr2[i]) {
			continue;
		} else {
			bret = GT_FALSE;
			break;
		}
	}
	
	return bret;
}


GT_STATUS MacAdd(oam_mac_list_t *OamMacCfg)
{
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;

	memset(&macEntry,0,sizeof(GT_ATU_ENTRY));	
	macEntry.macAddr.arEther[0] = OamMacCfg->MacAddr[0];
	macEntry.macAddr.arEther[1] = OamMacCfg->MacAddr[1];
	macEntry.macAddr.arEther[2] = OamMacCfg->MacAddr[2];
	macEntry.macAddr.arEther[3] = OamMacCfg->MacAddr[3];
	macEntry.macAddr.arEther[4] = OamMacCfg->MacAddr[4];
	macEntry.macAddr.arEther[5] = OamMacCfg->MacAddr[5];	

	macEntry.DBNum = 0;
	macEntry.portVec = ntohs(*(GT_U32 *)(&OamMacCfg->PortVec[0]));
	macEntry.prio = OamMacCfg->Priority;
	macEntry.entryState.ucEntryState = GT_UC_STATIC;

	if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK) {
		return status;
	}

	return GT_OK;
}

GT_STATUS MacAdd2(GT_U8 mac0,GT_U8 mac1,GT_U8 mac2,GT_U8 mac3,GT_U8 mac4,GT_U8 mac5,GT_U32 portList,GT_U8 priority)
{
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;

	memset(&macEntry,0,sizeof(GT_ATU_ENTRY));	
	macEntry.macAddr.arEther[0] = mac0;
	macEntry.macAddr.arEther[1] = mac1;
	macEntry.macAddr.arEther[2] = mac2;
	macEntry.macAddr.arEther[3] = mac3;
	macEntry.macAddr.arEther[4] = mac4;
	macEntry.macAddr.arEther[5] = mac5;	

	macEntry.DBNum = 0;
	macEntry.portVec = portList;
	macEntry.prio = priority;
	macEntry.entryState.ucEntryState = GT_UC_STATIC;

	if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK) {
		return status;
	}

	return GT_OK;
}

GT_STATUS MacListInit(void)
{
	if(os_mutex_init(&MacMutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return GT_ERROR;
	}
	
    ssi_list_init(&MacListHeader);
	
	return GT_OK;
}

GT_STATUS MacListAdd(oam_mac_list_t *OamMacCfg)
{
	GT_STATUS status = GT_OK;
	oam_mac_list_t	*pMac, *pMacNew;
	GT_U32 portvec_tmp1, portvec_tmp2;
	GT_BOOL bAdd = GT_FALSE;
	int i;
	
	if(ssi_list_count(&MacListHeader) > MAX_MAC_LIST_COUNT)
		return GT_ERROR;

	if(OamMacCfg == NULL)
		return GT_ERROR;

    /*******************************************************
	          Don't insert a exiting one
	 *******************************************************/
	os_mutex_lock(&MacMutex, OS_MUTEX_WAIT_FOREVER);
	
	for(pMac = (oam_mac_list_t *)ssi_list_first(&MacListHeader); pMac != NULL; pMac = (oam_mac_list_t *)ssi_list_next(&pMac->Node)) {
		if(MacIsEqual(pMac->MacAddr, OamMacCfg->MacAddr)) {
			portvec_tmp1 = ntohl(*(GT_U32 *)(&pMac->PortVec[0]));
			portvec_tmp2 = ntohl(*(GT_U32 *)(&OamMacCfg->PortVec[0]));
			if((portvec_tmp2 ^ portvec_tmp2) == 0) {
				os_mutex_unlock(&MacMutex);
				return GT_ALREADY_EXIST;
			}
			portvec_tmp1 |= portvec_tmp2;
			*(GT_U32 *)(&pMac->PortVec[0]) = htonl(portvec_tmp1);
			if(MacAdd(pMac) != GT_OK) {
				os_mutex_unlock(&MacMutex);
				return GT_FAIL;
			}
			os_mutex_unlock(&MacMutex);
			return GT_OK;
		}
	}
	pMacNew = (oam_mac_list_t *)pvPortMalloc(sizeof(oam_mac_list_t));
	if(pMacNew == NULL) {
		os_mutex_unlock(&MacMutex);
		printf("MacListAdd malloc memory is failed.\r\n");
		return GT_ERROR;
	}
	memset(pMacNew,0,sizeof(oam_mac_list_t));

    /*******************************************************
	          Insert the new one
	 *******************************************************/	
	memcpy(pMacNew->MacAddr, OamMacCfg->MacAddr, 6);
	memcpy(pMacNew->PortVec, OamMacCfg->PortVec, 4);
	pMacNew->Priority	= OamMacCfg->Priority;

	if(MacAdd(OamMacCfg) == GT_OK) {
		for(pMac = (oam_mac_list_t *)ssi_list_last(&MacListHeader),i=0; pMac != NULL; pMac = (oam_mac_list_t *)ssi_list_prev(&pMac->Node),i++) {
			if(MacIsGreater(OamMacCfg->MacAddr,pMac->MacAddr)) {
				ssi_list_insert(&MacListHeader, (SSI_LIST_NODE *)pMac,(SSI_LIST_NODE *)pMacNew);
				bAdd = GT_TRUE;
				status = GT_OK;
			}
		}
		if(!bAdd) {
			ssi_list_insert(&MacListHeader,NULL, (SSI_LIST_NODE *)pMacNew);
			status = GT_OK;	
		}
	} else {
		vPortFree(pMacNew);
		status = GT_FALSE;	
	}
	
	os_mutex_unlock(&MacMutex);

	return status;
}

GT_U32 MacListCount(SSI_LIST_HEAD* pMacList)
{
	return ssi_list_count(&MacListHeader);
}

GT_STATUS MacListReAdd(GT_U8 port)
{
	oam_mac_list_t  *pMac;	
	GT_STATUS nRet = GT_FALSE;
	GT_U32 portlist = 0; 

	os_mutex_lock(&MacMutex, OS_MUTEX_WAIT_FOREVER);
	for(pMac = (oam_mac_list_t *)ssi_list_first(&MacListHeader); pMac != NULL; pMac = (oam_mac_list_t *)ssi_list_next(&pMac->Node)) {
		portlist = ntohl(*(GT_U32 *)(&pMac->PortVec[0]));
		nRet = MacAdd2(pMac->MacAddr[0], pMac->MacAddr[1], pMac->MacAddr[2], 
					  pMac->MacAddr[3], pMac->MacAddr[4], pMac->MacAddr[5], 
					  portlist, pMac->Priority);
		if(nRet != GT_OK)
			return nRet;
	}
	os_mutex_unlock(&MacMutex);
	return nRet;
}

GT_STATUS MacLimitCountSet(GT_U8 count, GT_U32 logic_port)
{
	GT_STATUS status;
	GT_BOOL modeForwardUnknown;
	GT_BOOL modeLockedPort;
	GT_BOOL modeDropOnLock;
	GT_U32	port;
	oam_mac_list_t	maclist_node;
	
	if(gMacLimitCount[logic_port] != (int)count) {		
		if(count == 0) {
			modeForwardUnknown = GT_TRUE;
			modeLockedPort = GT_FALSE;
			modeDropOnLock = GT_FALSE;
		} else {		
			modeForwardUnknown = GT_FALSE;
			modeLockedPort = GT_TRUE;
			modeDropOnLock = GT_TRUE;
		}

		port = swif_Lport_2_Port(logic_port);
		
		if((status = gprtSetForwardUnknown(dev,port,modeForwardUnknown)) != GT_OK) {
			return GT_FAIL;
		}
		if((status = gprtSetLockedPort(dev,port,modeLockedPort)) != GT_OK) {
			return GT_FAIL;
		}
		if((status = gprtSetDropOnLock(dev,port,modeDropOnLock)) != GT_OK) {
			return GT_FAIL;
		}

		gMacLimitCount[logic_port] = 0;	

		gfdbRemovePort(dev,GT_FLUSH_ALL,port);


		MacAdd2(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7FF, 0);

		MacListReAdd(logic_port);
		
		gMacLimitCount[logic_port] = (int)count;			
	}

	return GT_OK;
}







