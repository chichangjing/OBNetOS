

/*************************************************************
 * Filename     : cli_obring.c
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
#include "robo_drv.h"

/* Other includes */
#include "conf_sys.h"
#include "cli_sys.h"
#include "cli_util.h"

#include "conf_comm.h"
#include "conf_map.h"
#include "conf_ring.h"

#include "obring.h"

extern ubyte4 gDomainID[kRCC_MAX_CLI_TASK+1];

int	cli_read_obring_domain_id(cli_env *pCliEnv, unsigned int *pDomainID)
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
			*pDomainID = gDomainID[i];
			return 0;
		}
	}

	ipAddr = OS_SPECIFIC_GET_ADDR(pCliEnv);
	CONVERT_ToStr(&ipAddr, buffer, kDTipaddress);

	cli_printf(pCliEnv, "Error: reading port number from CLI session from %s\r\n", buffer);

	return 1;
}


extern RLSTATUS cli_config_obring_mode_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "ring_mode", mConfigObringMode_Ring_mode, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
    {
		

	}
    return status;
}

extern RLSTATUS cli_config_obring_new_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;
    sbyte       *pVal3 = NULL;
    paramDescr  *pParamDescr3;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "domain_name", mConfigObringNew_Domain_name, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "domain_id", mConfigObringNew_Domain_id, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "node_prio", mConfigObringNew_Node_prio, &pParamDescr3 );
    if ( OK != status )
    {
		return(status);
    } else pVal3 = (sbyte*)(pParamDescr3->pValue);

    /* TO DO: Add your handler code here */
    {
		sbyte DomainName[8];
		sbyte2 DomainID;
		eNodePrio NodePrio;
		tRingConfigGlobal RingGlobalCfg;
		ubyte RecIndex;
		tRingConfigRec RingRecordCfg;
		ubyte2 usData;
		
		MEMSET(DomainName, 0, 8);
		STRCPY(DomainName, pVal1);
		CONVERT_StrTo(pVal2, &DomainID, kDTushort);
		if(COMPARE_Strings(pVal3, "high")) {
			NodePrio = PRIO_HIGH;
		} else if(COMPARE_Strings(pVal3, "medium")) {
			NodePrio = PRIO_MEDIUM;
		} else {
			NodePrio = PRIO_LOW;
		}

		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
		if(RingGlobalCfg.ucRecordNum == MAX_RING_NUM) {
			cli_printf(pCliEnv, "Warning: ring config record is full!\r\n");
			return STATUS_RCC_NO_ERROR;	
		}
		
		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum < MAX_RING_NUM)) {
			for(RecIndex=0; RecIndex<RingGlobalCfg.ucRecordNum; RecIndex++) {
				if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
					return STATUS_RCC_NO_ERROR;
				}
				usData = RingRecordCfg.usDomainId[0];
				usData = (usData << 8) | RingRecordCfg.usDomainId[1];
				if(usData == DomainID) {
					cli_printf(pCliEnv, "Error: domain id is already existing!\r\n");
					return STATUS_RCC_NO_ERROR;
				}
			}
			RingGlobalCfg.ucRecordNum++;

			memset(&RingRecordCfg, 0, sizeof(tRingConfigRec));
			RingRecordCfg.ucRingIndex = RingGlobalCfg.ucRecordNum - 1;
			RingRecordCfg.ucEnable = 0x01;
			memcpy(RingRecordCfg.ucDomainName, DomainName, 8);
			RingRecordCfg.usDomainId[0] = (ubyte)((DomainID & 0xFF00) >> 8);
			RingRecordCfg.usDomainId[1] = (ubyte)(DomainID & 0x00FF);
			RingRecordCfg.usRingId[0] = 0;
			RingRecordCfg.usRingId[1] = RingGlobalCfg.ucRecordNum;
			RingRecordCfg.ucRingMode = PORT_FAST_MODE;
			RingRecordCfg.ucNodePrio = NodePrio;
			RingRecordCfg.usAuthTime[0] = (ubyte)((DEFAULT_AUTH_TIME & 0xFF00) >> 8);
			RingRecordCfg.usAuthTime[1] = (ubyte)(DEFAULT_AUTH_TIME & 0x00FF);
			RingRecordCfg.usBallotTime[0] = (ubyte)((DEFAULT_BALLOT_TIME & 0xFF00) >> 8);
			RingRecordCfg.usBallotTime[1] = (ubyte)(DEFAULT_BALLOT_TIME & 0x00FF);				
			RingRecordCfg.usHelloTime[0] = (ubyte)((DEFAULT_HELLO_TIME & 0xFF00) >> 8);
			RingRecordCfg.usHelloTime[1] = (ubyte)(DEFAULT_HELLO_TIME & 0x00FF);
			RingRecordCfg.usFailTime[0] = (ubyte)((DEFAULT_FAIL_TIME & 0xFF00) >> 8);
			RingRecordCfg.usFailTime[1] = (ubyte)(DEFAULT_FAIL_TIME & 0x00FF);
			RingRecordCfg.ucPrimaryPort = 1;
			RingRecordCfg.ucSecondaryPort = 2;
		}
		
		if((RingGlobalCfg.ucRecordNum == 0x00) || (RingGlobalCfg.ucRecordNum > MAX_RING_NUM)) {
			RingGlobalCfg.ucGlobalEnable = 1;
			RingGlobalCfg.ucRecordNum = 1;
            
			memset(&RingRecordCfg, 0, sizeof(tRingConfigRec));
			RingRecordCfg.ucRingIndex = 0x00;
			RingRecordCfg.ucEnable = 0x01;
			memcpy(RingRecordCfg.ucDomainName, DomainName, 8);
			RingRecordCfg.usDomainId[0] = (ubyte)((DomainID & 0xFF00) >> 8);
			RingRecordCfg.usDomainId[1] = (ubyte)(DomainID & 0x00FF);
			RingRecordCfg.usRingId[0] = 0x00;
			RingRecordCfg.usRingId[1] = 0x01;
			RingRecordCfg.ucRingMode = PORT_FAST_MODE;
			RingRecordCfg.ucNodePrio = NodePrio;
			RingRecordCfg.usAuthTime[0] = (ubyte)((DEFAULT_AUTH_TIME & 0xFF00) >> 8);
			RingRecordCfg.usAuthTime[1] = (ubyte)(DEFAULT_AUTH_TIME & 0x00FF);	
			RingRecordCfg.usBallotTime[0] = (ubyte)((DEFAULT_BALLOT_TIME & 0xFF00) >> 8);
			RingRecordCfg.usBallotTime[1] = (ubyte)(DEFAULT_BALLOT_TIME & 0x00FF);				
			RingRecordCfg.usHelloTime[0] = (ubyte)((DEFAULT_HELLO_TIME & 0xFF00) >> 8);
			RingRecordCfg.usHelloTime[1] = (ubyte)(DEFAULT_HELLO_TIME & 0x00FF);
			RingRecordCfg.usFailTime[0] = (ubyte)((DEFAULT_FAIL_TIME & 0xFF00) >> 8);
			RingRecordCfg.usFailTime[1] = (ubyte)(DEFAULT_FAIL_TIME & 0x00FF);			
			RingRecordCfg.ucPrimaryPort = 1;
			RingRecordCfg.ucSecondaryPort = 2;
		}

		if(RingGlobalCfg.ucRecordNum > 0) {
			if(conf_set_ring_record(RingGlobalCfg.ucRecordNum - 1, &RingRecordCfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "Error: eeprom write failed\r\n");
				return STATUS_RCC_NO_ERROR;
			}
		}

		if(conf_set_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: eeprom write failed\r\n");
			return STATUS_RCC_NO_ERROR;
		}
	}

    return status;
}

extern RLSTATUS cli_config_obring_delete_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "domain_id", mConfigObringDelete_Domain_id, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
    {
		sbyte2 DomainID;
		tRingConfigGlobal RingGlobalCfg;
		ubyte RecIndex0, RecIndex1;
		tRingConfigRec RingRecordCfg;
		ubyte2 usData;
		
		CONVERT_StrTo(pVal, &DomainID, kDTushort);

		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			for(RecIndex0=0; RecIndex0<RingGlobalCfg.ucRecordNum; RecIndex0++) {
				if(conf_get_ring_record(RecIndex0, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
					return STATUS_RCC_NO_ERROR;
				}
				usData = RingRecordCfg.usDomainId[0];
				usData = (usData << 8) | RingRecordCfg.usDomainId[1];
				if(usData == DomainID) {
					break;
				}
			}

			if(RecIndex0 == RingGlobalCfg.ucRecordNum) {
				cli_printf(pCliEnv, "Error: invalid demain id!\r\n");
				return STATUS_RCC_NO_ERROR;				
			}
			
			for(RecIndex1=RecIndex0+1; RecIndex1<RingGlobalCfg.ucRecordNum; RecIndex1++) {
				if(conf_get_ring_record(RecIndex1, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
					return STATUS_RCC_NO_ERROR;
				}
				if(conf_set_ring_record(RecIndex1-1, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "Error: eeprom write failed\r\n");
					return STATUS_RCC_NO_ERROR;
				}				
			}
			RingGlobalCfg.ucRecordNum--;
			if(RingGlobalCfg.ucRecordNum == 0)
				RingGlobalCfg.ucGlobalEnable = 0;

			if(conf_set_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "Error: eeprom write failed\r\n");
				return STATUS_RCC_NO_ERROR;
			}
		}
	}
    return status;
}

extern RLSTATUS cli_config_obring_domain_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;
	ubyte4		domain_id;
	
    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "domain_id", mConfigObringDomain_id_Domain_id, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

	CONVERT_StrTo(pVal, &domain_id, kDTuinteger);
	if((domain_id > 31) || (domain_id == 0))  {
		cli_printf(pCliEnv, "Error: invalid domain_id\r\n");
		return STATUS_RCC_NO_ERROR;	
	}
	
    /* set RapidMark */
    status = RCC_RCB_WriteValueToRCB( pCliEnv, "gDomainID", NULL, pVal); 
    if ( OK > status )
    {
        return status;
    }

	/* TO DO: Add your handler code here */
	{
		tRingConfigGlobal RingGlobalCfg;
		ubyte RecIndex;
		tRingConfigRec RingRecordCfg;
		ubyte2 usData;

		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			for(RecIndex=0; RecIndex<RingGlobalCfg.ucRecordNum; RecIndex++) {
				if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
					return STATUS_RCC_NO_ERROR;
				}
				usData = RingRecordCfg.usDomainId[0];
				usData = (usData << 8) | RingRecordCfg.usDomainId[1];
				if(usData == (ubyte2)(domain_id & 0x0000FFFF)) {
					break;
				}
			}
			
			if(RecIndex == RingGlobalCfg.ucRecordNum) {
				cli_printf(pCliEnv, "Error: not find configuraion for domain id %d\r\n", domain_id);
				return STATUS_RCC_NO_ERROR;				
			}
		} else {
			cli_printf(pCliEnv, "Error: not find configuraion for domain id %d\r\n", domain_id);
			return STATUS_RCC_NO_ERROR;	
		}
	}
	
    return status;
}

extern RLSTATUS cli_config_obring_domain_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "domain_id", mConfigObringDomain_id_Domain_id, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* set RapidMark */
    status = RCC_RCB_WriteValueToRCB( pCliEnv, "gDomainID", NULL, pVal); 
    if ( OK > status )
    {
        return status;
    }

	/* TO DO: Add your handler code here */
	{
		ubyte4 domain_id;
		tRingConfigGlobal RingGlobalCfg;
		ubyte RecIndex;
		tRingConfigRec RingRecordCfg;
		ubyte2 usData;

		
		if(cli_read_obring_domain_id(pCliEnv, &domain_id) != 0) {
			cli_printf(pCliEnv, "Error: can't get DomainID\r\n");
			return STATUS_RCC_NO_ERROR;	
		}

		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			for(RecIndex=0; RecIndex<RingGlobalCfg.ucRecordNum; RecIndex++) {
				if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
					return STATUS_RCC_NO_ERROR;
				}
				usData = RingRecordCfg.usDomainId[0];
				usData = (usData << 8) | RingRecordCfg.usDomainId[1];
				if(usData == (ubyte2)(domain_id & 0x0000FFFF)) {
					break;
				}
			}
			if(RecIndex == RingGlobalCfg.ucRecordNum) {
				cli_printf(pCliEnv, "Error: invalid demain id!\r\n");
				return STATUS_RCC_NO_ERROR;				
			}

			RingRecordCfg.ucEnable = 0x01;
			if(conf_set_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "Error: eeprom write failed\r\n");
				return STATUS_RCC_NO_ERROR;
			}
		} else {
			cli_printf(pCliEnv, "Error: no config info for demain id %d!\r\n", domain_id);
			return STATUS_RCC_NO_ERROR;	
		}
	}

    return status;
}

extern RLSTATUS cli_config_obring_domain_disable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "domain_id", mConfigObringDomain_id_Domain_id, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* set RapidMark */
    status = RCC_RCB_WriteValueToRCB( pCliEnv, "gDomainID", NULL, pVal); 
    if ( OK > status )
    {
        return status;
    }
	
    /* TO DO: Add your handler code here */
	{
		ubyte4 domain_id;
		tRingConfigGlobal RingGlobalCfg;
		ubyte RecIndex;
		tRingConfigRec RingRecordCfg;
		ubyte2 usData;

		
		if(cli_read_obring_domain_id(pCliEnv, &domain_id) != 0) {
			cli_printf(pCliEnv, "Error: can't get DomainID\r\n");
			return STATUS_RCC_NO_ERROR;	
		}

		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			for(RecIndex=0; RecIndex<RingGlobalCfg.ucRecordNum; RecIndex++) {
				if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
					return STATUS_RCC_NO_ERROR;
				}
				usData = RingRecordCfg.usDomainId[0];
				usData = (usData << 8) | RingRecordCfg.usDomainId[1];
				if(usData == (ubyte2)(domain_id & 0x0000FFFF)) {
					break;
				}
			}
			if(RecIndex == RingGlobalCfg.ucRecordNum) {
				cli_printf(pCliEnv, "Error: invalid demain id!\r\n");
				return STATUS_RCC_NO_ERROR;				
			}

			RingRecordCfg.ucEnable = 0x00;
			if(conf_set_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "Error: eeprom write failed\r\n");
				return STATUS_RCC_NO_ERROR;
			}
		} else {
			cli_printf(pCliEnv, "Error: no config info for demain id %d!\r\n", domain_id);
			return STATUS_RCC_NO_ERROR;	
		}
	}
    return status;
}


extern RLSTATUS cli_config_obring_domain_ring_port_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;
    sbyte       *pVal3 = NULL;
    paramDescr  *pParamDescr3;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "domain_id", mConfigObringDomain_id_Domain_id, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* set RapidMark */
    status = RCC_RCB_WriteValueToRCB( pCliEnv, "gDomainID", NULL, pVal1); 
    if ( OK > status )
    {
        /* TO DO: Add your error-handling code here */

        return status;
    }


    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "ring_port_1", mConfigObringDomain_idRing_port_Ring_port_1, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "ring_port_2", mConfigObringDomain_idRing_port_Ring_port_2, &pParamDescr3 );
    if ( OK != status )
    {
		return(status);
    } else pVal3 = (sbyte*)(pParamDescr3->pValue);

    /* TO DO: Add your handler code here */

    {	
		ubyte4 domain_id;
		ubyte RingPort1, RingPort2;
		tRingConfigGlobal RingGlobalCfg;
		ubyte RecIndex;
		tRingConfigRec RingRecordCfg;
		ubyte2 usData;
		

		CONVERT_StrTo(pVal2, &RingPort1, kDTuchar);
		CONVERT_StrTo(pVal3, &RingPort2, kDTuchar);

		if((RingPort1 == 0) || (RingPort2 == 0) || (RingPort1 > MAX_PORT_NUM) || (RingPort2 > MAX_PORT_NUM) || (RingPort1 == RingPort2)) {
			cli_printf(pCliEnv, "Error: ring port number\r\n");
			return STATUS_RCC_NO_ERROR;	
		}
		
		if(cli_read_obring_domain_id(pCliEnv, &domain_id) != 0) {
			cli_printf(pCliEnv, "Error: can't get DomainID\r\n");
			return STATUS_RCC_NO_ERROR;	
		}

		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			for(RecIndex=0; RecIndex<RingGlobalCfg.ucRecordNum; RecIndex++) {
				if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
					return STATUS_RCC_NO_ERROR;
				}
				usData = RingRecordCfg.usDomainId[0];
				usData = (usData << 8) | RingRecordCfg.usDomainId[1];
				if(usData == (ubyte2)(domain_id & 0x0000FFFF)) {
					break;
				}
			}

			if(RecIndex == RingGlobalCfg.ucRecordNum) {
				cli_printf(pCliEnv, "Error: invalid demain id!\r\n");
				return STATUS_RCC_NO_ERROR;				
			}
			
			RingRecordCfg.ucPrimaryPort = RingPort1;
			RingRecordCfg.ucSecondaryPort = RingPort2;
			if(conf_set_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "Error: eeprom write failed\r\n");
				return STATUS_RCC_NO_ERROR;
			}
		}
	}
	

    return status;
}

extern RLSTATUS cli_config_obring_domain_hello_times_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "domain_id", mConfigObringDomain_id_Domain_id, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* set RapidMark */
    status = RCC_RCB_WriteValueToRCB( pCliEnv, "gDomainID", NULL, pVal1); 
    if ( OK > status )
    {
        /* TO DO: Add your error-handling code here */

        return status;
    }


    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "times", mConfigObringDomain_idHello_times_Times, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* TO DO: Add your handler code here */
	{
		ubyte4 domain_id;
		tRingConfigGlobal RingGlobalCfg;
		ubyte RecIndex;
		tRingConfigRec RingRecordCfg;
		ubyte2 usData, usHelloTimes;

		
		if(cli_read_obring_domain_id(pCliEnv, &domain_id) != 0) {
			cli_printf(pCliEnv, "Error: can't get DomainID\r\n");
			return STATUS_RCC_NO_ERROR;	
		}

		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			for(RecIndex=0; RecIndex<RingGlobalCfg.ucRecordNum; RecIndex++) {
				if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
					return STATUS_RCC_NO_ERROR;
				}
				usData = RingRecordCfg.usDomainId[0];
				usData = (usData << 8) | RingRecordCfg.usDomainId[1];
				if(usData == (ubyte2)(domain_id & 0x0000FFFF)) {
					break;
				}
			}
			if(RecIndex == RingGlobalCfg.ucRecordNum) {
				cli_printf(pCliEnv, "Error: invalid demain id!\r\n");
				return STATUS_RCC_NO_ERROR;				
			}

			CONVERT_StrTo(pVal2, &usHelloTimes, kDTushort);
			RingRecordCfg.usHelloTime[0] = (unsigned char)((usHelloTimes & 0xFF00) >> 8);
			RingRecordCfg.usHelloTime[1] = (unsigned char)(usHelloTimes & 0x00FF);
			if(conf_set_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "Error: eeprom write failed\r\n");
				return STATUS_RCC_NO_ERROR;
			}
		} else {
			cli_printf(pCliEnv, "Error: no config info for demain id %d!\r\n", domain_id);
			return STATUS_RCC_NO_ERROR;	
		}
	}
    return status;
}

extern RLSTATUS cli_config_obring_domain_fail_times_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "domain_id", mConfigObringDomain_id_Domain_id, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* set RapidMark */
    status = RCC_RCB_WriteValueToRCB( pCliEnv, "gDomainID", NULL, pVal1); 
    if ( OK > status )
    {
        /* TO DO: Add your error-handling code here */

        return status;
    }


    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "times", mConfigObringDomain_idFail_times_Times, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* TO DO: Add your handler code here */
	{
		ubyte4 domain_id;
		tRingConfigGlobal RingGlobalCfg;
		ubyte RecIndex;
		tRingConfigRec RingRecordCfg;
		ubyte2 usData, usFailTimes;

		
		if(cli_read_obring_domain_id(pCliEnv, &domain_id) != 0) {
			cli_printf(pCliEnv, "Error: can't get DomainID\r\n");
			return STATUS_RCC_NO_ERROR;	
		}

		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			for(RecIndex=0; RecIndex<RingGlobalCfg.ucRecordNum; RecIndex++) {
				if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "Error: eeprom read failed\r\n");
					return STATUS_RCC_NO_ERROR;
				}
				usData = RingRecordCfg.usDomainId[0];
				usData = (usData << 8) | RingRecordCfg.usDomainId[1];
				if(usData == (ubyte2)(domain_id & 0x0000FFFF)) {
					break;
				}
			}
			if(RecIndex == RingGlobalCfg.ucRecordNum) {
				cli_printf(pCliEnv, "Error: invalid demain id!\r\n");
				return STATUS_RCC_NO_ERROR;				
			}

			CONVERT_StrTo(pVal2, &usFailTimes, kDTushort);
			RingRecordCfg.usHelloTime[0] = (unsigned char)((usFailTimes & 0xFF00) >> 8);
			RingRecordCfg.usHelloTime[1] = (unsigned char)(usFailTimes & 0x00FF);
			if(conf_set_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "Error: eeprom write failed\r\n");
				return STATUS_RCC_NO_ERROR;
			}
		} else {
			cli_printf(pCliEnv, "Error: no config info for demain id %d!\r\n", domain_id);
			return STATUS_RCC_NO_ERROR;	
		}
	}
    return status;
}



extern RLSTATUS cli_config_obring_domain_primary_port_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "domain_id", mConfigObringDomain_id_Domain_id, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* set RapidMark */
    status = RCC_RCB_WriteValueToRCB( pCliEnv, "gDomainID", NULL, pVal1); 
    if ( OK > status )
    {
        /* TO DO: Add your error-handling code here */

        return status;
    }


    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "port", mConfigObringDomain_idPrimary_portEnable_Port, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* TO DO: Add your handler code here */

    {
		cli_printf(pCliEnv, "Waring: not support!\r\n");
	}
	
    return status;
}

extern RLSTATUS cli_config_obring_domain_primary_port_disable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "domain_id", mConfigObringDomain_id_Domain_id, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* set RapidMark */
    status = RCC_RCB_WriteValueToRCB( pCliEnv, "gDomainID", NULL, pVal); 
    if ( OK > status )
    {
        /* TO DO: Add your error-handling code here */

        return status;
    }


    /* TO DO: Add your handler code here */
    {
		cli_printf(pCliEnv, "Waring: not support!\r\n");	
	}
    return status;
}


extern RLSTATUS cli_show_obring_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;

#if OBRING_DEV
    /* TO DO: Add your handler code here */
    {
    	sbyte DomainName[8];
		sbyte PrimaryNeighborString[32], PrimaryBallotIdString[32], PrimaryPortStateString[40];
		sbyte SecondaryNeighborString[32], SecondaryBallotIdString[32], SecondaryPortStateString[40];
		tRingConfigGlobal RingGlobalCfg;
		ubyte RecIndex;
		tRingConfigRec RingRecordCfg;
		sbyte2 DomainID, RingID, HelloTime, FailTime, BallotTime, AuthTime;
		tRingConfigRec *pRingConfig;
		tRingState *pRingState;
		extern unsigned char DevMac[];
		extern tRingInfo RingInfo;
		extern tRingTimers RingTimer[];

		cli_printf(pCliEnv, "\r\n");
		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "    Error: Read OB-Ring configuration failed\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		if(RingGlobalCfg.ucGlobalEnable != 0x01) {
			cli_printf(pCliEnv, "    Warning: The OB-Ring protocol golbal disabled\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		cli_printf(pCliEnv, "    OB-Ring Number  : %d\r\n\r\n", RingGlobalCfg.ucRecordNum);

		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			for(RecIndex=0; RecIndex<RingGlobalCfg.ucRecordNum; RecIndex++) {
				if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "    Error: eeprom read failed\r\n\r\n");
					return STATUS_RCC_NO_ERROR;
				}
				pRingConfig = &(RingInfo.RingConfig[RecIndex]);
				pRingState = &(RingInfo.DevState[RecIndex]);

				/* For configuration */
				memcpy(DomainName, RingRecordCfg.ucDomainName, 8);
				DomainID = RingRecordCfg.usDomainId[0];
				DomainID = (DomainID << 8) | RingRecordCfg.usDomainId[1];
				AuthTime = RingRecordCfg.usAuthTime[0];
				AuthTime = (AuthTime << 8) | RingRecordCfg.usAuthTime[1];					
				BallotTime = RingRecordCfg.usBallotTime[0];
				BallotTime = (BallotTime << 8) | RingRecordCfg.usBallotTime[1];					
				HelloTime = RingRecordCfg.usHelloTime[0];
				HelloTime = (HelloTime << 8) | RingRecordCfg.usHelloTime[1];
				FailTime = RingRecordCfg.usFailTime[0];
				FailTime = (FailTime << 8) | RingRecordCfg.usFailTime[1];
			
				memset(&PrimaryPortStateString, 0, 40);
				memset(&SecondaryPortStateString, 0, 40);
				memset(&PrimaryNeighborString, 0, 32);
				memset(&SecondaryNeighborString, 0, 32);
				memset(&PrimaryBallotIdString, 0, 32);
				memset(&SecondaryBallotIdString, 0, 32);
				
				/* For status */
				sprintf(PrimaryPortStateString,"%s-%s-%s", 
					(pRingState->PortState[INDEX_PRIMARY].LinkState == LINK_UP)? 				"Up":"Down", 
					(pRingState->PortState[INDEX_PRIMARY].StpState == FORWARDING)? 				"Forwarding": \
					(pRingState->PortState[INDEX_PRIMARY].StpState == BLOCKING)? 				"Blocking":"Unkown",
					(pRingState->PortState[INDEX_PRIMARY].RunState == PORT_IDLE)? 				"PortIdle": \
					(pRingState->PortState[INDEX_PRIMARY].RunState == PORT_DOWN)? 				"PortDown": \
					(pRingState->PortState[INDEX_PRIMARY].RunState == PORT_AUTH_REQ)?    		"AuthActive": \
					(pRingState->PortState[INDEX_PRIMARY].RunState == PORT_AUTH_FAIL)? 			"AuthFail": \
					(pRingState->PortState[INDEX_PRIMARY].RunState == PORT_BALLOT_ACTIVE)? 		"BallotActive": \
					(pRingState->PortState[INDEX_PRIMARY].RunState == PORT_BALLOT_FINISH)? 		"BallotFinish":"Unkown");
				
				sprintf(SecondaryPortStateString,"%s-%s-%s", 
					(pRingState->PortState[INDEX_SECONDARY].LinkState == LINK_UP)? 				"Up":"Down", 
					(pRingState->PortState[INDEX_SECONDARY].StpState == FORWARDING)? 			"Forwarding": \
					(pRingState->PortState[INDEX_SECONDARY].StpState == BLOCKING)? 				"Blocking":"Unkown",
					(pRingState->PortState[INDEX_SECONDARY].RunState == PORT_IDLE)? 			"PortIdle": \
					(pRingState->PortState[INDEX_SECONDARY].RunState == PORT_DOWN)? 			"PortDown": \
					(pRingState->PortState[INDEX_SECONDARY].RunState == PORT_AUTH_REQ)? 		"AuthActive": \
					(pRingState->PortState[INDEX_SECONDARY].RunState == PORT_AUTH_FAIL)? 		"AuthFail": \
					(pRingState->PortState[INDEX_SECONDARY].RunState == PORT_BALLOT_ACTIVE)? 	"BallotActive": \
					(pRingState->PortState[INDEX_SECONDARY].RunState == PORT_BALLOT_FINISH)? 	"BallotFinish":"Unkown");
				
				
				if(pRingState->PortState[INDEX_PRIMARY].NeighborValid == HAL_TRUE) {
					sprintf(PrimaryNeighborString, "Port%02d-%02x:%02x:%02x:%02x:%02x:%02x",
						pRingState->PortState[INDEX_PRIMARY].NeighborPortNo,
						pRingState->PortState[INDEX_PRIMARY].NeighborMac[0],
						pRingState->PortState[INDEX_PRIMARY].NeighborMac[1],
						pRingState->PortState[INDEX_PRIMARY].NeighborMac[2],
						pRingState->PortState[INDEX_PRIMARY].NeighborMac[3],
						pRingState->PortState[INDEX_PRIMARY].NeighborMac[4],
						pRingState->PortState[INDEX_PRIMARY].NeighborMac[5]);
				} else {
					sprintf(PrimaryNeighborString, "%s", "*");
				}

				if(pRingState->PortState[INDEX_SECONDARY].NeighborValid == HAL_TRUE) {
					sprintf(SecondaryNeighborString, "Port%02d-%02x:%02x:%02x:%02x:%02x:%02x",
						pRingState->PortState[INDEX_SECONDARY].NeighborPortNo,
						pRingState->PortState[INDEX_SECONDARY].NeighborMac[0],
						pRingState->PortState[INDEX_SECONDARY].NeighborMac[1],
						pRingState->PortState[INDEX_SECONDARY].NeighborMac[2],
						pRingState->PortState[INDEX_SECONDARY].NeighborMac[3],
						pRingState->PortState[INDEX_SECONDARY].NeighborMac[4],
						pRingState->PortState[INDEX_SECONDARY].NeighborMac[5]);	
				} else {
					sprintf(SecondaryNeighborString, "%s", "*");
				}

				if((pRingConfig->ucEnable == 0x01) && (pRingState->PortState[INDEX_PRIMARY].NeighborValid == HAL_TRUE)) {
					sprintf(PrimaryBallotIdString, "%s-%02x%02x%02x%02x%02x%02x",
						(pRingState->PortState[INDEX_PRIMARY].BallotId.Prio == PRIO_HIGH)? "High" : \
						(pRingState->PortState[INDEX_PRIMARY].BallotId.Prio == PRIO_MEDIUM)? "Medium" : \
						(pRingState->PortState[INDEX_PRIMARY].BallotId.Prio == PRIO_LOW)? "Low" : "Unkown",
						pRingState->PortState[INDEX_PRIMARY].BallotId.Mac[0],
						pRingState->PortState[INDEX_PRIMARY].BallotId.Mac[1],
						pRingState->PortState[INDEX_PRIMARY].BallotId.Mac[2],
						pRingState->PortState[INDEX_PRIMARY].BallotId.Mac[3],
						pRingState->PortState[INDEX_PRIMARY].BallotId.Mac[4],
						pRingState->PortState[INDEX_PRIMARY].BallotId.Mac[5]);
				} else {
					sprintf(PrimaryBallotIdString, "%s", "*");
				}
				
				if((pRingConfig->ucEnable == 0x01) && (pRingState->PortState[INDEX_SECONDARY].NeighborValid == HAL_TRUE)) {
					sprintf(SecondaryBallotIdString, "%s-%02x%02x%02x%02x%02x%02x",
						(pRingState->PortState[INDEX_SECONDARY].BallotId.Prio == PRIO_HIGH)? "High" : \
						(pRingState->PortState[INDEX_SECONDARY].BallotId.Prio == PRIO_MEDIUM)? "Medium" : \
						(pRingState->PortState[INDEX_SECONDARY].BallotId.Prio == PRIO_LOW)? "Low" : "Unkown",
						pRingState->PortState[INDEX_SECONDARY].BallotId.Mac[0],
						pRingState->PortState[INDEX_SECONDARY].BallotId.Mac[1],
						pRingState->PortState[INDEX_SECONDARY].BallotId.Mac[2],
						pRingState->PortState[INDEX_SECONDARY].BallotId.Mac[3],
						pRingState->PortState[INDEX_SECONDARY].BallotId.Mac[4],
						pRingState->PortState[INDEX_SECONDARY].BallotId.Mac[5]);
				} else {
					sprintf(SecondaryBallotIdString, "%s", "*");
				}
				
				/* CLI print */
				cli_printf(pCliEnv, "    Ring#%02d information ... \r\n", RecIndex);
				cli_printf(pCliEnv, "    DomainId       : %d\r\n", DomainID);
				cli_printf(pCliEnv, "    DomainName     : %s\r\n", DomainName);
				cli_printf(pCliEnv, "    RingEnable     : %s\r\n", (RingRecordCfg.ucEnable == 0x01)?"Yes":"No");
				cli_printf(pCliEnv, "    RingPort       : Port%02d/Port%02d\r\n", RingRecordCfg.ucPrimaryPort, RingRecordCfg.ucSecondaryPort);
				cli_printf(pCliEnv, "    RingMode       : %s\r\n", (RingRecordCfg.ucRingMode == PORT_CUSTUM_MODE)?"Custum": \
																   (RingRecordCfg.ucRingMode == PORT_FAST_MODE)?"Fast":"Unkown");
				cli_printf(pCliEnv, "    NodePriority   : %s\r\n", 	(RingRecordCfg.ucNodePrio == PRIO_HIGH)?"High": \
																		(RingRecordCfg.ucNodePrio == PRIO_MEDIUM)?"Medium":\
																		(RingRecordCfg.ucNodePrio == PRIO_LOW)?"Low":"Unkown");
				
				cli_printf(pCliEnv, "    AuthTimer      : %d seconds, %s, %s\r\n", AuthTime, (RingTimer[RecIndex].AuthP.Active == 0)? "PrimaryStop":"PrimaryStart",
																				(RingTimer[RecIndex].AuthS.Active == 0)? "SecondaryStop":"SecondaryStart");				
				cli_printf(pCliEnv, "    BallotTimer    : %d seconds, %s, %s\r\n", BallotTime, (RingTimer[RecIndex].BallotP.Active == 0)? "PrimaryStop":"PrimaryStart",
																				(RingTimer[RecIndex].BallotS.Active == 0)? "SecondaryStop":"SecondaryStart");
				cli_printf(pCliEnv, "    HelloTimer     : %d seconds, %s\r\n", HelloTime, (RingTimer[RecIndex].Hello.Active == 0)? "Stop":"Start");
				cli_printf(pCliEnv, "    FailTimer      : %d seconds, %s\r\n\r\n", FailTime, (RingTimer[RecIndex].Fail.Active == 0)? "Stop":"Start");


				cli_printf(pCliEnv, "    RingState      : %s\r\n", 	(pRingState->RingState == RING_HEALTH)?"Health":\
																		(pRingState->RingState == RING_FAULT)?"Fault":"Unkown");
				cli_printf(pCliEnv, "    NodeType       : %s\r\n", 	(pRingState->NodeType == NODE_TYPE_MASTER)?"Master":\
																		(pRingState->NodeType == NODE_TYPE_TRANSIT)?"Transit":"Unkown");
				cli_printf(pCliEnv, "    NodeState      : %s\r\n", 
														(pRingState->NodeState == NODE_STATE_FAIL) ? 			"Fail" : \
														(pRingState->NodeState == NODE_STATE_COMPLETE) ? 		"Complete" : \
														(pRingState->NodeState == NODE_STATE_LINK_DOWN) ? 		"LinkDown" : \
														(pRingState->NodeState == NODE_STATE_LINK_UP) ? 		"LinkUp" : \
														(pRingState->NodeState == NODE_STATE_IDLE) ? 			"Idle" : \
														(pRingState->NodeState == NODE_STATE_PRE_FORWARDING)? 	"PreForwarding" : "Unkown");
				if(pRingState->NodeType == NODE_TYPE_MASTER) {
					cli_printf(pCliEnv, "    SwitchTimes    : %d\r\n", pRingState->SwitchTimes);
					cli_printf(pCliEnv, "    StormCount     : %d\r\n", pRingState->StormCount);
					cli_printf(pCliEnv, "    HelloSequence  : %d\r\n", pRingState->HelloSeq);
					cli_printf(pCliEnv, "    HelloElapsed   : %d ms\r\n", pRingState->HelloElapsed);
				}
				
				cli_printf(pCliEnv, "    AuthTimeCount  : Primary: %d, Secondary: %d\r\n", 
														pRingState->PortState[INDEX_PRIMARY].AuthTimoutCount,
														pRingState->PortState[INDEX_SECONDARY].AuthTimoutCount);
				cli_printf(pCliEnv, "\r\n");
				cli_printf(pCliEnv, "                     Port information display (DevMac: %02x:%02x:%02x:%02x:%02x:%02x)\r\n", DevMac[0], DevMac[1], DevMac[2], DevMac[3], DevMac[4], DevMac[5]);
				cli_printf(pCliEnv, "    ========================================================================================\r\n");	
				cli_printf(pCliEnv, "    RingPort   PortState                        NeighborPort-Mac          BallotId          \r\n");
				cli_printf(pCliEnv, "    ========================================================================================\r\n");	
				cli_printf(pCliEnv, "    Port%02d     %-32s %-25s %-20s \r\n", RingRecordCfg.ucPrimaryPort, PrimaryPortStateString, PrimaryNeighborString, PrimaryBallotIdString);
				cli_printf(pCliEnv, "    Port%02d     %-32s %-25s %-20s \r\n", RingRecordCfg.ucSecondaryPort, SecondaryPortStateString, SecondaryNeighborString, SecondaryBallotIdString);
			}
			
			cli_printf(pCliEnv, "\r\n");
		}

	}
#endif

    return status;
}

extern RLSTATUS obring_command_getnode_response_handle(cli_env *pCliEnv, tRMsgCmd *pMsgCmdRsp, unsigned char DstNodeIndex)
{
	sbyte WestPortInfoString[32];
	sbyte EastPortInfoString[32];
	sbyte SoftwareVersion[16];
	
	memset(&WestPortInfoString[0], 0, 32);
	memset(&EastPortInfoString[0], 0, 32);

	/* For WestPortInfoString */
	sprintf(WestPortInfoString,"P%02d-%s-%s", pMsgCmdRsp->Action.RspGetNode.WestPortNo,
		(pMsgCmdRsp->Action.RspGetNode.WestPortLink == LINK_UP)? 		"Up":"Down", 
		(pMsgCmdRsp->Action.RspGetNode.WestPortStp == FORWARDING)? 		"Forwarding": \
		(pMsgCmdRsp->Action.RspGetNode.WestPortStp == BLOCKING)? 		"Blocking":"Unkown");
	
	/* For EastPortInfoString */
	sprintf(EastPortInfoString,"P%02d-%s-%s", pMsgCmdRsp->Action.RspGetNode.EastPortNo,
		(pMsgCmdRsp->Action.RspGetNode.EastPortLink == LINK_UP)? 		"Up":"Down", 
		(pMsgCmdRsp->Action.RspGetNode.EastPortStp == FORWARDING)? 		"Forwarding": \
		(pMsgCmdRsp->Action.RspGetNode.EastPortStp == BLOCKING)? 		"Blocking":"Unkown");
	
	/* For software version */
	sprintf(SoftwareVersion, "%d.%d.%d.%d", pMsgCmdRsp->Action.RspGetNode.NodeVersion[0], pMsgCmdRsp->Action.RspGetNode.NodeVersion[1],
											pMsgCmdRsp->Action.RspGetNode.NodeVersion[2], pMsgCmdRsp->Action.RspGetNode.NodeVersion[3]);

	cli_printf(pCliEnv, "     %03d %-3s %02x:%02x:%02x:%02x:%02x:%02x   v%-9s   %-5s  %-19s  %-19s  %-6s\r\n", DstNodeIndex,
		(pMsgCmdRsp->Action.RspGetNode.NodeType == NODE_TYPE_MASTER)? "(M)": "   ",
		 pMsgCmdRsp->Action.RspGetNode.NodeMac[0],  pMsgCmdRsp->Action.RspGetNode.NodeMac[1],  pMsgCmdRsp->Action.RspGetNode.NodeMac[2], 
		  pMsgCmdRsp->Action.RspGetNode.NodeMac[3],  pMsgCmdRsp->Action.RspGetNode.NodeMac[4],  pMsgCmdRsp->Action.RspGetNode.NodeMac[5],  
		  SoftwareVersion, (pMsgCmdRsp->Action.RspGetNode.RingEnable == 0x01)? "Yes": "No",WestPortInfoString, EastPortInfoString,
		  (pMsgCmdRsp->Action.RspGetNode.RingState == RING_HEALTH)? "Health": "Fault");	

	return OK;
}

extern RLSTATUS cli_show_obring_topo_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "topo", mShowObring_Topo, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

#if OBRING_DEV
    /* TO DO: Add your handler code here */
    {
		sbyte WestPortInfoString[32];
		sbyte EastPortInfoString[32];	
		static unsigned char RequestId = 0;
		tRingConfigGlobal RingGlobalCfg;
		tRingConfigRec RingRecordCfg;
		tRingConfigRec *pRingConfig;
		tRingState *pRingState;		
		tRMsgCmd MsgCmdReq, MsgCmdRsp;
		ubyte SearchPort;
		ubyte RecIndex, SrcNodeIndex, DstNodeIndex, NodeIndexInc, PortIndex;
		int i, ret;
		extern unsigned char DevMac[];
		extern tRingInfo RingInfo;
		extern char FirmareVersion[];
		
		cli_printf(pCliEnv, "\r\n");
		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "    Error: Read OB-Ring configuration failed\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		if(RingGlobalCfg.ucGlobalEnable != 0x01) {
			cli_printf(pCliEnv, "    Warning: The OB-Ring protocol golbal disabled\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		cli_printf(pCliEnv, "    OB-Ring Number  : %d\r\n\r\n", RingGlobalCfg.ucRecordNum);

		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			for(RecIndex=0; RecIndex<RingGlobalCfg.ucRecordNum; RecIndex++) {
				if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
					cli_printf(pCliEnv, "Error: eeprom read failed\r\n\r\n");
					return STATUS_RCC_NO_ERROR;
				}
				pRingConfig = &(RingInfo.RingConfig[RecIndex]);
				pRingState = &(RingInfo.DevState[RecIndex]);

				memset(&WestPortInfoString[0], 0, 32);
				memset(&EastPortInfoString[0], 0, 32);
				
				/* CLI print */
				cli_printf(pCliEnv, "    Ring#%02d information ... \r\n", RecIndex);
				cli_printf(pCliEnv, "    =====================================================================================================\r\n");
				cli_printf(pCliEnv, "    Index    DeviceMacAddress   SoftVersion  RingEn  WestPortInformation  EastPortInformation  RingState \r\n");
				cli_printf(pCliEnv, "    =====================================================================================================\r\n");
				
				if(pRingState->PortState[INDEX_PRIMARY].NeighborValid == HAL_TRUE) {
					/* For EastPortInfoString */
					sprintf(EastPortInfoString,"P%02d-%s-%s", pRingConfig->ucPrimaryPort,
						(pRingState->PortState[INDEX_PRIMARY].LinkState == LINK_UP)? 	"Up":"Down", 
						(pRingState->PortState[INDEX_PRIMARY].StpState == FORWARDING)? 	"Forwarding": \
						(pRingState->PortState[INDEX_PRIMARY].StpState == BLOCKING)? 	"Blocking":"Unkown");
					/* For WestPortInfoString */
					sprintf(WestPortInfoString,"P%02d-%s-%s", pRingConfig->ucSecondaryPort,
						(pRingState->PortState[INDEX_SECONDARY].LinkState == LINK_UP)? 		"Up":"Down", 
						(pRingState->PortState[INDEX_SECONDARY].StpState == FORWARDING)? 	"Forwarding": \
						(pRingState->PortState[INDEX_SECONDARY].StpState == BLOCKING)? 		"Blocking":"Unkown");
				} else {
					if(pRingState->PortState[INDEX_SECONDARY].NeighborValid == HAL_TRUE) {
						/* For EastPortInfoString */
						sprintf(EastPortInfoString,"P%02d-%s-%s", pRingConfig->ucSecondaryPort,
							(pRingState->PortState[INDEX_SECONDARY].LinkState == LINK_UP)? 		"Up":"Down", 
							(pRingState->PortState[INDEX_SECONDARY].StpState == FORWARDING)? 	"Forwarding": \
							(pRingState->PortState[INDEX_SECONDARY].StpState == BLOCKING)? 		"Blocking":"Unkown");
						/* For WestPortInfoString */
						sprintf(WestPortInfoString,"P%02d-%s-%s", pRingConfig->ucPrimaryPort,
							(pRingState->PortState[INDEX_PRIMARY].LinkState == LINK_UP)? 		"Up":"Down", 
							(pRingState->PortState[INDEX_PRIMARY].StpState == FORWARDING)? 	"Forwarding": \
							(pRingState->PortState[INDEX_PRIMARY].StpState == BLOCKING)? 		"Blocking":"Unkown");
					} else {
						/* For EastPortInfoString */
						sprintf(EastPortInfoString,"P%02d-%s-%s", pRingConfig->ucPrimaryPort,
							(pRingState->PortState[INDEX_PRIMARY].LinkState == LINK_UP)? 	"Up":"Down", 
							(pRingState->PortState[INDEX_PRIMARY].StpState == FORWARDING)? 	"Forwarding": \
							(pRingState->PortState[INDEX_PRIMARY].StpState == BLOCKING)? 	"Blocking":"Unkown");
						/* For WestPortInfoString */
						sprintf(WestPortInfoString,"P%02d-%s-%s", pRingConfig->ucSecondaryPort,
							(pRingState->PortState[INDEX_SECONDARY].LinkState == LINK_UP)? 		"Up":"Down", 
							(pRingState->PortState[INDEX_SECONDARY].StpState == FORWARDING)? 	"Forwarding": \
							(pRingState->PortState[INDEX_SECONDARY].StpState == BLOCKING)? 		"Blocking":"Unkown");
					}
				}

				cli_printf(pCliEnv, "     001 %-3s %02x:%02x:%02x:%02x:%02x:%02x   v%-9s   %-5s  %-19s  %-19s  %-6s\r\n", 
					(pRingState->NodeType == NODE_TYPE_MASTER)? "(M)": "   ",
					DevMac[0], DevMac[1], DevMac[2], DevMac[3], DevMac[4], DevMac[5],
					  FirmareVersion, (pRingConfig->ucEnable == 0x01)? "Yes": "No",WestPortInfoString, EastPortInfoString,
					  (pRingState->RingState == RING_HEALTH)? "Health": "Fault");	


				SrcNodeIndex = 1;
				for(i=0; i<2; i++) {
					if(pRingState->PortState[i].LinkState == LINK_DOWN) 
						continue;
					
					cli_printf(pCliEnv, "     %s -->\r\n", (i==INDEX_PRIMARY)? "P1" : "P2");
					
					if(i == INDEX_PRIMARY)
						SearchPort = RingRecordCfg.ucPrimaryPort;
					else
						SearchPort = RingRecordCfg.ucSecondaryPort;
					
					DstNodeIndex = SrcNodeIndex + 1;
				
					while(1) {
						memset(&MsgCmdReq, 0, sizeof(tRMsgCmd));
						MsgCmdReq.Code = CMD_GET_NODE_REQ;
						MsgCmdReq.Action.ReqGetNode.ReqestId = RequestId++;
						MsgCmdReq.Action.ReqGetNode.SrcNodeIndex = SrcNodeIndex;
						MsgCmdReq.Action.ReqGetNode.DstNodeIndex = DstNodeIndex;
						MsgCmdReq.Action.ReqGetNode.NodeIndexInc = 1;
						ret = obring_command_send(pCliEnv, RecIndex, SearchPort, &MsgCmdReq, &MsgCmdRsp);
						if(ret == CMD_RET_CODE_TIMEOUT) {
							cli_printf(pCliEnv, "     Timeout!\r\n");
							return STATUS_RCC_NO_ERROR;
						} else {
							if(MsgCmdRsp.Action.RspGetNode.LastNodeFlag == CMD_RET_LAST_NODE_CHAIN) {
								obring_command_getnode_response_handle(pCliEnv, &MsgCmdRsp, DstNodeIndex);
								SrcNodeIndex = DstNodeIndex;
								break;
							} else if (MsgCmdRsp.Action.RspGetNode.LastNodeFlag == CMD_RET_LAST_NODE_RING) {
								cli_printf(pCliEnv, "\r\n");
								return OK;
							} else {
								obring_command_getnode_response_handle(pCliEnv, &MsgCmdRsp, DstNodeIndex);
								DstNodeIndex++;
							}
						}
					}
				}				
			}
			cli_printf(pCliEnv, "\r\n");
		}
	}
#endif

    return status;
}


extern RLSTATUS cli_debug_obring_disable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "ring_index", mDebugObringDisable_Ring_index, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
	{
		static unsigned char RequestId = 0;
		tRingConfigGlobal RingGlobalCfg;
		tRingConfigRec RingRecordCfg;
		tRingConfigRec *pRingConfig;
		tRingState *pRingState;		
		tRMsgCmd MsgCmdReq, MsgCmdRsp;
		ubyte SearchPort;
		ubyte RecIndex, SrcNodeIndex, DstNodeIndex, NodeIndexInc, PortIndex;
		int i, ret;
		extern unsigned char DevMac[];
		extern tRingInfo RingInfo;

		cli_printf(pCliEnv, "\r\n");
		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "    Error: Read OB-Ring configuration failed\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		if(RingGlobalCfg.ucGlobalEnable != 0x01) {
			cli_printf(pCliEnv, "    Warning: The OB-Ring protocol golbal disabled\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			CONVERT_StrTo(pVal, &RecIndex, kDTuchar);
			if(RecIndex >= RingGlobalCfg.ucRecordNum) {
				cli_printf(pCliEnv, "    Error: no configuration for ring index %d\r\n\r\n", RecIndex);
				return STATUS_RCC_NO_ERROR;	
			}
			
			if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "    Error: eeprom read failed\r\n\r\n");
				return STATUS_RCC_NO_ERROR;
			}
			
			pRingState = &(RingInfo.DevState[RecIndex]);

			cli_printf(pCliEnv, "    (Ring#%d) Disable all nodes ring protocol ...\r\n", RecIndex);

			if(pRingState->RingState == RING_HEALTH) {
				cli_printf(pCliEnv, "    Error: OB-Ring is health, please disconnect the ring port\r\n\r\n");
				return STATUS_RCC_NO_ERROR;
			}
			
			
			cli_printf(pCliEnv, "    ====================================\r\n");
			cli_printf(pCliEnv, "    Index  DeviceMacAddress     Result  \r\n");
			cli_printf(pCliEnv, "    ====================================\r\n");

			if(obring_disable(RecIndex) == 0)
				cli_printf(pCliEnv, "     001   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DevMac[0], DevMac[1], DevMac[2], DevMac[3], DevMac[4], DevMac[5], "Success");
			else
				cli_printf(pCliEnv, "     001   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DevMac[0], DevMac[1], DevMac[2], DevMac[3], DevMac[4], DevMac[5], "Failed");

			SrcNodeIndex = 1;
			for(i=0; i<2; i++) {
				if(pRingState->PortState[i].LinkState == LINK_DOWN) 
					continue;
				
				cli_printf(pCliEnv, "     %s -->\r\n", (i==INDEX_PRIMARY)? "P1" : "P2");
				
				if(i == INDEX_PRIMARY)
					SearchPort = RingRecordCfg.ucPrimaryPort;
				else
					SearchPort = RingRecordCfg.ucSecondaryPort;
				
				DstNodeIndex = SrcNodeIndex + 1;
			
				while(1) {
					memset(&MsgCmdReq, 0, sizeof(tRMsgCmd));
					memset(&MsgCmdRsp, 0, sizeof(tRMsgCmd));
					MsgCmdReq.Code = CMD_RING_DISABLE_REQ;
					MsgCmdReq.Action.ReqRingDisable.ReqestId = RequestId++;
					MsgCmdReq.Action.ReqRingDisable.SrcNodeIndex = SrcNodeIndex;
					MsgCmdReq.Action.ReqRingDisable.DstNodeIndex = DstNodeIndex;
					MsgCmdReq.Action.ReqRingDisable.NodeIndexInc = 1;
					ret = obring_command_send(pCliEnv, RecIndex, SearchPort, &MsgCmdReq, &MsgCmdRsp);
					if(ret == CMD_RET_CODE_TIMEOUT) {
						cli_printf(pCliEnv, "     Timeout!\r\n");
						return STATUS_RCC_NO_ERROR;
					} else {
						if(MsgCmdRsp.Action.RspRingDisable.LastNodeFlag == CMD_RET_LAST_NODE_CHAIN) {
							cli_printf(pCliEnv, "     %03d   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DstNodeIndex, 
										MsgCmdRsp.Action.RspRingDisable.NodeMac[0], MsgCmdRsp.Action.RspRingDisable.NodeMac[1], 
										MsgCmdRsp.Action.RspRingDisable.NodeMac[2], MsgCmdRsp.Action.RspRingDisable.NodeMac[3], 
										MsgCmdRsp.Action.RspRingDisable.NodeMac[4], MsgCmdRsp.Action.RspRingDisable.NodeMac[5], 
										(MsgCmdRsp.Action.RspRingDisable.RetCode == 0)? "Success":"Failed");
							SrcNodeIndex = DstNodeIndex;
							break;
						} else if (MsgCmdRsp.Action.RspRingDisable.LastNodeFlag == CMD_RET_LAST_NODE_RING) {
							cli_printf(pCliEnv, "\r\n");
							return OK;
						} else {
							cli_printf(pCliEnv, "     %03d   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DstNodeIndex, 
										MsgCmdRsp.Action.RspRingDisable.NodeMac[0], MsgCmdRsp.Action.RspRingDisable.NodeMac[1], 
										MsgCmdRsp.Action.RspRingDisable.NodeMac[2], MsgCmdRsp.Action.RspRingDisable.NodeMac[3], 
										MsgCmdRsp.Action.RspRingDisable.NodeMac[4], MsgCmdRsp.Action.RspRingDisable.NodeMac[5], 
										(MsgCmdRsp.Action.RspRingDisable.RetCode == 0)? "Success":"Failed");
							DstNodeIndex++;
						}
					}
				}
			}			
			cli_printf(pCliEnv, "\r\n");
		}

	}

    return status;
}



extern RLSTATUS cli_debug_obring_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "ring_index", mDebugObringEnable_Ring_index, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
	{
		static unsigned char RequestId = 0;
		tRingConfigGlobal RingGlobalCfg;
		tRingConfigRec RingRecordCfg;
		tRingConfigRec *pRingConfig;
		tRingState *pRingState;		
		tRMsgCmd MsgCmdReq, MsgCmdRsp;
		ubyte SearchPort;
		ubyte RecIndex, SrcNodeIndex, DstNodeIndex, NodeIndexInc, PortIndex;
		int i, ret;
		extern unsigned char DevMac[];
		extern tRingInfo RingInfo;

		cli_printf(pCliEnv, "\r\n");
		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "    Error: Read OB-Ring configuration failed\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		if(RingGlobalCfg.ucGlobalEnable != 0x01) {
			cli_printf(pCliEnv, "    Warning: The OB-Ring protocol golbal disabled\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			CONVERT_StrTo(pVal, &RecIndex, kDTuchar);
			if(RecIndex >= RingGlobalCfg.ucRecordNum) {
				cli_printf(pCliEnv, "    Error: no configuration for ring index %d\r\n\r\n", RecIndex);
				return STATUS_RCC_NO_ERROR;	
			}
			
			if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "    Error: eeprom read failed\r\n\r\n");
				return STATUS_RCC_NO_ERROR;
			}
			
			pRingState = &(RingInfo.DevState[RecIndex]);

			cli_printf(pCliEnv, "    (Ring#%d) Enable all nodes ring protocol ...\r\n", RecIndex);
			cli_printf(pCliEnv, "    ====================================\r\n");
			cli_printf(pCliEnv, "    Index  DeviceMacAddress     Result  \r\n");
			cli_printf(pCliEnv, "    ====================================\r\n");

			if(obring_enable(RecIndex) == 0)
				cli_printf(pCliEnv, "     001   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DevMac[0], DevMac[1], DevMac[2], DevMac[3], DevMac[4], DevMac[5], "Success");
			else
				cli_printf(pCliEnv, "     001   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DevMac[0], DevMac[1], DevMac[2], DevMac[3], DevMac[4], DevMac[5], "Failed");

			SrcNodeIndex = 1;
			for(i=0; i<2; i++) {
				if(pRingState->PortState[i].LinkState == LINK_DOWN) 
					continue;
				
				cli_printf(pCliEnv, "     %s -->\r\n", (i==INDEX_PRIMARY)? "P1" : "P2");
				
				if(i == INDEX_PRIMARY)
					SearchPort = RingRecordCfg.ucPrimaryPort;
				else
					SearchPort = RingRecordCfg.ucSecondaryPort;
				
				DstNodeIndex = SrcNodeIndex + 1;
			
				while(1) {
					memset(&MsgCmdReq, 0, sizeof(tRMsgCmd));
					memset(&MsgCmdRsp, 0, sizeof(tRMsgCmd));
					MsgCmdReq.Code = CMD_RING_ENABLE_REQ;
					MsgCmdReq.Action.ReqRingEnable.ReqestId = RequestId++;
					MsgCmdReq.Action.ReqRingEnable.SrcNodeIndex = SrcNodeIndex;
					MsgCmdReq.Action.ReqRingEnable.DstNodeIndex = DstNodeIndex;
					MsgCmdReq.Action.ReqRingEnable.NodeIndexInc = 1;
					ret = obring_command_send(pCliEnv, RecIndex, SearchPort, &MsgCmdReq, &MsgCmdRsp);
					if(ret == CMD_RET_CODE_TIMEOUT) {
						cli_printf(pCliEnv, "     Timeout!\r\n");
						return STATUS_RCC_NO_ERROR;
					} else {
						if(MsgCmdRsp.Action.RspRingEnable.LastNodeFlag == CMD_RET_LAST_NODE_CHAIN) {
							cli_printf(pCliEnv, "     %03d   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DstNodeIndex, 
										MsgCmdRsp.Action.RspRingEnable.NodeMac[0], MsgCmdRsp.Action.RspRingEnable.NodeMac[1], 
										MsgCmdRsp.Action.RspRingEnable.NodeMac[2], MsgCmdRsp.Action.RspRingEnable.NodeMac[3], 
										MsgCmdRsp.Action.RspRingEnable.NodeMac[4], MsgCmdRsp.Action.RspRingEnable.NodeMac[5], 
										(MsgCmdRsp.Action.RspRingEnable.RetCode == 0)? "Success":"Failed");
							SrcNodeIndex = DstNodeIndex;
							break;
						} else if (MsgCmdRsp.Action.RspRingEnable.LastNodeFlag == CMD_RET_LAST_NODE_RING) {
							cli_printf(pCliEnv, "\r\n");
							return OK;
						} else {
							cli_printf(pCliEnv, "     %03d   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DstNodeIndex, 
										MsgCmdRsp.Action.RspRingEnable.NodeMac[0], MsgCmdRsp.Action.RspRingEnable.NodeMac[1], 
										MsgCmdRsp.Action.RspRingEnable.NodeMac[2], MsgCmdRsp.Action.RspRingEnable.NodeMac[3], 
										MsgCmdRsp.Action.RspRingEnable.NodeMac[4], MsgCmdRsp.Action.RspRingEnable.NodeMac[5], 
										(MsgCmdRsp.Action.RspRingEnable.RetCode == 0)? "Success":"Failed");
							DstNodeIndex++;
						}
					}
				}
			}			
			cli_printf(pCliEnv, "\r\n");
		}
	}

    return status;
}

extern RLSTATUS cli_debug_obring_reboot_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "ring_index", mDebugObringReboot_Ring_index, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
	{
		static unsigned char RequestId = 0;
		tRingConfigGlobal RingGlobalCfg;
		tRingConfigRec RingRecordCfg;
		tRingConfigRec *pRingConfig;
		tRingState *pRingState;		
		tRMsgCmd MsgCmdReq, MsgCmdRsp;
		ubyte SearchPort;
		ubyte RecIndex, SrcNodeIndex, DstNodeIndex, NodeIndexInc, PortIndex;
		int i, ret;
		extern unsigned char DevMac[];
		extern tRingInfo RingInfo;

		cli_printf(pCliEnv, "\r\n");
		if(conf_get_ring_global(&RingGlobalCfg) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "    Error: Read OB-Ring configuration failed\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		if(RingGlobalCfg.ucGlobalEnable != 0x01) {
			cli_printf(pCliEnv, "    Warning: The OB-Ring protocol golbal disabled\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		}

		if((RingGlobalCfg.ucRecordNum > 0) && (RingGlobalCfg.ucRecordNum <= MAX_RING_NUM)) {
			CONVERT_StrTo(pVal, &RecIndex, kDTuchar);
			if(RecIndex >= RingGlobalCfg.ucRecordNum) {
				cli_printf(pCliEnv, "    Error: no configuration for ring index %d\r\n\r\n", RecIndex);
				return STATUS_RCC_NO_ERROR;	
			}
			
			if(conf_get_ring_record(RecIndex, &RingRecordCfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "    Error: eeprom read failed\r\n\r\n");
				return STATUS_RCC_NO_ERROR;
			}
			
			pRingState = &(RingInfo.DevState[RecIndex]);

			cli_printf(pCliEnv, "    (Ring#%d) Reboot all nodes ...\r\n", RecIndex);
			cli_printf(pCliEnv, "    ====================================\r\n");
			cli_printf(pCliEnv, "    Index  DeviceMacAddress     Result  \r\n");
			cli_printf(pCliEnv, "    ====================================\r\n");

			if(obring_reboot(RecIndex) == 0)
				cli_printf(pCliEnv, "     001   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DevMac[0], DevMac[1], DevMac[2], DevMac[3], DevMac[4], DevMac[5], "Rebooting");
			else
				cli_printf(pCliEnv, "     001   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DevMac[0], DevMac[1], DevMac[2], DevMac[3], DevMac[4], DevMac[5], "Failed");

			SrcNodeIndex = 1;
			for(i=0; i<2; i++) {
				if(pRingState->PortState[i].LinkState == LINK_DOWN) 
					continue;
				
				cli_printf(pCliEnv, "     %s -->\r\n", (i==INDEX_PRIMARY)? "P1" : "P2");
				
				if(i == INDEX_PRIMARY)
					SearchPort = RingRecordCfg.ucPrimaryPort;
				else
					SearchPort = RingRecordCfg.ucSecondaryPort;
				
				DstNodeIndex = SrcNodeIndex + 1;
			
				while(1) {
					memset(&MsgCmdReq, 0, sizeof(tRMsgCmd));
					memset(&MsgCmdRsp, 0, sizeof(tRMsgCmd));
					MsgCmdReq.Code = CMD_RING_REBOOT_REQ;
					MsgCmdReq.Action.ReqRingReboot.ReqestId = RequestId++;
					MsgCmdReq.Action.ReqRingReboot.SrcNodeIndex = SrcNodeIndex;
					MsgCmdReq.Action.ReqRingReboot.DstNodeIndex = DstNodeIndex;
					MsgCmdReq.Action.ReqRingReboot.NodeIndexInc = 1;
					ret = obring_command_send(pCliEnv, RecIndex, SearchPort, &MsgCmdReq, &MsgCmdRsp);
					if(ret == CMD_RET_CODE_TIMEOUT) {
						cli_printf(pCliEnv, "     Timeout!\r\n");
						return STATUS_RCC_NO_ERROR;
					} else {
						if(MsgCmdRsp.Action.RspRingReboot.LastNodeFlag == CMD_RET_LAST_NODE_CHAIN) {
							cli_printf(pCliEnv, "     %03d   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DstNodeIndex, 
										MsgCmdRsp.Action.RspRingReboot.NodeMac[0], MsgCmdRsp.Action.RspRingReboot.NodeMac[1], 
										MsgCmdRsp.Action.RspRingReboot.NodeMac[2], MsgCmdRsp.Action.RspRingReboot.NodeMac[3], 
										MsgCmdRsp.Action.RspRingReboot.NodeMac[4], MsgCmdRsp.Action.RspRingReboot.NodeMac[5], 
										(MsgCmdRsp.Action.RspRingReboot.RetCode == 0)? "Rebooting":"Failed");
							SrcNodeIndex = DstNodeIndex;
							break;
						} else if (MsgCmdRsp.Action.RspRingReboot.LastNodeFlag == CMD_RET_LAST_NODE_RING) {
							cli_printf(pCliEnv, "\r\n");
							return OK;
						} else {
							cli_printf(pCliEnv, "     %03d   %02x:%02x:%02x:%02x:%02x:%02x    %s\r\n", DstNodeIndex, 
										MsgCmdRsp.Action.RspRingReboot.NodeMac[0], MsgCmdRsp.Action.RspRingReboot.NodeMac[1], 
										MsgCmdRsp.Action.RspRingReboot.NodeMac[2], MsgCmdRsp.Action.RspRingReboot.NodeMac[3], 
										MsgCmdRsp.Action.RspRingReboot.NodeMac[4], MsgCmdRsp.Action.RspRingReboot.NodeMac[5], 
										(MsgCmdRsp.Action.RspRingReboot.RetCode == 0)? "Rebooting":"Failed");
							DstNodeIndex++;
						}
					}
				}
			}			
			cli_printf(pCliEnv, "\r\n");
		}
	}

    return status;
}

