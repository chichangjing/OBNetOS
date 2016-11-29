
#include "mconfig.h"
#if MARVELL_SWITCH

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* BSP include */
#include "misc_drv.h"

/* Marvell SDK includes */
#include <Copyright.h>
#include <msApi.h>
#include "msTypes.h"
#include "gtHwCntl.h"
#include <gtDrvConfig.h>
#include "msApiPrototype.h"

/* Others includes */
#include "oam_port.h"
#include "hal_swif_port.h"
#include "nms_global.h"

/* Stm32 Driver */
#include "stm32f2x7_smi.h"

#define MANUAL_MODE

GT_SYS_CONFIG   obSwitchCfg;
GT_QD_DEV       obSwitchDev;
GT_QD_DEV       *dev=&obSwitchDev;

void qdStatus(void);
extern unsigned char DevMac[];
extern hal_port_map_t gHalPortMap[];

	
int obSwitch_start(void)
{
	GT_STATUS status;
	GT_PHY_INFO	phyInfo;
	GT_U32	tmpLimit;
	GT_BOOL mode;
	GT_LPORT  hport;
	GT_U16	u16Data;
	extern dev_base_info_t	DeviceBaseInfo;
	
	/* Register all the required functions to QuarterDeck Driver. */
	memset((char*)&obSwitchCfg,0,sizeof(GT_SYS_CONFIG));
	memset((char*)&obSwitchDev,0,sizeof(GT_QD_DEV));

	obSwitchCfg.BSPFunctions.readMii   = obReadMii;
	obSwitchCfg.BSPFunctions.writeMii  = obWriteMii;
#ifdef USE_SEMAPHORE
	obSwitchCfg.BSPFunctions.semCreate = osSemCreate;
	obSwitchCfg.BSPFunctions.semDelete = osSemDelete;
	obSwitchCfg.BSPFunctions.semTake   = osSemWait;
	obSwitchCfg.BSPFunctions.semGive   = osSemSignal;
#else
	obSwitchCfg.BSPFunctions.semCreate = NULL;
	obSwitchCfg.BSPFunctions.semDelete = NULL;
	obSwitchCfg.BSPFunctions.semTake   = NULL;
	obSwitchCfg.BSPFunctions.semGive   = NULL;
#endif

	obSwitchCfg.initPorts = GT_FALSE;					/* Not init ports, it will be initialize in funtion swif_Initialize() */
	obSwitchCfg.skipInitSetup = GT_SKIP_INIT_SETUP;		/* Skip init setup, pls refer to funtion swif_Initialize() */
	obSwitchCfg.cpuPortNum = CONFIG_SWITCH_CPU_PORT;
#ifdef MANUAL_MODE										/* not defined. this is only for sample */
	/* user may want to use this mode when there are two QD switchs on the same MII bus. */
	obSwitchCfg.mode.scanMode = SMI_MANUAL_MODE;		/* Use QD located at manually defined base addr */
	obSwitchCfg.mode.baseAddr = 0x0;					/* valid value in this case is either 0 or 0x10 */
#else
#ifdef MULTI_ADDR_MODE
	obSwitchCfg.mode.scanMode = SMI_MULTI_ADDR_MODE;	/* find a QD in indirect access mode */
	obSwitchCfg.mode.baseAddr = 0x10;						/* this is the phyAddr used by QD family device. 
															Valid value are 1 ~ 31.*/
#else
	obSwitchCfg.mode.scanMode = SMI_AUTO_SCAN_MODE;		/* Scan 0 or 0x10 base address to find the QD */
	obSwitchCfg.mode.baseAddr = 0;
#endif
#endif

	if((status=qdLoadDriver(&obSwitchCfg, dev)) != GT_OK)
	{
		printf("qdLoadDriver return Failed\r\n");
		return status;
	}


#if 0
	if(MacAdd2(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7FF, 0) != GT_OK)
		printf("Add broadcast mac failed\r\n");
	if(MacAdd2(DevMac[0], DevMac[1], DevMac[2], DevMac[3], DevMac[4], DevMac[5], 0x4, 7) != GT_OK)
		printf("Add cpu mac failed\r\n");
#endif

	{
		GT_ATU_ENTRY macEntry;

		memset(&macEntry,0,sizeof(GT_ATU_ENTRY));	
		macEntry.macAddr.arEther[0] = DevMac[0];
		macEntry.macAddr.arEther[1] = DevMac[1];
		macEntry.macAddr.arEther[2] = DevMac[2];
		macEntry.macAddr.arEther[3] = DevMac[3];
		macEntry.macAddr.arEther[4] = DevMac[4];
		macEntry.macAddr.arEther[5] = DevMac[5];
		macEntry.DBNum = 0;
		macEntry.portVec = (CONFIG_SWITCH_CPU_PORT==0 ? 0x01 : 0x01 << CONFIG_SWITCH_CPU_PORT);
		macEntry.prio = 7;
		macEntry.entryState.ucEntryState = GT_UC_TO_CPU_STATIC;//GT_UC_STATIC;

		if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK) {
			return status;
		}	

		memset(&macEntry,0,sizeof(GT_ATU_ENTRY));	
		macEntry.macAddr.arEther[0] = 0xFF;
		macEntry.macAddr.arEther[1] = 0xFF;
		macEntry.macAddr.arEther[2] = 0xFF;
		macEntry.macAddr.arEther[3] = 0xFF;
		macEntry.macAddr.arEther[4] = 0xFF;
		macEntry.macAddr.arEther[5] = 0xFF;
		macEntry.DBNum = 0;
		macEntry.portVec = 0x7FF;
		macEntry.prio = 0;
		macEntry.entryState.ucEntryState = GT_UC_STATIC;

		if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK) {
			return status;
		}
	}

	/* Set cpu port to highest priority */
	if((status = gcosSetPortDefaultTc(dev,dev->cpuPortNum,7)) != GT_OK) {
        printf("Error: gcosSetPortDefaultTc failed\r\n");
		return status;
    }

#if 0
	/* Limit CPU port ingress/egress rate */
	if((status = grcSetEgressRate(dev, dev->cpuPortNum, GT_2M)) != GT_OK) {
        return status;
    }
#endif
	switch(dev->deviceId) {
		case GT_88E6095:
		/* Multicast frames with unknown DAs do not egress from cpu port */
	    if((status = gprtSetDefaultForward(dev, dev->cpuPortNum, GT_FALSE)) != GT_OK) {
	        printf("Error: gprtSetDefaultForward failed\r\n");
	        return status;
	    }

		/* Unicast frames with unknown DAs will not egress out of cpu port */
	    if((status = gprtSetForwardUnknown(dev, dev->cpuPortNum, GT_FALSE)) != GT_OK) {
	        printf("Error: gprtSetForwardUnknown failed\r\n");
	        return status;
	    }
		break;

		case GT_88E6097:
	    if((status = hwSetGlobal2RegField(dev, QD_REG_MANAGEMENT, 12, 1, 0x00)) != GT_OK) {
	        printf("Error: hwSetPortRegField failed\r\n");
	        return status;
	    }

		for(hport=0; hport<dev->numOfPorts; hport++) {
			if(hport != dev->cpuPortNum) {
				/* Egress all frames with unknown DA */
			    if((status = hwSetPortRegField(dev, hport, QD_REG_PORT_CONTROL, 2, 2, 0x3)) != GT_OK) {
			        printf("Error: hwSetPortRegField failed\r\n");
			        return status;
			    }
			}
		}
		
		/* Multicast or unicast frames with unknown DAs do not egress from cpu port */
	    if((status = hwSetPortRegField(dev, dev->cpuPortNum, QD_REG_PORT_CONTROL, 2, 2, 0x00)) != GT_OK) {
	        printf("Error: hwSetPortRegField failed\r\n");
	        return status;
	    }		
		break;

		default:
		break;
	}

	/* LED select LINK mode */
	if((status=hwSetPhyRegField(dev, 6, 0x16, 0, 4, 5)) != GT_OK) {
        printf("Error: hwSetPhyRegField failed\r\n");
		return status;
    }

	for(hport=0; hport<=7; hport++) {
		if(hport != dev->cpuPortNum)
			gprtPortRestartAutoNeg(dev, hport);
	}
	
#if BOARD_GE22103MA
	phyInfo.phyId = IS_CONFIGURABLE_PHY(dev,8);
	if((phyInfo.phyId & PHY_MODEL_MASK) != DEV_E1112) {
		
#if LPORT3_FORCE_1000M_FULL_LINKUP
		smi_getregfield(PHYADDR_PORT(8), SW_REG_PCS_CONTROL,0,16,&u16Data);
		u16Data &= 0xFFC0;
    	u16Data |= 0x3E;
		smi_setregfield(PHYADDR_PORT(8), SW_REG_PCS_CONTROL,0,16,u16Data);
#else		
		gHalPortMap[2].hport = 4;
		gHalPortMap[2].port_type = S100M_CABLE;
		DeviceBaseInfo.PortNum = 6;
#endif		
		
	} else {
		gprtPortRestartAutoNeg(dev, 8);
		
	}
#elif BOARD_GV3S_HONUE_QM
    
#endif

	/* Start the QuarterDeck */
	if((status=sysEnable(dev)) != GT_OK) {
		printf("sysConfig return Failed\n");
		return status;
	}

	return GT_OK;
}


GT_STATUS obSwitch_qdInit(void)
{
	int	 status = GT_OK;	
        
        qdStatus();
	return status;    
}

static const char* qdPortStpStates[] = {"Disable","Blocking","Learning","Forwarding"};	

static char* qdPortListToStr(GT_LPORT* portList, int portListNum, char* portListStr)
{
	int	port, idx, strIdx=0;
	
	for(idx=0; idx<portListNum; idx++) {
		port = portList[idx];
		sprintf(&portListStr[strIdx], "%d,", port);
		strIdx = strlen(portListStr);
	}
	portListStr[strIdx] = '\0';
	
	return portListStr;
}

void qdStatus(void)
{
	int 				port;
	GT_BOOL				linkState;
	GT_PORT_STP_STATE 	stpState;
	GT_PORT_STAT    	counters;
	GT_U16				pvid;
	GT_LPORT 			portList[11];
    GT_U8    			portNum;
	char				portListStr[100];


	MSG_PRINT(("Port  Link   PVID    State       RxCntr      TxCntr\n"));
	MSG_PRINT(("----------------------------------------------------\n"));
    for (port=0; port<11; port++) {
		gprtGetLinkState(dev, port, &linkState);
		gstpGetPortState(dev, port, &stpState);
		gprtGetPortCtr(dev,port, &counters);
		gstpGetPortState(dev, port, &stpState);
		gvlnGetPortVid(dev, port, &pvid);
		gvlnGetPortVlanPorts(dev, port, portList, &portNum);
		qdPortListToStr(portList, portNum, portListStr);

		MSG_PRINT((" %02d.  %4s    %d      %-10s   0x%-8x  0x%-8x\n",
					port, (linkState==GT_TRUE) ? "Up" : "Down",
					pvid, qdPortStpStates[stpState],
					counters.rxCtr, counters.txCtr));		
	}
}

GT_STATUS SwitchIntEnable(void)
{
	GT_STATUS status;
	GT_U8 lport, hwport;
	GT_U16 data;

	/* Enable interrupt for PHYInt.*/
	data = GT_PHY_INTERRUPT;
	if((status = eventSetActive(dev,data)) != GT_OK) {
		printf("eventSetActive returned failed\r\n");
		return status;
	}

	/* Enable Phy interrupt for every possible interrupt cause. */
	data = GT_LINK_STATUS_CHANGED;//GT_SPEED_CHANGED | GT_DUPLEX_CHANGED | 
	for(lport=1; lport<=MAX_PORT_NUM; lport++) {
		hwport = hal_swif_lport_2_hport(lport);
		if(hwport > 7)
			continue;
		
		if((status = gprtPhyIntEnable(dev,hwport,data)) != GT_OK) {
			printf("gprtPhyIntEnable returned failed\r\n");
			return status;
		}
	}

	return GT_OK;
}

GT_STATUS SwitchIntDisable(void)
{
	GT_STATUS status;
	GT_U8 lport, hwport;
	GT_U16 data;

	/* Disable interrupt .*/
	data = 0;
	if((status = eventSetActive(dev,data)) != GT_OK) {
		printf("eventSetActive returned failed\r\n");
		return status;
	}

	return GT_OK;
}

void IntProcTask(void *arg)
{
	GT_STATUS ret;
	GT_BOOL port_speed, port_duplex, port_link;
	GT_BOOL curLinkStat, oldLinkStat=GT_FALSE;
	HAL_PORT_SPEED_STATE SpeedStat;
	HAL_PORT_DUPLEX_STATE DuplexStat;
	HAL_MDI_MDIX_STATE mdi_mdix;
	GT_U16 intCause, phyIntCause;
	GT_U16 data, u16Data;
	GT_U8 lport, hwport;
	GT_U8 port_status[MAX_PORT_NUM];
	GT_U8 first_status_flag[MAX_PORT_NUM];
	int i;
	
	SwitchIntEnable();
	for(i=0;i<MAX_PORT_NUM;i++) {
		first_status_flag[i] = 1;
	}
	
	while(1) {
#if 0
		if(hwGetPhyRegField(dev, 5, SW_PHY_SPEC_STATUS_REG,6,1,&u16Data) == GT_OK) {
			printf("lport=%d crossover status, %d\r\n", 5, u16Data);
		}
#endif						
		intCause = 0;
		hwGetGlobalRegField(dev,0,0,7,&intCause);
		if(intCause & GT_PHY_INTERRUPT) {
			//SwitchIntDisable();
			
			if(gprtGetPhyIntPortSummary(dev,&data) != GT_OK) {
				data = 0;
				goto finished;
			}
						
			hwport = 0;		
			while(data) {
				if(hwport > 7)
					goto finished;

				if(hwport == dev->cpuPortNum) {
					data >>= 1;
					hwport++;
					continue;
				}

				if(((1<<hwport) & (dev->validPhyVec)) == 0) {
					data >>= 1;
					hwport++;
					continue;
				}
					
				if(data & 0x1) {
					lport = hal_swif_hport_2_lport(hwport);
					if(lport > MAX_PORT_NUM)
						goto finished;
					
					if(gprtGetPhyIntStatus(dev, hwport, &phyIntCause) != GT_OK) {						
						phyIntCause = 0;
					}

					port_status[lport] = 0;
					
					if(phyIntCause & GT_LINK_STATUS_CHANGED) {
						if((ret=gprtGetLinkState(dev, hwport, &port_link)) != GT_OK) {
							printf("Error: gprtGetLinkState, hwport=%d, ret=%d\r\n", hwport, ret);
							goto finished;
						}

						#if 0
						if((ret=gprtGetSpeed(dev, hwport, &port_speed)) != GT_OK) {
							printf("Error: gprtGetSpeed, hwport=%d, ret=%d\r\n", hwport, ret);
							goto finished;
						}
						#else
						/* Read PHY spec register */
						if((ret=hwGetPhyRegField(dev, hwport, 17,14,1, (GT_U16 *)&port_speed)) != GT_OK) {
							printf("Error: hwGetPhyRegField, hwport=%d, ret=%d\r\n", hwport, ret);
							goto finished;
						}	
						#endif
						if((ret=gprtGetDuplex(dev, hwport, &port_duplex)) != GT_OK) {
							printf("Error: gprtGetDuplex, hwport=%d, ret=%d\r\n", hwport, ret);
							goto finished;
						}
						if((ret=hwGetPhyRegField(dev, hwport, 17,6,1, (GT_U16 *)&u16Data)) != GT_OK) {
							printf("Error: hwGetPhyRegField, hwport=%d, ret=%d\r\n", hwport, ret);
							goto finished;
						}				
						port_status[lport] |= ((port_link == GT_TRUE) ? 0x80 : 0x00);
						port_status[lport] |= ((port_speed == GT_TRUE) ? 0x10 : 0x00);
						port_status[lport] |= ((port_duplex == GT_TRUE) ? 0x08 : 0x00);
						port_status[lport] |= ((u16Data == 1) ? 0x02 : 0x00);	/* u16Data=0: MDIX, u16Data=1: MDI */
						
						
						if(!((port_link == GT_FALSE) && (first_status_flag[lport-1] == 1))) {
							#if 1
							if(port_link == GT_TRUE) 
								printf("Port%d Link %s, %s-%s, %s\r\n", lport, (port_link == GT_TRUE)? "Up":"Down", (port_speed == GT_TRUE)? "100M":"10M", (port_duplex == GT_TRUE)?"Full":"Half", (u16Data == 1)?"MDI":"MDIX");
							else
								printf("Port%d Link %s\r\n", lport, (port_link == GT_TRUE)? "Up":"Down");
							#endif
							first_status_flag[lport-1] = 0;
							//TrapSend_PortChange(lport, port_status[lport]);
						}
					}
				}
				data >>= 1;
				hwport++;
			}
			//SwitchIntEnable();
		}
		
finished:

#if (BOARD_GE22103MA)|| (BOARD_GV3S_HONUE_QM)
	curLinkStat = GT_FALSE;
	if(gHalPortMap[2].hport == 8) {
		if((ret=gprtGetPhyLinkStatus(dev, 8, &curLinkStat)) == GT_OK) {
			if(oldLinkStat != curLinkStat) {
				port_status[2] = 0;
				port_status[2] |= ((curLinkStat == GT_TRUE) ? 0x80 : 0x00);
				hal_swif_port_get_speed(gHalPortMap[2].lport, &SpeedStat);
				port_status[2] |= (((u8)SpeedStat) << 4);
				hal_swif_port_get_duplex(gHalPortMap[2].lport, &DuplexStat);
				port_status[2] |= (((u8)SpeedStat) << 3);
				gprtGetPagedPhyReg(dev, 8, 17, 0, &u16Data);
				if(u16Data & 0x0040)
					mdi_mdix = MODE_MDIX;
				else
					mdi_mdix = MODE_MDI;
				
				#if 1
				if(curLinkStat == GT_TRUE) {
					printf("Port%d Link %s, %s-%s, %s\r\n", 3, (curLinkStat == GT_TRUE)? "Up":"Down", 
						(SpeedStat == SPEED_10M)? "10M":(SpeedStat == SPEED_100M)? "100M":(SpeedStat == SPEED_1000M)? "1000M":"Unkown", 
						(DuplexStat == FULL_DUPLEX)?"Full":(DuplexStat == HALF_DUPLEX)? "Half": "Unkown", 
						(mdi_mdix == MODE_MDI)?"MDI":"MDIX");
				} else
					printf("Port%d Link %s\r\n", 3, (curLinkStat == GT_TRUE)? "Up":"Down");
				#endif

				
				//TrapSend_PortChange(2, port_status[2]);
			}
			oldLinkStat = curLinkStat;
		}			
	}
#endif
		vTaskDelay(200);

	}
	
}

#endif