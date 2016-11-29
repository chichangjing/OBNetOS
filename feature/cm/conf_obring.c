

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

#include "obring.h"

#include "conf_comm.h"
#include "conf_map.h"


int conf_obring_read_global(tRingConfigGlobal *RingGlobalCfg)
{
	if(eeprom_read(NVRAM_OBRING_CFG_BASE, (u8 *)RingGlobalCfg, sizeof(tRingConfigGlobal)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_obring_write_global(tRingConfigGlobal *RingGlobalCfg)
{
	if(eeprom_page_write(NVRAM_OBRING_CFG_BASE, (u8 *)RingGlobalCfg, sizeof(tRingConfigGlobal)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_obring_read_record(tRingConfigRec *RingRecordCfg, unsigned char RecIndex)
{
	if(eeprom_read(NVRAM_OBRING_RECORD_CFG_BASE + RecIndex*NVRAM_OBRING_RECORD_SIZE, (u8 *)RingRecordCfg, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_obring_write_record(tRingConfigRec *RingRecordCfg, unsigned char RecIndex)
{
	if(eeprom_page_write(NVRAM_OBRING_RECORD_CFG_BASE + RecIndex*NVRAM_OBRING_RECORD_SIZE, (u8 *)RingRecordCfg, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_obring_enable_demain_id(unsigned char RecIndex)
{
	u8 ucData = 0x01;
	
	if(eeprom_page_write(NVRAM_OBRING_RECORD_CFG_BASE + RecIndex*NVRAM_OBRING_RECORD_SIZE, (u8 *)&ucData, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_obring_disable_demain_id(unsigned char RecIndex)
{
	u8 ucData = 0x00;
	
	if(eeprom_page_write(NVRAM_OBRING_RECORD_CFG_BASE + RecIndex*NVRAM_OBRING_RECORD_SIZE, (u8 *)&ucData, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_obring_write_hello_times(unsigned char RecIndex, unsigned short HelloTimes)
{
	tRingConfigRec RingRecCfg;

	if(eeprom_read(NVRAM_OBRING_RECORD_CFG_BASE + RecIndex*NVRAM_OBRING_RECORD_SIZE, (u8 *)&RingRecCfg, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	RingRecCfg.usHelloTime[0] = (unsigned char)((HelloTimes & 0xFF00) >> 8);
	RingRecCfg.usHelloTime[1] = (unsigned char)(HelloTimes & 0x00FF);
			
	if(eeprom_page_write(NVRAM_OBRING_RECORD_CFG_BASE + RecIndex*NVRAM_OBRING_RECORD_SIZE, (u8 *)&RingRecCfg, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_obring_write_fail_times(unsigned char RecIndex, unsigned short FailTimes)
{
	tRingConfigRec RingRecCfg;

	if(eeprom_read(NVRAM_OBRING_RECORD_CFG_BASE + RecIndex*NVRAM_OBRING_RECORD_SIZE, (u8 *)&RingRecCfg, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	RingRecCfg.usFailTime[0] = (unsigned char)((FailTimes & 0xFF00) >> 8);
	RingRecCfg.usFailTime[1] = (unsigned char)(FailTimes & 0x00FF);
			
	if(eeprom_page_write(NVRAM_OBRING_RECORD_CFG_BASE + RecIndex*NVRAM_OBRING_RECORD_SIZE, (u8 *)&RingRecCfg, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}


int conf_obring_write_ring_port(unsigned char RecIndex, unsigned char RingPort1, unsigned char RingPort2)
{
	tRingConfigRec RingRecCfg;

	if(eeprom_read(NVRAM_OBRING_RECORD_CFG_BASE + RecIndex*NVRAM_OBRING_RECORD_SIZE, (u8 *)&RingRecCfg, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	RingRecCfg.ucPrimaryPort = RingPort1;
	RingRecCfg.ucSecondaryPort = RingPort2;
	
	if(eeprom_page_write(NVRAM_OBRING_RECORD_CFG_BASE + RecIndex*NVRAM_OBRING_RECORD_SIZE, (u8 *)&RingRecCfg, sizeof(tRingConfigRec)) != I2C_SUCCESS)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

#endif



