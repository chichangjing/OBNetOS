/*  
 *  rcc_cmd.c
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

#ifdef __RCC_ENABLED__
#include "rcc.h"

#ifdef __ANSI_FILE_MANAGER_ENABLED__
#include "rc_ansifs.h"
#endif /* __ANSI_FILE_MANAGER_ENABLED__ */
#ifdef __FILE_MANAGER_ENABLED__ 
#include "rc_filemgr.h"
#endif /* __FILE_MANAGER_ENABLED__ */
 
#ifdef __SNMP_API_ENABLED__

#ifdef ENVOY_STACK_K
#include "rcm_envoy.h"
#include "rcm_ev_cnv.h"
#include "rcm_snmp.h"
#endif

#if (defined(__GENERIC_SNMP_V1__) || defined(__CMU_SNMP_V1__))
#include "rcm_cmu.h"
#include "rcm_cmu_cnv.h"
#include "rcm_snmp.h"
#endif

#include "rcm_mibway.h"

#endif /* __SNMP_API_ENABLED__*/

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_CMD_Snmp_Auto(cli_env *pCliEnv)
{
    RCC_EXT_WriteStr(pCliEnv, kRCC_MSG_SNMPAUTO);
    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_SNMPAUTO))
    {
        RCC_DisableFeature(pCliEnv, kRCC_FLAG_SNMPAUTO);
        RCC_EXT_WriteStrLine(pCliEnv, kRCC_MSG_DISABLED);
    }
    else
    {
        RCC_EnableFeature(pCliEnv, kRCC_FLAG_SNMPAUTO);
        RCC_EXT_WriteStrLine(pCliEnv, kRCC_MSG_ENABLED);
    }
    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_CMD_Snmp_Commit(cli_env *pCliEnv)
{
#ifdef __SNMP_API_ENABLED__
    snmpStatus  *pSnmpStatus = NULL;

    pSnmpStatus = OCSNMP_Commit(pCliEnv, 0);

    if (NULL != pSnmpStatus)
    {
        if (! NULL_STRING(pSnmpStatus->error_text))
            RCC_EXT_WriteStrLine(pCliEnv, pSnmpStatus->error_text);

        RC_FREE(pSnmpStatus);
    }
#endif /* __SNMP_API_ENABLED__ */

    return OK;
}

/*-----------------------------------------------------------------------*/

#ifdef __WIN32_OS__
/* clear DOS screen */
extern void RCC_CMD_DosClear(cli_env *pCliEnv)
{
    CONSOLE_SCREEN_BUFFER_INFO lpScreenBufferInfo;
    DWORD                      nLength;
    COORD                      dwWriteCoord;
    DWORD                      NumberOfCharsWritten;

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE == console)
        return;

    if (0 == GetConsoleScreenBufferInfo(console, &lpScreenBufferInfo))
        return;

    dwWriteCoord.X = 0;
    dwWriteCoord.Y = 0;
    nLength = lpScreenBufferInfo.dwSize.X * lpScreenBufferInfo.dwSize.Y;
    if (! FillConsoleOutputCharacter(
            console,                // handle to screen buffer
            ' ',                    // character
            nLength,                // number of cells
            dwWriteCoord,           // first coordinates
            &NumberOfCharsWritten   // number of cells written
    ))
        return;

    SetConsoleCursorPosition(console, dwWriteCoord);
}
#endif /* __WIN32_OS__ */

/*-----------------------------------------------------------------------*/

/* clear screen */
extern RLSTATUS RCC_CMD_Clear(cli_env *pCliEnv)
{
#ifdef __WIN32_OS__
    if (kRCC_CONN_CONSOLE == MCONN_GetConnType(pCliEnv))
        RCC_CMD_DosClear(pCliEnv);
    else
#endif /* __WIN32_OS__ */        
    {
        /* assume that non-Windows console is really telnet */
        RCC_EXT_WriteStr(pCliEnv, kRCC_TELNET_CLEAR);
    }
    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_CMD_Exec (cli_env *pCliEnv, sbyte *pBuffer, sbyte4 bufferLen, Boolean echo)
{
    RLSTATUS    status          = OK;
    sbyte      *pLine;
    sbyte      *pChar;
    sbyte4      lineLen;

    pChar = pLine = pBuffer;
    while (0 < bufferLen)
    {
        lineLen = 0;

        while (('\n' != *pChar) && ('\r' != *pChar))
        {
            pChar++;

            if (kRCC_MAX_CMD_LEN < ++lineLen)
                break;

            if (0 >= --bufferLen)
                break;
        }

        /* make it look like "normal" interaction */
        if (echo)
        {
            RCC_TASK_PrintPrompt(pCliEnv);
            RCC_EXT_Write(pCliEnv, pLine, lineLen);
            RCC_EXT_WriteStrLine(pCliEnv, "");
        }

        RCC_DB_SetCommand(pCliEnv, pLine, lineLen);
        status = RCC_DB_Process_CLI(pCliEnv);

        if ((OK != status) && (ERROR_GENERAL_NO_DATA != status))
            break;

        /* skip c/r */
        if ('\r' == *pChar)
        {
            pChar++;
            bufferLen--;
        }

        /* skip l/f */
        if ('\n' == *pChar)
        {
            pChar++;
            bufferLen--;
        }
        pLine = pChar;
    }

    return status;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_CMD_ExecFile (cli_env *pCliEnv, sbyte *pFileName, Boolean echo)
{
    RLSTATUS    status          = OK;
    sbyte      *pFileBuffer     = NULL;
    sbyte4      fileLen         = 0;

#if defined(__FILE_MANAGER_ENABLED__) || defined(__ANSI_FILE_MANAGER_ENABLED__)
    Boolean     isAnsi          = FALSE;
#endif

#ifdef __ANSI_FILE_MANAGER_ENABLED__
    pFileBuffer = ANSIFS_ReadFile(pCliEnv, &fileLen, pFileName);
    if (NULL != pFileBuffer)
        isAnsi = TRUE;
#endif /* __ANSI_FILE_MANAGER_ENABLED__ */

#ifdef __FILE_MANAGER_ENABLED__ 
    if ((!isAnsi) && (NULL == pFileBuffer))
        pFileBuffer = FILEMGR_RetrieveFile(&fileLen, pFileName);
#endif /* __FILE_MANAGER_ENABLED__ */

    if (NULL == pFileBuffer)
    {
        RCC_DisableFeature(pCliEnv, kRCC_FLAG_HISTORY);
        return RCC_ERROR_THROW(ERROR_GENERAL_FILE_NOT_FOUND);
    }

    RCC_EnableFeature(pCliEnv, kRCC_FLAG_EXEC);

    status = RCC_CMD_Exec(pCliEnv, pFileBuffer, fileLen, echo);

#ifdef __ANSI_FILE_MANAGER_ENABLED__
    if (isAnsi)
        ANSIFS_ReleaseFile(pFileBuffer);
#endif /* __ANSI_FILE_MANAGER_ENABLED__ */

#ifdef __FILE_MANAGER_ENABLED__ 
    if (! isAnsi)
        FILEMGR_ReleaseFile(pFileBuffer);
#endif /* __FILE_MANAGER_ENABLED__ */

    RCC_DisableFeature(pCliEnv, kRCC_FLAG_HISTORY);
    RCC_DisableFeature(pCliEnv, kRCC_FLAG_EXEC);

    return status;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_CMD_ExecRM(cli_env *pCliEnv, sbyte *pRapidMark, Boolean echo)
{
    RLSTATUS    status  = OK;
    sbyte       pRMBuffer[kMagicMarkupBufferSize];
    Length      rapidLength;

    if (OK != (status = RCC_RCB_ReadValueFromRCB(pCliEnv, pRapidMark, NULL, pRMBuffer, &rapidLength)))
        return status;

    return RCC_CMD_Exec(pCliEnv, pRMBuffer, rapidLength, echo);

}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_CMD_Who(cli_env *pCliEnv)
{
    sbyte4   index;
    cli_env *pOther;
    ubyte4   ipAddr;
    sbyte    buffer[32];

    for (index = 0; index < kRCC_MAX_CLI_TASK+1; index++)
    {
        if (NULL == (pOther = RCC_TELNETD_GetSession(index)))
            continue;

        if (NULL == CLIENV(pOther))
            continue;

        RCC_EXT_PrintString(pCliEnv, MMISC_GetLogin(pOther), 20, ' ');

        switch (MCONN_GetConnType(pOther))
        {
        case kRCC_CONN_CONSOLE:
            RCC_EXT_WriteStr(pCliEnv, MCONN_GetTermName(pOther));
            break;
        case kRCC_CONN_TELNET:
            ipAddr = OS_SPECIFIC_GET_ADDR(pOther);
            CONVERT_ToStr(&ipAddr, buffer, kDTipaddress);
            RCC_EXT_WriteStr(pCliEnv, buffer);
            break;
        }

        RCC_EXT_WriteStrLine(pCliEnv, "");
    }
    
    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_CMD_Write(cli_env *pCliEnv, sbyte *recipient, sbyte *message)
{
    sbyte4   index;
    cli_env *pOther;

    if (NULL_STRING(recipient))
        return RCC_ERROR_THROW(ERROR_RCC_INVALID_USER);

    if (NULL_STRING(message))
        return RCC_ERROR_THROW(ERROR_RCC_NO_PARAM_DATA);

    for (index = 0; index < kRCC_MAX_CLI_TASK+1; index++)
    {
        pOther = RCC_TELNETD_GetSession(index);

        if ((NULL == pOther) || (NULL == CLIENV(pOther)))
            continue;

        if (0 != COMPARE(recipient, MMISC_GetLogin(pOther)))
            continue;

        RCC_EXT_WriteStrLine(pOther, message);
        return OK;
    }    
    
    return RCC_ERROR_THROW(ERROR_RCC_INVALID_USER);
}


#endif /* __RCC_ENABLED__ */
