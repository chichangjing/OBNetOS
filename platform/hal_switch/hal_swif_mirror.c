/*******************************************************************
 * Filename     : hal_swif_mirror.c
 * Description  : Hardware Abstraction Layer for L2 Switch Port API
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *******************************************************************/
#include "mconfig.h"

/* Standard includes */
#include <stdio.h>

/* LwIP includes */
#include "lwip/netif.h"
#include "lwip/stats.h"

/* BSP includes */
#include "stm32f2x7_smi.h"
#include "soft_i2c.h"
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

#if MODULE_OBNMS
/* NMS includes */
#include "nms_comm.h"
#include "nms_if.h"
#endif

/* HAL for L2 includes */
#include "hal_swif_error.h"
#include "hal_swif_types.h"
#include "hal_swif_comm.h"
#include "hal_swif_port.h"
#include "hal_swif_mirror.h"

/* Other includes */
#include "cli_util.h"

#if MARVELL_SWITCH
extern GT_QD_DEV *dev;
#endif

/**************************************************************************
  * @brief  Set ingress mintor port
  * @param  IngressDestLport
  * @retval 
  *************************************************************************/
int hal_swif_set_ingress_mirror_dest(uint8 IngressDestLport)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_U32 IngressDestHport;

	if((IngressDestLport > MAX_PORT_NUM) || (IngressDestLport == 0))
		return HAL_SWIF_ERR_INVALID_LPORT;

	IngressDestHport = hal_swif_lport_2_hport(IngressDestLport);
	
	if((status = gsysSetIngressMonitorDest(dev, IngressDestHport)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  Set egress mirror port
  * @param  EgressDestLPort
  * @retval 
  *************************************************************************/
int hal_swif_set_egress_mirror_dest(uint8 EgressDestLPort)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_U32 EgressDestHport;

	if((EgressDestLPort > MAX_PORT_NUM) || (EgressDestLPort == 0))
		return HAL_SWIF_ERR_INVALID_LPORT;

	EgressDestHport = hal_swif_lport_2_hport(EgressDestLPort);

	if((status = gsysSetEgressMonitorDest(dev, EgressDestHport)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}

/**************************************************************************
  * @brief  Add mirror source port
  * @param  lport, MirrorMode
  * @retval 
  *************************************************************************/
int hal_swif_add_mirror_source(uint8 lport, HAL_MIRROR_MODE MirrorMode)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_U32 hport, IngressDestHport, EgressDestHport;

	if((lport > MAX_PORT_NUM) || (lport == 0))
		return HAL_SWIF_ERR_INVALID_LPORT;
	
	hport = hal_swif_lport_2_hport(lport);
	
	switch(MirrorMode) {
		case MIRROR_RX:
		if((status = gprtSetIngressMonitorSource(dev, hport, GT_TRUE)) != GT_OK) 
			return HAL_SWIF_ERR_MSAPI;				
		break;

		case MIRROR_TX:
		if((status = gprtSetEgressMonitorSource(dev, hport, GT_TRUE)) != GT_OK) 
			return HAL_SWIF_ERR_MSAPI;							
		break;

		case MIRROR_ALL:
		if((status = gprtSetEgressMonitorSource(dev, hport, GT_TRUE)) != GT_OK) 
			return HAL_SWIF_ERR_MSAPI;

		if((status = gprtSetIngressMonitorSource(dev, hport, GT_TRUE)) != GT_OK) 
			return HAL_SWIF_ERR_MSAPI;					
		break;

		default:
		break;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}

/**************************************************************************
  * @brief  Initialize configuration for port rate control
  * @param  none
  * @retval none
  *************************************************************************/
int hal_swif_mirror_conf_initialize(void)
{
#if SWITCH_CHIP_88E6095
#if 0
	uint8 lport;
	hal_mirror_conf_t PortMirror;
	uint16 MinitorDestConfig;
	uint8 IngressMonitorDestPort;
	uint8 EgressMonitorDestPort;
	uint8 mirror_source_cfg;
	HAL_MIRROR_MODE mirror_mode;

	if(eeprom_read(NVRAM_PORT_MIRROR_CFG_BASE, &(PortMirror.PortNum), MAX_PORT_NUM + 3) != I2C_SUCCESS) 
		return CONF_ERR_I2C;

	if((PortMirror.PortNum == 0) || (PortMirror.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;

	if(PortMirror.IngressMirrorDestConfig & MIRROR_DEST_MASK_INGRESS_ENABLE) {
		IngressMonitorDestPort = PortMirror.IngressMirrorDestConfig & MIRROR_DEST_MASK_INGRESS_PORT;
		if(hal_swif_set_ingress_mirror_dest(IngressMonitorDestPort) != HAL_SWIF_SUCCESS)
			return CONF_ERR_SWITCH_HAL;
	}

	if(PortMirror.EgressMirrorDestConfig & MIRROR_DEST_MASK_EGRESS_ENABLE) {
		EgressMonitorDestPort = PortMirror.EgressMirrorDestConfig & MIRROR_DEST_MASK_EGRESS_PORT;
		if(hal_swif_set_egress_mirror_dest(EgressMonitorDestPort) != HAL_SWIF_SUCCESS)
			return CONF_ERR_SWITCH_HAL;
	}	

	for(lport=1; lport<=PortMirror.PortNum; lport++) {
		mirror_source_cfg = PortMirror.MirrorSourceConfig[lport-1];
		mirror_mode = (HAL_MIRROR_MODE)(mirror_source_cfg & MIRROR_SRC_MASK_MODE) >> 6;

		if(hal_swif_add_mirror_source(lport, mirror_mode) != HAL_SWIF_SUCCESS)
			return CONF_ERR_SWITCH_HAL;
	}

	return CONF_ERR_NONE;
#else
	u8 i, j, hport, mirror_dst_port;
	mirror_conf_t PortMirrorConfig;
	u32 single_port_mirror_cfg;
	u32 mirror_dst_mask;
	int ret, valid_cfg_flag = 0;
		
	if(eeprom_read(NVRAM_PORT_MIRROR_CFG_BASE, &(PortMirrorConfig.PortNum), MAX_PORT_NUM * 4 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;

	if((PortMirrorConfig.PortNum == 0) || (PortMirrorConfig.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;

	for(i=0; i<PortMirrorConfig.PortNum; i++) {
		single_port_mirror_cfg = *(u32 *)&(PortMirrorConfig.SourcePort[i*4]);
		single_port_mirror_cfg = ntohl(single_port_mirror_cfg);
		hport = hal_swif_lport_2_hport(i+1);

		mirror_dst_mask = single_port_mirror_cfg & MIRROR_MASK_DEST;
		/* Check configuration for port */
		if((mirror_dst_mask & (1<<i)) == 0) {
			return CONF_ERR_INVALID_CFG;
		}

		/* Find mirror dst port number */
		for(j=0; j<PortMirrorConfig.PortNum; j++) {
			if((mirror_dst_mask & (1<<j)) && (j != i))
				break;
		}

		if(j<PortMirrorConfig.PortNum) {		
			mirror_dst_port = hal_swif_lport_2_hport(j+1);
			
			switch((single_port_mirror_cfg & MIRROR_MASK_MODE) >> 30) {
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
			valid_cfg_flag = 1;
		} 
	}

	if(valid_cfg_flag == 0)
		return CONF_ERR_NO_CFG;
	
	return CONF_ERR_NONE;
#endif

#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}

#if MODULE_OBNMS

extern u8 NMS_TxBuffer[];

void nms_rsp_set_port_mirror(u8 *DMA, u8 *RequestID, obnet_set_port_mirror *pPortMirror)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_MIRROR;

#if ((BOARD_FEATURE & L2_PORT_MIRROR) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if((pPortMirror->PortNum > MAX_PORT_NUM) || (pPortMirror->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(eeprom_page_write(NVRAM_PORT_MIRROR_CFG_BASE, (u8 *)&(pPortMirror->PortNum), pPortMirror->PortNum * 4 + 1) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = RSP_ERR_EEPROM_OPERATION;	
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}	
	}
#else
	RspSet.RetCode = 0x01;
	RspSet.Res = RSP_ERR_FEATURE_NOT_SUPPORT;	
#endif

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

void nms_rsp_get_port_mirror(u8 *DMA, u8 *RequestID)
{
	obnet_rsp_get_port_mirror RspGetMirror;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetMirror, 0, sizeof(obnet_rsp_get_port_mirror));
	RspGetMirror.GetCode = CODE_GET_MIRROR;

#if ((BOARD_FEATURE & L2_PORT_MIRROR) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if(eeprom_read(NVRAM_PORT_MIRROR_CFG_BASE, &(RspGetMirror.PortNum), MAX_PORT_NUM * 4 + 1) != I2C_SUCCESS) {
		RspGetMirror.RetCode = 0x01;
	} else {
		if((RspGetMirror.PortNum == 0) || (RspGetMirror.PortNum > MAX_PORT_NUM)) {
			memset(&RspGetMirror, 0, sizeof(obnet_rsp_get_port_mirror));
			RspGetMirror.GetCode = CODE_GET_MIRROR;
			RspGetMirror.RetCode = 0x01;
		} else
			RspGetMirror.RetCode = 0x00;		
	}
#else
	RspGetMirror.RetCode = 0x01;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetMirror, sizeof(obnet_rsp_get_port_mirror));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_port_mirror);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}

#endif	/* MODULE_OBNMS */



