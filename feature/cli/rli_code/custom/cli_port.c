
/*************************************************************
 * Filename     : cli_port.c
 * Description  : API for CLI
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

#include "mconfig.h"

/* Standard includes */
#include <string.h>
#include <stdlib.h>

/* Kernel includes. */

/* LwIP includes */
#include "lwip/inet.h"

/* BSP includes */
#include "soft_i2c.h"

/* Other includes */
#include "conf_sys.h"
#include "cli_sys.h"
#include "cli_util.h"


extern ubyte4 gPortNo[kRCC_MAX_CLI_TASK+1];

int	cli_read_port(cli_env *pCliEnv, unsigned int *pPort)
{
	int   i;
	cli_env *pOther;
	ubyte4   ipAddr;
	sbyte    buffer[16];

	for(i=0; i<kRCC_MAX_CLI_TASK+1; i++) {
		if(NULL == (pOther = RCC_TELNETD_GetSession(i)))
			continue;

		if(NULL == CLIENV(pOther))
			continue;

		if(pCliEnv == pOther) {
			*pPort = gPortNo[i];
			return 0;
		}
	}

	ipAddr = OS_SPECIFIC_GET_ADDR(pCliEnv);
	CONVERT_ToStr(&ipAddr, buffer, kDTipaddress);

	cli_printf(pCliEnv, "Error: reading port number from CLI session from %s\r\n", buffer);

	return 1;
}

int cli_str_to_macaddr(ubyte *a, char *s)
{
	char *p;
	int	i;
	ubyte4 val;

	for(i=0; i<5; i++) {
		p = (char*)strchr(s, ':');
		if(p == NULL) {
			return 1;
		} else {
			*p = '\0';
			sscanf(s, "%x", &val);
			a[i] = (ubyte)val;
			s = p+1;
		}
	}
	sscanf(s, "%x", &val);
	a[i] = (ubyte)val;
	
	return 0;
}

extern RLSTATUS cli_config_port_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;
	ubyte4		port_num;
	
    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "PortNo", mConfigPort_PortNo, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

	CONVERT_StrTo(pVal, &port_num, kDTuinteger);
	if((port_num > MAX_PORT_NUM) || (port_num == 0))  {
		cli_printf(pCliEnv, "Error: invalid port number\r\n");
		return STATUS_RCC_NO_ERROR;	
	}

    /* set RapidMark */
    status = RCC_RCB_WriteValueToRCB( pCliEnv, "gPortNo", NULL, pVal); 
    if ( OK > status )
    {
        /* TO DO: Add your error-handling code here */

        return status;
    }


    /* TO DO: Add your handler code here */

    return status;
}

extern RLSTATUS cli_config_port_security_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
	ubyte4 port;
	
	if(cli_read_port(pCliEnv, &port) != 0) {
		cli_printf(pCliEnv, "Error: can't get PortNo\r\n");
		return STATUS_RCC_NO_ERROR;	
	}

    return status;
}

extern RLSTATUS cli_config_port_no_security_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
	ubyte4 port;
	
	if(cli_read_port(pCliEnv, &port) != 0) {
		cli_printf(pCliEnv, "Error: can't get PortNo\r\n");
		return STATUS_RCC_NO_ERROR;	
	}

    return status;
}

extern RLSTATUS cli_config_port_security_maximum_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "maximum", mConfigPortPort_securityMaximum_Maximum, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */

    return status;
}

extern RLSTATUS cli_config_port_security_macaddr_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "mac-address", mConfigPortPort_securityMac_address_Mac_address, &pParamDescr);
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
	
#if 0
	ubyte4 logic_port, hw_port, portvec;
	ubyte mac_buf[6];
	mac_list_conf_t conf_maclist;
	oam_mac_list_t oam_maclist;
	
	if(cli_read_port(pCliEnv, &logic_port) != 0) {
		cli_printf(pCliEnv, "Error: can't get PortNo\r\n");
		return STATUS_RCC_NO_ERROR;	
	}

	hw_port = hal_swif_lport_2_hport((u8)logic_port);
	cli_str_to_macaddr(mac_buf, pVal);

	cli_printf(pCliEnv, "mac=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\r\n", mac_buf[0], mac_buf[1], mac_buf[2], mac_buf[3], mac_buf[4], mac_buf[5]);

	memcpy(oam_maclist.MacAddr, mac_buf, 6);
	portvec = 1<<hw_port;
	*(GT_UINT *)(&(oam_maclist.PortVec[0])) = htonl(portvec);
	oam_maclist.Priority = 0;

	//if(MacListAdd(&oam_maclist) != GT_OK) {
		//cli_printf(pCliEnv, "Error: MacListAdd function failed\r\n");
		//return STATUS_RCC_NO_ERROR;
	//}
#endif	
	
    return status;
}

