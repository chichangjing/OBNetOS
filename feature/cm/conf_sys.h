
#ifndef _CONF_SYS_H
#define _CONF_SYS_H


#define MAX_DEVICE_NAME_LEN		47
#define MAX_SERIAL_NUMBER_LEN	15

int conf_set_upgrade_flag(void);
int conf_clear_upgrade_flag(void);
int conf_cli_login_disable(void);
int conf_cli_login_enable(void);
int conf_get_cli_login_switch(u8 *disable);
int conf_get_bootdelay(u8 *seconds);
int conf_set_bootdelay(u8 *seconds);
int conf_get_mac_address(u8 *mac);
int conf_set_mac_address(u8 *mac);
int conf_set_device_name(u8 *dev_name);
int conf_set_serial_number(u8 *serial_number);
int conf_set_name_id(u8 *dev_name, u8 *serial_number);
int conf_get_name_id(u8 *dev_name, u8 *serial_number);
int conf_set_version(u8 *version);
int conf_get_version(u8 *version);
int conf_set_ip_info(u8 *ip_info);
int conf_get_ip_info(u8 *ip_info);

#endif

