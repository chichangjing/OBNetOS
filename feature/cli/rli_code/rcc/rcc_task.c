/*  
 *  rcc_task.c
 *
 *  This is a part of the RapidControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
 *  All rights reserved.
 *
 *  RapidControl, RapidControl for Web, RapidControl Backplane,
 *  RapidControl for Applets, MIBway, RapidControl Protocol, and
 *  RapidMark are trademarks of Rapid Logic, Inc.  All rights reserved.
 *
 */

#include "rc.h"
#include "rcc.h"
#include "rc_ignition.h"

#ifdef __RCC_ENABLED__

#ifdef RCC_LOG_HEADER
#include RCC_LOG_HEADER
#endif

#ifndef RCC_LOG_LOGIN_FN
#define RCC_LOG_LOGIN_FN(pLogin, pMsg)
#endif

#ifndef RCC_ERROR_HELP_FN
#define RCC_ERROR_HELP_FN RCC_HELP_ErrorHelp
#else
extern void RCC_ERROR_HELP_FN(cli_env *pCliEnv, cmdNode *pNode);
#endif

extern ErrorTable gErrorTable[];
extern sbyte4     gErrorTableCount;

/*-----------------------------------------------------------------------*/

extern void RCC_TASK_Cleanup(cli_env *pCliEnv)
{
    CliChannel *pChannel = MMISC_GetChannel(pCliEnv);

    if (NULL != pChannel)
    {
        pChannel->ThreadState = kThreadDead;
        pChannel->InUse       = FALSE;
    }

    RCC_TELNETD_DelSession(pCliEnv);

    /* custom clean up code */
#ifdef RCC_LOGOUT_FN
    RCC_LOGOUT_FN(pCliEnv);
#endif

    RCC_DB_DestroyEnvironment(pCliEnv);
}

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean TASK_Login(cli_env *pCliEnv)
{
    sbyte    pLogin[kRCC_MAX_LOGIN_LEN];
    sbyte    pPassword[kRCC_MAX_PASSWORD_LEN];
    sbyte4   iSecuRet      = 0;
    sbyte4   loginAttempts = 1;
    Boolean  status        = FALSE;

	if (kRCC_CONN_TELNET == MCONN_GetConnType(pCliEnv)) {
		extern void cli_telnet_welcome(cli_env *pCliEnv);
		cli_telnet_welcome(pCliEnv);
	} else {
		extern void cli_console_welcom(cli_env *pCliEnv);
		cli_console_welcom(pCliEnv);
	}
#ifdef __SKIP_LOGIN__
    Access   access;
    extern DTTypeInfo mAccessInfo;

    CONVERT_StrTypeTo(kRCC_DEFAULT_ACCESS, &access, &mAccessInfo);
    return TRUE;
#endif

	{
		Access   access;
    	extern DTTypeInfo mAccessInfo;		
      	ubyte disable;
		extern int conf_get_cli_login_switch(ubyte *);
        
		conf_get_cli_login_switch(&disable);
		if(disable == 0x01) {
			CONVERT_StrTypeTo(kRCC_DEFAULT_ACCESS, &access, &mAccessInfo);
			pCliEnv->UserLevel = ENUM_ACCESS_ENABLE;
			return TRUE;
		}	
	}

    MEMSET(pLogin,    0, kRCC_MAX_LOGIN_LEN);
    MEMSET(pPassword, 0, kRCC_MAX_PASSWORD_LEN);

    /* login loop */
    while (1)
    {
        iSecuRet = RCC_UTIL_SecuredAccess(pCliEnv, pLogin, pPassword);

        if (ERROR_RCC_RETRY == iSecuRet)
        {
            RCC_EXT_WriteStrLine(pCliEnv, "");
            RCC_EXT_WriteStrLine(pCliEnv, "");
            if (kRCC_MAX_LOGIN_ATTEMPTS <= loginAttempts++)
            {
                RCC_LOG_LOGIN_FN(pLogin, kRCC_MSG_LOGIN_FAILED);
                break;  
            }
        }

        else if (OK == iSecuRet)
        {
            RCC_EXT_WriteStrLine(pCliEnv, "");
            RCC_LOG_LOGIN_FN(pLogin, kRCC_MSG_LOGIN_OKAY);
            status = TRUE;
            break;
        }
    }

    RCC_EnableFeature(pCliEnv, kRCC_FLAG_ECHO);  
    
    return status;
}

/*-----------------------------------------------------------------------*/

extern void RCC_TASK_PrintPrompt(cli_env *pCliEnv)
{
    sbyte  *pPrompt  = MEDIT_Prompt(pCliEnv);
    sbyte  *pTail    = MEDIT_PromptTail(pCliEnv);
    ubyte4  memUsage = 0;
    sbyte   str[16];
    Boolean mem      = FALSE;

#ifdef __RCC_DEBUG_MEM_USAGE__
    mem      = TRUE;
#endif

#ifndef __OS_MALLOC_PROVIDED__
    memUsage = MemMgr_GetMemoryUsage();
#endif

    if (mem || debugFeature(kRCC_DEBUG_MEMORY))
    {
        sprintf(str, "{%d}", memUsage);
        RCC_EXT_WriteStr(pCliEnv, str);
    }

    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_RM_PROMPT))
        RCC_UTIL_RapidPrompt(pCliEnv, TRUE);

    if (RCC_NotEnabled(pCliEnv, kRCC_FLAG_RM_PROMPT))
    {
        RCC_EXT_WriteStr(pCliEnv, pPrompt);
        RCC_EXT_WriteStr(pCliEnv, pTail);
    }
    MEDIT_SetCursor(pCliEnv, 0);
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS 
TASK_TimeOut(cli_env *pCliEnv)
{
    sbyte4  timeOut = kTELNET_TIMEOUT_IN_MINUTES;
    sbyte4  length;
    sbyte  *pBuffer;

    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_WARNING)) {
        RCC_EXT_WriteStrLine(pCliEnv, kRCC_MSG_LOGOUT_NOW);
        return RCC_ERROR_THROW(ERROR_RCC_TIMEOUT);
    }

    length = RC_MAX( sizeof(kRCC_MSG_LOGOUT_IDLE),
                     sizeof(kRCC_MSG_LOGOUT_LEFT) );

    length += 18; /* wiggle room */

    if (NULL == (pBuffer = (RC_MALLOC(length))))
        return RCC_ERROR_THROW(ERROR_MEMMGR_NO_MEMORY);

    sprintf(pBuffer, kRCC_MSG_LOGOUT_IDLE, timeOut );
    RCC_EXT_WriteStr(pCliEnv, pBuffer);
    sprintf(pBuffer, kRCC_MSG_LOGOUT_LEFT, timeOut);
    RCC_EXT_WriteStr(pCliEnv, pBuffer);

    RCC_EnableFeature(pCliEnv, kRCC_FLAG_WARNING);

    RC_FREE(pBuffer);
	
	//return ERROR_RCC_TIMEOUT;
    return OK;
}

/*-----------------------------------------------------------------------*/

extern ubyte4
RCC_TASK_EvalError(RLSTATUS status, sbyte **ppMsg)
{
    ubyte4      flags;
    sbyte4      index;
    ErrorTable *pError = &gErrorTable[0];

    for (index = 0; index < gErrorTableCount; index++, pError++)
    {
        if (status != pError->status)
            continue;

        flags  = pError->flags;
        *ppMsg = pError->pErrorText;
        break;
    }

    return flags;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
TASK_PrintError(cli_env *pCliEnv, RLSTATUS status)
{
    sbyte   *pErrorMsg  = NULL;
    sbyte   *pErrorText = MMISC_GetErrorText(pCliEnv);               
    sbyte4   offset     = MMISC_GetErrorPos(pCliEnv);
    ubyte4   flags;
    Boolean  retry;
    Boolean  msg;

    flags  = RCC_TASK_EvalError(status, &pErrorMsg);
    msg    = TASK_NOMSG_K != (flags & TASK_NOMSG_K);
    retry  = TASK_RETRY_K == (flags & TASK_RETRY_K);
    retry &= RCC_IsEnabled(pCliEnv, kRCC_FLAG_RETRIES);

    if (retry)
        RCC_EXT_WriteStrLine(pCliEnv, "");
    else
        RCC_DB_ErrorLine(pCliEnv);

    RCC_EXT_WriteStr(pCliEnv, kRCC_MSG_ERROR_PREFIX);

    if (NULL == pErrorMsg)
        pErrorMsg = kMSG_ERROR_RCC_DEFAULT;

    RCC_EXT_WriteStr(pCliEnv, pErrorMsg);

    if (msg && ! NULL_STRING(pErrorText))
    {
        RCC_EXT_WriteStr(pCliEnv, pErrorText);
    }

    RCC_EXT_WriteStrLine(pCliEnv, "");

    if (HELP_SET(pCliEnv, HELP_FLAG_ERROR))
    {
        RCC_ERROR_HELP_FN(pCliEnv, MMISC_GetCmdNode(pCliEnv));
        HELP_INDENT_RESET_M(pCliEnv);
        /* if help output cancelled, need to reset output */
        RCC_EXT_EnablePrint(pCliEnv, TRUE);
    }

    if (retry)
    {
        RCC_EXT_WriteStrLine(pCliEnv, "");
        RCC_DB_ShowInput(pCliEnv);
        RCC_EXT_SetCursor(pCliEnv, (EditType) offset);
        RCC_EnableFeature(pCliEnv, kRCC_FLAG_LAST_INPUT);
    }
}

/*-----------------------------------------------------------------------*/

extern Boolean RCC_TASK_LineEval(cli_env *pCliEnv, Boolean haveInput)
{
    Boolean      readLoop      = TRUE;
    Boolean      exit          = FALSE;
    Boolean      all           = FALSE;
    RLSTATUS     status        = OK;

    if (! haveInput)
        RCC_TASK_PrintPrompt(pCliEnv);

    /* the main interpreter loop ...*/
    while (readLoop)
    {
        if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_KILL))
            break;

        MMISC_SetCmdNode(pCliEnv, NULL);
        MCONN_SetTimeOut(pCliEnv, kRCC_TIMEOUT);
        RCC_DisableFeature(pCliEnv, kRCC_FLAG_NOPRINT);
        RCC_DisableFeature(pCliEnv, kRCC_FLAG_NEWMODE);
        RCC_STATUS_LINE_FN(pCliEnv);

        if (! haveInput)
            status = RCC_EXT_ReadCmd(pCliEnv, TRUE);

        RCC_DisableFeature(pCliEnv, kRCC_FLAG_LAST_INPUT);

        switch (status)
        {
            case STATUS_RCC_NO_PROMPT:
                RCC_EnableFeature(pCliEnv, kRCC_FLAG_LAST_INPUT);
                continue;
            case ERROR_RCC_READ:
                readLoop = FALSE;
                continue;
			case ERROR_RCC_FAILURE:
                readLoop = FALSE;
                continue;
            case ERROR_RCC_TIMEOUT:
                if (OK != TASK_TimeOut(pCliEnv))
                    readLoop = FALSE;
                continue;
            case STATUS_RCC_EXIT_TO_ROOT:
                all     = TRUE;
            case STATUS_RCC_EXIT: /* fall through */
                exit    = TRUE;
                status  = OK;
                break;
            default:
                break;
        }

        if (0 < MEDIT_GetLength(pCliEnv))
            status = RCC_DB_Process_CLI(pCliEnv);

        switch (status)
        {
        case OK:
        case STATUS_RCC_NO_ERROR:
            break;
        case STATUS_RCC_LOGOUT:
            readLoop = FALSE;
            break;
        case STATUS_RCC_NO_PROMPT:
            RCC_EnableFeature(pCliEnv, kRCC_FLAG_LAST_INPUT); /* broadcast */
        case STATUS_RCC_NO_INTERMEDIATE:
        case ERROR_RCC_NO_ERROR_MSG:
        case ERROR_GENERAL_NO_DATA:
            status = OK;
            break;
        default:
            TASK_PrintError(pCliEnv, status);
            break;
        }

        if (exit)
        {
            RCC_DB_Exit(pCliEnv, all);
            exit = FALSE;
            all  = FALSE;
        }

        if (RCC_NotEnabled(pCliEnv, kRCC_FLAG_LAST_INPUT))
            RCC_TASK_PrintPrompt(pCliEnv);

        if (haveInput)
            break;

    }  /* while (1) - main interpreter loop */

    return readLoop;
}




extern void RCC_TASK_Readline(cli_env *pCliEnv)
{
    RLSTATUS     status        = ERROR_GENERAL;
 
    if (! TASK_Login(pCliEnv))
        return;

    /* run init script */
#ifdef kRCC_AUTOEXEC_FILE
    RCC_CMD_ExecFile(pCliEnv, kRCC_AUTOEXEC_FILE, FALSE);
#endif

    /* print out MOTD if available */
#ifdef kRCC_MOTD
    RCC_DB_PrintRM(pCliEnv, kRCC_MOTD);
#endif

    RCC_TASK_LineEval(pCliEnv, FALSE);
}

/*-----------------------------------------------------------------------*/

/* This function is a sample and should be replaced with 
 * a user specific function 
 */

extern Boolean 
RCC_TASK_ValidateLogin(cli_env *pCliEnv, sbyte *pLogin, sbyte *pPassword, Access *pAccLvl)
{
    extern DTTypeInfo mAccessInfo;

#ifndef __USE_PASSWORD_ONLY__
    if (0 != STRCMP(kRCC_DEFAULT_LOGIN, pLogin))
        return FALSE;
#endif

    if (0 != STRCMP(kRCC_DEFAULT_PASSWORD, pPassword))
        return FALSE;

    CONVERT_StrTypeTo(kRCC_DEFAULT_ACCESS, pAccLvl, &mAccessInfo);
    return TRUE;
}

#endif /* __RCC_ENABLED__ */
