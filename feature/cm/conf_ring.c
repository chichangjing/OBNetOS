
/*************************************************************
 * Filename     : conf_ring.c
 * Description  : API for ring configuration
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

#include "mconfig.h"

#if MODULE_RING

/* Standard includes */
#include "string.h"

/* BSP includes */
#include "stm32f2xx.h"
#include "soft_i2c.h"
#include "conf_comm.h"
#include "conf_map.h"
#include "conf_ring.h"

#include "obring.h"

int conf_get_ring_num(u8 *ring_num)
{
	tRingConfigGlobal RingGlobalCfg;
	
	if(eeprom_read(NVRAM_RING_GLOBAL_CFG_BASE, (u8 *)&RingGlobalCfg, sizeof(tRingConfigGlobal)) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	if(RingGlobalCfg.ucGlobalEnable == 0x01)
		*ring_num = RingGlobalCfg.ucRecordNum;
	else
		*ring_num = 0;
	
	return CONF_ERR_NONE;
}

int conf_set_ring_global(tRingConfigGlobal *RingGlobalCfg)
{
	if(eeprom_page_write(NVRAM_RING_GLOBAL_CFG_BASE, (u8 *)RingGlobalCfg, sizeof(tRingConfigGlobal)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_get_ring_global(tRingConfigGlobal *RingGlobalCfg)
{
	if(eeprom_read(NVRAM_RING_GLOBAL_CFG_BASE, (u8 *)RingGlobalCfg, sizeof(tRingConfigGlobal)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_set_ring_record(unsigned char RecIndex, tRingConfigRec *RingConfigRecord)
{
	if(eeprom_page_write(NVRAM_RING_RECORD_CFG_BASE +  RecIndex * NVRAM_RING_RECORD_SIZE, (u8 *)RingConfigRecord, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_get_ring_record(unsigned char RecIndex, tRingConfigRec *RingConfigRecord)
{
	if(eeprom_read(NVRAM_RING_RECORD_CFG_BASE +  RecIndex * NVRAM_RING_RECORD_SIZE, (u8 *)RingConfigRecord, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_set_ring_disable(unsigned char RecIndex)
{
	tRingConfigRec RingConfigRec;
		
	if(eeprom_read(NVRAM_RING_RECORD_CFG_BASE +  RecIndex * NVRAM_RING_RECORD_SIZE, (u8 *)&RingConfigRec, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	RingConfigRec.ucEnable = 0x00;
	
	if(eeprom_page_write(NVRAM_RING_RECORD_CFG_BASE +  RecIndex * NVRAM_RING_RECORD_SIZE, (u8 *)&RingConfigRec, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_set_ring_enable(unsigned char RecIndex)
{
	tRingConfigRec RingConfigRec;
		
	if(eeprom_read(NVRAM_RING_RECORD_CFG_BASE +  RecIndex * NVRAM_RING_RECORD_SIZE, (u8 *)&RingConfigRec, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	RingConfigRec.ucEnable = 0x01;
	
	if(eeprom_page_write(NVRAM_RING_RECORD_CFG_BASE +  RecIndex * NVRAM_RING_RECORD_SIZE, (u8 *)&RingConfigRec, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

#if 0
int conf_set_ring_config(ring_conf_t *cfg_data)
{
	if(eeprom_write(NVRAM_RING_CFG_BASE, (u8 *)cfg_data, sizeof(ring_conf_t)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_get_ring_config(ring_conf_t *cfg_data)
{
	if(eeprom_read(NVRAM_RING_CFG_BASE, (u8 *)cfg_data, sizeof(ring_conf_t)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}
#endif

#endif


