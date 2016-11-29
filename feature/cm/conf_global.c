
/*************************************************************
 * Filename     : conf_global.c
 * Description  : API for alarm configuration
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

/* Standard includes */
#include "string.h"
/* LwIP includes */
#include "lwip/inet.h"
/* BSP includes */
#include "stm32f2xx.h"
#include "soft_i2c.h"
/* Config includes */
#include "conf_comm.h"
#include "conf_map.h"
#include "conf_global.h"


#include "hal_swif_types.h"
#include "hal_swif_message.h"


int conf_set_trap_enable(u8 trap_enable)
{
	if(eeprom_page_write(NVRAM_TRAP_ENABLE, (u8 *)&trap_enable, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_get_trap_enable(u8 *trap_enable)
{
	if(eeprom_read(NVRAM_TRAP_ENABLE, (u8 *)trap_enable, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_set_trap_server_mac(u8 *mac)
{
	if(eeprom_page_write(NVRAM_TRAP_SERVER_MAC, (u8 *)mac, 6) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_get_trap_server_mac(u8 *mac)
{
	if(eeprom_read(NVRAM_TRAP_SERVER_MAC, (u8 *)mac, 6) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_set_trap_frame_gate(u32 trap_gate)
{
	u8 gate[4];

	gate[0] = (u8)((trap_gate & 0xff000000) >> 24);
	gate[1] = (u8)((trap_gate & 0x00ff0000) >> 16);
	gate[2] = (u8)((trap_gate & 0x0000ff00) >> 8);
	gate[3] = (u8)(trap_gate & 0x000000ff);
	
	if(eeprom_page_write(NVRAM_TRAP_FRAME_GATE, (u8 *)&gate[0], 4) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;	
}

int conf_get_trap_frame_gate(u32 *trap_gate)
{
	u8 gate[4];
	
	if(eeprom_read(NVRAM_TRAP_FRAME_GATE, (u8 *)&gate[0], 4) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	*trap_gate = ntohl(*(u32 *)&gate[0]);
		
	return CONF_ERR_NONE;
}


int conf_trap_init(void)
{
	int i;
	tTrapConfig TrapConfig;
	const u8 mac0[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	const u8 mac1[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	u32	TrapGate;
	extern hal_trap_info_t gTrapInfo;

	memset(&gTrapInfo, 0, sizeof(hal_trap_info_t));
	
	if(eeprom_read(NVRAM_TRAP_BASE, (u8 *)&TrapConfig, sizeof(tTrapConfig)) != I2C_SUCCESS) {
		return CONF_ERR_I2C;
	}

	if(TrapConfig.TrapEnable != TRAP_ENABLE) {
		return CONF_ERR_NO_CFG;
	}
	
	if( (memcmp(TrapConfig.TrapServerMac, mac0, 6) == 0) || \
		(memcmp(TrapConfig.TrapServerMac, mac1, 6) == 0) || \
		(TrapConfig.TrapServerMac[0] & 0x1) ) 
		return CONF_ERR_INVALID_MAC;

	gTrapInfo.FeatureEnable = HAL_TRUE;
	memcpy(gTrapInfo.ServerMac, TrapConfig.TrapServerMac, 6);
	gTrapInfo.GateMask = *(u32 *)&(TrapConfig.TrapFrameGate[0]);
	gTrapInfo.GateMask = ntohl(gTrapInfo.GateMask);
	for(i=0; i<MAX_TRAP_TYPE_NUM; i++) {
		gTrapInfo.RequestID[i] = 0;
		gTrapInfo.SendEnable[i] = HAL_FALSE;
	}		
 
	return CONF_ERR_NONE;
}

int conf_alarm_kin_init(void)
{
 
	return CONF_ERR_NONE;
}
