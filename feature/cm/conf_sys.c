
/*************************************************************
 * Filename     : conf_conf.c
 * Description  : API for system configuration
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
 
/* Standard includes */
#include "stdio.h"
#include "ctype.h"
#include "string.h"

/* BSP includes */
#include "soft_i2c.h"

/* Other includes */
#include "conf_comm.h"
#include "conf_map.h"
#include "conf_sys.h"

int conf_set_upgrade_flag(void)
{
	u8 upgrade_flag;
	
	upgrade_flag = 0x80;
	if(eeprom_write(NVRAM_UPGRADE_FLAG, &upgrade_flag, 1) == I2C_FAILURE)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int conf_clear_upgrade_flag(void)
{
	u8 upgrade_flag;
	
	upgrade_flag = 0x00;
	if(eeprom_write(NVRAM_UPGRADE_FLAG, &upgrade_flag, 1) == I2C_FAILURE)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int conf_cli_login_disable(void)
{
	u8 disable;
	
	disable = 0x01;
	if(eeprom_write(NVRAM_CLI_LOGIN_DISABLE, &disable, 1) == I2C_FAILURE)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int conf_cli_login_enable(void)
{
	u8 disable;
	
	disable = 0xff;
	if(eeprom_write(NVRAM_CLI_LOGIN_DISABLE, &disable, 1) == I2C_FAILURE)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int conf_get_cli_login_switch(u8 *disable)
{
	if(eeprom_read(NVRAM_CLI_LOGIN_DISABLE, disable, 1) == I2C_FAILURE)
		return CONF_ERR_I2C;
		
	return CONF_ERR_NONE;
}

int conf_get_bootdelay(u8 *seconds)
{
	if(eeprom_read(NVRAM_BOOT_DELAY, seconds, 1) == I2C_FAILURE)
		return CONF_ERR_I2C;

	return CONF_ERR_NONE;
}

int conf_set_bootdelay(u8 *seconds)
{
	if(eeprom_write(NVRAM_BOOT_DELAY, seconds, 1) == I2C_FAILURE)
		return CONF_ERR_I2C;

	return CONF_ERR_NONE;
}

int conf_get_mac_address(u8 *mac)
{
	int ret;
	u8 invalid_mac1[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	u8 invalid_mac2[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	u8 data[6];

	ret = eeprom_read(NVRAM_MAC, data, 6);
	if(ret) {
		if((memcmp(data, invalid_mac1, 6) == 0) || (memcmp(data, invalid_mac2, 6) == 0))
			return CONF_ERR_INVALID_MAC;
		else 
			memcpy(mac, data, 6);	
	} else {
		return CONF_ERR_I2C;
	}

	return CONF_ERR_NONE;
}

int conf_set_mac_address(u8 *mac)
{
	u8 invalid_mac1[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	u8 invalid_mac2[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if((memcmp(mac, invalid_mac1, 6) == 0) || (memcmp(mac, invalid_mac2, 6) == 0)) {
		return CONF_ERR_INVALID_MAC;
	}

	if(eeprom_write(NVRAM_MAC, mac, 6) == I2C_FAILURE)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int conf_set_device_name(u8 *dev_name)
{
	return CONF_ERR_NONE;
}

int conf_set_serial_number(u8 *serial_number)
{
	return CONF_ERR_NONE;
}

int conf_set_name_id(u8 *dev_name, u8 *serial_number)
{
	int ret = CONF_ERR_NONE;
	
	if(strlen((char *)dev_name) > MAX_DEVICE_NAME_LEN)
		ret = CONF_ERR_OVERLENGTH;

	if(strlen((char *)serial_number) > MAX_SERIAL_NUMBER_LEN)
		ret = CONF_ERR_OVERLENGTH;
	
	if(eeprom_page_write(NVRAM_DEVICE_NAME, dev_name, (u8)(MAX_DEVICE_NAME_LEN + 1)) != I2C_SUCCESS)
		ret = CONF_ERR_I2C;

	if(eeprom_page_write(NVRAM_SERIAL_NUMBER, serial_number, (u8)(MAX_SERIAL_NUMBER_LEN + 1)) != I2C_SUCCESS)
		ret = CONF_ERR_I2C;
	
	return ret;
}

int conf_get_name_id(u8 *dev_name, u8 *serial_number)
{
	int ret = CONF_ERR_NONE;
	
	if(eeprom_read(NVRAM_DEVICE_NAME, dev_name, (u8)(MAX_DEVICE_NAME_LEN+1)) != I2C_SUCCESS) 
		ret = CONF_ERR_I2C;

	if(isprint(dev_name[0]) == 0) {
		memset(dev_name, 0, MAX_DEVICE_NAME_LEN+1);
	}

	if(eeprom_read(NVRAM_SERIAL_NUMBER, serial_number, (u8)(MAX_SERIAL_NUMBER_LEN+1)) != I2C_SUCCESS)
		ret = CONF_ERR_I2C;

	if(isprint(serial_number[0]) == 0) {
		memset(serial_number, 0, MAX_SERIAL_NUMBER_LEN+1);
	}
	
/*
	if(ManuInfo_Read_SerialNumber((char *)serial_number) != BDINFO_SUCCESS)
		return CONF_ERR_I2C;

	if(isalnum(serial_number[0]) == 0)
		memset(serial_number, 0, MAX_SERIAL_NUMBER_LEN+1);
*/	
	return ret;
}

int conf_set_version(u8 *version)
{
	if(eeprom_page_write(NVRAM_VERSION_BASE, version, CONF_VERSION_SIZE) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	return CONF_ERR_NONE;
}

int conf_get_version(u8 *version)
{
	if(eeprom_read(NVRAM_VERSION_BASE, version, CONF_VERSION_SIZE) != I2C_SUCCESS)
		return CONF_ERR_I2C;

	if(isprint(version[0]) == 0) {
		memset(&version[0], 0, CONF_SYS_VERSION_SIZE);
	}

	if(isprint(version[CONF_SYS_VERSION_SIZE]) == 0) {
		memset(&version[CONF_SYS_VERSION_SIZE], 0, CONF_HARDWARE_VERSION_SIZE);
	}

	if(isprint(version[CONF_SYS_VERSION_SIZE + CONF_HARDWARE_VERSION_SIZE]) == 0) {
		memset(&version[CONF_SYS_VERSION_SIZE + CONF_HARDWARE_VERSION_SIZE], 0, CONF_SOFT_VERSION_SIZE);
	}
	
	return CONF_ERR_NONE;
}

int conf_set_ip_info(u8 *ip_info)
{
	if(eeprom_page_write(NVRAM_IP, ip_info, 12) != I2C_SUCCESS)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int conf_get_ip_info(u8 *ip_info)
{
	if(eeprom_read(NVRAM_IP, ip_info, 12) != I2C_SUCCESS)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}


