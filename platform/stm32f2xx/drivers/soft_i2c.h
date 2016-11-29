

#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"

#define I2C_SUCCESS					1
#define I2C_FAILURE					0

#define EEPROM_SLAVE_ADDR			0xA0

#define EEPROM_START_ADDRESS		0x0  
#define EEPROM_END_ADDRESS			0x1FFF
#define EEPROM_SIZE					0x2000

/* */
#define EPROM_ADDR_MAC				0x0160
#define EPROM_ADDR_IP				0x02C0
#define EPROM_ADDR_NETMASK			0x02C4
#define EPROM_ADDR_GATEWAY			0x02C8


void I2C_GPIO_Config(void);
void I2C_WriteEnable(void);
void I2C_WriteDisable(void);
int I2C_WriteByte(u8 SendByte, u16 WriteAddress, u8 DeviceAddress);
int I2C_ReadByte(u8 *ReadBype, u16 ReadAddress, u8 DeviceAddress);
int I2C_Read(u8 *pBuffer, u8 length, u16 ReadAddress, u8 DeviceAddress);
int I2C_Write(u8 *pBuffer, u8 length, u16 WriteAddress, u8 DeviceAddress);

int eeprom_read(u16 address, u8 *pBuffer, u8 length);
int eeprom_write(u16 address, u8 *pBuffer, u8 length);
int eeprom_page_write(u16 address, u8 *pBuffer, u8 length);

#ifdef __cplusplus
}
#endif

#endif






