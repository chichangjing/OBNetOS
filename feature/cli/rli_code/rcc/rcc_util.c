/*  
 *  rcc_util.c
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

#ifdef RCC_AUTH_CALLBACK_FN
extern Boolean RCC_AUTH_CALLBACK_FN(cli_env *pCliEnv, sbyte *login, sbyte *password, Access *AccLvl);
#else
#error You must define RCC_AUTH_CALLBACK_FN to be your authentication callback function
#endif

#ifdef __RCC_ENABLE_GETHOSTNAME__
#  ifdef CUSTOM_RCC_GETHOSTNAME
     extern RLSTATUS CUSTOM_RCC_GETHOSTNAME(sbyte *pBuf, Counter bufLen);
#  else
#    define CUSTOM_RCC_GETHOSTNAME gethostname
#  endif /*- __RCC_ENABLE_GETHOSTNAME__ -*/
#endif


/*  this recreates code already in wcontrol codebase, which is specific
    to that blade. This is a hack to provide same functionality */

/* is there a rc_options.h define for max size of a RapidMark name? */
#define kMARKUP_MAX_LENGTH  64
enum markFlag_E {markInvalid_E, markBegin_E, markStart_E, markMidstream_E, markEnd_E, markFinal_E}; 


static helpInfo mHelpDefaults = 
{
    0,
    kRCC_HELP_WIDTH,
    kRCC_HELP_LEADER,
    kRCC_HELP_TITLE,
    kRCC_HELP_PREFIX,
    kRCC_HELP_NODE_DELIMITER,
    kRCC_HELP_DELIMITER,
    kRCC_HELP_SYNTAX,
    kRCC_HELP_SYNTAX_TAIL
};

/* for debugging -- centralized error trap */
extern RLSTATUS RCC_UTIL_ErrorThrow(RLSTATUS status)
{
    return status;
}
    
extern RLSTATUS RCC_ReplaceRapidMarkData(cli_env *pCliEnv, sbyte *pOrig, sbyte *pOutput, Length bufLen)
{
    sbyte   *pBuf        = NULL;
    sbyte   *pMark       = NULL;
    sbyte   *pHead       = NULL;
    sbyte    flag        = markInvalid_E;
    Counter  markLength  = 0;
    Length   outLength   = 0;
    Length   rapidLength = 0;
    Length   preCount    = 0;
    sbyte   *pSrc        = pOrig;
    sbyte   *pDest       = pOutput;
    Boolean  rapidMarks  = FALSE;
    sbyte    pRapidName[kMARKUP_MAX_LENGTH + 1];
    sbyte    pRapidValue[kMagicMarkupBufferSize];
    RLSTATUS status;

    if (NULL == pSrc)
        return RCC_ERROR;

    pBuf = pHead = pSrc;
    while ('\0' != *pBuf)
    {
        flag       = markInvalid_E;
        preCount   = 0;
        markLength = 0;

        while ('\0' != *pBuf && flag != markFinal_E)
        {
            switch (flag)
            {
            case markBegin_E:
                if (kMagicMarkupStartChar1 /*'%'*/ == *pBuf)
                    flag = markStart_E;
                else
                    flag = markInvalid_E;
                break;
            case markStart_E:
                flag  = markMidstream_E;
                pMark = pBuf;
                break;
            case markMidstream_E:
                markLength++;
                if (kMagicMarkupEndChar0 /*'#'*/ == *pBuf)
                    flag = markEnd_E;
                break;
            case markEnd_E:
                if (kMagicMarkupEndChar1 /*'$'*/ == *pBuf)
                    flag = markFinal_E;
                else
                    flag = markMidstream_E;
                break;
            default:
                if (kMagicMarkupStartChar0 /*'$'*/ == *pBuf) 
                    flag = markBegin_E;
                else
                    preCount++;
                break;
            }
            pBuf++;
        }

        /* no rapidmarks */
        if (markFinal_E != flag && !rapidMarks)
            return STATUS_RCC_NO_RAPIDMARKS;

        /* get rapidmark data */
        markLength = RC_MIN(markLength, kMARKUP_MAX_LENGTH);
        STRNCPY(pRapidName, pMark, markLength);
        pRapidName[markLength] = '\0';

        status = RCC_RCB_ReadValueFromRCB(pCliEnv, pRapidName, NULL, 
                                          pRapidValue, &rapidLength);

        if (OK != status)
            return status;

        rapidMarks = TRUE;

        /* copy text before rapidmark */
        preCount   = RC_MIN(preCount, bufLen - outLength);
        STRNCPY(pDest, pHead, preCount);
        pDest     += preCount;
        outLength += preCount;

        if (markFinal_E == flag)
        {
            /* copy rapid mark data */
            rapidLength = RC_MIN(rapidLength, bufLen - outLength);
            STRNCPY(pDest, pRapidValue, rapidLength);
            pDest     += rapidLength;
            outLength += rapidLength;
        }
        pHead = pBuf;

    }
    outLength = RC_MIN(outLength, bufLen);
    pOutput[outLength] = '\0';

    return OK;
}

/*-----------------------------------------------------------------------*/

extern void
RCC_UTIL_SetPrompt(cli_env *pCliEnv, sbyte* promptText)
{
    if (NULL == promptText)
        RCC_DisableFeature(pCliEnv, kRCC_FLAG_MYPROMPT);
    else
    {
        RCC_EnableFeature(pCliEnv, kRCC_FLAG_MYPROMPT);
        STRNCPY(MEDIT_Prompt(pCliEnv), promptText, kRCC_MAX_PROMPT_LEN - 1);
    }
    RCC_UTIL_RapidPrompt(pCliEnv, FALSE);
}

/*-----------------------------------------------------------------------*/

extern void
RCC_UTIL_SetPromptTail(cli_env *pCliEnv, sbyte* pText)
{
    sbyte  *pTail = MEDIT_PromptTail(pCliEnv);
    sbyte4  width;

    STRNCPY(pTail, pText, kRCC_MAX_PROMPT_TAIL_LEN - 1);

    width = STRLEN(MEDIT_Prompt(pCliEnv)) 
          + STRLEN(MEDIT_PromptTail(pCliEnv));

    MEDIT_SetPromptLen(pCliEnv, width);
}

/*-----------------------------------------------------------------------*/

extern void RCC_UTIL_RapidPrompt(cli_env *pCliEnv, Boolean print)
{
    sbyte4     width        = 0;
    sbyte     *pPrompt      = MEDIT_Prompt(pCliEnv);
    sbyte      tempBuf[kRCC_MAX_PROMPT_LEN];

    /* replace any rapidmarks */
    if (OK != RCC_ReplaceRapidMarkData(pCliEnv, pPrompt, tempBuf, sizeof(tempBuf)))
    {
        RCC_DisableFeature(pCliEnv, kRCC_FLAG_RM_PROMPT);

        width = STRLEN(pPrompt) 
              + STRLEN(MEDIT_PromptTail(pCliEnv));

        MEDIT_SetPromptLen(pCliEnv, width);
        return;
    }

    RCC_EnableFeature(pCliEnv, kRCC_FLAG_RM_PROMPT);

    width = STRLEN(tempBuf) 
          + STRLEN(MEDIT_PromptTail(pCliEnv));

    MEDIT_SetPromptLen(pCliEnv, width);

    if (print)
    {
        RCC_EXT_WriteStr(pCliEnv, tempBuf);
        RCC_EXT_WriteStr(pCliEnv, MEDIT_PromptTail(pCliEnv));
    }
}

/*-----------------------------------------------------------------------*/

/* update prompt when changing intermediate modes  */
extern void RCC_UTIL_UpdatePrompt(cli_env *pCliEnv)
{
    Boolean    multiple     = FALSE;
    Boolean    custom       = RCC_IsEnabled(pCliEnv, kRCC_FLAG_MYPROMPT);
    Length     maxLength    = 0;
    paramList *pList        = MMISC_GetRootParam(pCliEnv);
    cmdNode   *pCmdNode     = MMISC_GetCurrRoot(pCliEnv);
    sbyte     *pSrcPrompt   = MMISC_GetNodePrompt(pCmdNode);
    sbyte     *pPrompt      = MEDIT_Prompt(pCliEnv);

    /* setting the prompt in a handler lasts only for the duration 
     * of the mode that was entered when the the prompt was set --
     * it should exist until the mode is exited (another intermediate
     * mode or exiting to a previous mode)
     */
    RCC_DisableFeature(pCliEnv, kRCC_FLAG_MYPROMPT);
    if (custom)
        return;

    if (RCC_NotEnabled(pCliEnv, kRCC_FLAG_NEWMODE))
        return;

    MEMSET(pPrompt, 0, kRCC_MAX_PROMPT_LEN);

#ifdef __RCC_ENABLE_GETHOSTNAME__
    /* use the hostname as a default prompt */
    if (-1 == CUSTOM_RCC_GETHOSTNAME(pPrompt, kRCC_MAX_PROMPT_LEN))
        STRNCPY(pPrompt, kRCC_DEFAULT_PROMPT, kRCC_MAX_PROMPT_LEN);
#else
    STRNCPY(pPrompt, kRCC_DEFAULT_PROMPT, kRCC_MAX_PROMPT_LEN);
#endif /*- __RCC_ENABLE_GETHOSTNAME__ -*/

    /* intermediate mode? */
    if (0 < MMISC_GetModeDepth(pCliEnv))
    {
        STRNCAT(pPrompt, "(", kRCC_MAX_PROMPT_LEN - STRLEN(pPrompt));
        if (! NULL_STRING(pSrcPrompt))
        {
            STRNCAT(pPrompt, pSrcPrompt, kRCC_MAX_PROMPT_LEN - STRLEN(pPrompt));
        }
        else
        {
            /* no prompt - construct one out of intermediate nodes */
            for ( ; NULL != pList; pList = pList->pNext)
            {
                if (NULL == (pCmdNode = pList->pCmdNode))
                    continue;

                if (FLAG_SET(pCmdNode, kRCC_COMMAND_NO_EXEC))
                    break;

                if (FLAG_SET(pCmdNode, kRCC_COMMAND_GLOBAL))
                    break;

                if (0 >= (maxLength = kRCC_MAX_PROMPT_LEN - STRLEN(pPrompt)))
                    break;

                if (multiple)
                    STRNCAT(pPrompt, kRCC_PROMPT_DELIMITER, maxLength);

                STRNCAT(pPrompt, pCmdNode->pKeyword, maxLength);
                maxLength -= STRLEN(pCmdNode->pKeyword);
                multiple = TRUE;
            }
        }
        STRCAT(pPrompt, ")");
    }

    RCC_UTIL_RapidPrompt(pCliEnv, FALSE);
    RCC_DisableFeature(pCliEnv, kRCC_FLAG_NEWMODE);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_UTIL_Init(cli_env *pCliEnv)
{
    ubyte4  system;
    ubyte4  help;

    RCC_EnableFeature(pCliEnv, kRCC_FLAG_NEWMODE);
    RCC_UTIL_UpdatePrompt(pCliEnv);
    RCC_UTIL_SetPromptTail(pCliEnv, kRCC_DEFAULT_PROMPT_TAIL);

    /********* system settings ***********/ 

    system = kRCC_FLAG_ECHO     |   /* output is initially visible  */
             kRCC_DEFAULT_ORDER;    /* keep track of param order    */     

#ifndef __DISABLE_PAGED_OUTPUT__
    /* enable paging of output */
    //system |= kRCC_FLAG_MORE;
#endif

#ifdef __SNMP_API_ENABLED__
#ifdef __RCC_SNMP_AUTOCOMMIT__
    /* automatically commit snmp sets */
    system |= kRCC_FLAG_SNMPAUTO;
#endif
#endif

#ifdef __RCC_DASHED_ERROR_LINE__
    /* use dashes in error line */
    system |= kRCC_FLAG_DASHES;
#endif

#if defined(RCC_EDIT_STATUS_FN) || defined(RCC_STATUS_LINE_FN)
    /* customer implemented telnet status line */
    system |= kRCC_FLAG_STATUS;
#endif

    MMISC_SetHelpInfo(pCliEnv, mHelpDefaults);

    /* enable intermediate mode */
#ifndef __NO_INTERMEDIATE_MODE__
    system |= kRCC_FLAG_MODE;
#endif

    /* retries need cursor positioning */
#if ! (defined(__DISABLE_VT_ESCAPES__) || defined(__RCC_DISABLE_RETRIES__))
    system |= kRCC_FLAG_RETRIES;
#endif

    RCC_EnableFeature(pCliEnv, system);

    /*************  help system **************/

    help = 0;

#ifdef __HELP_ALL_PARAMS__
    help |= HELP_FLAG_PARAMS;
#endif

#ifdef __HELP_SAME_LINE__
    help |= HELP_FLAG_SAME_LINE;
#endif

#ifndef __HELP_DISABLE_FIXED_WIDTH__
    help |= HELP_FLAG_FIXED_WIDTH;
#endif

#ifndef __RCC_NO_HELP_INDENT__
    help |= HELP_FLAG_INDENT;
#endif

#ifdef __RCC_SHOW_NO__
    help |= HELP_FLAG_SHOW_NO;
#endif

#ifdef __RCC_HIDE_BLANK_HELP__
    help |= HELP_FLAG_HIDE_BLANK;
#endif

#ifdef __HELP_SHOWS_NEXT_PARAM__
    help |= HELP_FLAG_NEXT;
#endif

#ifdef __HELP_SHOWS_EXECUTABLE_NODES__
    help |= HELP_FLAG_EXECUTABLE;
#endif

#ifdef __HELP_SKIP_NODE__
    help |= HELP_FLAG_SKIP_NODE;
#endif

#ifdef  __ENABLE_THIS_HELP__
    /* include help for the current node */
    help |= HELP_FLAG_THIS;
#endif

#ifdef  __ENABLE_ERROR_HELP__
    /* show help when user enters invalid command */
    help |= HELP_FLAG_ERROR;
#endif

    SET_HELP(pCliEnv, help);


#ifdef __ANSI_FILE_MANAGER_ENABLED__
#ifdef kRCC_DEFAULT_PATH
    ANSIFS_Init(kRCC_DEFAULT_PATH);
#else
    ANSIFS_Init("");
#endif
#endif /* __ANSI_FILE_MANAGER_ENABLED__ */

    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_UTIL_SetAccess(cli_env *pCliEnv, sbyte *pAccessStr)
{
    Access    newAccess;
    RLSTATUS  status;
    extern    DTTypeInfo  mAccessInfo;

    if (OK != (status = CONVERT_StrTypeTo(pAccessStr, &newAccess, &mAccessInfo)))
        return status;

    MMISC_SetAccess(pCliEnv, newAccess);
    return OK;
}

/*-----------------------------------------------------------------------*/

/* change level w/out changing groups */

extern RLSTATUS RCC_UTIL_SetAccessLevel(cli_env *pCliEnv, sbyte4 level)
{
    Access    newAccess;

    /* make sure level is withing bounds */
    if (level & (~ __RLI_ACCESS_LEVEL_MASK__))
        return RCC_ERROR_THROW(ERROR_GENERAL_OUT_OF_RANGE);

    /* keep existing groups */
    newAccess  = MMISC_GetAccess(pCliEnv) & __RLI_ACCESS_OPTION_MASK__;
    newAccess |= level & __RLI_ACCESS_LEVEL_MASK__;

    MMISC_SetAccess(pCliEnv, newAccess);
    return OK;
}

/*-----------------------------------------------------------------------*/

/* change groups w/out changing level */

extern RLSTATUS RCC_UTIL_SetAccessGroups(cli_env *pCliEnv, sbyte *pGroups)
{
    Access    newAccess;
    RLSTATUS  status;
    extern    DTTypeInfo  mAccessInfo;

    if (OK != (status = CONVERT_StrTypeTo(pGroups, &newAccess, &mAccessInfo)))
        return status;

    /* keep existing level */
    newAccess  &= __RLI_ACCESS_OPTION_MASK__;
    newAccess  |= MMISC_GetAccess(pCliEnv) & __RLI_ACCESS_LEVEL_MASK__;

    MMISC_SetAccess(pCliEnv, newAccess);
    return OK;

}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_UTIL_SecuredAccess(cli_env *pCliEnv, sbyte *pLogin, sbyte *pPassword)
{
    Access      access;

    RCC_EnableFeature(pCliEnv, kRCC_FLAG_ECHO);  

#ifndef __USE_PASSWORD_ONLY__
    RCC_EXT_WriteStr(pCliEnv, kRCC_LOGIN_PROMPT);

    RCC_EXT_ReadCmd(pCliEnv, FALSE);

    if (kRCC_MAX_LOGIN_LEN <= MEDIT_GetLength(pCliEnv))
        return RCC_ERROR_THROW(ERROR_RCC_RETRY);

	if ( 0 == MEDIT_GetLength(pCliEnv))
    	return RCC_ERROR_THROW(ERROR_RCC_RETRY);

    MEDIT_CopyFromInput(pCliEnv, pLogin);
#endif

    RCC_EXT_WriteStr(pCliEnv, kRCC_PASSWORD_PROMPT);

    RCC_EXT_LocalEcho(pCliEnv, FALSE);
    RCC_EXT_ReadCmd(pCliEnv,   FALSE);
    RCC_EXT_LocalEcho(pCliEnv, TRUE);

    if (kRCC_MAX_PASSWORD_LEN <= MEDIT_GetLength(pCliEnv))
        return RCC_ERROR_THROW(ERROR_RCC_RETRY);

    MEDIT_CopyFromInput(pCliEnv, pPassword);

    if (! RCC_AUTH_CALLBACK_FN(pCliEnv, pLogin, pPassword, &access))
	    return RCC_ERROR_THROW(ERROR_RCC_RETRY);

	/* store the username, fry the password on the stack */
    MMISC_SetAccess(pCliEnv, access);
	MMISC_SetLogin(pCliEnv, pLogin);
	MEMSET(pPassword, 0, STRLEN(pPassword));

    return OK;
}

#endif /* __RCC_ENABLED__ */
