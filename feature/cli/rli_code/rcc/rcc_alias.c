/*  
 *  rcc_alias.c
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

#ifdef __RCC_ENABLED__

#ifndef RL_STATIC
#define RL_STATIC static
#endif

#define kRCC_END_OF_DUMP    -1
#define kRCC_ALIAS_WIDTH    30

extern void 
RCC_ALIAS_DisplayAliases(cli_env *pCliEnv)
{
    sbyte4      index;
    AliasTable *pAliases    = MMISC_AliasPtr(pCliEnv);
    CmdAlias   *pAlias      = pAliases->alias;

    for (index = 0; index < pAliases->numEntries; index++, pAlias++)
    {
        RCC_EXT_PrintString(pCliEnv, pAlias->pName, kRCC_ALIAS_WIDTH, ' ');
        RCC_EXT_WriteStrLine(pCliEnv, pAlias->pText);
    }
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_ALIAS_DeleteAlias(cli_env *pCliEnv, sbyte *aliasName)
{
    AliasTable *pAliases    = MMISC_AliasPtr(pCliEnv);
    CmdAlias   *pAlias      = pAliases->alias;
    sbyte4      index       = 0;
    sbyte4      count       = 0;
    sbyte4      delta       = 0;
    sbyte4      block       = 0;
    Boolean     deleted     = FALSE;
    sbyte      *pLower;
    sbyte      *pUpper;
    
    for (index = 0; index < pAliases->numEntries; index++, pAlias++)
    {
        if (0 != COMPARE(pAlias->pName, aliasName))
            continue;

        deleted = TRUE;
        delta   = STRLEN(pAlias->pName) + STRLEN(pAlias->pText) + 2;

        /* shift all remaining data down */
        pLower = pAlias->pName;
        pUpper = pLower + delta;
        block  = pAliases->bufferUsed - (pUpper - pAliases->data);
        for (count = 0; count < block; count++, pLower++, pUpper++)
            *pLower = *pUpper;

        /* null remainder */
        MEMSET(pUpper, 0, delta);
        break;
    }

    if (! deleted)
        return OK;

    /* shift all remaining entries down */
    for ( ; index < pAliases->numEntries; index++, pAlias++)
    {
        if (index < pAliases->numEntries - 1)
        {
            pAlias[0] = pAlias[1];
            pAlias->pName -= delta;
            pAlias->pText -= delta;
        }
    }

    pAliases->bufferUsed -= delta;
    pAliases->numEntries--;

    return OK;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
ALIAS_Init(cli_env *pCliEnv, cmdNode *pNode)
{
    AliasTable *pAliases = MMISC_AliasPtr(pCliEnv);

    pAliases->pAliasCmd = pNode->pKeyword;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_ALIAS_CreateAlias(cli_env *pCliEnv, sbyte *aliasName, sbyte *aliasText)
{
    RLSTATUS    status      = OK;
    AliasTable *pAliases    = MMISC_AliasPtr(pCliEnv);
    sbyte      *buffer      = pAliases->data;
    cmdNode    *pNode       = MMISC_GetExecNode(pCliEnv);

    sbyte4      currLen;
    sbyte4      nameLen;
    sbyte4      textLen;
    sbyte4      index;

    if (NULL_STRING(aliasName) || NULL_STRING(aliasText))
        return RCC_ERROR_THROW(RCC_ERROR);

    /* don't let user change "alias" itself!! */
    if ((NULL != pNode) && (0 == COMPARE(pNode->pKeyword, aliasName)))
        return RCC_ERROR_THROW(RCC_ERROR);

    ALIAS_Init(pCliEnv, pNode);

    if (kRCC_ALIAS_COUNT <= pAliases->numEntries)
        return RCC_ERROR_THROW(ERROR_RCC_MAX_ALIASES);

    /* remove prior definition, if it exists */
    RCC_ALIAS_DeleteAlias(pCliEnv, aliasName);
    index   = pAliases->numEntries;
    currLen = pAliases->bufferUsed;

    nameLen = STRLEN(aliasName) + 1; /* don't forget null term */
    textLen = STRLEN(aliasText) + 1; /* don't forget null term */
    if (kRCC_ALIAS_BUFFER_SIZE < nameLen + textLen + currLen)
        return RCC_ERROR_THROW(RCC_ERROR);

    buffer += currLen;
    STRCPY(buffer, aliasName);
    pAliases->alias[index].pName = buffer;

    buffer += nameLen;
    STRCPY(buffer, aliasText);
    pAliases->alias[index].pText = buffer;

    pAliases->bufferUsed += (nameLen + textLen);
    pAliases->numEntries++;

    return status;
}

/*-----------------------------------------------------------------------*/

/* store alias info for next session */
extern RLSTATUS 
RCC_ALIAS_AliasDump(cli_env *pCliEnv, sbyte *pDumpBuffer, sbyte4 bufferSize)
{
    AliasTable *pAliases    = MMISC_AliasPtr(pCliEnv);
    CmdAlias   *pAlias      = pAliases->alias;
    sbyte      *pWrite      = pDumpBuffer;
    sbyte4      writeSize   = 0;
    sbyte4      remaining   = bufferSize;
    sbyte4      index;

    if (NULL == pDumpBuffer)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    for (index = 0; index < pAliases->numEntries; index++, pAlias++)
    {
        writeSize = RC_MIN(remaining, (sbyte4) STRLEN(pAlias->pName));
        STRNCPY(pWrite, pAlias->pName, remaining);
        remaining -= writeSize;
        pWrite    += writeSize;
        if (0 < remaining)
        {
            *(++pWrite) = 0;
            remaining--;
        }

        writeSize = RC_MIN(remaining, (sbyte4) STRLEN(pAlias->pText));
        STRNCPY(pWrite, pAlias->pText, remaining);
        remaining -= writeSize;
        pWrite    += writeSize;
        if (0 < remaining)
        {
            *(++pWrite) = 0;
            remaining--;
        }
    }

    *pWrite = kRCC_END_OF_DUMP;

    return OK;
}

/*-----------------------------------------------------------------------*/

/* retrieve alias info from prior saved session */
extern RLSTATUS 
RCC_ALIAS_AliasRestore(cli_env *pCliEnv, sbyte *pDumpBuffer, sbyte4 bufferSize)
{
    sbyte      *pRead       = pDumpBuffer;
    sbyte      *pName       = NULL;
    sbyte      *pText       = NULL;

    if (NULL == pDumpBuffer)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    while (0 < bufferSize)
    {
        pText = pName = pRead;
        while (0 != *(pText++))
            bufferSize--;
        RCC_ALIAS_CreateAlias(pCliEnv, pName, pText);
        pRead = pText;
        while (0 != *(pRead++))
            bufferSize--;

        if ((sbyte) kRCC_END_OF_DUMP != *pRead)
            break;
    }

    return OK;
}

#endif /* __RCC_ENABLED__ */
