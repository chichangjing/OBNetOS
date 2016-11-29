
/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stm32f2xx.h>
#include <soft_i2c.h>
#include <bdinfo.h>
#include <cli.h>

#include "cmd_bdinfo.h"

static cli_cmd_t cmd_macset	= {
	0,
	"macset",
	"Set Mac address",
	"<mac-address>",
	CmdMacSet,0,0,0
};

static cli_cmd_t cmd_bdinfoset	= {
	0,
	"bdinfoset",
	"Set factory infomation",
	"<field-index> <type> <string>",
	CmdBdinfoSet,0,0,0
};

static cli_cmd_t cmd_bdinfoget	= {
	0,
	"bdinfoget",
	"Get factory infomation",
	"<cr>",
	CmdBdinfoGet,0,0,0
};

static cli_cmd_t cmd_bdinfoclear	= {
	0,
	"bdinfoclear",
	"Clear factory infomation item <0-5>",
	"<field-index>",
	CmdBdinfoClear,0,0,0
};

static void ExtractMacAddr (unsigned char *macAddrP, char *macStr)
{
	char *s = macStr;   
	char *e;
	int i;
	
	for (i=0; i<6; ++i) {
		strtoul(s, &e, 16);
		if((unsigned int)e - (unsigned int)s > 2) {
			char tmp[3];
			memcpy(tmp, s, 2);
			tmp[2] =0;
			macAddrP[i] = s ? strtoul(tmp, NULL, 16) : 0;
			s += 2;
		}
		else {
			macAddrP[i] = s ? strtoul(s, &e, 16) : 0;
			if (s) s = (*e) ? e+1 : e;
		}
	}
}


void CmdMacSet(int argc, char **argv)
{
	u8 macAddr[MANU_MAC_LEN];
	
	if (argc != 2) {
		printf ("Usage  : macset <mac-address>\r\n");
		printf ("Example: macset 11:33:1133-22-11\r\n");
		return;
	} else {
		ExtractMacAddr(macAddr, argv[1]);
		if(I2C_Write(macAddr, MANU_MAC_LEN, EEPROM_ADDR_MAC, EEPROM_SLAVE_ADDR) == 0) {
			printf("I2C write error\r\n");
		} 
	}
}

void CmdBdinfoSet(int argc, char **argv)
{
	u8 index, type;
	u16 address;
	int strLength;
	ManuStrField_t	StrModule;
		
	if (argc != 4) {
		printf ("Usage: bdinfoset <index> <type> <string>\r\n");
		printf ("       <index> = 0,1,2,3,4,5\r\n");
		printf ("       <type>  = 0x01 : ModuleName\r\n");
		printf ("               = 0x02 : ModuleRev\r\n");
		printf ("               = 0x03 : SerialNumber\r\n");
		printf ("               = 0x04 : FactoryDate\r\n");
		printf ("               = 0x05 : Other1\r\n");
		printf ("               = 0x06 : Other2\r\n");
		printf ("Example: bdinfoset 3 4 2014-06-06\r\n");
		return;
	} else {
		index = (unsigned char)strtoul(argv[1], NULL, 16);
		type = (unsigned char)strtoul(argv[2], NULL, 16);
		printf("Write address: 0x%04x, type: 0x%02x\r\n", EEPROM_ADDR_MANUFACTORY_INFO+index*16, type);
		if(index<MANU_STR_FIELD_NUM) {
			if((type>0x00) && (type<0xFF)) {
				memset(&StrModule, 0xFF, sizeof(ManuStrField_t));
				StrModule.Type = type;
				if((strLength = (unsigned char)strlen(argv[3])) > MANU_STRING_LEN) {
					StrModule.Length = MANU_STRING_LEN;
					strncpy(StrModule.String, argv[3], MANU_STRING_LEN);
					StrModule.String[MANU_STRING_LEN] = '\0';
				} else {
					StrModule.Length = (unsigned char)strLength;
					strcpy(StrModule.String, argv[3]);
				}
				address = EEPROM_ADDR_MANUFACTORY_INFO+index*16;
				if(I2C_Write((u8 *)&StrModule, sizeof(ManuStrField_t), address, EEPROM_SLAVE_ADDR) == 0) {
					printf("Error: I2C write faild\r\n");
				}
			} else {
				printf("Invalid Type\r\n");
			}
		}
		else {
			printf("Invalid Index\r\n");
		}
	}
}

void CmdBdinfoGet(int argc, char **argv)
{
	int i, numItem=0;
	u16 address;
	ManuStrField_t	StrModule;
	u8 mac[MANU_MAC_LEN];

	if(I2C_Read(mac, MANU_MAC_LEN, EEPROM_ADDR_MAC, EEPROM_SLAVE_ADDR) == 0) {
		printf("Error: I2C read mac faild\r\n");
		return;
	} 
	printf("## Mac address : %02x-%02x-%02x-%02x-%02x-%02x\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	printf("## Manufactory infomation list ......\r\n");
	for(i=0; i<MANU_STR_FIELD_NUM; i++) { 
		address = EEPROM_ADDR_MANUFACTORY_INFO+i*16;
		if(I2C_Read((u8 *)&StrModule, sizeof(ManuStrField_t), address, EEPROM_SLAVE_ADDR) == 0) {
			printf("Error: I2C read faild at 0x%04x\r\n", address);
			return;
		} else {
			if(StrModule.Type != 0xFF) {
				numItem++;
				switch(StrModule.Type) {
					case 0x01:
					printf("   ModuleName   = %s\r\n", StrModule.String);
					break;
					case 0x02:
					printf("   ModuleRev    = %s\r\n", StrModule.String);
					break;
					case 0x03:
					printf("   SerialNumber = %s\r\n", StrModule.String);
					break;
					case 0x04:
					printf("   FactoryDate  = %s\r\n", StrModule.String);	
					break;
					case 0x05:
					printf("   Other1       = %s\r\n", StrModule.String);	
					break;
					case 0x06:
					printf("   Other2       = %s\r\n", StrModule.String);	
					break;										
					default:
					printf("   Unkown(0x%02x) = %s\r\n", StrModule.Type,StrModule.String);							
					break;
				}
			}
		} 
	}
	if(numItem == 0)
		printf(" * No information\r\n");
	
}

void CmdBdinfoClear(int argc, char **argv)
{
	u8 index;
	u16 address;
	ManuStrField_t	StrModule;
	
	if((argc < 2) || (argc > 2)) {
		printf ("Usage: bdinfoclear <index>\r\n");
		printf ("       <index> = 0,1,2,3,4,5\r\n");
		return;
	}
	
	index = (unsigned char)strtoul(argv[1], NULL, 16);
	if(index<MANU_STR_FIELD_NUM) {
		address = EEPROM_ADDR_MANUFACTORY_INFO+index*16;
		memset(&StrModule, 0xFF, sizeof(ManuStrField_t));
		if(I2C_Write((u8 *)&StrModule, sizeof(ManuStrField_t), address, EEPROM_SLAVE_ADDR) == 0) {
			printf("Error: I2C write faild\r\n");
		}
	}
}

void RegisterFactoryCommand(void)
{
	cli_register_command(&cmd_macset);
	cli_register_command(&cmd_bdinfoset);
	cli_register_command(&cmd_bdinfoget);
	cli_register_command(&cmd_bdinfoclear);
}

