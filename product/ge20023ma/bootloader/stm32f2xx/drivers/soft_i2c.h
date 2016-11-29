

#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"

#define EEPROM_SIZE						0x2000

#define I2C_SUCCESS						1
#define I2C_FAILURE						0

#define EEPROM_SLAVE_ADDR				0xA0

/* */
#define EPROM_ADDR_UPGRADE_FLAG			0x40
#define EPROM_ADDR_BOOT_DELAY			0x41
#define EPROM_ADDR_CONSOLE_ENABLE		0x42
#define EPROM_ADDR_LOADER_VERSION		0x48
#define EPROM_ADDR_FIRMWARE_SIZE		0x50
#define EPROM_ADDR_FIRMWARE_CRC32		0x54	
#define MAX_LOADER_VERSION_SIZE			8
/* Mac address */
#define EPROM_ADDR_MAC					0x0160

/* Device property */
#define EPROM_ADDR_DEVICE_NAME			0x0200		
#define EPROM_ADDR_SYS_VERSION			0x0230
#define EPROM_ADDR_HARD_VERSION			0x0250
#define EPROM_ADDR_SOFT_VERSION			0x0270
#define EPROM_ADDR_IP					0x02C0
#define EPROM_ADDR_NETMASK				0x02C4
#define EPROM_ADDR_GATEWAY				0x02C8

#define EPROM_ADDR_UART_CFG_BASE		0x0300
#define EPROM_ADDR_UART_CFG(port)		(EPROM_ADDR_UART_CFG_BASE+0x80*port)

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






