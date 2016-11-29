
/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* BSP includes */
#include <stm32f2xx.h>
#include "soft_i2c.h"

/* Other include */
#include "cli.h"
#include "cmd_misc.h"

/*************************************************************************************************************/
static cli_cmd_t cmd_reset	= {
	0,
	"reset",
	"Reset System",
	"<cr>",
	CmdReset,0,0,0
};

static cli_cmd_t cmd_dump_memory	= {
	0,
	"memory-dump",
	"Memory display",
	"<addr> [nBytes] [format]",
	CmdDumpMemory,0,0,0
};

static cli_cmd_t cmd_dump_eeprom	= {
	0,
	"eeprom-dump",
	"Eeprom memory display",
	"<addr> [nBytes] [format]",
	CmdDumpEeprom,0,0,0
};

static cli_cmd_t cmd_set_bootdelay = {
	0,
	"bootdelay",
	"Set the bootdelay value",
	"<seconds>",
	CmdSetBootDelay,0,0,0
};

static cli_cmd_t cmd_set_console_switch = {
	0,
	"console",
	"Set the console enable/disable",
	"<enable/disable>",
	CmdSetConsoleSwitch,0,0,0
};

static cli_cmd_t cmd_clear_eeprom = {
	0,
	"clear-eeprom",
	"Clear eeprom data",
	"<address> <len>",
	CmdClearEeprom,0,0,0
};

/*************************************************************************************************************/

void CmdReset(int argc, char **argv)
{
	NVIC_SystemReset();
}

void CmdDumpMemory(int argc, char **argv)
{
	unsigned long	addr=0, size, length;
	unsigned long	i, nbytes, linebytes;
	unsigned char	*cp;

	length = 0x40;
	size = 1;

	if ((argc < 2) || (argc > 4)) {
		printf("Usage: memory-dump <addr> [nBytes] [format]\r\n");
		printf("option: [format] should be 1,2,4, default is 1.\r\n");
		return;
	} else if(argc == 2) {
		addr = strtoul(argv[1], 0, 16);
	} else if(argc == 3){
		addr = strtoul(argv[1], 0, 16);
		length = strtoul(argv[2], 0, 16);
	} else if(argc == 4) {
		addr = strtoul(argv[1], 0, 16);
		length = strtoul(argv[2], 0, 16);
		size = strtoul(argv[3], 0, 16);
	}

	nbytes = length;
	do {
		unsigned char	linebuf[16];
		unsigned int	*uip = (unsigned int   *)linebuf;
		unsigned short	*usp = (unsigned short *)linebuf;
		unsigned char	*ucp = linebuf;

		printf("%08lx:", addr);
		linebytes = (nbytes>16)?16:nbytes;
		for (i=0; i<linebytes; i+= size) {
			if (size == 4) {
				printf(" %08x", (*uip++ = *((unsigned int *)addr)));
			} else if (size == 2) {
				printf(" %04x", (*usp++ = *((unsigned short *)addr)));
			} else {
				printf(" %02x", (*ucp++ = *((unsigned char *)addr)));
			}
			addr += size;
		}
		printf("    ");
		cp = linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				printf(".");
			else
				printf("%c", *cp);
			cp++;
		}
		printf("\r\n");
		nbytes -= linebytes;

	} while (nbytes > 0);

	return;
}

void CmdDumpEeprom(int argc, char **argv)
{
	unsigned long	addr=0, size, length;
	unsigned long	i, nbytes, linebytes;
	unsigned char	*cp;

	length = 0x40;
	size = 1;

	if ((argc < 2) || (argc > 4)) {
		printf("Usage: eeprom-dump <addr> [nBytes] [format]\r\n");
		printf("option: [format] should be 1,2,4, default is 1.\r\n");
		return;
	} else if(argc == 2) {
		addr = strtoul(argv[1], 0, 16);
	} else if(argc == 3){
		addr = strtoul(argv[1], 0, 16);
		length = strtoul(argv[2], 0, 16);
	} else if(argc == 4) {
		addr = strtoul(argv[1], 0, 16);
		length = strtoul(argv[2], 0, 16);
		size = strtoul(argv[3], 0, 16);
	}

	nbytes = length;
	do {
		unsigned char	linebuf[16];
		unsigned int	*uip = (unsigned int   *)linebuf;
		unsigned short	*usp = (unsigned short *)linebuf;
		unsigned char	*ucp = linebuf;

		linebytes = (nbytes>16)?16:nbytes;
		if(I2C_Read(linebuf, (u8)linebytes, (u16)addr, EEPROM_SLAVE_ADDR) != 1) {
			printf("Error: i2c read\r\n");
			return;
		}
		
		printf("%08lx:", addr);
		
		for (i=0; i<linebytes; i+= size) {
			if (size == 4) {
				printf(" %08x", *uip++);
			} else if (size == 2) {
				printf(" %04x", *usp++);
			} else {
				printf(" %02x", *ucp++);
			}
			addr += size;
		}
		printf("    ");
		cp = linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				printf(".");
			else
				printf("%c", *cp);
			cp++;
		}
		printf("\r\n");
		nbytes -= linebytes;

	} while (nbytes > 0);

	return;
}


void CmdSetBootDelay(int argc, char **argv)
{
	unsigned char bootdelay;
	
	if(argc != 2) {
		printf("Usage: bootdelay <delay-time>\r\n");
		printf("Param: <delay-time> : 0-5\r\n");
		return;
	}

	bootdelay = (unsigned char)strtoul(argv[1], 0, 10);
	if(bootdelay > 5) {
		printf("Warning: Bootdelay value is large than 5\r\n");
		return;
	}
	
	if(eeprom_write(EPROM_ADDR_BOOT_DELAY, &bootdelay, 1) == I2C_SUCCESS)
		printf("Bootdelay is setup to %d seconds\r\n", bootdelay);
	else
		printf("Error: eeprom write\r\n");
}

void CmdSetConsoleSwitch(int argc, char **argv)
{
	unsigned char swflag;
	
	if(argc != 2) {
		printf("Usage: console <enable/disable>\r\n");
		return;
	}

	if(strcmp(argv[1], "enable") == 0) {
		swflag = 0x01;
		if(eeprom_write(EPROM_ADDR_CONSOLE_ENABLE, &swflag, 1) == I2C_SUCCESS)
			printf("Console will be enable at next boot\r\n");
		else
			printf("Error: eeprom write\r\n");	
	} else if(strcmp(argv[1], "disable") == 0) {
		swflag = 0x00;
		if(eeprom_write(EPROM_ADDR_CONSOLE_ENABLE, &swflag, 1) == I2C_SUCCESS)
			printf("Console will be disable at next boot\r\n");
		else
			printf("Error: eeprom write\r\n");		
	} else {
		printf("Invalid parameter!\r\n");
	}

}

void CmdClearEeprom(int argc, char **argv)
{
	unsigned short address, len;
	unsigned short i, loop, left_size;
	unsigned char tempdata[0x80];
	
	if(argc != 3) {
		printf("Usage: clear-eeprom <address> <len>\r\n");
		return;
	}

	address = (unsigned short)strtoul(argv[1], 0, 16);
	len = (unsigned short)strtoul(argv[2], 0, 16);
	if((address + len) > EEPROM_SIZE) {
		printf("Error: (address+len) is large than eeprom size\r\n");
		return;
	}

	for(i=0; i<0x80; i++)
		tempdata[i] = 0xff;

	loop = len/0x80;
	left_size = len%0x80;

	for(i=0; i<loop; i++) {
		//printf("Writing address 0x%04x ...\r\n", address + i * 0x80);
		if(eeprom_page_write((u16)(address + i*0x80), tempdata, 0x80) != I2C_SUCCESS) {
			printf("Error: eeprom_page_write, address 0x%04x\r\n", address + i * 0x80);
			return;
		}
	}

	if(left_size > 0) {
		//printf("Writing address 0x%04x ...\r\n", address + loop * 0x80);
		if(eeprom_page_write((u16)(address + loop*0x80), tempdata, left_size) != I2C_SUCCESS) {
			printf("Error: eeprom_page_write, address 0x%04x\r\n", address + loop * 0x80);
			return;
		}
	}
	
}


void RegisterMiscCommand(void)
{
	cli_register_command(&cmd_reset);
	cli_register_command(&cmd_dump_memory);
	cli_register_command(&cmd_dump_eeprom);
	cli_register_command(&cmd_clear_eeprom);
	cli_register_command(&cmd_set_bootdelay);
	cli_register_command(&cmd_set_console_switch);
}


