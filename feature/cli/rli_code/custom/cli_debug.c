
/*************************************************************
 * Filename     : cli_debug.c
 * Description  : API for CLI
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

#include "mconfig.h"

/* Standard includes */
#include <string.h>
#include <stdlib.h>

/* Kernel includes. */

/* BSP includes */
#if SWITCH_CHIP_BCM53101
#include "robo_drv.h"
#endif
#include "flash_if.h"
#include "soft_i2c.h"

/* Other includes */
#include "conf_sys.h"
#include "cli_sys.h"
#include "cli_util.h"

#define MAX_REGADDR_LEN	8


static int cli_regaddr_range_parse(sbyte *str_range, ubyte *addr_start, ubyte *addr_end, ubyte max_addr, ubyte max_addr_len)
{
	ubyte4 i, error_code;
	ubyte4 addr_range[2], addr_top;
	ubyte4 strLen;
	
	if(str_range == NULL)
		return -1;
	if((addr_start == NULL) || (addr_end == NULL))
		return -1;

	strLen = strlen(str_range);
	
    error_code = 0;
    i = 0;
    addr_top = 0;
    addr_range[0] = 0;
    addr_range[1] = 0;
    while ((i < strLen) && (error_code == 0 )) {
		switch(str_range[i]) {
			case '\'':
			case '\"':
			case 'x':
			case 'X':
				break;
	        case '0':
	        case '1':
	        case '2':
	        case '3':
	        case '4':
	        case '5':
	        case '6':
	        case '7':
	        case '8':
	        case '9':
				addr_range[addr_top] = addr_range[addr_top] * 16 + str_range[i] - '0';
            	break;
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				addr_range[addr_top] = addr_range[addr_top] * 16 + str_range[i] - 'a' + 10;
            	break;	
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				addr_range[addr_top] = addr_range[addr_top] * 16 + str_range[i] - 'A' + 10;
            	break;					
	        case '-':
	            if (addr_top == 0)
					addr_top = 1;
	            else 
					error_code = 1;
	            break;
	        default:
	            error_code = 1;
	            break;	
		}
		i++;
	}

	if(error_code == 0) {
        if(addr_top == 0) {
            if(addr_range[0] <= max_addr) {
				*addr_start = (ubyte)addr_range[0];
				*addr_end = (ubyte)addr_range[0];
            } else 
				error_code = 1;
        } else {
            if(addr_range[1] < addr_range[0] || addr_range[0] > max_addr || addr_range[1] > max_addr) 
				error_code = 1;
			else {
				*addr_start = (ubyte)addr_range[0];
				*addr_end = (ubyte)addr_range[1];
			}
		}
    }

    if (error_code) {
        return -1;
    }	

	return 0;
}


static int cli_regval_string_parse(sbyte *str_val, ubyte *buffer, ubyte *buflen, ubyte max_buflen)
{
	ubyte4 i, error_code, data_input_flag;

	ubyte4 strLen, datatmp;
	ubyte data_index;
	
	if(str_val == NULL)
		return -1;
	if((buffer == NULL) || (buflen == NULL))
		return -1;

	strLen = strlen(str_val);

    error_code = 0;
    i = 0;
    data_index = 0;
    datatmp = 0;
    *buflen = 0;
	data_input_flag = 0;
    while ((i < strLen) && (error_code == 0 )) {
		switch(str_val[i]) {
			case '\'':
			case '\"':
			case 'x':
			case 'X':
				break;
	        case '0':
	        case '1':
	        case '2':
	        case '3':
	        case '4':
	        case '5':
	        case '6':
	        case '7':
	        case '8':
	        case '9':
				data_input_flag = 1;
				if(data_index + 1 > max_buflen) {
					error_code = 1;
					break;
				}
				datatmp = datatmp * 16 + str_val[i] - '0';
				if(datatmp > 0xFF)
					error_code = 1;				
            	break;				
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				data_input_flag = 1;
				if(data_index + 1 > max_buflen) {
					error_code = 1;
					break;
				}
				datatmp = datatmp * 16 + str_val[i] - 'a' + 10;
				if(datatmp > 0xFF)
					error_code = 1;				
            	break;				
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				data_input_flag = 1;
				if(data_index + 1 > max_buflen) {
					error_code = 1;
					break;
				}
				datatmp = datatmp * 16 + str_val[i] - 'A' + 10;
				if(datatmp > 0xFF)
					error_code = 1;
            	break;
	        case '-':
	            if(data_input_flag == 0) {
					error_code = 1;
	            } else {
					if(data_index + 1 > max_buflen) {
						error_code = 1;
						break;
					}
					buffer[data_index] = (ubyte)datatmp;
					data_index++;
					data_input_flag = 0;
					datatmp = 0;
				}
	            break;
	        default:
	            error_code = 1;
	            break;	
		}
		i++;
	}

	if(error_code == 0) {
		if((data_index == 0) && (data_input_flag == 1)) {
			buffer[0] = (ubyte)datatmp;
			*buflen = 1;
		} else if((*buflen == 0) && (data_input_flag == 0)) {
			error_code = 1;
		} else if((*buflen > 0) && (data_input_flag == 0)) {
			error_code = 1;
		} else {
			buffer[data_index] = (ubyte)datatmp;
			*buflen = data_index + 1;
		}
	}
	
    if (error_code) {
        return -1;
    }	

	return 0;	
}


RLSTATUS cli_debug_switch_getreg_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "page", mDebugSwitchGetreg_Page, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "address", mDebugSwitchGetreg_Address, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* TO DO: Add your handler code here */

    {
#if SWITCH_CHIP_BCM53101      
    	ubyte pageVal;
		ubyte addr_start, addr_end;
		ubyte value[MAX_REGADDR_LEN];
		int i;
		
		pageVal = (ubyte)strtoul(pVal1, 0, 16);

		/* Parse the address string, and max address range is 8 */
		if(cli_regaddr_range_parse(pVal2, &addr_start, &addr_end, 0xFF, MAX_REGADDR_LEN) < 0) {
			cli_printf(pCliEnv, "Error: address string parse failed!\r\n");
			return FALSE;
		}

		if((addr_end - addr_start + 1) > MAX_REGADDR_LEN) {
			cli_printf(pCliEnv, "Error: address length must be less than or equal to %d!\r\n", MAX_REGADDR_LEN);
			return FALSE;
		}
	
		if(robo_read(pageVal, addr_start, value, addr_end-addr_start + 1) == 0) {
			cli_printf(pCliEnv, "R-(page%02xh: address %02xh-%02xh) = %d'", pageVal, addr_start, addr_end, (addr_end-addr_start + 1)*8);
			for(i=addr_end-addr_start; i>=0; i--)
				cli_printf(pCliEnv, "%02x", value[i]);
			cli_printf(pCliEnv, "\r\n");
		} else {
			cli_printf(pCliEnv, "Error: setreg failed\r\n");
			return FALSE;
		}
#endif
		
	}
	
    return status;
}

RLSTATUS cli_debug_switch_setreg_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;
    sbyte       *pVal3 = NULL;
    paramDescr  *pParamDescr3;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "page", mDebugSwitchSetreg_Page, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "address", mDebugSwitchSetreg_Address, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "value", mDebugSwitchSetreg_Value, &pParamDescr3 );
    if ( OK != status )
    {
		return(status);
    } else pVal3 = (sbyte*)(pParamDescr3->pValue);

    /* TO DO: Add your handler code here */
    {
#if SWITCH_CHIP_BCM53101      
    	ubyte pageVal;
		ubyte addr_start, addr_end;
		ubyte value[MAX_REGADDR_LEN];
		ubyte value_wr[MAX_REGADDR_LEN];
		ubyte dataLen;
		int i;
		
		//CONVERT_StrTo(pVal1, &pageVal, kDTuchar);
		pageVal = (ubyte)strtoul(pVal1, 0, 16);
		
		/* Parse the address string, and max address range is 8 */
		if(cli_regaddr_range_parse(pVal2, &addr_start, &addr_end, 0xFF, MAX_REGADDR_LEN) < 0) {
			cli_printf(pCliEnv, "Error: address string parse failed!\r\n");
			return FALSE;
		}

		if((addr_end - addr_start + 1) > MAX_REGADDR_LEN) {
			cli_printf(pCliEnv, "Error: address length must be less than or equal to %d!\r\n", MAX_REGADDR_LEN);
			return FALSE;
		}

		
		if(cli_regval_string_parse(pVal3, value, &dataLen, MAX_REGADDR_LEN) < 0) {
			cli_printf(pCliEnv, "Error: reg value string parse failed!\r\n");
			return FALSE;
		}

		if(dataLen != addr_end-addr_start + 1) {
			cli_printf(pCliEnv, "Error: address length is not equal to data length!\r\n");
			return FALSE;
		}

		for(i=0; i<dataLen; i++) {
			value_wr[i] = value[dataLen-1-i];
		}
				
		if(robo_write(pageVal, addr_start, value_wr, addr_end-addr_start + 1) == 0) {
			cli_printf(pCliEnv, "W-(page%02xh: address %02xh-%02xh) = %d'", pageVal, addr_start, addr_end, dataLen*8);
			for(i=0; i<dataLen; i++)
				cli_printf(pCliEnv, "%02x", value[i]);			
			cli_printf(pCliEnv, "\r\n", dataLen);
		} else {
			cli_printf(pCliEnv, "Error: setreg failed\r\n", dataLen);
			return FALSE;
		}

		if(robo_read(pageVal, addr_start, value, addr_end-addr_start + 1) == 0) {
			cli_printf(pCliEnv, "R-(page%02xh: address %02xh-%02xh) = %d'", pageVal, addr_start, addr_end, (addr_end-addr_start + 1)*8);
			for(i=addr_end-addr_start; i>=0; i--)
				cli_printf(pCliEnv, "%02x", value[i]);
			cli_printf(pCliEnv, "\r\n");
		} else {
			cli_printf(pCliEnv, "Error: getreg failed\r\n");
			return FALSE;
		}	
#endif		
	}

	
    return status;
}

RLSTATUS cli_debug_eeprom_dump_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "offset", mDebugEepromDump_Offset, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get optional parameter */
    if (OK == RCC_DB_RetrieveParam(pParams, "nbytes", mDebugEepromDump_Nbytes, &pParamDescr2 ))
    {
        pVal2 = (sbyte*)(pParamDescr2->pValue);
    }

    /* TO DO: Add your handler code here */
	{
    	ubyte2 eeprom_address, nbytes;
		ubyte4 i, linebytes;
		
		eeprom_address = (ubyte2)strtoul(pVal1, 0, 16);
		if(eeprom_address > EEPROM_END_ADDRESS) {
			cli_printf(pCliEnv, "Error: Invalid address 0x%08x\r\n",eeprom_address);
			return STATUS_RCC_NO_ERROR;
		}

		if(pVal2 == NULL)
			nbytes = 64;
		else 
			nbytes = (ubyte2)strtoul(pVal2, 0, 10);
		
		if((nbytes > EEPROM_SIZE) || (eeprom_address+nbytes > EEPROM_END_ADDRESS+1)) {
			cli_printf(pCliEnv, "Error: Invalid dump-size %d\r\n", nbytes);
			return STATUS_RCC_NO_ERROR;					
		}

		do {
			ubyte	linebuf[16];
			ubyte	*ucp = linebuf;

			cli_printf(pCliEnv, "0x%08x:  ", eeprom_address);
			linebytes = (nbytes > 16)?16:nbytes;
				
			if(eeprom_read(eeprom_address, linebuf, (u8)linebytes) != I2C_SUCCESS) {
				cli_printf(pCliEnv, "Error: eeprom read failed!\r\n");
				return STATUS_RCC_NO_ERROR;	
			}
			
			for (i=0; i<linebytes; i+= 1) {
				cli_printf(pCliEnv, "%02x ", *ucp++);
				eeprom_address += 1;
			}
			cli_printf(pCliEnv, "\r\n");
			nbytes -= linebytes;

		} while (nbytes > 0);
		cli_printf(pCliEnv, "\r\n");	
	}

	return status;	
}

RLSTATUS cli_debug_eeprom_clear_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "offset", mDebugEepromClear_Offset, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "nbytes", mDebugEepromClear_Nbytes, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* TO DO: Add your handler code here */
    {
    	ubyte2 eeprom_address, nbytes;
		ubyte2 i,loop,left_size;
		ubyte tempdata[0x80];
		
		eeprom_address = (ubyte2)strtoul(pVal1, 0, 16);
		if(eeprom_address > EEPROM_END_ADDRESS) {
			cli_printf(pCliEnv, "Error: Invalid address 0x%08x (max=0x%08x)\r\n",eeprom_address,EEPROM_END_ADDRESS);
			return STATUS_RCC_NO_ERROR;
		}
		nbytes = (ubyte2)strtoul(pVal2, 0, 10);
		if((nbytes > EEPROM_SIZE) || (eeprom_address+nbytes > EEPROM_END_ADDRESS+1)) {
			cli_printf(pCliEnv, "Error: Invalid dump-size %d (max=%d)\r\n", nbytes,EEPROM_SIZE-eeprom_address);
			return STATUS_RCC_NO_ERROR;					
		}


		for(i=0; i<0x80; i++)
			tempdata[i] = 0xff;

		loop = nbytes/0x80;
		left_size = nbytes%0x80;

		for(i=0; i<loop; i++) {
			//cli_printf(pCliEnv, "Writing address 0x%04x ...\r\n", address + i * 0x80);
			if(eeprom_page_write((u16)(eeprom_address + i*0x80), tempdata, 0x80) != I2C_SUCCESS) {
				cli_printf(pCliEnv, "Error: eeprom_page_write, address 0x%04x\r\n", eeprom_address + i * 0x80);
				return STATUS_RCC_NO_ERROR;;
			}
		}

		if(left_size > 0) {
			//cli_printf(pCliEnv, "Writing address 0x%04x ...\r\n", address + loop * 0x80);
			if(eeprom_page_write((u16)(eeprom_address + loop*0x80), tempdata, left_size) != I2C_SUCCESS) {
				cli_printf(pCliEnv, "Error: eeprom_page_write, address 0x%04x\r\n", eeprom_address + loop * 0x80);
				return STATUS_RCC_NO_ERROR;;
			}
		}
	
	}
	
    return status;
}

extern RLSTATUS cli_debug_eeprom_test_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;
    sbyte       *pVal3 = NULL;
    paramDescr  *pParamDescr3;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "offset", mDebugEepromTest_Offset, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "nbytes", mDebugEepromTest_Nbytes, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "value", mDebugEepromTest_Value, &pParamDescr3 );
    if ( OK != status )
    {
		return(status);
    } else pVal3 = (sbyte*)(pParamDescr3->pValue);

    /* TO DO: Add your handler code here */
	{
    	ubyte2 eeprom_address, nbytes;
		ubyte value;
		ubyte2 i,loop,left_size;
		ubyte tempdata[0x80];
		
		eeprom_address = (ubyte2)strtoul(pVal1, 0, 16);
		if(eeprom_address > EEPROM_END_ADDRESS) {
			cli_printf(pCliEnv, "Error: Invalid address 0x%08x\r\n",eeprom_address);
			return STATUS_RCC_NO_ERROR;
		}
		nbytes = (ubyte2)strtoul(pVal2, 0, 10);
		if((nbytes > EEPROM_SIZE) || (eeprom_address+nbytes > EEPROM_END_ADDRESS+1)) {
			cli_printf(pCliEnv, "Error: Invalid dump-size %d\r\n", nbytes);
			return STATUS_RCC_NO_ERROR;					
		}

		value = (ubyte)strtoul(pVal3, 0, 16);
		
		
		for(i=0; i<0x80; i++)
			tempdata[i] = value;

		loop = nbytes/0x80;
		left_size = nbytes%0x80;

		for(i=0; i<loop; i++) {
			//cli_printf(pCliEnv, "Writing address 0x%04x ...\r\n", address + i * 0x80);
			if(eeprom_page_write((u16)(eeprom_address + i*0x80), tempdata, 0x80) != I2C_SUCCESS) {
				cli_printf(pCliEnv, "Error: eeprom_page_write, address 0x%04x\r\n", eeprom_address + i * 0x80);
				return STATUS_RCC_NO_ERROR;;
			}
		}

		if(left_size > 0) {
			//cli_printf(pCliEnv, "Writing address 0x%04x ...\r\n", address + loop * 0x80);
			if(eeprom_page_write((u16)(eeprom_address + loop*0x80), tempdata, left_size) != I2C_SUCCESS) {
				cli_printf(pCliEnv, "Error: eeprom_page_write, address 0x%04x\r\n", eeprom_address + loop * 0x80);
				return STATUS_RCC_NO_ERROR;;
			}
		}
	
	}
    return status;
}


extern RLSTATUS cli_debug_eeprom_format_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;

    /* TO DO: Add your handler code here */
	{
    	ubyte2 eeprom_address;
		ubyte2 i,loop,left_size;
		ubyte tempdata[0x80];
		
		eeprom_address = 0x300;
		for(i=0; i<0x80; i++)
			tempdata[i] = 0xFF;

		loop = (EEPROM_SIZE - eeprom_address) / 0x80;
		left_size = (EEPROM_SIZE - eeprom_address) % 0x80;

		cli_printf(pCliEnv, "Formating eeprom ... ");
		
		for(i=0; i<loop; i++) {
			//cli_printf(pCliEnv, "Writing address 0x%04x ...\r\n", eeprom_address + i * 0x80);
			if(eeprom_page_write((u16)(eeprom_address + i*0x80), tempdata, 0x80) != I2C_SUCCESS) {
				cli_printf(pCliEnv, "\r\nError: eeprom_page_write, address 0x%04x\r\n", eeprom_address + i * 0x80);
				return STATUS_RCC_NO_ERROR;;
			}
		}
		
		if(left_size > 0) {
			//cli_printf(pCliEnv, "Writing address 0x%04x ...\r\n", address + loop * 0x80);
			if(eeprom_page_write((u16)(eeprom_address + loop*0x80), tempdata, left_size) != I2C_SUCCESS) {
				cli_printf(pCliEnv, "Error: eeprom_page_write, address 0x%04x\r\n", eeprom_address + loop * 0x80);
				return STATUS_RCC_NO_ERROR;;
			}
		}

		cli_printf(pCliEnv, "done\r\n");
	}
	
    return status;
}

#if 0

RLSTATUS cli_debug_dump_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;
    sbyte       *pVal3 = NULL;
    paramDescr  *pParamDescr3;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "flash|eeprom", mDebugDump_FlashQppeeprom, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "offset", mDebugDump_Offset, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* get optional parameter */
    if (OK == RCC_DB_RetrieveParam(pParams, "nbytes", mDebugDump_Nbytes, &pParamDescr3 ))
    {
        pVal3 = (sbyte*)(pParamDescr3->pValue);
    }


    /* TO DO: Add your handler code here */

    {	
		if(COMPARE_Strings(pVal1, "flash")) {
	    	ubyte4 flash_offset, nbytes;
			ubyte4 i, linebytes;
			ubyte *cp;
		
			flash_offset = (ubyte4)strtoul(pVal2, 0, 16);
			if(flash_offset > FLASH_SIZE) {
				cli_printf(pCliEnv, "Error: Invalid offset address 0x%08x\r\n",flash_offset);
				return STATUS_RCC_NO_ERROR;
			}

			if(pVal3 == NULL)
				nbytes = 64;
			else 
				nbytes = (ubyte4)strtoul(pVal3, 0, 10);
			
			if((nbytes > FLASH_SIZE) || (flash_offset+nbytes > FLASH_SIZE)) {
				cli_printf(pCliEnv, "Error: Invalid dump-size %d\r\n", nbytes);
				return STATUS_RCC_NO_ERROR;					
			}		

			do {
				ubyte	linebuf[16];
				ubyte	*ucp = linebuf;

				cli_printf(pCliEnv, "0x%08x:  ", FLASH_START_ADDRESS + flash_offset);
				linebytes = (nbytes > 16)?16:nbytes;
				for (i=0; i<linebytes; i+= 1) {
					cli_printf(pCliEnv, "%02x ", (*ucp++ = *(ubyte *)(FLASH_START_ADDRESS + flash_offset)));
					flash_offset += 1;
				}
/*
				cli_printf(pCliEnv, "    ");
				cp = linebuf;
				for (i=0; i<linebytes; i++) {
					if ((*cp < 0x20) || (*cp > 0x7e))
						cli_printf(pCliEnv, ".");
					else
						cli_printf(pCliEnv, "%c", *cp);
					cp++;
				}
*/
				cli_printf(pCliEnv, "\r\n");
				nbytes -= linebytes;

			} while (nbytes > 0);
			cli_printf(pCliEnv, "\r\n");	
		}



		if(COMPARE_Strings(pVal1, "eeprom")) {
	    	ubyte2 eeprom_address, nbytes;
			ubyte4 i, linebytes;
			ubyte *cp, *buf;
			
			eeprom_address = (ubyte2)strtoul(pVal2, 0, 16);
			if(eeprom_address > EEPROM_END_ADDRESS) {
				cli_printf(pCliEnv, "Error: Invalid address 0x%08x\r\n",eeprom_address);
				return STATUS_RCC_NO_ERROR;
			}

			if(pVal3 == NULL)
				nbytes = 64;
			else 
				nbytes = (ubyte4)strtoul(pVal3, 0, 10);
			
			if((nbytes > EEPROM_SIZE) || (eeprom_address+nbytes > EEPROM_END_ADDRESS+1)) {
				cli_printf(pCliEnv, "Error: Invalid dump-size %d\r\n", nbytes);
				return STATUS_RCC_NO_ERROR;					
			}

			do {
				ubyte	linebuf[16];
				ubyte	*ucp = linebuf;

				cli_printf(pCliEnv, "0x%08x:  ", eeprom_address);
				linebytes = (nbytes > 16)?16:nbytes;
					
				if(eeprom_read(eeprom_address, linebuf, (u8)linebytes) != I2C_SUCCESS) {
					cli_printf(pCliEnv, "Error: eeprom read failed!\r\n");
					return STATUS_RCC_NO_ERROR;	
				}
				
				for (i=0; i<linebytes; i+= 1) {
					cli_printf(pCliEnv, "%02x ", *ucp++);
					eeprom_address += 1;
				}
				cli_printf(pCliEnv, "\r\n");
				nbytes -= linebytes;

			} while (nbytes > 0);
			cli_printf(pCliEnv, "\r\n");	
		}		
	}
	
    return status;
}
#endif

RLSTATUS cli_debug_module_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "add|del|show", mDebugModule_AddQppdelQppshow, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get optional parameter */
    if (OK == RCC_DB_RetrieveParam(pParams, "module", mDebugModule_Module, &pParamDescr2 ))
    {
        pVal2 = (sbyte*)(pParamDescr2->pValue);
    }


    /* TO DO: Add your handler code here */
    {
		ubyte4	DebugModules;
		ubyte4	module = 0;
		
		if(pVal2 == NULL) {
			if(COMPARE_Strings(pVal1, "show")) {
				DebugModules = cli_debug_module_read(pCliEnv);
				cli_printf(pCliEnv, "Following modules is debug-enable :\r\n");
				if(DebugModules & DBG_NMS)
					cli_printf(pCliEnv, "* NMS module\r\n");
				if(DebugModules & DBG_OBRING)
					cli_printf(pCliEnv, "* OB-Ring module\r\n");
				if(DebugModules & DBG_UART)
					cli_printf(pCliEnv, "* UART module\r\n");				
				if((DebugModules & (DBG_NMS | DBG_OBRING | DBG_UART)) == 0)
					cli_printf(pCliEnv, "* no module\r\n");
			} else {		
				cli_printf(pCliEnv, "\r\nError: Missing module name\r\n\r\n");
				return STATUS_RCC_NO_ERROR;
			}
		} else {
			if(COMPARE_Strings(pVal2, "nms")) {
				module = DBG_NMS;
			} else if(COMPARE_Strings(pVal2, "obring")) {
				module = DBG_OBRING;
			} else if(COMPARE_Strings(pVal2, "uart")) {
				module = DBG_UART;					
			} else {
				cli_printf(pCliEnv, "\r\nError: unkown module name\r\n\r\n");
				return STATUS_RCC_NO_ERROR;
			}
			
			if(COMPARE_Strings(pVal1, "add")) {
				cli_debug_module_add(pCliEnv, module);
			} else if(COMPARE_Strings(pVal1, "del")) {
				cli_debug_module_del(pCliEnv, module);
			} else if(COMPARE_Strings(pVal1, "show")) {
				cli_printf(pCliEnv, "\r\nError: Too many parameters\r\n\r\n");
				return STATUS_RCC_NO_ERROR;
			}
		}	
	}
	
    return status;
}

extern RLSTATUS cli_debug_login_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "enable|disable", mDebugLogin_EnableQppdisable, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
    {
		if(COMPARE_Strings(pVal, "enable")) {
			conf_cli_login_enable();
		} else if(COMPARE_Strings(pVal, "disable")) {
			conf_cli_login_disable();
		}	
	}
    return status;
}



extern RLSTATUS cli_debug_diag_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;
#if MODULE_RS485
    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "board_name", mDebugDiag_Board_name, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
    {
		extern void UartTest(void *pCliEnv);
		
		if(COMPARE_Strings(pVal, "GE22103MA")) {
				cli_printf(pCliEnv, "\r\n");
				UartTest(pCliEnv);
				cli_printf(pCliEnv, "\r\n");
		} else if(COMPARE_Strings(pVal, "GE2C400U")) {
				cli_printf(pCliEnv, "\r\n");
				UartTest(pCliEnv);
				cli_printf(pCliEnv, "\r\n");
		} else if(COMPARE_Strings(pVal, "GE1040PU")) {
				cli_printf(pCliEnv, "\r\n");
				UartTest(pCliEnv);
				cli_printf(pCliEnv, "\r\n");				
		} else {
			cli_printf(pCliEnv, "\r\nError: Not support!\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		}	
	}
#endif /* MODULE_RS485 */
    return status;
}

