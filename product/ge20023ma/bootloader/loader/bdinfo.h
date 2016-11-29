
#ifndef __BDINFO_H
#define __BDINFO_H

#define BDINFO_SUCCESS				(0)
#define BDINFO_ERR_I2C_RW			(-1)	
#define BDINFO_ERR_INVALID_DATA		(-2)

#define EEPROM_ADDR_MANUFACTORY_INFO	0x100	
#define EEPROM_ADDR_MAC					0x160
#define MANUFACTORY_INFO_SIZE			128		/* 128 bytes */

#define MANU_STR_TYPE_MIN	0x00
#define MANU_STR_TYPE_MAX	0x05

typedef enum {
	MODULE_NAME		= 0x00,
	MODULE_REV		= 0x01,
	SERIAL_NUMBER	= 0x02,
	FACTORY_DATE	= 0x03,
	OTHER1			= 0x04,
	OTHER2			= 0x05
} ManuStrType_e;

#define MANU_STR_FIELD_NUM		6
#define MANU_STRING_LEN			13
#define MANU_MAC_LEN			6

typedef struct _ManuStrField {
	unsigned char Type;
	unsigned char Length;
	char String[MANU_STRING_LEN+1];
} ManuStrField_t;

typedef struct _ManufactoryInfo {
	ManuStrField_t	StrField[MANU_STR_FIELD_NUM];
	unsigned char	MacAddress[MANU_MAC_LEN];
	unsigned char	Reserved[42];
} ManuInfo_t; 


#ifdef __cplusplus
 extern "C" {
#endif

int ManuInfo_Read (ManuInfo_t * pManuInfo);
int ManuInfo_Write(ManuInfo_t * pManuInfo);
int ManuInfo_Print(ManuInfo_t * pManuInfo);

int ManuInfo_Read_SerialNumber(char *SerialNumber);

#ifdef __cplusplus
}
#endif

#endif

