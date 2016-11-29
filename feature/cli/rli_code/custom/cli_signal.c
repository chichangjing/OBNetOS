/*************************************************************
 * Filename     : cli_signal.c
 * Description  : API for CLI
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/

/* Other includes */
#include "cli_sys.h"
#include "cli_util.h"
#include "conf_comm.h"
#include "conf_signal.h"

/* lwip include */
#include "lwip/ip_addr.h"

RLSTATUS
cli_show_signal_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       ip[16] = {0};
    ubyte2      port = 0;
    ubyte       ChanNum;
    tKinAlarmConfig KinAlarmCfg;
    
    cli_printf(pCliEnv, "\r\n  Signal startup global configuration : \r\n");
    cli_printf(pCliEnv, "  ===============================================\r\n");
    cli_printf(pCliEnv, "            Name          :        Value         \r\n");
    cli_printf(pCliEnv, "  ===============================================\r\n");
    
    if(signal_cfg_fetch((tKinAlarmConfig *)&KinAlarmCfg) != CONF_ERR_NONE)
    {
        cli_printf(pCliEnv, "Error: Signal fetch signal configration failed!\r\n");
        return STATUS_RCC_NO_ERROR;
    }
    
    if(KinAlarmCfg.FeatureEnable != ENABLE)
        cli_printf(pCliEnv, "    Kin Enable            : no \r\n");
    else
        cli_printf(pCliEnv, "    Kin Enable            : yes \r\n");

    cli_printf(pCliEnv, "    Sample Cycle          : %d ms \r\n", KinAlarmCfg.SampleCycle);

    if(KinAlarmCfg.RemoveJitterEnable != ENABLE)
        cli_printf(pCliEnv, "    Remove Jitter Enable  : no \r\n");
    else
        cli_printf(pCliEnv, "    Remove Jitter Enable  : yes \r\n");

    cli_printf(pCliEnv, "    Jitter Probe Time     : %d ms \r\n", KinAlarmCfg.JitterProbeTime);

    if(KinAlarmCfg.WorkMode == WORKMODE_UDP)
        cli_printf(pCliEnv, "    Work Mode             : udp \r\n");
    else
        cli_printf(pCliEnv, "    Work Mode             : unkonw \r\n");
    
    CONVERT_ToStr(&KinAlarmCfg.ModeConfig.UdpParam.ServerIP, ip, kDTipaddress);
    cli_printf(pCliEnv, "         Server ip        : %s \r\n",ip);
    port = *(ubyte2 *)KinAlarmCfg.ModeConfig.UdpParam.ServerPort;
    cli_printf(pCliEnv, "         Server port      : %d \r\n",ntohs(port));
    
    cli_printf(pCliEnv, "\r\n  Signal Kin channel table : \r\n");    
    cli_printf(pCliEnv, "  ===============================================\r\n");
    cli_printf(pCliEnv, "     Channel           Enable          AlarmType   \r\n");
    cli_printf(pCliEnv, "  ===============================================\r\n");

    ChanNum = KinAlarmCfg.ChanConfig.ChanNum;
    if(ChanNum > MAX_KSIG_IN_ALARM_CHANNEL_NUM || ChanNum == 0){
        cli_printf(pCliEnv, "Error: invalid channel number\r\n");
		return STATUS_RCC_NO_ERROR;	
    }

    for(ubyte i=1; i<=ChanNum; i++){
        cli_printf(pCliEnv, "\t%02d", i);
        cli_printf(pCliEnv, "\t\t");
        if(CHAN_GET_EN(KinAlarmCfg.ChanConfig.Data[i-1]) == ENABLE)
            cli_printf(pCliEnv, "%-3s","yes");
        else
            cli_printf(pCliEnv, "%-3s","no");
            
        cli_printf(pCliEnv, "\t\t");
        if(CHAN_GET_ALARM_TYPE(KinAlarmCfg.ChanConfig.Data[i-1]) == ALARMTYPE_CLOSE){
            cli_printf(pCliEnv, "%-8s","close");
        }
        else if(CHAN_GET_ALARM_TYPE(KinAlarmCfg.ChanConfig.Data[i-1]) == ALARMTYPE_OPEN){
            cli_printf(pCliEnv, "%-8s","open");
        }else if(CHAN_GET_ALARM_TYPE(KinAlarmCfg.ChanConfig.Data[i-1]) == ALARMTYPE_DOWN){
            cli_printf(pCliEnv, "%-8s","down");
        }else{
            cli_printf(pCliEnv, "%-8s","--");
        }
        cli_printf(pCliEnv, "\t\r\n");
    }
    cli_printf(pCliEnv, "\r\n");
    
    return status;
}

RLSTATUS
cli_jitter_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;
    ubyte       time = 0;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "enable/disable", mConfigSignalJitter_EnableQfsdisable, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "time", mConfigSignalJitter_Time, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* TO DO: Add your handler code here */  
    if(pVal1 == NULL)
        return status;
    
    if(COMPARE_Strings(pVal1, "enable")) {
        if(set_jitter_en(ENABLE) != CONF_ERR_NONE)
        {
            cli_printf(pCliEnv, "Error: Signal disable remove jitter set failed!\r\n");
            return STATUS_RCC_NO_ERROR;
        }
    } else if(COMPARE_Strings(pVal1, "disable")) {
        if(set_jitter_en(DISABLE) != CONF_ERR_NONE)
        {
            cli_printf(pCliEnv, "Error: Signal enable remove jitter set failed!\r\n");
            return STATUS_RCC_NO_ERROR;
        }
    }

    if(pVal2 == NULL)
        return status;
    
    CONVERT_StrTo(pVal2, &time, kDTuinteger);
    if(set_jitter_probe(time) != CONF_ERR_NONE)
    {
        cli_printf(pCliEnv, "Error: Signal remove jitter time set failed!\r\n");
        return STATUS_RCC_NO_ERROR;
    }

    return status;
}

RLSTATUS
cli_work_mode_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;
    sbyte       *pVal3 = NULL;
    paramDescr  *pParamDescr3;
    ubyte4      ipaddr;
    ubyte2      port;
    
    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "mode", mConfigSignalWorkmode_Mode, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "ip", mConfigSignalWorkmode_Ip, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "port", mConfigSignalWorkmode_Port, &pParamDescr3 );
    if ( OK != status )
    {
		return(status);
    } else pVal3 = (sbyte*)(pParamDescr3->pValue);

    /* TO DO: Add your handler code here */  
    if(pVal1 == NULL)
        return status;
    
    if(COMPARE_Strings(pVal1, "udp")) {
        if(set_work_mode(WORKMODE_UDP) != CONF_ERR_NONE)
        {
            cli_printf(pCliEnv, "Error: Signal work mode set failed!\r\n");
            return STATUS_RCC_NO_ERROR;
        }
    } else if(COMPARE_Strings(pVal1, "tcp")) {
        if(set_work_mode(WORKMODE_TCP) != CONF_ERR_NONE)
        {
            cli_printf(pCliEnv, "Error: Signal work mode set failed!\r\n");
            return STATUS_RCC_NO_ERROR;
        }
    }

    if(pVal2 == NULL)
        return status;
    
    CONVERT_StrTo(pVal2, &ipaddr, kDTipaddress);
    if(set_server_ip(ipaddr) != CONF_ERR_NONE)
    {
        cli_printf(pCliEnv, "Error: Signal server ip set failed!\r\n");
        return STATUS_RCC_NO_ERROR;
    }
    
    if(pVal3 == NULL)
        return status;
    
    CONVERT_StrTo(pVal3, &port, kDTushort);
    port = htons(port);
    if(set_server_port(port) != CONF_ERR_NONE)
    {
        cli_printf(pCliEnv, "Error: Signal server port set failed!\r\n");
        return STATUS_RCC_NO_ERROR;
    }
    
    return status;
}

RLSTATUS
cli_chan_num_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;
    ubyte       num = 0;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "channum", mConfigSignalChannum_Channum, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
    CONVERT_StrTo(pVal, &num, kDTuchar);
    if(set_chan_num(num) != CONF_ERR_NONE)
    {
        cli_printf(pCliEnv, "Error: Channel number set failed!\r\n");
        return STATUS_RCC_NO_ERROR;
    }

    return status;
}

RLSTATUS
cli_chan_param_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;
    sbyte       *pVal3 = NULL;
    paramDescr  *pParamDescr3;
    ubyte       chanid = 0;
    ubyte       type   = 0;
    
    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "chanid", mConfigSignalChanparam_Chanid, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "enable/disable", mConfigSignalChanparam_EnableQfsdisable, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "alarmtype", mConfigSignalChanparam_Alarmtype, &pParamDescr3 );
    if ( OK != status )
    {
		return(status);
    } else pVal3 = (sbyte*)(pParamDescr3->pValue);
    
    /* TO DO: Add your handler code here */
    CONVERT_StrTo(pVal1, &chanid, kDTuchar);
    if(COMPARE_Strings(pVal2, "enable")) {
        if(set_chan_en(chanid, ENABLE) != CONF_ERR_NONE)
        {
            cli_printf(pCliEnv, "Error: Channel enable set failed!\r\n");
            return STATUS_RCC_NO_ERROR;
        }
    } else if(COMPARE_Strings(pVal2, "disable")) {
        if(set_chan_en(chanid, DISABLE) != CONF_ERR_NONE)
        {
            cli_printf(pCliEnv, "Error: Channel disable set failed!\r\n");
            return STATUS_RCC_NO_ERROR;
        }
    }
    
    if(COMPARE_Strings(pVal3, "CLOSE")) {
        if(set_chan_type(chanid, ALARMTYPE_CLOSE) != CONF_ERR_NONE)
        {
            cli_printf(pCliEnv, "Error: Channel alarm type set failed!\r\n");
            return STATUS_RCC_NO_ERROR;
        }
    } else if(COMPARE_Strings(pVal3, "OPEN")) {
        if(set_chan_type(chanid, ALARMTYPE_OPEN) != CONF_ERR_NONE)
        {
            cli_printf(pCliEnv, "Error: Channel alarm type set failed!\r\n");
            return STATUS_RCC_NO_ERROR;
        }
    }else if(COMPARE_Strings(pVal3, "DOWN")){
        if(set_chan_type(chanid, ALARMTYPE_DOWN) != CONF_ERR_NONE)
        {
            cli_printf(pCliEnv, "Error: Channel alarm type set failed!\r\n");
            return STATUS_RCC_NO_ERROR;
        }
    }

    return status;
}

RLSTATUS
cli_signal_enable_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;

    /* TO DO: Add your handler code here */
    if(set_signal_en(ENABLE) != CONF_ERR_NONE)
    {
        cli_printf(pCliEnv, "Error: Signal disable set failed!\r\n");
        return STATUS_RCC_NO_ERROR;
    }

    return status;
}

RLSTATUS
cli_signal_no_enable_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;

    /* TO DO: Add your handler code here */
    if(set_signal_en(DISABLE) != CONF_ERR_NONE)
    {
        cli_printf(pCliEnv, "Error: Signal enable set failed!\r\n");
        return STATUS_RCC_NO_ERROR;
    }

    return status;
}

RLSTATUS
cli_samp_cycle_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;
    ubyte       time = 0;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "time", mConfigSignalSampcycle_Time, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);

    /* TO DO: Add your handler code here */
    CONVERT_StrTo(pVal, &time, kDTuchar);
    if(set_sampcycle_time(time) != CONF_ERR_NONE)
    {
        cli_printf(pCliEnv, "Error: Signal enable set failed!\r\n");
        return STATUS_RCC_NO_ERROR;
    }

    return status;
}

RLSTATUS
cli_no_support_display_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    
    cli_printf(pCliEnv, "Error: Signal cli set non-support! \r\n");
    
    return  STATUS_RCC_NO_ERROR;
}