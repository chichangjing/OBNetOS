
/*************************************************************
 * Filename     : cli_sys.c
 * Description  : API for CLI
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#include "mconfig.h"

#include "svn_revision.h"
/* Standard includes. */
#include "ctype.h"
#include "string.h"

/* LwIP includes */
#include "lwip/netif.h"
#include "lwip/stats.h"

/* BSP include */
#include "timer.h"
#include "soft_i2c.h"
#include "misc_drv.h"
#if ROBO_SWITCH
#include "robo_drv.h"
#elif MARVELL_SWITCH
#include "msApi.h"
#endif

#include "svn_revision.h"

/* Other includes */
#include "cli_sys.h"
#include "cli_util.h"
#include "conf_comm.h"
#include "conf_map.h"
#include "conf_sys.h"

#if MODULE_RING
#include "ob_ring.h"
#endif

#include "hal_swif_error.h"
#include "hal_swif_port.h"
#include "hal_swif_mac.h"
#include "hal_swif_aggregation.h"
#include "hal_swif_qos.h"

#if MARVELL_SWITCH
extern GT_QD_DEV *dev;
#endif

extern dev_base_info_t	DeviceBaseInfo;

RLSTATUS cli_exec_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
	RLSTATUS    status = OK;
	char 		pass[kRCC_MAX_PASSWORD_LEN + 1];
	char 		rpass[kRCC_MAX_PASSWORD_LEN + 1];

	if ( pCliEnv->UserLevel >= ENUM_ACCESS_ENABLE ) {
		return OK;
	}

	memset( pass, 0 , kRCC_MAX_PASSWORD_LEN + 1 );

	RCC_EnableFeature(pCliEnv, kRCC_FLAG_ECHO);  
	RCC_EXT_WriteStr(pCliEnv, kRCC_PASSWORD_PROMPT);

	RCC_EXT_LocalEcho(pCliEnv, FALSE);
	RCC_EXT_ReadCmd(pCliEnv,   FALSE);
	RCC_EXT_LocalEcho(pCliEnv, TRUE);

	if ( kRCC_MAX_PASSWORD_LEN <= MEDIT_GetLength(pCliEnv) ) {
		RCC_EXT_WriteStr(pCliEnv, "\r\n% Invalid enable password!\r\n");
	    return status;
	}

	MEDIT_CopyFromInput(pCliEnv, pass);

    if (0 != STRCMP(kRCC_DEFAULT_PASSWORD, pass)) {
		RCC_EXT_WriteStr(pCliEnv, "\r\n% Invalid enable password!\r\n");
        return FALSE;
    } else {
		RCC_EXT_WriteStr(pCliEnv, "\r\nCLI Access level enabled!");
	}

	pCliEnv->UserLevel = ENUM_ACCESS_ENABLE;

	STRCPY(MEDIT_PromptTail(pCliEnv), " # ");

	RCC_EXT_WriteStr( pCliEnv, "\r\n" );

    return status;
}


RLSTATUS cli_exec_exit_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;
	sbyte4		modeDepth  = MMISC_GetModeDepth(pCliEnv);
	
    /* get optional parameter */
    if (OK == RCC_DB_RetrieveParam((struct paramList*) pParams, "all", mExit_All, &pParamDescr ))
    {
        pVal = (sbyte*)(pParamDescr->pValue);
    }

    if (modeDepth != 0)
        status = (NULL == pVal) ? STATUS_RCC_EXIT : STATUS_RCC_EXIT_ALL;
    else
        status = STATUS_RCC_LOGOUT;
    
    return status;
}


RLSTATUS cli_exec_reset_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte   	temp[10];
    sbyte4		byteCount;
	
    while(1)
    {
        RCC_EXT_WriteStr(pCliEnv,"Are you sure you want to reset system(y/n) ? ");
        MEMSET(temp,0,sizeof(temp));
        RCC_EXT_Gets(pCliEnv, temp, 5, &byteCount, TRUE);
        if ((STRNCMP(temp,"y",STRLEN(temp))==OK) || (STRNCMP(temp,"Y",STRLEN(temp))==OK)){
#ifdef __FreeRTOS_OS__
			RCC_EXT_WriteStr(pCliEnv,"System will be reboot after 1 second ...\r\n\r\n");
			RebootDelayMs(1000);
			status = STATUS_RCC_LOGOUT;
            //NVIC_SystemReset();
#endif
            break;
        }
        if ((STRNCMP(temp,"n",STRLEN(temp))==OK) || (STRNCMP(temp,"N",STRLEN(temp))==OK)) {
            break;
        }
    }
    return status;
}


RLSTATUS cli_show_task_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
	
#ifdef __FreeRTOS_OS__
	signed char *pcWriteBuffer = NULL;
	
	if(!(pcWriteBuffer = (signed char *)pvPortMalloc(1024))) {
		RCC_EXT_WriteStr( pCliEnv, "pvPortMalloc failed!\r\n");
		return STATUS_RCC_NO_ERROR;
	}
#if (configUSE_TRACE_FACILITY == 1)
	RCC_EXT_WriteStr( pCliEnv, "\r\n" );
	RCC_EXT_WriteStr( pCliEnv, "  ==============================================\r\n");
	RCC_EXT_WriteStr( pCliEnv, "  TaskName     Status  Priority  StackFree  TCB#\r\n");
	RCC_EXT_WriteStr( pCliEnv, "  ==============================================");
	vTaskList(pcWriteBuffer);
	RCC_EXT_WriteStr( pCliEnv, (char *)pcWriteBuffer);
	RCC_EXT_WriteStr( pCliEnv, "\r\n" );
#endif
	vPortFree(pcWriteBuffer);

#endif    
    return status;
}

RLSTATUS cli_show_task_runtime_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
	
#ifdef __FreeRTOS_OS__
	signed char *pcWriteBuffer = NULL;
	
	if(!(pcWriteBuffer = (signed char *)pvPortMalloc(1024))) {
		RCC_EXT_WriteStr( pCliEnv, "pvPortMalloc failed!\r\n");
		return STATUS_RCC_NO_ERROR;
	}

	#if ( configUSE_TRACE_FACILITY == 1 )
	RCC_EXT_WriteStr( pCliEnv, "\r\n");
	RCC_EXT_WriteStr( pCliEnv, "  ========================================\r\n");
	RCC_EXT_WriteStr( pCliEnv, "  TaskName        AbsoluteTime  Percentage \r\n");
	RCC_EXT_WriteStr( pCliEnv, "  ========================================");
	vTaskList(pcWriteBuffer);
	vTaskGetRunTimeStats(pcWriteBuffer);
	RCC_EXT_WriteStr( pCliEnv, (char *)pcWriteBuffer);
	RCC_EXT_WriteStr( pCliEnv, "\r\n");
	#endif
	
    vPortFree(pcWriteBuffer);
#endif    
    return status;
}

RLSTATUS cli_config_bootdelay_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get optional parameter */
    if (OK == RCC_DB_RetrieveParam(pParams, "seconds", mConfigBootdelay_Seconds, &pParamDescr ))
    {
        pVal = (sbyte*)(pParamDescr->pValue);
    }

    /* TO DO: Add your handler code here */
    {
		ubyte bootdelay;
		
		if(pVal == NULL) {
			if(conf_get_bootdelay(&bootdelay) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "Error: read eeprom failed!\r\n");
				return STATUS_RCC_NO_ERROR;
			}			
			if(((bootdelay == 0) || (bootdelay > 0)) && (bootdelay <= 5))
				cli_printf(pCliEnv, "Bootdelay is %d seconds\r\n", bootdelay);
			else
				cli_printf(pCliEnv, "Bootdelay is not initialized, set default 3 seconds\r\n");
			return status;
		}

		CONVERT_StrTo(pVal, &bootdelay, kDTuchar);

		if(conf_set_bootdelay(&bootdelay) != CONF_ERR_NONE) {
			cli_printf(pCliEnv, "Error: write eeprom failed!\r\n");
			return STATUS_RCC_NO_ERROR;
		}
		
	}
    return status;
}

RLSTATUS cli_config_ip_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;
    sbyte       *pVal3 = NULL;
    paramDescr  *pParamDescr3;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "ip", mConfigIp_Ip, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get optional parameter */
    if (OK == RCC_DB_RetrieveParam(pParams, "netmask", mConfigIp_Netmask, &pParamDescr2 ))
    {
        pVal2 = (sbyte*)(pParamDescr2->pValue);
    }

    /* get optional parameter */
    if (OK == RCC_DB_RetrieveParam(pParams, "gateway", mConfigIp_Gateway, &pParamDescr3 ))
    {
        pVal3 = (sbyte*)(pParamDescr3->pValue);
    }

    /* TO DO: Add your handler code here */
    {
    	ubyte4 ipaddr, netmask, gateway;
		ip_info_t ip_info;
		
        CONVERT_StrTo(pVal1, &ipaddr, kDTipaddress);
		
		if(pVal2 != NULL)
        	CONVERT_StrTo(pVal2, &netmask, kDTipaddress);
		else
			CONVERT_StrTo("255.255.255.0", &netmask, kDTipaddress);

		if(pVal3 != NULL)
        	CONVERT_StrTo(pVal3, &gateway, kDTipaddress);
		else
			gateway = cli_htonl((cli_ntohl(ipaddr) & 0xffffff00UL) | 0x1);

		memcpy(ip_info.ip, (u8 *)&ipaddr, 4);
		memcpy(ip_info.netmask, (u8 *)&netmask, 4);
		memcpy(ip_info.gateway, (u8 *)&gateway, 4);

		if(check_ipaddress((ubyte4)cli_htonl(ipaddr)) < 0) {
			RCC_EXT_WriteStr( pCliEnv, "Error: Invalid ip address\r\n");
			return FALSE;
		}

		if(check_netmask((ubyte4)cli_htonl(netmask)) < 0) {
			RCC_EXT_WriteStr( pCliEnv, "Error: Invalid netmask\r\n");
			return FALSE;
		}

		if(check_ip_mask((ubyte4)cli_htonl(ipaddr), (ubyte4)cli_htonl(netmask)) < 0) {
			RCC_EXT_WriteStr( pCliEnv, "Error: Relationship ip and netmask\r\n");
			return FALSE;
		}
		
		RCC_EXT_WriteStr( pCliEnv, "Network is configured to ...\r\n");

		cli_printf(pCliEnv, "   IP Address ........ : %d.%d.%d.%d\r\n", ip_info.ip[0], ip_info.ip[1], ip_info.ip[2], ip_info.ip[3]);
		cli_printf(pCliEnv, "   Subnet Mask ....... : %d.%d.%d.%d\r\n", ip_info.netmask[0], ip_info.netmask[1], ip_info.netmask[2], ip_info.netmask[3]);
		cli_printf(pCliEnv, "   Default Gateway ... : %d.%d.%d.%d\r\n", ip_info.gateway[0], ip_info.gateway[1], ip_info.gateway[2], ip_info.gateway[3]);

		conf_set_ip_info((ubyte *)&ip_info);
		
    }
		
    return status;
}

RLSTATUS cli_show_memory_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
	size_t		freesize;

	cli_printf(pCliEnv, "RTOS heap information :\r\n");
	cli_printf(pCliEnv, "    Total size ... %d bytes (%d Kbytes)\r\n", configTOTAL_HEAP_SIZE, configTOTAL_HEAP_SIZE/1024);
	freesize = xPortGetFreeHeapSize();
	cli_printf(pCliEnv, "     Free size ... %d bytes (%d Kbytes)\r\n", freesize, freesize/1024);
	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "LwIP heap information :\r\n");
	cli_printf(pCliEnv, "    Total size ... %d bytes (%d Kbytes)\r\n", lwip_stats.mem.avail, lwip_stats.mem.avail/1024);
	cli_printf(pCliEnv, "     Free size ... %d bytes (%d Kbytes)\r\n", lwip_stats.mem.avail-lwip_stats.mem.used, (lwip_stats.mem.avail-lwip_stats.mem.used)/1024);

    return status;
}

RLSTATUS cli_show_system_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
	int timeout;
	unsigned value;
#if 0
	cli_printf(pCliEnv, "Switch software reset\r\n");
	if(!(dev->fgtWriteMii(dev, 0x1b, 0x04, 0x8000))) {	// Switch software reset  
		cli_printf(pCliEnv, "ETH_WritePHYRegister failed\r\n");
		return STATUS_RCC_NO_ERROR;
	}

	TimerDelayMs(500);
	
	timeout=0;
	do {
		timeout++;	
		dev->fgtReadMii(dev, 0x1b, 0x04, &value);
		
	} while ((value & 0x8000) && (timeout < 0xFFFFF));
#endif
#if 0
	cli_printf(pCliEnv, "Mac flush all ... ");
	if(hal_swif_mac_flush_all() != 0)
		cli_printf(pCliEnv, "failed\r\n");
	cli_printf(pCliEnv, "done\r\n");
#endif

	//IO_Send(0x12);
    return status;
}


RLSTATUS cli_show_register_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;
    sbyte       *pVal3 = NULL;
    paramDescr  *pParamDescr3;
    u8          regData[8] = {0};
    sbyte       page = 0;
    sbyte       address = 0;
    sbyte       lenth = 0;
    

    /* get optional parameter */
    if (OK == RCC_DB_RetrieveParam(pParams, "page", mShowRegister_Page, &pParamDescr1 ))
    {
        pVal1 = (sbyte*)(pParamDescr1->pValue);
    }

    /* get optional parameter */
    if (OK == RCC_DB_RetrieveParam(pParams, "address", mShowRegister_Address, &pParamDescr2 ))
    {
        pVal2 = (sbyte*)(pParamDescr2->pValue);
    }

    /* get optional parameter */
    if (OK == RCC_DB_RetrieveParam(pParams, "lenth", mShowRegister_Lenth, &pParamDescr3 ))
    {
        pVal3 = (sbyte*)(pParamDescr3->pValue);
    }

    /* TO DO: Add your handler code here */
#if ROBO_SWITCH    
    CONVERT_StrTo(pVal1, &page, kDTuchar);
    CONVERT_StrTo(pVal2, &address, kDTuchar);
    CONVERT_StrTo(pVal3, &lenth, kDTuchar);
    
    if(robo_read(page, address, (u8 *)regData, lenth) != 0){
        cli_printf(pCliEnv, "    Error: Read register data failed\r\n\r\n");
		return STATUS_RCC_NO_ERROR;
    }

    cli_printf(pCliEnv, "Page %02Xh Address %02Xh : ", page, address);
    for(s8 i=lenth-1; i>=0; i--){
        cli_printf(pCliEnv, "%02X", regData[i]);
    }
    cli_printf(pCliEnv, "h\r\n");
#endif

    return status;
}

RLSTATUS cli_show_version_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
	ubyte		version[CONF_SYS_VERSION_SIZE+1];
	extern char FirmareVersion[];
	
	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "    Bootloader Version : ");
	if(eeprom_read(NVRAM_LOADER_VERSION, version, MAX_LOADER_VERSION_SIZE) == I2C_SUCCESS) {
		if(isprint(version[0]) == 0) {
			cli_printf(pCliEnv, "not initialized\r\n");
		} else
			cli_printf(pCliEnv, "%s\r\n", version);
	} else
		cli_printf(pCliEnv, "i2c read failed\r\n");

#if RELEASE_TRUNK_VERSION
	cli_printf(pCliEnv, "    Firmware Version   : %s r%-5d\r\n", FirmareVersion, SVN_REVISION);
#else
	cli_printf(pCliEnv, "    Firmware Revision  : r%-5d\r\n", SVN_REVISION);
#endif
	cli_printf(pCliEnv, "    Build Time         : %s\r\n", BUILD_TIME);

	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "    System Version     : ");
	memset(version, 0, CONF_SYS_VERSION_SIZE+1);
	if(eeprom_read(NVRAM_SYS_VERSION, version, CONF_SYS_VERSION_SIZE) == I2C_SUCCESS) {
		if(isprint(version[0]) == 0) {
			cli_printf(pCliEnv, "not initialized\r\n");
		} else
			cli_printf(pCliEnv, "%s\r\n", version);
	} else
		cli_printf(pCliEnv, "i2c read failed\r\n");


	cli_printf(pCliEnv, "    Hardware Version   : ");
	memset(version, 0, CONF_SYS_VERSION_SIZE+1);
	if(eeprom_read(NVRAM_HARDWARE_VERSION, version, CONF_HARDWARE_VERSION_SIZE) == I2C_SUCCESS) {
		if(isprint(version[0]) == 0) {
			cli_printf(pCliEnv, "not initialized\r\n");
		} else
			cli_printf(pCliEnv, "%s\r\n", version);
	} else
		cli_printf(pCliEnv, "i2c read failed\r\n");

	cli_printf(pCliEnv, "    Software Version   : ");
	memset(version, 0, CONF_SYS_VERSION_SIZE+1);
	if(eeprom_read(NVRAM_SOFT_VERSION, version, CONF_SOFT_VERSION_SIZE) == I2C_SUCCESS) {
		if(isprint(version[0]) == 0) {
			cli_printf(pCliEnv, "not initialized\r\n");
		} else
			cli_printf(pCliEnv, "%s\r\n", version);
	} else
		cli_printf(pCliEnv, "i2c read failed\r\n");
#if BOARD_GV3S_HONUE_QM
#include "halsw_i2c.h"
#include "common.h"
#include "fpga_api.h"
    
	cli_printf(pCliEnv, "    FPGA Code ID       : ");
	if(SWI2cSequentialRead(I2CDEV_0, FPGAADD1, FPGA_CODE_ID_ADDR, (unsigned char *)&version, 4) == 0) {
        cli_printf(pCliEnv, "%X.", (version[0]>>4)&0x0F);
        cli_printf(pCliEnv, "%X", (version[1]>>4)&0x0F);
        cli_printf(pCliEnv, "%X.", (version[0])&0x0F);
        cli_printf(pCliEnv, "%X", (version[2]>>4)&0x0F);
        cli_printf(pCliEnv, "%X", (version[1])&0x0F);
        cli_printf(pCliEnv, "%X.", (version[2])&0x0F);
        cli_printf(pCliEnv, "%X", (version[3]>>4)&0x0F);
        cli_printf(pCliEnv, "%X\r\n", (version[3])&0x0F);
	}else{
        cli_printf(pCliEnv, "i2c read failed\r\n");
    }	
#endif
	
	cli_printf(pCliEnv, "\r\n");
    return status;
}

RLSTATUS cli_show_port_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;

	hal_swif_port_show_config(pCliEnv);
	
    return status;
}

RLSTATUS cli_show_port_status_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;

	hal_swif_port_show_status(pCliEnv);
	hal_swif_aggr_show_status(pCliEnv);
    return status;
}

RLSTATUS cli_show_port_neigbor_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;

	hal_swif_port_show_neigbor(pCliEnv);
	
    return status;
}

RLSTATUS cli_show_port_traffic_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;

	hal_swif_port_show_traffic(pCliEnv);
	
    return status;
}

RLSTATUS cli_show_device_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
	struct netif *pNetIf = netif_find("st0");
#if MODULE_RING	
	DevNode_t	*pRingDevNode = Ring_GetNode();
#endif

	cli_printf(pCliEnv, "\r\nBoard interrupt priority list ...\r\n");
	cli_printf(pCliEnv, "    Priority for Timer3       : %d\r\n", NVIC_GetPriority(TIM3_IRQn));
	cli_printf(pCliEnv, "    Priority for Ethernet     : %d\r\n", NVIC_GetPriority(ETH_IRQn));
	cli_printf(pCliEnv, "    Priority for SysTick      : %d\r\n", NVIC_GetPriority(SysTick_IRQn));
	
#if BOARD_GE2C400U
	cli_printf(pCliEnv, "    Priority for Usart2 (485) : %d\r\n", NVIC_GetPriority(USART2_IRQn));
	cli_printf(pCliEnv, "    Priority for Usart3 (485) : %d\r\n", NVIC_GetPriority(USART3_IRQn));
	cli_printf(pCliEnv, "    Priority for Usart6 (com) : %d\r\n", NVIC_GetPriority(USART6_IRQn));
#elif BOARD_GE1040PU
	cli_printf(pCliEnv, "    Priority for Usart1 (485) : %d\r\n", NVIC_GetPriority(USART1_IRQn));
	cli_printf(pCliEnv, "    Priority for Usart3 (485) : %d\r\n", NVIC_GetPriority(USART3_IRQn));
	cli_printf(pCliEnv, "    Priority for Usart6 (com) : %d\r\n", NVIC_GetPriority(USART6_IRQn));
#elif BOARD_GE22103MA
	cli_printf(pCliEnv, "    Priority for Usart1 (485) : %d\r\n", NVIC_GetPriority(USART1_IRQn));
	cli_printf(pCliEnv, "    Priority for Uart5  (485) : %d\r\n", NVIC_GetPriority(UART5_IRQn));
	cli_printf(pCliEnv, "    Priority for Usart3 (com) : %d\r\n", NVIC_GetPriority(USART3_IRQn));
#elif BOARD_GE20023MA
	cli_printf(pCliEnv, "    Priority for Usart1 (485) : %d\r\n", NVIC_GetPriority(USART1_IRQn));
	cli_printf(pCliEnv, "    Priority for Usart2 (485) : %d\r\n", NVIC_GetPriority(USART2_IRQn));
	cli_printf(pCliEnv, "    Priority for Uart5  (com) : %d\r\n", NVIC_GetPriority(UART5_IRQn));
#elif BOARD_GV3S_HONUE_QM
	cli_printf(pCliEnv, "    Priority for Usart1 (485) : %d\r\n", NVIC_GetPriority(USART1_IRQn));
	cli_printf(pCliEnv, "    Priority for Uart5  (485) : %d\r\n", NVIC_GetPriority(UART5_IRQn));
	cli_printf(pCliEnv, "    Priority for Usart3 (com) : %d\r\n", NVIC_GetPriority(USART3_IRQn));
#endif

	cli_printf(pCliEnv, "\r\nDevice ethernet information :\r\n");
	cli_printf(pCliEnv, "    MAC Address         : %02x-%02x-%02x-%02x-%02x-%02x\r\n", 
						pNetIf->hwaddr[0], pNetIf->hwaddr[1], pNetIf->hwaddr[2], pNetIf->hwaddr[3], pNetIf->hwaddr[4], pNetIf->hwaddr[5]);

	if(pNetIf != NULL) {
		cli_printf(pCliEnv, "    IP Address          : %s\r\n", ip_ntoa(&(pNetIf->ip_addr)));
		cli_printf(pCliEnv, "    Subnet mask         : %s\r\n", ip_ntoa(&(pNetIf->netmask)));
		cli_printf(pCliEnv, "    Default Gateway     : %s\r\n", ip_ntoa(&(pNetIf->gw)));
	} else {
		cli_printf(pCliEnv, "Error: netif_find failed\r\n");
	}

#if MODULE_RING	
	cli_printf(pCliEnv, "\r\nDevice node information :\r\n");
	if(pRingDevNode->RingEnable) {
	cli_printf(pCliEnv, "    OB-Ring protocol    : Enable\r\n");
	cli_printf(pCliEnv, "    Device node type    : %s\r\n", (pRingDevNode->NodeType == SINGLE_NODE)? "Single Node" : \
															(pRingDevNode->NodeType == RING_MASTER_NODE)? "Ring Master Node" : \
															(pRingDevNode->NodeType == RING_TRANSFER_NODE)? "Ring Transfer Node" : \
															(pRingDevNode->NodeType == CHAIN_MASTER_NODE)? "Chain Master Node" : \
															(pRingDevNode->NodeType == CHAIN_TRANSFER_NODE)? "Chain Transfer Node" : \
															(pRingDevNode->NodeType == CHAIN_SLAVER_NODE)? "Chain Slaver Node" : "Unkown Type");

	cli_printf(pCliEnv, "    Fast aging state    : %s\r\n", (pRingDevNode->FlushState == FLUSH_IDLE)? "Flush IDLE" : \
															(pRingDevNode->FlushState == RING_FLUSH_START)? "Ring Flushing" : \
															(pRingDevNode->FlushState == RING_FLUSH_COMPLETE)? "Ring Flush Done" : \
															(pRingDevNode->FlushState == CHAIN_FLUSH_START)? "Chain Flushing" : \
															(pRingDevNode->FlushState == CHAIN_FLUSH_COMPLETE)? "Chain Flush Done" : "Unkown State");

	cli_printf(pCliEnv, "\r\nRing port information   :\r\n");
	cli_printf(pCliEnv, "    PortA Link/STP      : %s, %s\r\n",	(pRingDevNode->PortInfo[RING_LOGIC_PORT_A].LinkState == LINK_UP)? "Link Up" : "Link Down", \
																(pRingDevNode->PortInfo[RING_LOGIC_PORT_A].StpState == FORWARDING)? "Forwarding" : "Blocking");
	cli_printf(pCliEnv, "    PortB Link/STP      : %s, %s\r\n",	(pRingDevNode->PortInfo[RING_LOGIC_PORT_B].LinkState == LINK_UP)? "Link Up" : "Link Down", \
																(pRingDevNode->PortInfo[RING_LOGIC_PORT_B].StpState == FORWARDING)? "Forwarding" : "Blocking");
	} else
	cli_printf(pCliEnv, "    OB-Ring protocol    : Disable\r\n");
#endif

	cli_printf(pCliEnv, "\r\n");

    return status;	
}


RLSTATUS cli_show_mac_addr_table_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
	RLSTATUS    status = OK;
	
#if SWITCH_CHIP_88E6095
	GT_STATUS stat;
	GT_ATU_ENTRY MacEntry;
	char s[20];
	GT_U8 i, port;
	int index;
	GT_U8 *logic_port_list, *list;
	int list_length;
	GT_U32 hport_vec;
	GT_U32 trunkId;
	
	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  =========================================================\r\n");
	cli_printf(pCliEnv, "   No.  Mac Address          EntryState           PortList \r\n");
	cli_printf(pCliEnv, "  =========================================================\r\n");

	memset(&MacEntry,0,sizeof(GT_ATU_ENTRY));

	index = 0;
	
	while(1) {

		if((stat = gfdbGetAtuEntryNext(dev,&MacEntry)) != GT_OK)
			break;
		
		if((MacEntry.macAddr.arEther[0] == 0xff) && (MacEntry.macAddr.arEther[1] == 0xff) && (MacEntry.macAddr.arEther[2] == 0xff) && 
			(MacEntry.macAddr.arEther[3] == 0xff) && (MacEntry.macAddr.arEther[4] == 0xff) && (MacEntry.macAddr.arEther[5] == 0xff))
			break;	

		if((MacEntry.macAddr.arEther[0] & 0x1) == 1) {	// multicast address
			switch(MacEntry.entryState.mcEntryState) {
				case GT_MC_STATIC:
				sprintf(s,"%s","Mc-Static");
				break;

				case GT_MC_PRIO_MGM_STATIC:
				sprintf(s,"%s","Mc-Static-PrioMGM");
				break;
				
				default:
				sprintf(s,"%s","Mc-Unkown");
				break;
			}	
		} else {										// unicast address
			switch(MacEntry.entryState.ucEntryState) {
				case GT_UC_STATIC:
				sprintf(s,"%s","Uc-Static");
				break;

				case GT_UC_TO_CPU_STATIC:
				sprintf(s,"%s","Uc-Static-ToCPU");
				break;
				
				case GT_UC_DYNAMIC:
				sprintf(s,"%s","Uc-Dynamic");
				break;
				
				default:
				sprintf(s,"%s","Uc-Unkown");
				break;
			}
		}

		logic_port_list = (GT_U8 *)OS_SPECIFIC_MALLOC(dev->maxPorts);
		hport_vec = dev->validPortVec & MacEntry.portVec;
		list_length=0;
		for(i=0, list=logic_port_list; i<dev->maxPorts; i++) {
			if((MacEntry.portVec >> i) & 0x1) {
				if(i == dev->cpuPortNum)
					*list++ = 0;
				else
					*list++ = hal_swif_hport_2_lport(i);
				list_length++;
			}
		}
		
		cli_printf(pCliEnv, "   %03d  (%02x-%02x-%02x-%02x-%02x-%02x)  %-17s    ",
				index,
				MacEntry.macAddr.arEther[0],
				MacEntry.macAddr.arEther[1],
				MacEntry.macAddr.arEther[2],
				MacEntry.macAddr.arEther[3],
				MacEntry.macAddr.arEther[4],
				MacEntry.macAddr.arEther[5], s);

		
		for(i=0, list=logic_port_list; i<list_length; i++) {
			if(i==list_length-1) {
				if(*list == 0) {
					cli_printf(pCliEnv, "CpuPort");
				} else {
					cli_printf(pCliEnv, "P%d", *list);
				}
			} else {
				if(*list == 0) {
					cli_printf(pCliEnv, "CpuPort,");
					list++;
				} else {
					cli_printf(pCliEnv, "P%d,", *list++);
				}
			}
		}
		cli_printf(pCliEnv, "\r\n");

		OS_SPECIFIC_FREE(logic_port_list);

		index++;
	}

	cli_printf(pCliEnv, "\r\n");
#else
	hal_swif_mac_unicast_show(pCliEnv);
#endif
	return status;	
}

RLSTATUS cli_show_priority_queue_map_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
	RLSTATUS    status = OK;

#if SWITCH_CHIP_88E6095
#if (BOARD_TYPE == BT_GE22103MA)
    uint8 priority,i,j;
    uint8 queue_num;
    
    cli_printf(pCliEnv, "\r\n");
    cli_printf(pCliEnv, " The corresponding relationship between CoS(IEEE 802.1p) prior and queue\r\n");
    
    cli_printf(pCliEnv, " ");
    for(j=0; j<=3; j++){
        cli_printf(pCliEnv, "===================");
    }
    cli_printf(pCliEnv, "\r\n");
    for(j=0; j<=3; j++){
        cli_printf(pCliEnv, " CoS Prior - Queue ");
    }
    cli_printf(pCliEnv, "\r\n");
    cli_printf(pCliEnv, " ");
    for(j=0; j<=3; j++){
        cli_printf(pCliEnv, "===================");
    }
    cli_printf(pCliEnv, "\r\n");
    for(i=0; i<=1; i++){
        for(j=0; j<=3; j++){
            priority = i+j*2; 
            if(hal_swif_get_qos_cos_to_queue_map(priority, &queue_num) != HAL_SWIF_SUCCESS){
                cli_printf(pCliEnv, "Warning: IEEETag priority to queue map read failed\r\n");
                return STATUS_RCC_NO_ERROR;
            }
            cli_printf(pCliEnv, "        %2d - %d     ",priority, queue_num);  
        }
        cli_printf(pCliEnv, "\r\n");
    }
    cli_printf(pCliEnv, "\r\n");
    
    cli_printf(pCliEnv, " The corresponding relationship between ToS(IP DiffServ/DSCP) prior and queue\r\n");
    cli_printf(pCliEnv, " ");
    for(j=0; j<=3; j++){
        cli_printf(pCliEnv, "===================");
    }
    cli_printf(pCliEnv, "\r\n");
    for(j=0; j<=3; j++){
        cli_printf(pCliEnv, " ToS Prior - Queue ");
    }
    cli_printf(pCliEnv, "\r\n");
    cli_printf(pCliEnv, " ");
    for(j=0; j<=3; j++){
        cli_printf(pCliEnv, "===================");
    }
    cli_printf(pCliEnv, "\r\n");
    for(i=0; i<=15; i++){
        for(j=0; j<=3; j++){
            priority = i+j*16; 
            if(hal_swif_get_qos_tos_to_queue_map(priority, &queue_num) != HAL_SWIF_SUCCESS){
                cli_printf(pCliEnv, "Warning: IPTosOrDSCP to queue map read failed\r\n");
                return STATUS_RCC_NO_ERROR;
            }
            cli_printf(pCliEnv, "        %2d - %d     ",priority, queue_num);  
        }
        cli_printf(pCliEnv, "\r\n");
    }
    cli_printf(pCliEnv, "\r\n");
    cli_printf(pCliEnv, "\t\tQueue Prior: 0-low 1-normal 2-high 3-highest\r\n");
	cli_printf(pCliEnv, "\r\n");
	
#endif	
#endif

	return status;	
}

RLSTATUS cli_show_qos_set(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
	RLSTATUS    status = OK;

#if SWITCH_CHIP_88E6095
#if (BOARD_TYPE == BT_GE22103MA)
    HAL_QOS_MODE qos_mode;
    GT_LPORT lport;
    HAL_BOOL enable;
    uint8 priority_level;
    GT_EGRESS_MODE egress_mode;
    
    cli_printf(pCliEnv, "\r\n");
    cli_printf(pCliEnv, "  Schedule Mode : ");
    if(hal_swif_get_qos_schedule_mode(&qos_mode) != HAL_SWIF_SUCCESS){
        cli_printf(pCliEnv, "Warning: Schedule Mode read failed\r\n");
        return STATUS_RCC_NO_ERROR;
    }
    
    if(qos_mode == QOS_MODE_WWR){
        cli_printf(pCliEnv, "WRR\r\n");
    }else{
        cli_printf(pCliEnv, "SP\r\n");
    }
	cli_printf(pCliEnv, "\r\n");
    cli_printf(pCliEnv, "  ========================================================\r\n");
    cli_printf(pCliEnv, "  Port\tCoS\tToS\tCoS/ToS\tDefaultPrior\tEgressMode\r\n");
    cli_printf(pCliEnv, "  ========================================================\r\n");
    
    for(lport=1; lport<=DeviceBaseInfo.PortNum; lport++) {
        cli_printf(pCliEnv, "  %02d\t", lport);
        
        if(hal_swif_get_qos_priority_cos_enable(lport, &enable) != HAL_SWIF_SUCCESS){     
            cli_printf(pCliEnv, "Warning: The user priority mapping state read failed\r\n");
            return STATUS_RCC_NO_ERROR;
        }
        cli_printf(pCliEnv, "%-3s\t", enable == HAL_TRUE ? "Yes":"No");
        
        if(hal_swif_get_qos_priority_tos_enable(lport, &enable) != HAL_SWIF_SUCCESS){     
            cli_printf(pCliEnv, "Warning: The IP priority mapping state read failed\r\n");
            return STATUS_RCC_NO_ERROR;
        }
        cli_printf(pCliEnv, "%-3s\t", enable == HAL_TRUE ? "Yes":"No");
        
        if(hal_swif_get_qos_priority_both_enable(lport, &enable) != HAL_SWIF_SUCCESS){     
            cli_printf(pCliEnv, "Warning: CoS/ToS priority mapping rule read failed\r\n");
            return STATUS_RCC_NO_ERROR;
        }
        cli_printf(pCliEnv, "%-3s\t", enable == HAL_TRUE ? "CoS":"ToS");
        
        if(hal_swif_get_qos_port_default_prority_level(lport, &priority_level) != HAL_SWIF_SUCCESS){     
            cli_printf(pCliEnv, "Warning: The default traffic class read failed\r\n");
            return STATUS_RCC_NO_ERROR;
        }
        cli_printf(pCliEnv, "%-6s\t\t", priority_level == 0 ? "low":             
                                        priority_level == 2 ? "normal":
                                        priority_level == 4 ? "high":
                                        priority_level == 6 ? "highest":"unkwon");
        
        if(hal_swif_get_port_egress_mode(lport, &egress_mode) != HAL_SWIF_SUCCESS){     
            cli_printf(pCliEnv, "Warning: The egress mode read failed\r\n");
            return STATUS_RCC_NO_ERROR;
        }
        cli_printf(pCliEnv, "%-8s\t\r\n", egress_mode == GT_UNMODIFY_EGRESS ? "UNMODIFY":
                                          egress_mode == GT_UNTAGGED_EGRESS ? "UNTAGGED":
                                          egress_mode == GT_TAGGED_EGRESS ? "TAGGED":
                                          egress_mode == GT_ADD_TAG ? "ADD_TAG":"unkwon");
    }
    
    cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  SP  : Strict  Priority\r\n");
	cli_printf(pCliEnv, "  WRR : Weight Round Robin 8:4:2:1\r\n");
    
    cli_printf(pCliEnv, "  UNMODIFY : Frames are transmited unmodified\r\n");
    cli_printf(pCliEnv, "  UNTAGGED : All frames are transmited untagged\r\n");
    cli_printf(pCliEnv, "  TAGGED   : All frames are transmited tagged\r\n");
    cli_printf(pCliEnv, "  ADD_TAG  : Always add a tag. (or double tag)\r\n");
	cli_printf(pCliEnv, "\r\n");
    
#endif	
#endif

	return status;	
}

RLSTATUS cli_show_vlan_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
	RLSTATUS    status = OK;

#if SWITCH_CHIP_88E6095
#if (BOARD_TYPE == BT_GE22103MA)
	GT_U16 vid;
	GT_8 tag[MAX_PORT_NUM][3];
	GT_STATUS ret;
	GT_VTU_ENTRY vtuEntry;
	GT_LPORT port;
	GT_U8 hport;
	int i;
	
	gtMemSet(&vtuEntry,0,sizeof(GT_VTU_ENTRY));
	vtuEntry.vid = 0xfff;
	ret = gvtuGetEntryFirst(dev,&vtuEntry);
	if(ret != GT_OK) {
		if(ret == GT_NO_SUCH) {
			cli_printf(pCliEnv, "Warning: Vlan entry does not exist\r\n");
		} else {
			cli_printf(pCliEnv, "gvtuGetEntryFirst return failed, ret=%d\r\n", ret);
		}
		return STATUS_RCC_NO_ERROR;
	}

	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  =========");
    for(i=0; i<DeviceBaseInfo.PortNum; i++) {
        cli_printf(pCliEnv, "=====");
    }
    cli_printf(pCliEnv, "\r\n");
    
    cli_printf(pCliEnv, "  VlanID    ");
    for(i=0; i<DeviceBaseInfo.PortNum; i++) {
        cli_printf(pCliEnv, "P%d   ",i+1);
    }
	cli_printf(pCliEnv, "\r\n");

	cli_printf(pCliEnv, "  =========");
    for(i=0; i<DeviceBaseInfo.PortNum; i++) {
        cli_printf(pCliEnv, "=====");
    }
    cli_printf(pCliEnv, "\r\n");

	for(i=0; i<DeviceBaseInfo.PortNum; i++) {
		hport = hal_swif_lport_2_hport(i+1);
		
		if(vtuEntry.vtuData.memberTagP[hport] == MEMBER_EGRESS_UNMODIFIED)
			strcpy(tag[i], "UM");
		else if(vtuEntry.vtuData.memberTagP[hport] == NOT_A_MEMBER)
			strcpy(tag[i], "NA");
		else if(vtuEntry.vtuData.memberTagP[hport] == MEMBER_EGRESS_UNTAGGED)
			strcpy(tag[i], "-T");
		else if(vtuEntry.vtuData.memberTagP[hport] == MEMBER_EGRESS_TAGGED)
			strcpy(tag[i], "+T");
		else
			strcpy(tag[i], "??");
		
	}

    cli_printf(pCliEnv, "    %-4d    ",vtuEntry.vid);
    for(i=0; i<DeviceBaseInfo.PortNum; i++) {
        cli_printf(pCliEnv, "%-2s   ",tag[i]);
    }
	cli_printf(pCliEnv, "\r\n");
	
	while(1) {
		if((ret = gvtuGetEntryNext(dev,&vtuEntry)) != GT_OK)
			break;
		
		for(i=0; i<DeviceBaseInfo.PortNum; i++) {
			hport = hal_swif_lport_2_hport(i+1);
			
			if(vtuEntry.vtuData.memberTagP[hport] == MEMBER_EGRESS_UNMODIFIED)
				strcpy(tag[i], "UM");
			else if(vtuEntry.vtuData.memberTagP[hport] == NOT_A_MEMBER)
				strcpy(tag[i], "NA");
			else if(vtuEntry.vtuData.memberTagP[hport] == MEMBER_EGRESS_UNTAGGED)
				strcpy(tag[i], "-T");
			else if(vtuEntry.vtuData.memberTagP[hport] == MEMBER_EGRESS_TAGGED)
				strcpy(tag[i], "+T");
			else
				strcpy(tag[i], "??");
			
		}
        
        cli_printf(pCliEnv, "    %-4d    ",vtuEntry.vid);
        for(i=0; i<DeviceBaseInfo.PortNum; i++) {
            cli_printf(pCliEnv, "%-2s   ",tag[i]);
        }
        cli_printf(pCliEnv, "\r\n");
        
    }

	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  UM : Member egress unmodified\r\n");
	cli_printf(pCliEnv, "  NA : Not a member\r\n");
	cli_printf(pCliEnv, "  -T : Member egress untagged\r\n");
	cli_printf(pCliEnv, "  +T : Member egress tagged\r\n");
	cli_printf(pCliEnv, "\r\n");
	
#endif	
#endif

	return status;	
}

#if 0
void cli_display_counter3(cli_env *pCliEnv, GT_STATS_COUNTER_SET3 *statsCounter)
{
	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "  InGoodOctetsLo : %-8i  ", statsCounter->InGoodOctetsLo);
	cli_printf(pCliEnv, "  InGoodOctetsHi : %-8i \r\n", statsCounter->InGoodOctetsHi);
	cli_printf(pCliEnv, "  InBadOctets    : %-8i  ", statsCounter->InBadOctets);
	cli_printf(pCliEnv, "  OutFCSErr      : %-8i \r\n", statsCounter->OutFCSErr);
	cli_printf(pCliEnv, "  InUnicasts     : %-8i  ", statsCounter->InUnicasts);
	cli_printf(pCliEnv, "  Deferred       : %-8i \r\n", statsCounter->Deferred);
	cli_printf(pCliEnv, "  InBroadcasts   : %-8i  ", statsCounter->InBroadcasts);
	cli_printf(pCliEnv, "  InMulticasts   : %-8i \r\n", statsCounter->InMulticasts);
	cli_printf(pCliEnv, "  64Octets       : %-8i  ", statsCounter->Octets64);
	cli_printf(pCliEnv, "  127Octets      : %-8i \r\n", statsCounter->Octets127);
	cli_printf(pCliEnv, "  255Octets      : %-8i  ", statsCounter->Octets255);
	cli_printf(pCliEnv, "  511Octets      : %-8i \r\n", statsCounter->Octets511);
	cli_printf(pCliEnv, "  1023Octets     : %-8i  ", statsCounter->Octets1023);
	cli_printf(pCliEnv, "  MaxOctets      : %-8i \r\n", statsCounter->OctetsMax);
	cli_printf(pCliEnv, "  OutOctetsLo    : %-8i  ", statsCounter->OutOctetsLo);
	cli_printf(pCliEnv, "  OutOctetsHi    : %-8i \r\n", statsCounter->OutOctetsHi);
	cli_printf(pCliEnv, "  OutUnicasts    : %-8i  ", statsCounter->OutUnicasts);
	cli_printf(pCliEnv, "  Excessive      : %-8i \r\n", statsCounter->Excessive);
	cli_printf(pCliEnv, "  OutMulticasts  : %-8i  ", statsCounter->OutMulticasts);
	cli_printf(pCliEnv, "  OutBroadcasts  : %-8i \r\n", statsCounter->OutBroadcasts);
	cli_printf(pCliEnv, "  Single         : %-8i  ", statsCounter->Single);
	cli_printf(pCliEnv, "  OutPause       : %-8i \r\n", statsCounter->OutPause);
	cli_printf(pCliEnv, "  InPause        : %-8i  ", statsCounter->InPause);
	cli_printf(pCliEnv, "  Multiple       : %-8i \r\n", statsCounter->Multiple);
	cli_printf(pCliEnv, "  Undersize      : %-8i  ", statsCounter->Undersize);
	cli_printf(pCliEnv, "  Fragments      : %-8i \r\n", statsCounter->Fragments);
	cli_printf(pCliEnv, "  Oversize       : %-8i  ", statsCounter->Oversize);
	cli_printf(pCliEnv, "  Jabber         : %-8i \r\n", statsCounter->Jabber);
	cli_printf(pCliEnv, "  InMACRcvErr    : %-8i  ", statsCounter->InMACRcvErr);
	cli_printf(pCliEnv, "  InFCSErr       : %-8i \r\n", statsCounter->InFCSErr);
	cli_printf(pCliEnv, "  Collisions     : %-8i  ", statsCounter->Collisions);
	cli_printf(pCliEnv, "  Late           : %-8i \r\n", statsCounter->Late);
	cli_printf(pCliEnv, "\r\n");
}
#endif

RLSTATUS cli_show_counters_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
	RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "port", mShowCounters_Port, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
    {
    	ubyte lport;
	
    	CONVERT_StrTo(pVal, &lport, kDTuchar);
		if(lport > MAX_PORT_NUM) {
			cli_printf(pCliEnv, "\r\nError: Invalid port number\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		} else {
			hal_swif_port_show_counters(pCliEnv, lport);
		}	
    }

	return status;	
}

RLSTATUS cli_clear_counters_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "port", mClearCounters_Port, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
    {
    	ubyte lport;
	
    	CONVERT_StrTo(pVal, &lport, kDTuchar);
		if(lport > MAX_PORT_NUM) {
			cli_printf(pCliEnv, "\r\nError: Invalid port number\r\n\r\n");
			return STATUS_RCC_NO_ERROR;
		} else {
			if(hal_swif_port_clear_counters_flag(lport) != HAL_SWIF_SUCCESS) {
				cli_printf(pCliEnv, "\r\nError: Clear MIB counters failed\r\n\r\n");
				return STATUS_RCC_NO_ERROR;
			}
		}
    }

	return status;	
}

RLSTATUS cli_clear_mac_addr_table_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
   RLSTATUS    status = OK;

    /* TO DO: Add your handler code here */

    if(hal_swif_mac_flush_all() != HAL_SWIF_SUCCESS) {
       cli_printf(pCliEnv, "\r\nError: Clear MAC address table failed\r\n\r\n");
       return STATUS_RCC_NO_ERROR;
    }

   return status;	
}

RLSTATUS cli_config_traffic_statistic_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "port-list", mConfigTraffic_statistic_Port_list, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */

    {
    	sbyte	*portid;
    	sbyte	*portbuf = NULL;
		ubyte	LogicPortNo;
		ubyte	portlist[8];
		int num,i;
		
		portbuf = (sbyte *)RC_MALLOC(strlen(pVal)+1);
		if (portbuf == NULL) {
			cli_printf(pCliEnv, "\r\nError: Allocate memory failed!\r\n\r\n");
			return STATUS_RCC_NO_ERROR;			
    	}

		memset(portbuf, 0, strlen(pVal)+1);
		memset(portlist, 0, 8);
		strcpy(portbuf, pVal);
    	portid = strtok(portbuf, ",");

		num=0;
		while (portid != NULL) {
			CONVERT_StrTo(portid, &LogicPortNo, kDTuchar);
			portlist[num] = LogicPortNo;
			num++;
			if(num > 8) {
				cli_printf(pCliEnv, "\r\nError: port total number is out of range(1-8)\r\n\r\n");			 
             	goto end;
        	 }
        	 portid = strtok(NULL, ",");		
		}
		
		cli_printf(pCliEnv, "\r\nPort traffic statistic list: ");
		for(i=0; i<num;i++) {
			cli_printf(pCliEnv, "P%d ", portlist[i]);
		}
		cli_printf(pCliEnv, "\r\n\r\n");
end:
    if (portbuf)
    	RC_FREE(portbuf);		
    }
	
    return status;
}

