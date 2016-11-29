/*******************************************************************
 * Filename     : hal_swif_qos.c
 * Description  : Hardware Abstraction Layer for L2 Switch QoS API
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
#include "hal_swif_qos.h"

/* Other includes */
#include "cli_util.h"

#if MARVELL_SWITCH
extern GT_QD_DEV *dev;
#endif

/**************************************************************************
  * @brief  Get qos schedule mode
  * @param  qos_mode
  * @retval 
  *************************************************************************/
int hal_swif_get_qos_schedule_mode(HAL_QOS_MODE *qos_mode)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_BOOL	mode;  

	if((status = gsysGetSchedulingMode(dev,&mode)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}
    if(mode == GT_FALSE){
        *qos_mode = QOS_MODE_SP;
    } else {
        *qos_mode = QOS_MODE_WWR;
    }

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  Set qos schedule mode
  * @param  qos_mode
  * @retval 
  *************************************************************************/
int hal_swif_set_qos_schedule_mode(HAL_QOS_MODE qos_mode)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_BOOL	mode;  

	if(qos_mode == QOS_MODE_SP) {
		mode = GT_FALSE;
	} else {
		mode = GT_TRUE;
	}

	if((status = gsysSetSchedulingMode(dev,mode)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  Get IEEETag priority to queue map
  * @param  priority: 0 ~ 7, 3 bits
  *			queue_num: 0-3
  * @retval 
  *************************************************************************/
int hal_swif_get_qos_cos_to_queue_map(uint8 priority, uint8 *queue_num)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;

	if((status = gcosGetUserPrio2Tc(dev, priority, queue_num)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  Set IEEETag priority to queue map
  * @param  priority: 0 ~ 7, 3 bits
  *			queue_num: 0-3
  * @retval 
  *************************************************************************/
int hal_swif_set_qos_cos_to_queue_map(uint8 priority, uint8 queue_num)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;

	if((status = gcosSetUserPrio2Tc(dev, priority, queue_num)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  Get IPTosOrDSCP to queue map
  * @param  priority: 0 ~ 63, 6 bits
  *			queue_num: 0-3
  * @retval 
  *************************************************************************/
int hal_swif_get_qos_tos_to_queue_map(uint8 priority, uint8 *queue_num)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;

	if((status = gcosGetDscp2Tc(dev, priority, queue_num)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif		
}

/**************************************************************************
  * @brief  Set IPTosOrDSCP to queue map
  * @param  priority: 0 ~ 63, 6 bits
  *			queue_num: 0-3
  * @retval 
  *************************************************************************/
int hal_swif_set_qos_tos_to_queue_map(uint8 priority, uint8 queue_num)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;

	if((status = gcosSetDscp2Tc(dev, priority, queue_num)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif		
}

/**************************************************************************
  * @brief  Get user IEEETags to priority mapping
  * @param  lport: logic port
  *			enable: 1-enable  0-disable
  * @retval 
  *************************************************************************/
int hal_swif_get_qos_priority_cos_enable(uint8 lport, HAL_BOOL *enable)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_LPORT hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	hport = hal_swif_lport_2_hport(lport);

	if((status = gqosGetUserPrioMapEn(dev, hport, (GT_BOOL *)enable)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  Use IEEETags to priority mapping
  * @param  lport: logic port
  *			enable: 1-enable  0-disable
  * @retval 
  *************************************************************************/
int hal_swif_set_qos_priority_cos_enable(uint8 lport, HAL_BOOL enable)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_LPORT hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	hport = hal_swif_lport_2_hport(lport);
	
	if((status = gqosUserPrioMapEn(dev, hport, enable)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  Get User TOS and/or DiffServ fields to priority mapping
  * @param  lport: logic port
  *			enable: 1-enable  0-disable
  * @retval 
  *************************************************************************/
int hal_swif_get_qos_priority_tos_enable(uint8 lport, HAL_BOOL *enable)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_LPORT hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	hport = hal_swif_lport_2_hport(lport);
	
	if((status = gqosGetIpPrioMapEn(dev, hport, (OUT GT_BOOL *)enable)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  Use TOS and/or DiffServ fields to priority mapping
  * @param  lport: logic port
  *			enable: 1-enable  0-disable
  * @retval 
  *************************************************************************/
int hal_swif_set_qos_priority_tos_enable(uint8 lport, HAL_BOOL enable)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_LPORT hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	hport = hal_swif_lport_2_hport(lport);
	
	if((status = gqosIpPrioMapEn(dev, hport, enable)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  Get IEEE Tag has higher/lower priority than IP priority fields
  * @param  enable: 0: use ip fields for priority mapping when both fields are set
  *					1: use tag fields for priority mapping when both fields are set
  * @retval 
  *************************************************************************/
int hal_swif_get_qos_priority_both_enable(uint8 lport, HAL_BOOL *enable)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_LPORT hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	hport = hal_swif_lport_2_hport(lport);
	
	if((status = gqosGetPrioMapRule(dev, hport, (GT_BOOL *)enable)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  IEEE Tag has higher/lower priority than IP priority fields
  * @param  enable: 0: use ip fields for priority mapping when both fields are set
  *					1: use tag fields for priority mapping when both fields are set
  * @retval 
  *************************************************************************/
int hal_swif_set_qos_priority_both_enable(uint8 lport, HAL_BOOL enable)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_LPORT hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	hport = hal_swif_lport_2_hport(lport);
	
	if((status = gqosSetPrioMapRule(dev, hport, enable)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}

	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif	
}

/**************************************************************************
  * @brief  Port default priority get
  * @param  lport: logic port
  *         priority_level: 0 ~ 3, 2 bits
  * @retval 
  *************************************************************************/
int hal_swif_get_qos_port_default_prority_level(uint8 lport, uint8 *priority_level)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_LPORT hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	hport = hal_swif_lport_2_hport(lport);
	
	if((status = gcosGetPortDefaultTc(dev, hport,(GT_U8 *)priority_level)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}
	
	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}


/**************************************************************************
  * @brief  This routine get the port egress mode.
  * @param  lport: logic port
  *         egress_mode: 
  * @retval 
  *************************************************************************/
int hal_swif_get_port_egress_mode(uint8 lport, GT_EGRESS_MODE *egress_mode)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_LPORT hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	hport = hal_swif_lport_2_hport(lport);
	
    if((status = gprtGetEgressMode(dev, hport, egress_mode)) != GT_OK) {
        return HAL_SWIF_ERR_MSAPI;
    }
	
	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}

/**************************************************************************
  * @brief  This routine set the port egress mode.
  * @param  lport: logic port
  *         egress_mode: 
  * @retval 
  *************************************************************************/
int hal_swif_set_port_egress_mode(uint8 lport, GT_EGRESS_MODE egress_mode)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_LPORT hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	hport = hal_swif_lport_2_hport(lport);
	
    if((status = gprtSetEgressMode(dev, hport, (GT_EGRESS_MODE)egress_mode)) != GT_OK) {
        return HAL_SWIF_ERR_MSAPI;
    }
	
	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}

/**************************************************************************
  * @brief  Port default priority set
  * @param  lport: logic port
  *         priority_level: 0 ~ 3, 2 bits
  * @retval 
  *************************************************************************/
int hal_swif_set_qos_port_default_prority_level(uint8 lport, uint8 priority_level)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_LPORT hport;
	
	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;

	hport = hal_swif_lport_2_hport(lport);
	
	if((status = gcosSetPortDefaultTc(dev, hport, priority_level * 2)) != GT_OK) {
		return HAL_SWIF_ERR_MSAPI;
	}
	
	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}

/**************************************************************************
  * @brief  Initialize configuration for QoS
  * @param  none
  * @retval none
  *************************************************************************/
int hal_swif_qos_conf_initialize(void)
{
#if SWITCH_CHIP_88E6095
	int status;
	hal_qos_conf_t PortQosConf;
	HAL_QOS_MODE qos_mode;
	HAL_BOOL prio_enable;
	uint8 lport, hport, queue_num, priority_level, egress_mode;
	uint8 byte_index, shift_bits;
	uint8 i, prior;
	
	if(eeprom_read(NVRAM_QOS_CFG_BASE, (u8 *)&PortQosConf, sizeof(hal_qos_conf_t)) != I2C_SUCCESS)
        return CONF_ERR_I2C;
    
    if((PortQosConf.PortNum == 0) || (PortQosConf.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;
    	 
    /*	IEEE802.3ac Tag Priority Mapping */
    for(i=0, prior=7; i<8; i++, prior--) {
        byte_index = i/4;
        shift_bits = (prior%4)*2;
        queue_num = ((PortQosConf.CosMapping[byte_index] >> shift_bits) & 0x03);
        status = hal_swif_set_qos_cos_to_queue_map(prior, queue_num);
		if(status != HAL_SWIF_SUCCESS) 
            return CONF_ERR_MSAPI;
    }
    
	/* IPv4/IPv6 Priority Mapping */
	for(i=0, prior=63; i<64; i++, prior--) {   
        byte_index = i/4;
        shift_bits = (prior%4)*2;
        queue_num = ((PortQosConf.TosMapping[byte_index] >> shift_bits) & 0x03);
        status = hal_swif_set_qos_tos_to_queue_map(prior, queue_num);
		if(status != HAL_SWIF_SUCCESS) 
            return CONF_ERR_MSAPI;
	} 
    
    /* Set queuing schemes mode */    
    if(PortQosConf.QosFlag[1] & 0x01) {
		qos_mode = QOS_MODE_SP;
    } else {
		qos_mode = QOS_MODE_WWR;
    }
    status = hal_swif_set_qos_schedule_mode(qos_mode);
	if(status != HAL_SWIF_SUCCESS) 
        return CONF_ERR_MSAPI;

	/* QoS port config */
	for(lport=1; lport<=PortQosConf.PortNum; lport++) {
		#if 1
		/* Enable/Disable IEEETag priority mapping */
		if(PortQosConf.PortQosConfig[lport-1] & QOS_MASK_PRIORITY_MAP_COS) 
			prio_enable = HAL_TRUE;
		else 
			prio_enable = HAL_FALSE;
        status = hal_swif_set_qos_priority_cos_enable(lport, prio_enable);
		if(status != HAL_SWIF_SUCCESS) 
            return CONF_ERR_MSAPI;

		/* Enable/Disable TosOrDSCP priority mapping */
		if(PortQosConf.PortQosConfig[lport-1] & QOS_MASK_PRIORITY_MAP_TOS) 
			prio_enable = HAL_TRUE;
		else 
			prio_enable = HAL_FALSE;		
        status = hal_swif_set_qos_priority_tos_enable(lport, prio_enable);
		if(status != HAL_SWIF_SUCCESS) 
            return CONF_ERR_MSAPI;

		/* Priority mapping using when both fields are set */
		if(PortQosConf.PortQosConfig[lport-1] & QOS_MASK_PRIORITY_MAP_BOTH) 
			prio_enable = HAL_TRUE;
		else 
			prio_enable = HAL_FALSE;		
        status = hal_swif_set_qos_priority_both_enable(lport, prio_enable);
		if(status != HAL_SWIF_SUCCESS) 
            return CONF_ERR_MSAPI;	

		/* Set Port Qos default priority */
		priority_level = (PortQosConf.PortQosConfig[lport-1] & QOS_MASK_DEFAULT_PRIORITY) >> 2;	
        status = hal_swif_set_qos_port_default_prority_level(lport, priority_level);
		if(status != HAL_SWIF_SUCCESS) 
            return CONF_ERR_MSAPI;	

		egress_mode = PortQosConf.PortQosConfig[lport-1] & QOS_MASK_EGRESS_MODE;
        status = hal_swif_set_port_egress_mode(lport, (GT_EGRESS_MODE)egress_mode);
        if(status != HAL_SWIF_SUCCESS) 
            return CONF_ERR_MSAPI;
		#else
        GT_U8 port     = 0;
        GT_U8 tmpMask   = 0;
        GT_U8 tmpData   = 0;
        GT_BOOL en   = GT_FALSE;
        GT_EGRESS_MODE  mode;
        IN GT_U8 trClass = 0;
		
        port = hal_swif_lport_2_hport(lport);
        
        tmpData = PortQosConf.PortQosConfig[lport-1];
        QOS_MASK(4, 1, tmpMask);
        tmpData &= tmpMask;
        en = (tmpData == QOS_COS_MAP_EN)?GT_TRUE:GT_FALSE;
         /* Use IEEE Tag */
		if((status = gqosUserPrioMapEn(dev,port,en)) != GT_OK)
			return CONF_ERR_MSAPI;

        tmpData = PortQosConf.PortQosConfig[lport-1];
        QOS_MASK(5, 1, tmpMask);
        tmpData &= tmpMask;
        en = (tmpData == QOS_TOS_MAP_EN)?GT_TRUE:GT_FALSE;
		/* Use IPv4/IPv6 priority fields (use IP) */
		if((status = gqosIpPrioMapEn(dev,port,en)) != GT_OK)
			return CONF_ERR_MSAPI;

        tmpData = PortQosConf.PortQosConfig[lport-1];
        QOS_MASK(6, 1, tmpMask);
        tmpData &= tmpMask;
        en = (tmpData == QOS_PRIO_MAP_RULE)?GT_TRUE:GT_FALSE;
		/* IEEE Tag has higher priority than IP priority fields */
		if((status = gqosSetPrioMapRule(dev,port,en)) != GT_OK)
			return CONF_ERR_MSAPI;
		

        tmpData = PortQosConf.PortQosConfig[lport-1];
        QOS_MASK(2, 2, tmpMask);
        tmpData &= tmpMask;
        trClass = (tmpData >> 2)*2;
        /* Each port's default priority is set to [trClass]. */
		if((status = gcosSetPortDefaultTc(dev,port,trClass)) != GT_OK)
			return CONF_ERR_MSAPI;
		

        tmpData = PortQosConf.PortQosConfig[lport-1];
        QOS_MASK(0, 2, tmpMask);
        tmpData &= tmpMask;
        mode = tmpData;
        /* Each port's EgressMode is set to [mode]. */
        if((status = gprtSetEgressMode(dev,port,mode)) != GT_OK)
			return CONF_ERR_MSAPI;
        		
		#endif
	}

	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}

#if MODULE_OBNMS

extern u8 NMS_TxBuffer[];

void nms_rsp_set_qos(u8 *DMA, u8 *RequestID, obnet_set_qos *pQos)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_QOS;

#if ((BOARD_FEATURE & L2_QOS) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if((pQos->PortNum > MAX_PORT_NUM) || (pQos->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = 0x10;
	} else {
		if(eeprom_page_write(NVRAM_QOS_CFG_BASE, (u8 *)&(pQos->CosMapping[0]), 20 + pQos->PortNum + 1) != I2C_SUCCESS) {
			RspSet.RetCode = 0x01;
			RspSet.Res = 0x0F;	
		} else {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		}	
	}
#else
	RspSet.RetCode = 0x01;
	RspSet.Res = 0x1F;
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

void nms_rsp_get_qos(u8 *DMA, u8 *RequestID)
{
	obnet_rsp_get_qos RspGetQos;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	/************************************************/
	/* To add */
	memset(&RspGetQos, 0, sizeof(obnet_rsp_get_qos));
	RspGetQos.GetCode = CODE_GET_QOS;

#if ((BOARD_FEATURE & L2_QOS) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if(eeprom_read(NVRAM_QOS_CFG_BASE, (u8 *)&(RspGetQos.CosMapping[0]), 20 + MAX_PORT_NUM + 1) != I2C_SUCCESS) {
		RspGetQos.RetCode = 0x01;
	} else {
		if((RspGetQos.PortNum == 0) || (RspGetQos.PortNum > MAX_PORT_NUM)) {
			memset(&RspGetQos, 0, sizeof(obnet_rsp_get_qos));
			RspGetQos.GetCode = CODE_GET_QOS;
			RspGetQos.RetCode = 0x01;
		} else
			RspGetQos.RetCode = 0x00;
	}
#else
	RspGetQos.RetCode = 0x01;
#endif
	
	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetQos, sizeof(obnet_rsp_get_qos));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_qos);
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


