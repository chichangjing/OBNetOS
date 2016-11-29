
/*******************************************************************
 * Filename     : hal_swif_rate_ctrl.c
 * Description  : Hardware Abstraction Layer for L2 Switch Port API
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *******************************************************************/
#include "mconfig.h"

/* Standard includes */
#include <stdio.h>

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
#include "hal_swif_rate_ctrl.h"

/* Other includes */
#include "cli_util.h"

#if MARVELL_SWITCH
extern GT_QD_DEV *dev;
#endif

/**************************************************************************
  * @brief  set logic port ingress rate
  * @param  lport
  * @retval 
  *************************************************************************/
int hal_swif_set_ingress_rate
(	uint8 lport, 
	HAL_INGRESS_FRAME_TYPE frame_type, 
	HAL_INGRESS_RATE ingress_rate,
	HAL_BOOL pri1_mode, 
	HAL_BOOL pri2_mode, 
	HAL_BOOL pri3_mode
)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_U32 hport;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	hport = hal_swif_lport_2_hport(lport);

	if(ingress_rate != RATE_NO_LIMIT) {
	    if((status = grcSetLimitMode(dev, hport, frame_type)) != GT_OK) {
	        return HAL_SWIF_ERR_MSAPI;
	    }
		if((status = grcSetPri1Rate(dev, hport, pri1_mode)) != GT_OK) {
			return HAL_SWIF_ERR_MSAPI;
        }
		if((status = grcSetPri2Rate(dev, hport, pri2_mode)) != GT_OK) {
			return HAL_SWIF_ERR_MSAPI;
        }
		if((status = grcSetPri3Rate(dev, hport, pri3_mode)) != GT_OK) {
			return HAL_SWIF_ERR_MSAPI;
        }		
		if((status = grcSetPri0Rate(dev, hport, ingress_rate)) != GT_OK) {
			return HAL_SWIF_ERR_MSAPI;
        }
	}
	
	return HAL_SWIF_SUCCESS;
#else
	return HAL_SWIF_FAILURE;
#endif
}

/**************************************************************************
  * @brief  set logic port egress rate
  * @param  lport
  * @retval 
  *************************************************************************/
int hal_swif_set_egress_rate(uint8 lport, HAL_EGRESS_RATE egress_rate)
{
#if SWITCH_CHIP_88E6095
	GT_STATUS status;
	GT_U32 hport;

	if(lport > MAX_PORT_NUM)
		return HAL_SWIF_ERR_INVALID_LPORT;
	hport = hal_swif_lport_2_hport(lport);
	
	if(egress_rate != RATE_NO_LIMIT) {
		if((status = grcSetEgressRate(dev, hport, egress_rate)) != GT_OK) {
			return HAL_SWIF_ERR_MSAPI;
        }
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
int hal_swif_rate_ctrl_conf_initialize(void)
{
#if SWITCH_CHIP_88E6095
	uint8 lport;
	hal_rate_conf_t PortRateConfig;
	uint16 single_port_rate_cfg;
	HAL_INGRESS_FRAME_TYPE rate_limit_mode;
	HAL_INGRESS_RATE ingress_rate, pri0_rate;
	HAL_EGRESS_RATE egress_rate;
	HAL_BOOL pri1_mode, pri2_mode, pri3_mode, pri0_mode, ingress_limit_enable, egress_limit_enable;
	int ret;
	
	if(eeprom_read(NVRAM_PORT_RATE_CFG_BASE, &(PortRateConfig.PortNum), MAX_PORT_NUM * 2 + 1) != I2C_SUCCESS) 
		return CONF_ERR_I2C;

	if((PortRateConfig.PortNum == 0) || (PortRateConfig.PortNum > MAX_PORT_NUM))
		return CONF_ERR_NO_CFG;

	for(lport=1; lport<=PortRateConfig.PortNum; lport++) {
		single_port_rate_cfg = *(uint16 *)&(PortRateConfig.RateCtrlConfig[(lport-1)*2]);
		single_port_rate_cfg = cli_ntohs(single_port_rate_cfg);
		
#if (OB_NMS_PROTOCOL_VERSION == 1)
		rate_limit_mode = (HAL_INGRESS_FRAME_TYPE)((single_port_rate_cfg & RATE_CTRL_MASK_INGRESS_LIMIT_MODE) >> 14);
		pri3_mode = (HAL_BOOL)((single_port_rate_cfg & RATE_CTRL_MASK_INGRESS_PRI3_RATE) >> 13);
		pri2_mode = (HAL_BOOL)((single_port_rate_cfg & RATE_CTRL_MASK_INGRESS_PRI2_RATE) >> 12);
		pri1_mode = (HAL_BOOL)((single_port_rate_cfg & RATE_CTRL_MASK_INGRESS_PRI1_RATE) >> 11);
		pri0_rate = (HAL_INGRESS_RATE)((single_port_rate_cfg & RATE_CTRL_MASK_INGRESS_PRI0_RATE) >> 8);
		egress_rate = (HAL_EGRESS_RATE)(single_port_rate_cfg & RATE_CTRL_MASK_EGRESS_RATE);

		if(hal_swif_set_ingress_rate(lport, rate_limit_mode, pri0_rate, pri1_mode, pri2_mode, pri3_mode) != HAL_SWIF_SUCCESS)
			return CONF_ERR_SWITCH_HAL;
		
		if(hal_swif_set_egress_rate(lport, (HAL_EGRESS_RATE)egress_rate) != HAL_SWIF_SUCCESS)
			return CONF_ERR_SWITCH_HAL;	
#else
		rate_limit_mode = (HAL_INGRESS_FRAME_TYPE)((single_port_rate_cfg & RATE_CTRL_MASK_INGRESS_LIMIT_MODE) >> 14);
		ingress_rate = (HAL_INGRESS_RATE)((single_port_rate_cfg & RATE_CTRL_MASK_INGRESS_RATE) >> 8);
		egress_rate = (HAL_EGRESS_RATE)(single_port_rate_cfg & RATE_CTRL_MASK_EGRESS_RATE);
		ingress_limit_enable = (HAL_BOOL)((single_port_rate_cfg & RATE_CTRL_MASK_INGRESS_ENBALE) >> 13);
		egress_limit_enable = (HAL_BOOL)((single_port_rate_cfg & RATE_CTRL_MASK_EGRESS_ENABLE) >> 12);

		if(ingress_limit_enable) {
			if(hal_swif_set_ingress_rate(lport, rate_limit_mode, ingress_rate, HAL_FALSE, HAL_FALSE, HAL_FALSE) != HAL_SWIF_SUCCESS)
				return CONF_ERR_SWITCH_HAL;
		}

		if(egress_limit_enable) {
			if(hal_swif_set_egress_rate(lport, (HAL_EGRESS_RATE)egress_rate) != HAL_SWIF_SUCCESS)
				return CONF_ERR_SWITCH_HAL;
		}
#endif
	}

	return CONF_ERR_NONE;
#else
	return CONF_ERR_NOT_SUPPORT;
#endif
}

#if MODULE_OBNMS

extern u8 NMS_TxBuffer[];

void nms_rsp_set_rate_ctrl(u8 *DMA, u8 *RequestID, obnet_set_rate_ctrl * pRateCtrl)
{
	OBNET_SET_RSP RspSet;	
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_RATE;

#if ((BOARD_FEATURE & L2_PORT_RATE_CTRL) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if((pRateCtrl->PortNum > MAX_PORT_NUM) || (pRateCtrl->PortNum == 0)) {
		RspSet.RetCode = 0x01;
		RspSet.Res = RSP_ERR_INVALID_CONFIGURATION;
	} else {
		if(eeprom_page_write(NVRAM_PORT_RATE_CFG_BASE, (u8 *)&(pRateCtrl->PortNum), pRateCtrl->PortNum * 2 + 1) != I2C_SUCCESS) {
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

void nms_rsp_get_rate_ctrl(u8 *DMA, u8 *RequestID)
{
	obnet_rsp_get_rate_ctrl RspGetRateCtrl;
	u16 RspLength;
	u8	PortNum;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	/* fill the response data */
	memset(&RspGetRateCtrl, 0, sizeof(obnet_rsp_get_rate_ctrl));
	RspGetRateCtrl.GetCode = CODE_GET_RATE;

#if ((BOARD_FEATURE & L2_PORT_RATE_CTRL) && (SWITCH_CHIP_TYPE == CHIP_88E6095))
	if(eeprom_read(NVRAM_PORT_RATE_CFG_BASE, &(RspGetRateCtrl.PortNum), 1) != I2C_SUCCESS) {
		RspGetRateCtrl.RetCode = 0x01;
	} else {
		if((RspGetRateCtrl.PortNum == 0) || (RspGetRateCtrl.PortNum > MAX_PORT_NUM)) {
			RspGetRateCtrl.RetCode = 0x01;
		} else {
			if(eeprom_read(NVRAM_PORT_RATE_CFG_BASE + 1, &(RspGetRateCtrl.RateControl[0]), RspGetRateCtrl.PortNum * 2) != I2C_SUCCESS) {
				RspGetRateCtrl.RetCode = 0x01;
			} else {
				RspGetRateCtrl.RetCode = 0x00;
			}
		}
	}
#else
	RspGetRateCtrl.RetCode = 0x01;
#endif

	if(RspGetRateCtrl.RetCode == 0x01) {
		memset(&RspGetRateCtrl, 0, sizeof(obnet_rsp_get_rate_ctrl));
		RspGetRateCtrl.GetCode = CODE_GET_RATE;
		RspGetRateCtrl.RetCode = 0x01;
	}
	
	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetRateCtrl, sizeof(obnet_rsp_get_rate_ctrl));
	RspLength = PAYLOAD_OFFSET + sizeof(obnet_rsp_get_rate_ctrl);
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


