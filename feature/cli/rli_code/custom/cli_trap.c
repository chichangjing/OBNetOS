

/*************************************************************
 * Filename     : cli_trap.c
 * Description  : Command interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

/* Standard includes */
#include <stdlib.h>
#include <string.h>

/* LwIP includes */

/* BSP includes */

/* Other includes */
#include "cli_sys.h"
#include "cli_util.h"
#include "conf_comm.h"
#include "conf_sys.h"
#include "conf_global.h"

#include "hal_swif_types.h"
#include "hal_swif_message.h"

RLSTATUS cli_config_trap_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    ubyte	mac_all_zero[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	ubyte	trap_enable;
	
	if(conf_get_trap_enable(&trap_enable) != CONF_ERR_NONE) {
		cli_printf(pCliEnv, "Error: Trap enable get failed!\r\n");
		return STATUS_RCC_NO_ERROR;
	}

	if((trap_enable != TRAP_ENABLE) && (trap_enable != TRAP_DISABLE)) {
		if(conf_set_trap_enable(TRAP_ENABLE) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap enable set failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if(conf_set_trap_server_mac(mac_all_zero) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap server mac set failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if(conf_set_trap_frame_gate(0) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap gate set failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}
	}

	if(trap_enable == TRAP_ENABLE)
		return OK;

	if(trap_enable == TRAP_DISABLE) {
		if(conf_set_trap_enable(TRAP_ENABLE) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap enable set failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}
	}
	
    return status;
}

RLSTATUS cli_config_trap_no_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
	ubyte	mac_all_zero[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	ubyte	trap_enable;
	
	if(conf_get_trap_enable(&trap_enable) != CONF_ERR_NONE) {
		cli_printf(pCliEnv, "Error: Trap enable get failed!\r\n");
		return STATUS_RCC_NO_ERROR;
	}

	if((trap_enable != TRAP_ENABLE) && (trap_enable != TRAP_DISABLE)) {
		if(conf_set_trap_enable(TRAP_DISABLE) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap enable set failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if(conf_set_trap_server_mac(mac_all_zero) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap server mac set failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if(conf_set_trap_frame_gate(0) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap gate set failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}
	}

	if(trap_enable == TRAP_DISABLE)
		return OK;

	if(trap_enable == TRAP_ENABLE) {
		if(conf_set_trap_enable(TRAP_DISABLE) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap enable set failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}
	}
	
    return status;
}

RLSTATUS cli_config_trap_server_mac_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "mac-addr", mConfigTrapServer_mac_Mac_addr, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
    {
		ubyte macAddr[6];
		ubyte trap_enable;
		
		if(conf_get_trap_enable(&trap_enable) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap enable get failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
		if(trap_enable != TRAP_ENABLE) {
			cli_printf(pCliEnv, "Error: Trap feature is not enable\r\n");
			return STATUS_RCC_NO_ERROR;
		}
			
		cli_extract_mac(macAddr, pVal);
		cli_printf(pCliEnv, "Server MAC set to %02x-%02x-%02x-%02x-%02x-%02x ... ", 
			macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
		
		if(conf_set_trap_server_mac(macAddr) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Failed\r\n");
			return STATUS_RCC_NO_ERROR;
		} else {
			cli_printf(pCliEnv, "Done\r\n");
		}
	}
	
    return status;
}

RLSTATUS cli_config_trap_add_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "trapType", mConfigTrapAdd_TrapType, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
    {
    	ubyte4 trap_gate = 0;
		ubyte trap_enable;
		
		if(conf_get_trap_enable(&trap_enable) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap enable get failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
		if(trap_enable != TRAP_ENABLE) {
			cli_printf(pCliEnv, "Error: Trap feature is not enable\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
		if(conf_get_trap_frame_gate(&trap_gate) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap gate get failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
		if(COMPARE_Strings(pVal, "REBOOT")) {
			trap_gate |= TRAP_MASK_DEV_REBOOT;
		} else if(COMPARE_Strings(pVal, "PORT")) {
			trap_gate |= TRAP_MASK_PORT_STATUS;
		} else if(COMPARE_Strings(pVal, "RING")) {
			trap_gate |= TRAP_MASK_RING_STATUS;
		} else if(COMPARE_Strings(pVal, "VOL")) {
			trap_gate |= TRAP_MASK_VOL_OVER;
		} else if(COMPARE_Strings(pVal, "TRAFFIC")) {
			trap_gate |= TRAP_MASK_TRAFFIC_OVER;
		}

		if(conf_set_trap_frame_gate(trap_gate) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap gate get failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}		
	}
	
    return status;
}

RLSTATUS cli_config_trap_delete_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "trapType", mConfigTrapDelete_TrapType, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
    {
    	ubyte4 trap_gate = 0;
		ubyte trap_enable;
		
		if(conf_get_trap_enable(&trap_enable) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap enable get failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
		if(trap_enable != TRAP_ENABLE) {
			cli_printf(pCliEnv, "Error: Trap feature is not enable\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
		if(conf_get_trap_frame_gate(&trap_gate) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap gate get failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
		if(COMPARE_Strings(pVal, "REBOOT")) {
			trap_gate &= ~TRAP_MASK_DEV_REBOOT;
		} else if(COMPARE_Strings(pVal, "PORT")) {
			trap_gate &= ~TRAP_MASK_PORT_STATUS;
		} else if(COMPARE_Strings(pVal, "RING")) {
			trap_gate &= ~TRAP_MASK_RING_STATUS;
		} else if(COMPARE_Strings(pVal, "VOL")) {
			trap_gate &= ~TRAP_MASK_VOL_OVER;
		} else if(COMPARE_Strings(pVal, "TRAFFIC")) {
			trap_gate &= ~TRAP_MASK_TRAFFIC_OVER;
		}

		if(conf_set_trap_frame_gate(trap_gate) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap gate get failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}		
	}
	
    return status;
}

RLSTATUS cli_config_trap_show_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;

    /* TO DO: Add your handler code here */
    {
    	ubyte4 trap_gate = 0;
		ubyte trap_enable;
		ubyte mac[6];
		int i;
		
		if(conf_get_trap_enable(&trap_enable) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap enable get failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
		if(trap_enable != TRAP_ENABLE) {
			cli_printf(pCliEnv, "Trap Feature is disabled\r\n");
			return OK;
		} else {
			cli_printf(pCliEnv, "Trap Feature     : Enable\r\n");
		}

		cli_printf(pCliEnv, "Trap Server MAC  : ");
		if(conf_get_trap_server_mac(mac) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap server mac get failed!\r\n");
		} else {
			cli_printf(pCliEnv, "%02x-%02x-%02x-%02x-%02x-%02x\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}

		cli_printf(pCliEnv, "Trap Frame Gate  : ");
		if(conf_get_trap_frame_gate(&trap_gate) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: Trap gate get failed!\r\n");
		} else {
			if(trap_gate & TRAP_MASK_DEV_REBOOT)
				cli_printf(pCliEnv, "REBOOT ");
			if(trap_gate & TRAP_MASK_PORT_STATUS)
				cli_printf(pCliEnv, "PORT ");			
			if(trap_gate & TRAP_MASK_RING_STATUS)
				cli_printf(pCliEnv, "RING ");
			if(trap_gate & TRAP_MASK_VOL_OVER)
				cli_printf(pCliEnv, "VOL ");
			if(trap_gate & TRAP_MASK_TRAFFIC_OVER)
				cli_printf(pCliEnv, "TRAFFIC ");
			cli_printf(pCliEnv, "\r\n");
		}
	}
	
    return status;
}



