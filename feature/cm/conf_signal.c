/*************************************************************
 * Filename     : conf_signal.c
 * Description  : API for system configuration
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/

#include "conf_signal.h"
#include "conf_global.h"

/* BSP includes */
#include "soft_i2c.h"
#include "conf_map.h"
#include "conf_comm.h"

int get_signal_en(u8 *en)
{
    u8	SigEn=0;
	
	if(eeprom_read(NVRAM_ALARM_KIN_ENABLE, &SigEn, 1) == I2C_SUCCESS) {
		*en = SigEn;
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int set_signal_en(u8 en)
{
	if(eeprom_write(NVRAM_ALARM_KIN_ENABLE, &en, 1) == I2C_SUCCESS) 
		return CONF_ERR_NONE;
	else 
		return CONF_ERR_I2C;
}

int get_sampcycle_time(u8 *time)
{
    u8	SampCycleTime=0;
	
	if(eeprom_read(NVRAM_ALARM_KIN_SAMPCYCLE, (u8 *)&SampCycleTime, 1) == I2C_SUCCESS) {
		*time = SampCycleTime;
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int set_sampcycle_time(u8 time)
{
	if(eeprom_write(NVRAM_ALARM_KIN_SAMPCYCLE, (u8 *)&time, 1) == I2C_SUCCESS)
		return CONF_ERR_NONE;
    else 
		return CONF_ERR_I2C;
}

int get_jitter_en(u8 *en)
{
    u8	JitterEn=0;
	
	if(eeprom_read(NVRAM_ALARM_KIN_JITTEREN, &JitterEn, 1) == I2C_SUCCESS) {
		*en = JitterEn;
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int set_jitter_en(u8 en)
{
	if(eeprom_write(NVRAM_ALARM_KIN_JITTEREN, &en, 1) == I2C_SUCCESS) 
		return CONF_ERR_NONE;
	else 
		return CONF_ERR_I2C; 
}

int get_jitter_probe(u8 *time)
{
    u8	JitterTime=0;
	
	if(eeprom_read(NVARM_ALARM_KIN_JITTERTIM, (u8 *)&JitterTime, 1) == I2C_SUCCESS) {
		*time = JitterTime;
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int set_jitter_probe(u8 time)
{
	if(eeprom_write(NVARM_ALARM_KIN_JITTERTIM, &time, 1) == I2C_SUCCESS) 
		return CONF_ERR_NONE;
	else 
		return CONF_ERR_I2C; 
}

int get_work_mode(u8 *mode)
{
    u8	WorkMode=0;
	
	if(eeprom_read(NVARM_ALARM_KIN_WORKMODE, &WorkMode, 1) == I2C_SUCCESS) {
		*mode = WorkMode;
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int set_work_mode(u8 mode)
{
    if(eeprom_write(NVARM_ALARM_KIN_WORKMODE, &mode, 1) == I2C_SUCCESS) 
		return CONF_ERR_NONE;
	else 
		return CONF_ERR_I2C;
}

int get_server_ip(u32 *ip_info)
{
	if(eeprom_read(NVARM_ALARM_KIN_IP, (u8 *)ip_info, 4) != I2C_SUCCESS)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int set_server_ip(u32 ip_info)
{
	if(eeprom_write(NVARM_ALARM_KIN_IP, (u8 *)&ip_info, 4) != I2C_SUCCESS)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int get_server_port(u16 *port)
{
	if(eeprom_read(NVARM_ALARM_KIN_PORT, (u8 *)port, 2) != I2C_SUCCESS)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int set_server_port(u16 port)
{
	if(eeprom_write(NVARM_ALARM_KIN_PORT, (u8 *)&port, 2) != I2C_SUCCESS)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int get_chan_num(u8 *num)
{
	if(eeprom_read(NVARM_ALARM_KIN_CHAN_NUM, (u8 *)num, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int set_chan_num(u8 num)
{
	if(eeprom_write(NVARM_ALARM_KIN_CHAN_NUM, (u8 *)&num, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int get_chan_en(u8 chanid, u8 *en)
{
    u8 tmp;
    
	if(eeprom_read(NVARM_ALARM_KIN_CHAN_NUM, (u8 *)&tmp, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
    *en = tmp >> 0x07;
	
	return CONF_ERR_NONE;
}

int set_chan_en(u8 chanid, u8 en)
{
    u8 tmp;
    
    if(eeprom_read(NVARM_ALARM_KIN_CHAN_PARAM(chanid), (u8 *)&tmp, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
    
    if((en & 0x01) == ENABLE)
        tmp = tmp | 0x80;
    else if((en & 0x01) == DISABLE)
        tmp = tmp & 0x7f;
    if(eeprom_write(NVARM_ALARM_KIN_CHAN_PARAM(chanid), (u8 *)&tmp, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int get_chan_type(u8 chanid, u8 *type)
{
    u8 tmp;
    
	if(eeprom_read(NVARM_ALARM_KIN_CHAN_PARAM(chanid), (u8 *)&tmp, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
    *type = tmp & 0x03;
	
	return CONF_ERR_NONE;
}

int set_chan_type(u8 chanid, u8 type)
{
    u8 tmp;

    if(eeprom_read(NVARM_ALARM_KIN_CHAN_PARAM(chanid), (u8 *)&tmp, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
    
    tmp = (tmp & 0xfc) | (type & 0x03);
	if(eeprom_write(NVARM_ALARM_KIN_CHAN_PARAM(chanid), (u8 *)&tmp, 1) != I2C_SUCCESS)
		return CONF_ERR_I2C;
	
	return CONF_ERR_NONE;
}

int signal_cfg_fetch(tKinAlarmConfig *KinAlarmCfg)
{
    if(eeprom_read(NVRAM_ALARM_KIN_BASE, (u8 *)KinAlarmCfg, sizeof(tKinAlarmConfig)) != I2C_SUCCESS)
        return CONF_ERR_I2C;
    
    return CONF_ERR_NONE;
}