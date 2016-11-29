
/* Standard includes */
#include "stdio.h"
#include "string.h"

/* BSP includes */
#include "bdinfo.h"
#include "soft_i2c.h"

int ManuInfo_Read (ManuInfo_t * pManuInfo) 
{
	if(eeprom_read(EEPROM_ADDR_MANUFACTORY_INFO, (u8 *)pManuInfo, sizeof(ManuInfo_t)) != I2C_SUCCESS) {
		return BDINFO_ERR_I2C_RW;
	}

	return BDINFO_SUCCESS;
}

int ManuInfo_Read_SerialNumber(char *SerialNumber)
{
	int i;
	ManuInfo_t ManuInfo;
	
	if(eeprom_read(EEPROM_ADDR_MANUFACTORY_INFO, (u8 *)&ManuInfo, sizeof(ManuInfo_t)) != I2C_SUCCESS) {
		return BDINFO_ERR_I2C_RW;
	} 

	for(i=0; i<MANU_STR_FIELD_NUM; i++) { 
		if(ManuInfo.StrField[i].Type != SERIAL_NUMBER)
			strcpy(SerialNumber, ManuInfo.StrField[i].String);
	}

	return BDINFO_SUCCESS;
}


int ManuInfo_Write(ManuInfo_t * pManuInfo)
{
	return BDINFO_SUCCESS;
}

int ManuInfo_Print(ManuInfo_t * pManuInfo)
{
	int i;

	printf("Device MacAddress = %02X-%02X-%02X-%02X-%02X-%02X\r\n", 	
		pManuInfo->MacAddress[0], pManuInfo->MacAddress[1], pManuInfo->MacAddress[2],
		pManuInfo->MacAddress[3], pManuInfo->MacAddress[4], pManuInfo->MacAddress[5]);
	
	for(i=0; i<MANU_STR_FIELD_NUM; i++) { 
		if(pManuInfo->StrField[i].Type != 0xFF) {
			switch(pManuInfo->StrField[i].Type) {
				case 0x01:
				printf("  %d. ModuleName   = %s\r\n", i, pManuInfo->StrField[i].String);
				break;
				case 0x02:
				printf("  %d. ModuleRev    = %s\r\n", i, pManuInfo->StrField[i].String);
				break;
				case 0x03:
				printf("  %d. SerialNumber = %s\r\n", i, pManuInfo->StrField[i].String);
				break;
				case 0x04:
				printf("  %d. FactoryDate  = %s\r\n", i, pManuInfo->StrField[i].String);	
				break;
				case 0x05:
				printf("  %d. Other1       = %s\r\n", i, pManuInfo->StrField[i].String);	
				break;
				case 0x06:
				printf("  %d. Other2       = %s\r\n", i, pManuInfo->StrField[i].String);	
				break;										
				default:
				printf("  %d. Unkown(0x%02x) = %s\r\n", i, pManuInfo->StrField[i].Type, pManuInfo->StrField[i].String);							
				break;
			}
		}
		
	}
        
    return BDINFO_SUCCESS;
}