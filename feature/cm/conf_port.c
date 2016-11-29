


/*************************************************************
 * Filename     : conf_port.c
 * Description  : API for port configuration
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

/* Standard includes */
#include "stdio.h"
#include "string.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* LwIP includes */
#include "lwip/inet.h"

/* BSP includes */
#include "stm32f2xx.h"
#include "soft_i2c.h"
#include "conf_comm.h"
#include "conf_map.h"
#include "conf_alarm.h"
#include "conf_port.h"

#include "nms_alarm.h"
#include "ob_ring.h"
#include "hal_swif_port.h"
#if SWITCH_CHIP_88E6095
#include <msApi.h>
#include <gtSem.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>
#include "oam_port.h"
#endif


#if SWITCH_CHIP_88E6095
extern GT_QD_DEV *dev;
GT_BOOL gPortSecurityEn[MAX_PORT_NUM] = {GT_FALSE};
#endif


unsigned char gMacLimitMaximum[MAX_PORT_NUM] = {0};


int conf_port_base_init(void)
{
	u8 i, hport;
	port_conf_t PortConfig;
	u32 single_port_cfg;
	HAL_PORT_SPEED_DUPLEX speed_duplex;
	u16 u16Data;
	int ret;
	
	if(eeprom_read(NVRAM_PORT_CFG_BASE, (u8 *)&PortConfig, sizeof(port_conf_t)) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	if((PortConfig.PortNum == 0) || (PortConfig.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;

	for(i=0; i<PortConfig.PortNum; i++) {
		single_port_cfg = *(u32 *)&(PortConfig.PortConfig[i*4]);
		single_port_cfg = ntohl(single_port_cfg);
		hport = hal_swif_lport_2_hport(i+1);

		if((single_port_cfg & PC_MASK_ENABLE) == 0) {
			hal_swif_port_set_stp_state(i+1, DISABLED);
		}
		
		if((single_port_cfg & PC_MASK_AUTO_NEG) == 0) {
			speed_duplex = (HAL_PORT_SPEED_DUPLEX)(((single_port_cfg & PC_MASK_SPEED_DUPLEX) >> 3) & 0xFF);

#if SWITCH_CHIP_88E6095
			if(hport != 9 && hport != 10) {
				switch(speed_duplex) {
					case S10M_HALF:
					if(hport == 8) {
						/* Disable auto-negotiation, 10M half-duplex */
						if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0x8000)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }						
					} else {
						/* Disable auto-negotiation */
						if((ret = gprtPortAutoNegEnable(dev, hport, GT_FALSE)) != GT_OK) {
				            printf("Error: gprtPortAutoNegEnable, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* LED0 OFF */
						if((ret=hwSetPhyRegField(dev,hport, 0x19, 0, 2, 2)) != GT_OK) 	{
				            printf("Error: hwSetPhyRegField, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* Set speed to 10M bps */
						if((ret = gprtSetPortSpeed(dev, hport, PHY_SPEED_10_MBPS)) != GT_OK) {
							printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }		
						/* Set duplex to half-duplex */
						if((ret = gprtSetPortDuplexMode(dev, hport, GT_FALSE)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }						
					}
					break;

					case S10M_FULL:
					if(hport == 8) {
						/* Disable auto-negotiation, 10M full-duplex */
						if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0x8100)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }	
					} else {
						/* Disable auto-negotiation */
						if((ret = gprtPortAutoNegEnable(dev, hport, GT_FALSE)) != GT_OK) {
				            printf("Error: gprtPortAutoNegEnable, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }					
						/* LED0 OFF */
						if((ret=hwSetPhyRegField(dev,hport, 0x19, 0, 2, 2)) != GT_OK) 	{
				            printf("Error: hwSetPhyRegField, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* Set speed to 10M bps */
						if((ret = gprtSetPortSpeed(dev, hport, PHY_SPEED_10_MBPS)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* Set duplex to full-duplex */
						if((ret = gprtSetPortDuplexMode(dev, hport, GT_TRUE)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
					}						
					break;

					case S100M_HALF:
					if(hport == 8) {
						/* Disable auto-negotiation, 100M half-duplex */
						if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0xa000)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
					} else {
						/* Disable auto-negotiation */
						if((ret = gprtPortAutoNegEnable(dev, hport, GT_FALSE)) != GT_OK) {
				            printf("Error: gprtPortAutoNegEnable, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* Set speed to 100M bps */
						if((ret = gprtSetPortSpeed(dev, hport, PHY_SPEED_100_MBPS)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* Set duplex to half-duplex */
						if((ret = gprtSetPortDuplexMode(dev, hport, GT_FALSE)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }					
					}
					break;

					case S100M_FULL:
					if(hport == 8) {
						/* Disable auto-negotiation, 100M full-duplex */
						if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0xa100)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
					} else {
						/* Disable auto-negotiation */
						if((ret = gprtPortAutoNegEnable(dev, hport, GT_FALSE)) != GT_OK) {
				            printf("Error: gprtPortAutoNegEnable, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* Set speed to 100M bps */
						if((ret = gprtSetPortSpeed(dev, hport, PHY_SPEED_100_MBPS)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* Set duplex to full-duplex */
						if((ret = gprtSetPortDuplexMode(dev, hport, GT_TRUE)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }					
					}
					break;
					
					case S1000M_HALF:
					if(hport == 8) {
						/* Disable auto-negotiation, 1000M half-duplex */
						if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0x8140)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* Set speed to 1000M bps */
						if((ret = gpcsSetForceSpeed(dev, hport, PHY_SPEED_1000_MBPS)) != GT_OK) {
				            printf("Error: gpcsSetForceSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* Set duplex to half-duplex */
						if((ret = gpcsSetDpxValue(dev, hport, GT_TRUE)) != GT_OK) {
				            printf("Error: gpcsSetDpxValue, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }						
						if((ret = gpcsSetForcedDpx(dev, hport, GT_TRUE)) != GT_OK) {
				            printf("Error: gpcsSetForcedDpx, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }						
					} else {
			            /* Do nothing */
					}

					break;	
					
					case S1000M_FULL:
					if(hport == 8) {
						/* Disable auto-negotiation, 1000M full-duplex */
						if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, 0x8140)) != GT_OK) {
				            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* Set speed to 1000M bps */
						if((ret = gpcsSetForceSpeed(dev, hport, PHY_SPEED_1000_MBPS)) != GT_OK) {
				            printf("Error: gpcsSetForceSpeed, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
						/* Set duplex to full-duplex */
						if((ret = gpcsSetDpxValue(dev, hport, GT_TRUE)) != GT_OK) {
				            printf("Error: gpcsSetDpxValue, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }						
						if((ret = gpcsSetForcedDpx(dev, hport, GT_TRUE)) != GT_OK) {
				            printf("Error: gpcsSetForcedDpx, hwport=%d, ret=%d\r\n", hport,ret);
							return CONF_ERR_MSAPI;
				        }
					} else {
					    /* Do nothing */
					}
					break;
				
					default:
					break;
				}
			}
#else
			//swif_SetPortSpeedDuplex(hport, speed_duplex);
#endif
		}

		switch((single_port_cfg & PC_MASK_MDIX) >> 1) {
			case 0x0:	/* MDIX */
#if BOARD_GE22103MA || BOARD_GV3S_HONUE_QM
			//printf("Set port %d to MDIX mode\r\n", i+1);
			if(hport <= 7) {
				if((ret = hwSetPhyRegField(dev, hport, QD_PHY_SPEC_CONTROL_REG,4,2, 0x0)) != GT_OK) {
		            printf("Error: hwSetPhyRegField, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
				}
				if((ret = hwPhyReset(dev,hport,0xFF)) != GT_OK) {
		            printf("Error: hwPhyReset, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
				}
			}

			if(hport == 8) {
				if((ret = gprtGetPagedPhyReg(dev, hport, 16, 0, &u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
				u16Data = (u16Data & 0xFF9F) | 0x0020;
				if((ret = gprtSetPagedPhyReg(dev, hport, 16, 0, u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
				/* Phy soft reset */
				if((ret = gprtGetPagedPhyReg(dev, hport, 0, 0, &u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
				u16Data = u16Data | 0x8000;
				if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
			}			
#endif
			break;

			case 0x1:	/* MDI */
#if BOARD_GE22103MA || BOARD_GV3S_HONUE_QM
			//printf("Set port %d to MDI mode\r\n", i+1);
			if(hport <= 7) {
				if((ret = hwSetPhyRegField(dev, hport, QD_PHY_SPEC_CONTROL_REG,4,2, 0x1)) != GT_OK) {
		            printf("Error: hwSetPhyRegField, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
				}
				if((ret = hwPhyReset(dev,hport,0xFF)) != GT_OK) {
		            printf("Error: hwPhyReset, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
				}
			}
			
			if(hport == 8) {
				if((ret = gprtGetPagedPhyReg(dev, hport, 16, 0, &u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
				u16Data = (u16Data & 0xFF9F) | 0x0000;
				if((ret = gprtSetPagedPhyReg(dev, hport, 16, 0, u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
				/* Phy soft reset */
				if((ret = gprtGetPagedPhyReg(dev, hport, 0, 0, &u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
				u16Data = u16Data | 0x8000;
				if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }				
			}				
#endif				
			break;

			case 0x2:	/* AutoMDIX */
			case 0x3:	/* AutoMDIX */
#if BOARD_GE22103MA || BOARD_GV3S_HONUE_QM
			if(hport <= 7) {
				if((ret = hwSetPhyRegField(dev, hport, QD_PHY_SPEC_CONTROL_REG,4,2, 0x3)) != GT_OK) {
		            printf("Error: hwSetPhyRegField, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
				}
				if((ret = hwPhyReset(dev,hport,0xFF)) != GT_OK) {
		            printf("Error: hwPhyReset, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
				}
			}
			if(hport == 8) {
				if((ret = gprtGetPagedPhyReg(dev, hport, 16, 0, &u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
				u16Data = (u16Data & 0xFF9F) | 0x0060;
				if((ret = gprtSetPagedPhyReg(dev, hport, 16, 0, u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
				/* Phy soft reset */
				if((ret = gprtGetPagedPhyReg(dev, hport, 0, 0, &u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
				u16Data = u16Data | 0x8000;
				if((ret = gprtSetPagedPhyReg(dev, hport, 0, 0, u16Data)) != GT_OK) {
		            printf("Error: gprtSetPortSpeed, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }				
			}
#elif BOARD_GE20023MA
			swif_SetPortAutoMDIX(hport);
#endif
			break;

			default:
			break;
		}

		if(single_port_cfg & PC_MASK_FLOW_CTRL) {
#if BOARD_GE22103MA	|| BOARD_GV3S_HONUE_QM
			if((hport != 9) && (hport != 10)) {	
				/* Program Phy's Pause bit in AutoNegotiation Advertisement Register. */
				if((ret = gprtSetPause(dev,hport,GT_PHY_PAUSE)) != GT_OK) {
		            printf("Error: gprtSetPause, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
				if((single_port_cfg & PC_MASK_AUTO_NEG) != 0) {
					/* Restart AutoNegotiation of the given Port's phy */
					if((ret = gprtPortRestartAutoNeg(dev,hport)) != GT_OK) {
			            printf("Error: gprtPortRestartAutoNeg, hwport=%d, ret=%d\r\n", hport,ret);
						return CONF_ERR_MSAPI;
			        }
				}
			}
			/* Program Port's Flow Control. */
			if((ret = gprtSetForceFc(dev,hport,1)) != GT_OK) {
	            printf("Error: gprtSetForceFc, hwport=%d, ret=%d\r\n", hport,ret);
				return CONF_ERR_MSAPI;
	        }

			if((ret = gsysSetFlowControlMessage(dev,GT_TRUE)) != GT_OK) {
	            printf("Error: gprtSetForceFc, hwport=%d, ret=%d\r\n", hport,ret);
				return CONF_ERR_MSAPI;
	        }
#if 0
			if((hport != 9) && (hport != 10)) {
				/* Program Phy's Pause bit in AutoNegotiation Advertisement Register. */
				if((ret = gprtSetPause(dev,hport,GT_PHY_NO_PAUSE)) != GT_OK) {
		            printf("Error: gprtSetPause, hwport=%d, ret=%d\r\n", hport,ret);
					return CONF_ERR_MSAPI;
		        }
				
				if((single_port_cfg & PC_MASK_AUTO_NEG) != 0) {
					/* Restart AutoNegotiation of the given Port's phy */
					if((ret = gprtPortRestartAutoNeg(dev,hport)) != GT_OK) {
			            printf("Error: gprtPortRestartAutoNeg, hwport=%d, ret=%d\r\n", hport,ret);
						return CONF_ERR_MSAPI;
			        }
				}
			}

			/* Program Port's Flow Control. */
			if((ret = gprtSetForceFc(dev,hport,0)) != GT_OK) {
	            printf("Error: gprtSetForceFc, hwport=%d, ret=%d\r\n", hport,ret);
				return CONF_ERR_MSAPI;
	        }
			if((ret = gsysSetFlowControlMessage(dev,GT_FALSE)) != GT_OK) {
	            printf("Error: gprtSetForceFc, hwport=%d, ret=%d\r\n", hport,ret);
				return CONF_ERR_MSAPI;
	        }	
#endif
#endif			
		}
	}
	
	return CONF_ERR_NONE;
}

int conf_port_rate_init(void)
{
#if SWITCH_CHIP_88E6095
	u8 i, hport;
	port_rate_conf_t PortRateConfig;
	u16 single_port_rate_cfg;
	u16 rate_limit_mode;
	u16 pri1_mode, pri2_mode, pri3_mode, pri0_rate, egress_rate;
	int ret;
		
	if(eeprom_read(NVRAM_PORT_RATE_CFG_BASE, &(PortRateConfig.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;

	if((PortRateConfig.PortNum == 0) || (PortRateConfig.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;

	for(i=0; i<PortRateConfig.PortNum; i++) {

		single_port_rate_cfg = *(u16 *)&(PortRateConfig.PortConfig[i*2]);
		single_port_rate_cfg = ntohs(single_port_rate_cfg);
		hport = swif_Lport_2_Port(i+1);

		rate_limit_mode = (single_port_rate_cfg & PRC_MASK_INGRESS_MODE) >> 14;
		pri3_mode = (single_port_rate_cfg & PRC_MASK_PRI3_RATE) >> 13;
		pri2_mode = (single_port_rate_cfg & PRC_MASK_PRI2_RATE) >> 12;
		pri1_mode = (single_port_rate_cfg & PRC_MASK_PRI1_RATE) >> 11;
		pri0_rate = (single_port_rate_cfg & PRC_MASK_PRI0_RATE) >> 8;
		egress_rate = single_port_rate_cfg & PRC_MASK_EGRESS_RATE;

		#if 0
		printf("hport=%02d, cfg=0x%08x, devname=0x%08x, rate_limit_mode=%d, pri3_mode=%d, pri2_mode=%d, pri1_mode=%d, pri0_rate=%d, egress_rate=%d\r\n",
			hport, single_port_rate_cfg, dev->devName, rate_limit_mode, pri3_mode, pri2_mode, pri1_mode, pri0_rate, egress_rate);
		#endif

        if((ret = grcSetLimitMode(dev, hport, rate_limit_mode)) != GT_OK) {
            printf("Error: grcSetLimitMode, ret=%d\r\n", ret);
            return CONF_ERR_MSAPI;
        }
            
		if(pri0_rate != 0) {
			if((ret = grcSetPri3Rate(dev, hport, pri3_mode)) != GT_OK) {
	            printf("Error: grcSetPri3Rate, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;
	        }
			if((ret = grcSetPri2Rate(dev, hport, pri2_mode)) != GT_OK) {
	            printf("Error: grcSetPri2Rate, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;
	        }
			if((ret = grcSetPri1Rate(dev, hport, pri1_mode)) != GT_OK) {
	            printf("Error: grcSetPri1Rate, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;
	        }
			if((ret = grcSetPri0Rate(dev, hport, pri0_rate)) != GT_OK) {
	            printf("Error: grcSetPri0Rate, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;
	        }
		}

		if(egress_rate != 0) {
			if((ret = grcSetEgressRate(dev, hport, egress_rate)) != GT_OK) {
	            printf("Error: grcSetEgressRate, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;
	        }
		}
	}

	return CONF_ERR_NONE;
#else
    return CONF_ERR_NOT_SUPPORT;
#endif
}

int conf_port_isolation_init(void)
{
#if SWITCH_CHIP_88E6095
	u8 i, j, k, hport, hport_tmp;
	port_isolation_conf_t PortIsolationConfig;
	u16 single_port_isolation_cfg;
	u16 single_port_ports_member;
	GT_LPORT memPorts[MAX_PORT_NUM+1];
	GT_U8 memPortsLen;
	int ret;
		
	if(eeprom_read(NVRAM_PORT_ISOLATION_CFG_BASE, &(PortIsolationConfig.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;

	if((PortIsolationConfig.PortNum == 0) || (PortIsolationConfig.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;

	for(i=0; i<PortIsolationConfig.PortNum; i++) {
		single_port_isolation_cfg = *(u16 *)&(PortIsolationConfig.VlanMap[i*2]);
		single_port_isolation_cfg = ntohs(single_port_isolation_cfg);
		hport = swif_Lport_2_Port(i+1);

		if(single_port_isolation_cfg & PIC_MASK_LIMIT_SWITCH) {
			
		}
		
		single_port_ports_member = single_port_isolation_cfg & PIC_MASK_PORTS_MEMBER;
		memset(memPorts, 0, (MAX_PORT_NUM+1) * sizeof(GT_LPORT));
		k=0;
		for(j=0; j<PortIsolationConfig.PortNum; j++) {
			if(j == i)
				continue;
			if(single_port_ports_member & (1<<j)) {
				hport_tmp = swif_Lport_2_Port(j+1);
				memPorts[k] = GT_PORT_2_LPORT(hport_tmp);
				k++;
			}
		}
		memPorts[k] = dev->cpuPortNum;
		k++;		
		memPortsLen = k;
		
		if((ret = gvlnSetPortVlanPorts(dev,GT_PORT_2_LPORT(hport),memPorts,memPortsLen)) != GT_OK) {
        	printf("Error: gvlnSetPortVlanPorts, ret=%d\r\n", ret);
			return CONF_ERR_MSAPI;
		}		

		#if 0
		if((ret = gvlnGetPortVlanPorts(dev,GT_PORT_2_LPORT(hport),memPorts,&memPortsLen)) != GT_OK) {
        	printf("Error: gvlnGetPortVlanPorts, ret=%d\r\n", ret);
			return CONF_ERR_MSAPI;
		}
	    printf("PortMember: %d,%d,%d,%d,%d,%d,%d\r\n",  
			GT_LPORT_2_PORT(memPorts[0]), GT_LPORT_2_PORT(memPorts[1]), GT_LPORT_2_PORT(memPorts[2]), 
			GT_LPORT_2_PORT(memPorts[3]), GT_LPORT_2_PORT(memPorts[4]), GT_LPORT_2_PORT(memPorts[5]), GT_LPORT_2_PORT(memPorts[6]));
		#endif
			
		
	}
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}

int conf_port_vlan_init(void)
{
#if SWITCH_CHIP_88E6095
	u8 i, hport;
	port_vlan_conf_t PortVlanConfig;
	u16 single_port_vlan_cfg;
	u16 pvid, regval;
	u16 ingress_filter_rule;
	int ret;
		
	if(eeprom_read(NVRAM_PORT_VLAN_CFG_BASE, &(PortVlanConfig.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;

	if((PortVlanConfig.PortNum == 0) || (PortVlanConfig.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;
	
	for(i=0; i<PortVlanConfig.PortNum; i++) {
		single_port_vlan_cfg = *(u16 *)&(PortVlanConfig.VlanConfig[i*2]);
		single_port_vlan_cfg = ntohs(single_port_vlan_cfg);
		hport = swif_Lport_2_Port(i+1);

		pvid = single_port_vlan_cfg & PVC_MASK_VID_NUMBER;
		ingress_filter_rule = (single_port_vlan_cfg & PVC_MASK_VID_CHECK_MODE) >> 13;

		/* refer to datasheet page 185 */
		if((pvid<1) || (pvid>4094)) {
			printf("Error: invalid pvid number %d\r\n", pvid);
			return CONF_ERR_MSAPI;
		}
		
		if((ret = gvlnSetPortVid(dev, hport, pvid)) != GT_OK) {
        	printf("Error: gvlnSetPortVid, ret=%d\r\n", ret);
			return CONF_ERR_MSAPI;	
		}	
		
		if(single_port_vlan_cfg & PVC_MASK_VID_FORCE) {
			if((ret = gvlnSetPortVlanForceDefaultVID(dev, hport, GT_TRUE)) != GT_OK) {
            	printf("Error: gvlnGetPortVlanForceDefaultVID, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;	
			}
		}
		#if 0
		if(hwReadPortReg(dev, hport, QD_REG_PORT_CONTROL2, &regval) == GT_OK) {
			printf("REG_PORT_CONTROL2 = 0x%04x\r\n", regval);
		}
		#endif
		
		/* refer to datasheet page 187, 802.1Q mode */
		switch(ingress_filter_rule) {
			case INGRESS_FILTER_RULE_NOT_CHECK:
			if((ret = gvlnSetPortVlanDot1qMode(dev, hport, GT_DISABLE)) != GT_OK) {
            	printf("Error: gvlnSetPortVlanDot1qMode, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;	
			}				
			break;

			case INGRESS_FILTER_RULE_PRIO_CHECK:
			if((ret = gvlnSetPortVlanDot1qMode(dev, hport, GT_FALLBACK)) != GT_OK) {
            	printf("Error: gvlnSetPortVlanDot1qMode, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;	
			}				
			break;

			case INGRESS_FILTER_RULE_CHECK:
			if((ret = gvlnSetPortVlanDot1qMode(dev, hport, GT_CHECK)) != GT_OK) {
            	printf("Error: gvlnSetPortVlanDot1qMode, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;	
			}				
			break;

			case INGRESS_FILTER_RULE_SECURE:
			if((ret = gvlnSetPortVlanDot1qMode(dev, hport, GT_SECURE)) != GT_OK) {
            	printf("Error: gvlnSetPortVlanDot1qMode, ret=%d\r\n", ret);
				return CONF_ERR_MSAPI;	
			}				
			break;

			default:
			break;
		}	
	}
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}

int conf_port_mirror_init(void)
{
#if SWITCH_CHIP_88E6095	
	u8 i, j, hport, mirror_dst_port;
	port_mirror_conf_t PortMirrorConfig;
	u32 single_port_mirror_cfg;
	u32 mirror_dst_mask;
	int ret;
		
	if(eeprom_read(NVRAM_PORT_MIRROR_CFG_BASE, &(PortMirrorConfig.PortNum), MAX_PORT_NUM * 4 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;

	if((PortMirrorConfig.PortNum == 0) || (PortMirrorConfig.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;

	for(i=0; i<PortMirrorConfig.PortNum; i++) {
		single_port_mirror_cfg = *(u32 *)&(PortMirrorConfig.SourcePort[i*4]);
		single_port_mirror_cfg = ntohl(single_port_mirror_cfg);
		hport = swif_Lport_2_Port(i+1);

		mirror_dst_mask = single_port_mirror_cfg & PMC_MASK_MIRROR_DEST;
		if((mirror_dst_mask & (1<<i)) == 0) {
			return CONF_ERR_INVALID_CFG;
		}

		for(j=0; j<PortMirrorConfig.PortNum; j++) {
			if((mirror_dst_mask & (1<<j)) && (j != i))
				break;
		}

		if(j<PortMirrorConfig.PortNum) {		
			mirror_dst_port = swif_Lport_2_Port(j+1);
			
			switch(single_port_mirror_cfg >> 30) {
				case 0:	/* RX */
				if((ret = gprtSetIngressMonitorSource(dev, hport, GT_TRUE)) != GT_OK) {
            		printf("Error: gprtSetIngressMonitorSource, ret=%d\r\n", ret);
					return CONF_ERR_MSAPI;
        		}
				
				if((ret = gsysSetIngressMonitorDest(dev, mirror_dst_port)) != GT_OK) {
            		printf("Error: gsysSetIngressMonitorDest, ret=%d\r\n", ret);
					return CONF_ERR_MSAPI;
        		}						
				break;

				case 1:	/* TX */
				if((ret = gprtSetEgressMonitorSource(dev, hport, GT_TRUE)) != GT_OK) {
            		printf("Error: gprtSetEgressMonitorSource, ret=%d\r\n", ret);
					return CONF_ERR_MSAPI;
        		}
				
				if((ret = gsysSetEgressMonitorDest(dev, mirror_dst_port)) != GT_OK) {
            		printf("Error: gsysSetEgressMonitorDest, ret=%d\r\n", ret);
					return CONF_ERR_MSAPI;
        		}									
				break;

				case 2:	/* ALL */
				if((ret = gprtSetEgressMonitorSource(dev, hport, GT_TRUE)) != GT_OK) {
            		printf("Error: gprtSetEgressMonitorSource, ret=%d\r\n", ret);
					return CONF_ERR_MSAPI;
        		}
				if((ret = gprtSetIngressMonitorSource(dev, hport, GT_TRUE)) != GT_OK) {
            		printf("Error: gprtSetIngressMonitorSource, ret=%d\r\n", ret);
					return CONF_ERR_MSAPI;
        		}
				
				if((ret = gsysSetIngressMonitorDest(dev, mirror_dst_port)) != GT_OK) {
            		printf("Error: gsysSetIngressMonitorDest, ret=%d\r\n", ret);
					return CONF_ERR_MSAPI;
        		}

				if((ret = gsysSetEgressMonitorDest(dev, mirror_dst_port)) != GT_OK) {
            		printf("Error: gsysSetEgressMonitorDest, ret=%d\r\n", ret);
					return CONF_ERR_MSAPI;
        		}					
				break;

				default:
				break;
			}
		}
	}
	
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif	
}

int conf_port_qos_init(void)
{
#if SWITCH_CHIP_88E6095
    int status = GT_OK;
    port_qos_conf_t PortQosConf = {0};
    IN GT_U8 trClass = 0;
    
	if(eeprom_read(NVRAM_QOS_CFG_BASE, (u8 *)&PortQosConf, 20 + MAX_PORT_NUM + 1) != I2C_SUCCESS)
        return CONF_ERR_I2C;
    
    if((PortQosConf.PortNum == 0) || (PortQosConf.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;
    	 
    /*	IEEE 802.3ac Tag (Priority 0 ~ 7, 3 bits) */
    for(int prior=7, i=0; prior>=0; prior--, i++)
    {
        int byten  = 0;
        int shiftn = 0;
        
        byten = i/4;
        shiftn = (prior%4)*2;
        
        trClass = ((PortQosConf.CosMapping[byten] >> shiftn) &0x03);
        
        /*	Priority [prior] is using OBNetOS Queue [trClass]. */
        if((status = gcosSetUserPrio2Tc(dev,prior,trClass)) != GT_OK)
        {
            printf("gcosSetUserPrio2Tc returned fail.\r\n");
            return CONF_ERR_MSAPI;
        }
    }
    
	/* IPv4/IPv6 (Priority 0 ~ 63, 6 bits) */
	for(int i=0, prior=63; prior>=0; prior--, i++)
	{   
        int byten  = 0;
        int shiftn = 0;
        
        byten = i/4;
        shiftn = (prior%4)*2;
        
        trClass = ((PortQosConf.TosMapping[byten] >> shiftn) &0x03);
        
        /*	Priority [prior] is using OBNetOS Queue [trClass]. */
		if((status = gcosSetDscp2Tc(dev,prior,trClass)) != GT_OK)
		{
			printf("gcosSetDscp2Tc returned fail.\r\n");
			return CONF_ERR_MSAPI;
		}
	}   
    
    /* Set fixed priority or weighted fair queuing schemes */    
    if((PortQosConf.QosFlag[1] & QOS_SHEDULE_MODE) == QOS_WRR_MODE)
    {
        status = gsysSetSchedulingMode(dev,GT_TRUE);
        if(status != GT_OK)
        {
            printf("gsysSetSchedulingMode rerurned fail.\r\n");
            return CONF_ERR_MSAPI;
        }
    }
    else if((PortQosConf.QosFlag[1] & QOS_SHEDULE_MODE) == QOS_STRICT_MODE)
    {
        status = gsysSetSchedulingMode(dev,GT_FALSE);
        if(status != GT_OK)
        {
            printf("gsysSetSchedulingMode rerurned fail.\r\n");
            return CONF_ERR_MSAPI;
        }
    }
    
    /* Set port COS/TOS */ 
	for(u8 lport=0; lport<PortQosConf.PortNum; lport++)
	{
        GT_U8 port     = 0;
        GT_U8 tmpMask   = 0;
        GT_U8 tmpData   = 0;
        GT_BOOL en   = GT_FALSE;
        GT_EGRESS_MODE  mode;
        
        port = swif_Lport_2_Port(lport + 1);
        
        tmpData = PortQosConf.PortQosConfig[lport];
        QOS_MASK(4, 1, tmpMask);
        tmpData &= tmpMask;
        en = (tmpData == QOS_COS_MAP_EN)?GT_TRUE:GT_FALSE;
         /* Use IEEE Tag */
		if((status = gqosUserPrioMapEn(dev,port,en)) != GT_OK)
		{
			printf("gqosUserPrioMapEn return Failed\r\n");
			return CONF_ERR_MSAPI;
		}

        tmpData = PortQosConf.PortQosConfig[lport];
        QOS_MASK(5, 1, tmpMask);
        tmpData &= tmpMask;
        en = (tmpData == QOS_TOS_MAP_EN)?GT_TRUE:GT_FALSE;
		/* Use IPv4/IPv6 priority fields (use IP) */
		if((status = gqosIpPrioMapEn(dev,port,en)) != GT_OK)
		{
			printf("gqosIpPrioMapEn return Failed\r\n");
			return CONF_ERR_MSAPI;
		}

        tmpData = PortQosConf.PortQosConfig[lport];
        QOS_MASK(6, 1, tmpMask);
        tmpData &= tmpMask;
        en = (tmpData == QOS_PRIO_MAP_RULE)?GT_TRUE:GT_FALSE;
		/* IEEE Tag has higher priority than IP priority fields */
		if((status = gqosSetPrioMapRule(dev,port,en)) != GT_OK)
		{
			printf("gqosSetPrioMapRule return Failed\r\n");
			return CONF_ERR_MSAPI;
		}

        tmpData = PortQosConf.PortQosConfig[lport];
        QOS_MASK(2, 2, tmpMask);
        tmpData &= tmpMask;
        trClass = (tmpData >> 2)*2;
        /* Each port's default priority is set to [trClass]. */
		if((status = gcosSetPortDefaultTc(dev,port,trClass)) != GT_OK)
		{
			printf("gcosSetDscp2Tc returned fail.\r\n");
			return CONF_ERR_MSAPI;
		}

        tmpData = PortQosConf.PortQosConfig[lport];
        QOS_MASK(0, 2, tmpMask);
        tmpData &= tmpMask;
        mode = tmpData;
        /* Each port's EgressMode is set to [mode]. */
        if((status = gprtSetEgressMode(dev,port,mode)) != GT_OK)
        {
            printf("gprtSetEgressMode returned fail.\r\n");
			return CONF_ERR_MSAPI;
        }
	}
    
    return CONF_ERR_NONE;
#else /* SWITCH_CHIP_88E6095 */
    return CONF_ERR_NOT_SUPPORT;      
#endif /* SWITCH_CHIP_88E6095 */
}

int conf_port_security_init(void)
{
#if SWITCH_CHIP_88E6095
	unsigned char i,j,hport;
	port_security_conf_t cfg_security;
	port_security_record_conf_t port_security_rec;	
	unsigned short cfg_single_port;
	u32 port_list_vec, security_port_vec, lport_list, hwport_vec;
	int ret;
	
	//unsigned char maclist_count;
	//oam_mac_list_t maclist_node;
	//mac_list_conf_t	cfg_maclist;
	 
	if(eeprom_read(NVRAM_PORT_SECURITY_CFG_BASE, (u8 *)&cfg_security, sizeof(port_security_conf_t)) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	//buffer_dump_console((u8 *)&cfg_security, sizeof(port_security_conf_t));

	if((cfg_security.PortNum > MAX_PORT_NUM) || (cfg_security.PortNum == 0x00))
		return CONF_ERR_NO_CFG;

	if((cfg_security.TotalRecordCount == 00) || (cfg_security.TotalRecordCount > MAX_PORT_SECURITY_RECORD_COUNT))
		return CONF_ERR_NO_CFG;	

	security_port_vec = 0;
	for(i=0; i<cfg_security.PortNum; i++) {
		cfg_single_port = *(u16 *)&(cfg_security.SecuriyConfig[i*2]);
		cfg_single_port = ntohs(cfg_single_port);
		hport = swif_Lport_2_Port(i+1);

		if(cfg_single_port & PSC_MASK_SECURITY_ENABLE) {
			gPortSecurityEn[i] = GT_TRUE;
			/*
			if((ret = gprtSetForwardUnknown(dev,hport,GT_FALSE)) != GT_OK) {
        		printf("Error: gprtSetForwardUnknown, hport=%d, ret=%d\r\n", hport, ret);
				return CONF_ERR_MSAPI;
			}
			if((ret = gprtSetLockedPort(dev,hport,GT_TRUE)) != GT_OK) {
        		printf("Error: gprtSetLockedPort, hport=%d, ret=%d\r\n", hport, ret);
				return CONF_ERR_MSAPI;
			}
			if((ret = gprtSetDropOnLock(dev,hport,GT_TRUE)) != GT_OK) {
        		printf("Error: gprtSetDropOnLock, hport=%d, ret=%d\r\n", hport, ret);
				return CONF_ERR_MSAPI;
			}	
			*/		

			if((ret = gfdbRemovePort(dev,GT_FLUSH_ALL,hport)) != GT_OK) {
        		printf("Error: gfdbRemovePort, hport=%d, ret=%d\r\n", hport, ret);
				return CONF_ERR_MSAPI;
			}			
			
			#if 1
			if((ret = gprtSetLearnDisable(dev, hport, GT_TRUE)) != GT_OK) {
        		printf("Error: gprtSetLearnDisable, hport=%d, ret=%d\r\n", hport, ret);
				return CONF_ERR_MSAPI;
    		}	
			#endif
		} else
			gPortSecurityEn[i] = GT_FALSE;

		if(gPortSecurityEn[i])
			security_port_vec |= 1<<i;
	}


	for(i=0; i<cfg_security.TotalRecordCount; i++) {
		if(eeprom_read(NVRAM_PORT_SECURITY_REC_CFG_BASE + i*sizeof(port_security_record_conf_t), (u8 *)&port_security_rec, sizeof(port_security_record_conf_t)) != I2C_SUCCESS)
			return CONF_ERR_I2C;

		port_list_vec = *(u32 *)&(port_security_rec.PortVec[0]);
		port_list_vec = ntohl(port_list_vec);
		
		//lport_list = security_port_vec & port_list_vec;
		hwport_vec = 0;
		for(j=0; j<cfg_security.PortNum; j++) {
			if(port_list_vec & (1<<j)) {

				hport = swif_Lport_2_Port(j+1);
				hwport_vec |= 1<<hport;
			}
		}

		//printf("port_list_vec=0x%08x, hwport_vec=0x%08x\r\n",port_list_vec,hwport_vec);
		
		MacAdd2(port_security_rec.MacAddr[0], port_security_rec.MacAddr[1], port_security_rec.MacAddr[2],
				port_security_rec.MacAddr[3], port_security_rec.MacAddr[4], port_security_rec.MacAddr[5], hwport_vec, port_security_rec.Priority);
	}



#if 0

	MacListInit();
	maclist_count = cfg_security.MacListCount;
	if(maclist_count > MAX_MAC_LIST_COUNT)
		return CONF_ERR_INVALID_CFG;

	if(maclist_count > 0) {
		for(j=0; j<maclist_count; j++) {
			if(eeprom_read(NVRAM_MAC_LIST_CFG_BASE + i*sizeof(mac_list_conf_t), (u8 *)&(cfg_maclist), sizeof(mac_list_conf_t)) != I2C_SUCCESS) 
				return CONF_ERR_I2C;
			memcpy(maclist_node.MacAddr, cfg_maclist.MacAddr, 6);
			memcpy(maclist_node.PortVec, cfg_maclist.PortVec, 4);
			maclist_node.Priority = 0;
			
			MacListAdd(&maclist_node);
		}
	}
	
	for(i=0; i<MAX_PORT_NUM; i++) {
		cfg_single_port = *(u16 *)&(cfg_security.PortConfig[i*2]);
		cfg_single_port = ntohs(cfg_single_port);
		if(cfg_single_port & PSC_MASK_SECURITY_ENABLE) {
			gPortSecurityEn[i] = GT_TRUE;
			gMacLimitMaximum[i] = (unsigned char)(cfg_single_port & PSC_MASK_SECURITY_MAC_LIMIT_MAXIMUM);
		} else {
			gPortSecurityEn[i] = GT_FALSE;
			gMacLimitMaximum[i] = 0;
		}
	}
#endif
	
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}


int conf_port_trunk_init(void)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_U32 trunkId, trunkMask, mask;
	port_trunk_conf_t port_trunk_cfg;
	port_trunk_record_conf_t port_trunk_rec;
	u8	trunk_port_array[MAX_PORT_IN_TRUNK];
	u32 port_list_vec;
	u32 hwport_vec;
	u8 hport;
	int i,j,k;
	int index, nTrunkPort;
	
	if(eeprom_read(NVRAM_PORT_TRUNK_CFG_BASE, (u8 *)&port_trunk_cfg, sizeof(port_trunk_conf_t)) != I2C_SUCCESS) {
		return CONF_ERR_I2C;
	}

	if((port_trunk_cfg.PortNum == 0) || (port_trunk_cfg.PortNum > MAX_PORT_NUM) || (port_trunk_cfg.TotalRecordCount > MAX_PORT_TRUNK_RECORD_COUNT) || (port_trunk_cfg.TotalRecordCount == 0)) {
		return CONF_ERR_NO_CFG;
	}

	for(i=0; i<port_trunk_cfg.TotalRecordCount; i++) {
		if(eeprom_read(NVRAM_PORT_TRUNK_RECORD_CFG_BASE + i * sizeof(port_trunk_record_conf_t), (u8 *)&port_trunk_rec, sizeof(port_trunk_record_conf_t)) != I2C_SUCCESS) {
			return CONF_ERR_I2C;
		}

		trunkId = port_trunk_rec.TrunkId;
		if(trunkId > MAX_TRUNK_ID)
			return CONF_ERR_NO_CFG;
		
		port_list_vec = *(u32 *)&(port_trunk_rec.MemberVec[0]);
		port_list_vec = ntohl(port_list_vec);
		hwport_vec = 0;
		nTrunkPort = 0;
		for(j=0; j<port_trunk_cfg.PortNum; j++) {
			if(port_list_vec & (1<<j)) {
				hport = swif_Lport_2_Port(j+1);
				hwport_vec |= 1<<hport;
				
				/* enabled trunk on the given port */
				if((status = gprtSetTrunkPort(dev, hport, GT_TRUE, trunkId)) != GT_OK) {
					return CONF_ERR_MSAPI;
				}
				//printf("trunkId = %d, hport=%d\r\n", trunkId, hport);
				nTrunkPort++;
				if(nTrunkPort == MAX_PORT_IN_TRUNK)
					break;				
			}
		}

		/* Set Trunk Route Table for the given Trunk ID */
		if((status = gsysSetTrunkRouting(dev, trunkId, hwport_vec)) != GT_OK) {
			return CONF_ERR_MSAPI;
		}
#if 1
		memset(trunk_port_array, 0, MAX_PORT_IN_TRUNK);
		index = 0;
		for(j=0; j<dev->numOfPorts; j++) {
			if(hwport_vec & (1<<j)) {
				trunk_port_array[index] = j;
				index++;
				if(index == MAX_PORT_IN_TRUNK)
					break;					
			}
		}

		/* Set Trunk Mask Table for load balancing. */
		trunkMask = 0x7FF;
		trunkMask &= ~hwport_vec;
		for(k=0; k<8; k++) {
			index = k % nTrunkPort;
			mask = trunkMask | (1 << trunk_port_array[index]);
			//printf("0x%04x ", mask);
			if((status = gsysSetTrunkMaskTable(dev, k, mask)) != GT_OK) {
				return CONF_ERR_MSAPI;
			}
		}
#endif
	}
	
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}


int conf_8021q_vlan_init(void)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_VTU_ENTRY vtuEntry;
	GT_U16 vid;
	GT_U16 hport;
	vlan_conf_t vlan_cfg;
	vlan_record_conf_t vlan_rec;
	GT_U8 vlan_setup[12];
	int i,j,x,y;


	if(eeprom_read(NVRAM_VLAN_CFG_BASE, (u8 *)&vlan_cfg, sizeof(vlan_conf_t)) != I2C_SUCCESS) {
		return CONF_ERR_I2C;
	}

	if((vlan_cfg.PortNum == 0) || (vlan_cfg.PortNum > MAX_PORT_NUM) || (vlan_cfg.TotalRecordCount > MAX_VLAN_RECORD_COUNT) || (vlan_cfg.TotalRecordCount == 0)) {
		return CONF_ERR_NO_CFG;
	}

	
	/* 1) Clear VLAN ID Table */
	if((status = gvtuFlush(dev)) != GT_OK) {
		printf("gvtuFlush returned failed\r\n");
		return CONF_ERR_MSAPI;
	}

	/* 2) Setup 802.1Q mode for each port except CPU port. Refer to function conf_port_vlan_init() */

	/* 3) Enable 802.1Q for CPU port as GT_FALLBACK mode */
	if((status = gvlnSetPortVlanDot1qMode(dev, dev->cpuPortNum, GT_FALLBACK)) != GT_OK) {
		printf("gvlnSetPortVlanDot1qMode return Failed, port=%d, ret=%d\r\n", dev->cpuPortNum, status);
		return CONF_ERR_MSAPI;
	}

	/* 4) Add VLAN ID and add vlan members */
	
	for(i=0; i<vlan_cfg.TotalRecordCount; i++) {
		if(eeprom_read(NVRAM_VLAN_RECORD_CFG_BASE + i * sizeof(vlan_record_conf_t), (u8 *)&vlan_rec, sizeof(vlan_record_conf_t)) != I2C_SUCCESS) {
			return CONF_ERR_I2C;
		}
		vid = *(GT_U16 *)&(vlan_rec.VLanID[0]);
		vid = ntohs(vid);
		
		y=0;
		for(j=2; j>=0; j--) {
			for(x=0; x<4; x++) {
				vlan_setup[y] = (vlan_rec.VLanSetting[j] >> x*2) & 0x3;
				y++;
			}
		}
		#if 0
		printf("vid=%d, vlan_setup: %d %d %d %d %d %d %d %d %d %d %d %d\r\n", vid,
			vlan_setup[0], vlan_setup[1], vlan_setup[2], vlan_setup[3], vlan_setup[4], vlan_setup[5], 
			vlan_setup[6], vlan_setup[7], vlan_setup[8], vlan_setup[9], vlan_setup[10], vlan_setup[11]);
		#endif
		
		gtMemSet(&vtuEntry,0,sizeof(GT_VTU_ENTRY));
		vtuEntry.DBNum = 0;
		vtuEntry.vid = vid;			
		for(j=0; j<vlan_cfg.PortNum; j++) {
			hport = swif_Lport_2_Port(j+1);
			vtuEntry.vtuData.memberTagP[hport] = vlan_setup[j];
		}
		vtuEntry.vtuData.memberTagP[dev->cpuPortNum] = MEMBER_EGRESS_UNTAGGED;

		if((status = gvtuAddEntry(dev,&vtuEntry)) != GT_OK) {
			printf("gvtuAddEntry return Failed\n");
			return CONF_ERR_MSAPI;
		}
	
	}
	
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}

int conf_static_multicast_init(void)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;
	mcast_conf_t mcast_cfg;
	mcast_record_conf_t mcast_rec;
	u16 port_list_vec;
	u32 hwport_vec;
	u8 hport;
	int i,j;
	
	if(eeprom_read(NVRAM_MCAST_CFG_BASE, (u8 *)&mcast_cfg, sizeof(mcast_conf_t)) != I2C_SUCCESS) {
		return CONF_ERR_I2C;
	}

	if((mcast_cfg.PortNum == 0) || (mcast_cfg.PortNum > MAX_PORT_NUM) || (mcast_cfg.TotalRecordCount > MAX_MCAST_RECORD_COUNT) || (mcast_cfg.TotalRecordCount == 0)) {
		return CONF_ERR_NO_CFG;
	}

	for(i=0; i<mcast_cfg.TotalRecordCount; i++) {
		if(eeprom_read(NVRAM_MCAST_RECORD_CFG_BASE + i * sizeof(mcast_record_conf_t), (u8 *)&mcast_rec, sizeof(mcast_record_conf_t)) != I2C_SUCCESS) {
			return CONF_ERR_I2C;
		}

		if((mcast_rec.Mac[0] & 0x1) != 1)
			continue;

		port_list_vec = *(u16 *)&(mcast_rec.Member[0]);
		port_list_vec = ntohs(port_list_vec);
		
		hwport_vec = 0;
		for(j=0; j<mcast_cfg.PortNum; j++) {
			if(port_list_vec & (1<<j)) {
				hport = swif_Lport_2_Port(j+1);
				hwport_vec |= 1<<hport;
			}
		}
		
		memset(&macEntry,0,sizeof(GT_ATU_ENTRY));	
		macEntry.macAddr.arEther[0] = mcast_rec.Mac[0];
		macEntry.macAddr.arEther[1] = mcast_rec.Mac[1];
		macEntry.macAddr.arEther[2] = mcast_rec.Mac[2];
		macEntry.macAddr.arEther[3] = mcast_rec.Mac[3];
		macEntry.macAddr.arEther[4] = mcast_rec.Mac[4];
		macEntry.macAddr.arEther[5] = mcast_rec.Mac[5];	

		macEntry.DBNum = 0;
		macEntry.portVec = hwport_vec;
		macEntry.prio = 0;
		macEntry.entryState.mcEntryState = GT_MC_STATIC;

		if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK) {
			printf("gfdbAddMacEntry return failed, ret=%d\r\n", status);
			return CONF_ERR_MSAPI;
		}
	}
	
	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}


int conf_global_init(void)
{
	alarm_conf_t conf_alarm;
	const u8 mac0[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	const u8 mac1[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	u8 *pData;
	int i, ret;
	u32	TrapGate;
	extern u32 gTrapEnableMember;
	
	if(eeprom_read(NVRAM_GLOBAL_CFG_BASE, (u8 *)&conf_alarm, sizeof(alarm_conf_t)) != I2C_SUCCESS) {
		return CONF_ERR_I2C;
	}

	if((memcmp(conf_alarm.ServerMac, mac0, 6) == 0) || (memcmp(conf_alarm.ServerMac, mac1, 6) == 0))
		return CONF_ERR_NO_CFG;

	if(conf_alarm.ServerMac[0] & 0x1) // Multicast address
		return CONF_ERR_NO_CFG;

	gTrapEnableMember = 0;
	
	if((ret = conf_get_alarm_trap_gate(&TrapGate)) != CONF_ERR_NONE) {
		return CONF_ERR_I2C;
	} else {
		TrapGate = ntohl(TrapGate);
	}

	if((TrapGate & TRAP_MASK_RING_PORT_STAT) == TRAP_MASK_RING_PORT_STAT) {
		gTrapEnableMember |= TRAP_EN_MEMBER_RING_ALARM;	
	}

	if((TrapGate & TRAP_MASK_DEV_PORT_STAT) == TRAP_MASK_DEV_PORT_STAT) {
		gTrapEnableMember |= TRAP_EN_MEMBER_PORT_STAT;	
	}

	if(gTrapEnableMember)
		TrapFrameInit();

#if SWITCH_CHIP_88E6095
{
	extern void IntProcTask(void *arg);
	
	if(gTrapEnableMember & TRAP_EN_MEMBER_PORT_STAT)
		xTaskCreate(IntProcTask, "tSwInt", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 4, NULL);
}
#endif
	/* conf_alarm.AgeTime = 0x14;	*/
	/* AgeTime * 15 seconds = 20 * 15 = 300s */	 
	
	return CONF_ERR_NONE;
}

int conf_port_security_add_mac(int logic_port, mac_list_conf_t *mac_node)
{
	
	return CONF_ERR_NONE;
}


int conf_port_security_update_maclist(int logic_port, u8 *mac)
{

	return CONF_ERR_NONE;
}

int conf_port_security_enable(unsigned int logic_port)
{
#if 0
	port_security_conf_t cfg_security;
	unsigned short cfg_single_port;
	
	if((logic_port == 0) || (logic_port > MAX_PORT_NUM))
		return CONF_ERR_PARAM;

	if(eeprom_read(NVRAM_PORT_SECURITY_CFG_BASE, &(cfg_security.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;

	cfg_single_port = (cfg_security.PortConfig[(logic_port-1)*2]);
	cfg_single_port = ntohs(cfg_single_port);

	if(cfg_single_port & 0x8000)
		return CONF_ERR_NONE;
	
	cfg_single_port |= 0x8000;
	cfg_security.PortConfig[(logic_port-1)*2] = htons(cfg_single_port);

	if(eeprom_page_write(NVRAM_PORT_SECURITY_CFG_BASE, &(cfg_security.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;
#endif
	return CONF_ERR_NONE;
}


int conf_port_security_disable(unsigned int logic_port)
{
#if 0
	port_security_conf_t cfg_security;
	unsigned short cfg_single_port;
	
	if((logic_port == 0) || (logic_port > MAX_PORT_NUM))
		return CONF_ERR_PARAM;

	if(eeprom_read(NVRAM_PORT_SECURITY_CFG_BASE, &(cfg_security.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;

	cfg_single_port = cfg_security.PortConfig[(logic_port-1)*2];
	cfg_single_port = ntohs(cfg_single_port);

	if((cfg_single_port & 0x8000) == 0)
		return CONF_ERR_NONE;
	
	cfg_single_port &= 0x7FFF;
	cfg_security.PortConfig[(logic_port-1)*2] = htons(cfg_single_port);

	if(eeprom_page_write(NVRAM_PORT_SECURITY_CFG_BASE, &(cfg_security.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;
#endif
	return CONF_ERR_NONE;
}

void conf_initialize(void)
{
	int ret;

	printf("Loading configuration ...\r\n");
	
	printf("   Port base config        : ");
	ret = conf_port_base_init();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}

	printf("   Port rate config        : ");
	ret = conf_port_rate_init();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}

	printf("   Port isolation config   : ");
	ret = conf_port_isolation_init();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
	
	printf("   Port vlan config        : ");
	ret = conf_port_vlan_init();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}

	printf("   802.1q vlan config      : ");
	ret = conf_8021q_vlan_init();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
	
	printf("   Static multicast config : ");
	ret = conf_static_multicast_init();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}

	printf("   Port mirror config      : ");
	ret = conf_port_mirror_init();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
    
	printf("   Port QOS config         : ");
    ret = conf_port_qos_init();
    if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
    
	printf("   Port security config    : ");
	ret = conf_port_security_init();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}

	printf("   Port trunk config       : ");
	ret = conf_port_trunk_init();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
    
	printf("   Global config           : ");
	ret = conf_global_init();
	if((ret != CONF_ERR_NONE) && (ret != CONF_ERR_NO_CFG) && (ret != CONF_ERR_NOT_SUPPORT)) {
		printf("Failed\r\n");
	} else {
		if(ret == CONF_ERR_NO_CFG)
			printf("Ignored\r\n");
		else if(ret == CONF_ERR_NOT_SUPPORT)
			printf("Not support\r\n");
		else
			printf("Done\r\n");
	}
	
	printf("\r\n");
}	



