/*  
 *  rcc_rcb.c
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

#ifdef __SNMP_API_ENABLED__
#include "rcm_mibway.h"
#include <stdio.h>
#endif /* __SNMP_API_ENABLED__ */

#define kTempBufferSize     64

#ifndef __MACRO_REPEAT_NEST_FULL__
#define MACRO_INDEX(env)	env->MacroIndex
#else
#define MACRO_INDEX(env)    env->Macro_Index[env->Macro_NestDepth]
#endif	/* __MACRO_REPEAT_NEST_FULL__ */

/*-----------------------------------------------------------------------*/

extern RLSTATUS
RCC_RCB_WriteValueToRCB(cli_env *pCliEnv, sbyte *pRapidMark, sbyte *pArgs, sbyte *pVal )
{
    RLSTATUS    status;

#ifdef __SNMP_API_ENABLED__
    OCSNMP_SetPostFlag(pCliEnv);
#endif 

    /* shouldn't DB_WriteData deal with invalid input and 
       eliminate the need to query first? */
    status = DB_QueryValue(pCliEnv, pRapidMark, pVal);
    if (OK == status)
        status = DB_WriteData(pCliEnv, pRapidMark, pVal, pArgs);

    return status;
}



/*-----------------------------------------------------------------------*/

static Boolean RCC_RCB_CompareIndexedValues(void *p_idxObj1, void *p_idxObj2)
{
    return (((IndexedElement *)p_idxObj1)->p_sdrObject == 
            ((IndexedElement *)p_idxObj2)->p_sdrObject);

} /* RCC_RCB_CompareIndexedValues */


/*-----------------------------------------------------------------------*/

static RLSTATUS
RCC_RCB_ReadNormalElementFromRCB(cli_env *pCliEnv, sbyte *pRapidMark,
                                 sbyte *pArgs, sbyte *pOutputBuffer, 
                                 Length *pOutputLen )
{
    RLSTATUS        status;

    *pOutputLen = 0;

    status = DB_ReadData(pCliEnv, pRapidMark, pOutputBuffer, pArgs);

#ifdef __SNMP_API_ENABLED__
	if (kNoSuchInstance == status)
        return OK;
#endif

    if ( OK == status )
        *pOutputLen = STRLEN(pOutputBuffer);

    return status;
}


#ifndef  __DISABLE_STRUCTURES__

/*-----------------------------------------------------------------------*/

static RLSTATUS
RCC_RCB_ReadIndexedElementFromRCB(cli_env *p_envVar, sbyte *pRapidMark,
                                 sbyte *pArgs, sbyte *pOutputBuffer, 
                                 Length *pOutputLen )
{
    IndexedElement   *p_idxNewObject;
    RLSTATUS          status = OK;
    list             *listHead;

    *pOutputLen = 0;

#ifndef __MACRO_REPEAT_NEST_FULL__
	listHead = p_envVar->p_lstIndexedValues; 
#else
    listHead = p_envVar->p_MacroLstIndexedValues[p_envVar->Macro_NestDepth];
#endif

    if (NULL == (p_idxNewObject = RC_MALLOC(sizeof(IndexedElement))))
        return RCC_ERROR_THROW(ERROR_MEMMGR_NO_MEMORY);

    p_idxNewObject->p_sdrObject = DB_GetSDR(pRapidMark);

    if (NULL == pList_FindObject(listHead, p_idxNewObject, 
                                 RCC_RCB_CompareIndexedValues ))
    {                                           /* add to list */
        p_idxNewObject->Index = 0;

        if (OK != (status = List_AddObject(listHead, p_idxNewObject)))
        {
            RC_FREE(p_idxNewObject);
            return status;
        }

        if (NULL == (p_idxNewObject->pDynamicLocation = 
            DB_SDR_Private_ObjectLoc(p_envVar, p_idxNewObject->p_sdrObject)))
        {
            status = ERROR_GENERAL_NO_DATA;
        }
    }
    else
    {                                           /* already in list */
        RC_FREE(p_idxNewObject);
		p_idxNewObject = NULL;
    }

    status = DB_ReadIndexedData(p_envVar, pRapidMark, pOutputBuffer, pArgs);

    if ( OK == status )
        *pOutputLen = STRLEN(pOutputBuffer);

    return status;

} /* RCC_RCB_ReadIndexedElementFromRCB */

#endif /* __DISABLE_STRUCTURES__ */


/*-----------------------------------------------------------------------*/

extern RLSTATUS
RCC_RCB_ReadValueFromRCB(cli_env *pCliEnv, sbyte *pRapidMark, 
                         sbyte *pArgs, sbyte *pOutputBuffer, 
                         Length *pOutputLen)
{
    ubyte2        elementType;
    RLSTATUS      status = OK;

    *pOutputLen = 0;

    if (OK != (status = DB_ElementType(pRapidMark, &elementType)))
        return status;

     /*  Get the data format */
    switch (elementType)
    {
        case kNormalElement:
            status = RCC_RCB_ReadNormalElementFromRCB(pCliEnv, pRapidMark, pArgs,
                                                      pOutputBuffer, pOutputLen);
            break;

#ifndef  __DISABLE_STRUCTURES__
        case kIndexedElement:
            status = RCC_RCB_ReadIndexedElementFromRCB(pCliEnv, pRapidMark, pArgs,
                                                       pOutputBuffer, pOutputLen);
            break;
#endif /* __DISABLE_STRUCTURES__ */
        
        default:
            return RCC_ERROR_THROW(ERROR_GENERAL);
    }

    return status;

} /* RCC_RCB_ReadValueFromRCB */


/* 
 * Uses MIBway's display overload feature. 
 * e.g., pGetThis == "MACADDR:mibObject.1.5" 
 */

extern RLSTATUS
RCC_RCB_OverloadReadFromRCB(cli_env *pCliEnv, sbyte *pGetThis,
                                  sbyte *pOutputBuffer, Length *pOutputLen)
{
     sbyte   RapidMark[(kMaxMagicMarkupIdLen - kMaxArgsLen) + 1];
     sbyte*  pArgs;
     sbyte*  pTmpDest;
     Counter bytesWritten;

     RapidMark[0] = '\0';

     /* copy out RM component */
     bytesWritten = 0;
     pTmpDest     = RapidMark;

     /* RapidMark == "MACADDR" */
     while (*pGetThis != '\0')
     {
         if (!ISALPHANUMERIC(*pGetThis))
             break;

         *pTmpDest = *pGetThis;
         pTmpDest++;
         pGetThis++;
         bytesWritten++;

         /* don't overwrite the stack! */
         if ((kMaxMagicMarkupIdLen + kMaxArgsLen) <= bytesWritten)
             break;
     }

     /* null terminate the string */
     *pTmpDest = '\0';

     /* pArgs == ":mibObject.1.5" */
     pArgs = pGetThis;

     return RCC_RCB_ReadValueFromRCB(pCliEnv, RapidMark, pArgs, 
                                     pOutputBuffer, pOutputLen);  
} /* RCC_RCB_OverloadReadFromRCB */



extern RLSTATUS
RCC_RCB_OverloadWriteToRCB(cli_env *pCliEnv, sbyte *pSetThis, sbyte *pInputBuffer)
{
     /* Uses MIBway's display overload feature. ex. pGetThis == "MACADDR:mibObject.1.5" */
     sbyte   RapidMark[(kMaxMagicMarkupIdLen - kMaxArgsLen) + 1];
     sbyte*  pArgs;
     sbyte*  pTmpDest;
     Counter bytesWritten;

     RapidMark[0] = '\0';

     /* copy out RM component */
     bytesWritten = 0;
     pTmpDest     = RapidMark;

     /* RapidMark == "MACADDR" */
     while (*pSetThis != '\0')
     {
         if (!ISALPHANUMERIC(*pSetThis))
             break;

        *pTmpDest = *pSetThis;

        pTmpDest++;
        pSetThis++;
        bytesWritten++;

        /* don't overwrite the stack! */
        if ((kMaxMagicMarkupIdLen + kMaxArgsLen) <= bytesWritten)
            break;
     }

     /* null terminate the string */
     *pTmpDest = '\0';

     /* pArgs == ":mibObject.1.5" */
     pArgs = pSetThis;

     return RCC_RCB_WriteValueToRCB(pCliEnv,RapidMark, pArgs, pInputBuffer);

} /* RCC_RCB_OverloadWriteToRCB */

/*-----------------------------------------------------------------------*/

#ifdef __SNMP_API_ENABLED__
static RLSTATUS 
RCC_RCB_GetMetaRapidMark(cli_env *pCliEnv, sbyte* pName, sbyte* pArgs)
{
    sbyte*       pValBuffer;
    RLSTATUS     status;

    if (NULL == (pValBuffer = RC_MALLOC(kTempBufferSize)))
        return RCC_ERROR_THROW(ERROR_MEMMGR_NO_MEMORY);

    status = DB_ReadData(pCliEnv, pName, pValBuffer, pArgs);
    
    RC_FREE(pValBuffer);

    return status;
}
#endif /* __SNMP_API_ENABLED__ */

/*-----------------------------------------------------------------------*/

static RLSTATUS 
RCC_RCB_DetermineStart(cli_env *pCliEnv, TableDescr *pTableDescr, Counter *pStart)
{
    sbyte*      pValBuffer;
    Length      outputLen;
    Counter     start  = 0;
    RLSTATUS    status = OK;

     /* Use the Dynamic Start, if there is one... */
    if (NULL != pTableDescr->pDynamicStart)
    {
        if (NULL == (pValBuffer = RC_MALLOC(kTempBufferSize)))
            return RCC_ERROR_THROW(ERROR_MEMMGR_NO_MEMORY);

        status = RCC_RCB_ReadValueFromRCB(pCliEnv, pTableDescr->pDynamicStart, NULL,
                                          pValBuffer, &outputLen);
        if (OK != status)
        {
            RC_FREE(pValBuffer);
            return status;
        }

        start = ATOI(pValBuffer);
        RC_FREE(pValBuffer);
    }
    else
    {    
        start = pTableDescr->rowStart;
    }

    *pStart = start;
    return status;
}

 

/*-----------------------------------------------------------------------*/

static RLSTATUS 
RCC_RCB_DetermineEnd(cli_env *pCliEnv, TableDescr *pTableDescr, Counter *pEnd)
{
    sbyte      *pValBuffer;
    Length      outputLen;
    RLSTATUS    status = OK;

    if (NULL == pTableDescr->pDynamicEnd)
        *pEnd = pTableDescr->rowEnd;
    else
    {
        if (NULL == (pValBuffer = RC_MALLOC(kTempBufferSize)))
        {
            *pEnd = 0;
            return RCC_ERROR_THROW(ERROR_MEMMGR_NO_MEMORY);
        }

        status = RCC_RCB_ReadValueFromRCB(pCliEnv, pTableDescr->pDynamicEnd, NULL, 
                                          pValBuffer, &outputLen);
        if (OK == status)
            *pEnd = ATOI(pValBuffer);

        RC_FREE(pValBuffer);
    }

    return status;
}



/*-----------------------------------------------------------------------*/
#ifndef	__DISABLE_STRUCTURES__

static RLSTATUS
RCC_RCB_PrepNormalTableToStart(cli_env *pCliEnv, TableDescr *pTableDescr,
                               Counter *pIndex, RLSTATUS *pListStatus)
{
    Counter      idx, start, colEntryNum;
    sbyte       *pTempBuffer;
    Length       outputLen   = 0;
    RLSTATUS     status      = OK;
    RLSTATUS     listStatus  = OK;
    ColumnEntry *column;

    /* init counters */
    MACRO_INDEX(pCliEnv) = 0;

    if (OK != (status = RCC_RCB_DetermineStart(pCliEnv, pTableDescr, &start)))
        return status;

    if (NULL == (pTempBuffer = RC_MALLOC(kMagicMarkupBufferSize)))
        return RCC_ERROR_THROW(ERROR_MEMMGR_NO_MEMORY);

    Cache_FlushTempObjects(pCliEnv->phCacheHandle);
    for (idx = 0; idx < start; idx++)
    {
        colEntryNum = 0;
        column      = pTableDescr->colEntries;
        while ( (OK == status) && 
                (colEntryNum < pTableDescr->numColEntries) )
        {
            status = RCC_RCB_ReadValueFromRCB(pCliEnv, 
                                              column->pRapidMark, 
                                              column->pArgs,
                                              pTempBuffer,
                                              &outputLen);     
            colEntryNum++;
            column++;
        }

        if (OK != status)
        {
            Cache_FlushTempObjects(pCliEnv->phCacheHandle);
            break;
        }

        listStatus = DB_HandleGetNextIndex(pCliEnv);
        Cache_FlushTempObjects(pCliEnv->phCacheHandle);

        if (OK != listStatus)
            break;

        MACRO_INDEX(pCliEnv)++;
    }

    RC_FREE(pTempBuffer);

    *pIndex      = idx;
    *pListStatus = listStatus;

    return status;

} /* RCC_RCB_PrepNormalTableToStart */

#endif	/* ! __DISABLE_STRUCTURES__ */

#ifdef __SNMP_API_ENABLED__

/*-----------------------------------------------------------------------*/

extern RLSTATUS
RCC_RCB_CalculateCreateTableArgLen(TableDescr *pTableDescr, Length *pArgLen )
{
    Length  tempLen = 0;
    Counter i;

    if ( NULL != pTableDescr->pTableName )
        tempLen += STRLEN(pTableDescr->pTableName);

    tempLen++; /* extra ',' */

    if ( NULL != pTableDescr->pLengthInstance )
        tempLen += STRLEN(pTableDescr->pLengthInstance);

    tempLen++; /* extra ',' */

    if ( NULL != pTableDescr->pFirstInstance )
        tempLen += STRLEN(pTableDescr->pFirstInstance);

    tempLen++; /* extra ',' */

    if ( NULL != pTableDescr->pSkipAhead )
        tempLen += STRLEN(pTableDescr->pSkipAhead);

    tempLen++; /* extra ',' */

    if ( NULL != pTableDescr->pFilterRule )
        tempLen += STRLEN(pTableDescr->pFilterRule);

    tempLen++; /* extra ',' */

    if ( NULL != pTableDescr->pFilterType )
        tempLen += STRLEN(pTableDescr->pFilterType);


    if (NULL != pTableDescr->pCreateTableArgList)
    {
        tempLen += (STRLEN(pTableDescr->pCreateTableArgList) + 1); /* +1 for leading comma */
    }
    else
    {
        for (i = 0; i < pTableDescr->numColEntries; i++)
        {
            if ( NULL != pTableDescr->colEntries[i].pRapidMark )
                tempLen += STRLEN(pTableDescr->colEntries[i].pRapidMark);

            tempLen++;
        }
    }

    /* Little extra buffer */
    tempLen += 16;
    *pArgLen = tempLen;

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS
RCC_RCB_BuildCreateTableArg(TableDescr *pTableDescr, sbyte *pArg )
{
    Counter i;

    sprintf(pArg, "(%s,%s,%s,%s,%s,%s,",  pTableDescr->pTableName,
                                            pTableDescr->pLengthInstance,
                                            pTableDescr->pFirstInstance,
                                            pTableDescr->pSkipAhead,
                                            pTableDescr->pFilterRule,
                                            pTableDescr->pFilterType );
    
    if (NULL == pTableDescr->pCreateTableArgList)
    {
        /* Last column has a ")" terminator vs. a "," separator */
        for ( i = 0; i < (pTableDescr->numColEntries - 1); i++ )
        {
            STRCAT(pArg,pTableDescr->colEntries[i].pRapidMark);
            STRCAT(pArg,",");
        }

        STRCAT(pArg,pTableDescr->colEntries[i].pRapidMark);
    }
    else
    {
        STRCAT(pArg, pTableDescr->pCreateTableArgList);
    }

    STRCAT(pArg,")");

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS
RCC_RCB_BuildOtherTableArg(TableDescr *pTableDescr, sbyte *pOtherTableArgs )
{
	sprintf(pOtherTableArgs, "(%s)", pTableDescr->pTableName);

    return OK;
}



/*-----------------------------------------------------------------------*/

static RLSTATUS
RCC_RCB_PrepSNMPTableToStart(cli_env *pCliEnv, TableDescr *pTableDescr,
                             Counter *pIndex, RLSTATUS *pListStatus,
                             sbyte *pCreateTableArg, sbyte *pOtherTableArg)
{
    Counter      start;
    RLSTATUS     status = OK;

    /* init counters */
    MACRO_INDEX(pCliEnv) = 0;

    RCC_RCB_BuildOtherTableArg(pTableDescr, pOtherTableArg);
    RCC_RCB_BuildCreateTableArg(pTableDescr, pCreateTableArg);

    status = RCC_RCB_GetMetaRapidMark(pCliEnv, "createTable", pCreateTableArg );
    if ( OK > status )
        return status;
   
    status = RCC_RCB_DetermineStart(pCliEnv, pTableDescr, &start);
    if ( OK > status )
        return status;

    *pIndex      = start;
    *pListStatus = OK;

    return OK;

} /* RCC_RCB_PrepSNMPTableToStart */

#endif /* __SNMP_API_ENABLED__ */


/*-----------------------------------------------------------------------*/
#ifndef	__DISABLE_STRUCTURES__ 

static RLSTATUS
RCC_RCB_IterateTable(cli_env *pCliEnv,  TableDescr *pTableDescr,
                     Counter  startIndex,    sbyte *pOutputBuffer,
                     Length  *pOutputLen,    sbyte *pColSeparator, 
                     sbyte   *pRowSeparator, sbyte *pOtherTableArgs)
{
    sbyte       *pOutputBufferMarker;
    Counter      colEntryNum, idx, end;
    sbyte4       outputLen;
    RLSTATUS     status = OK;
    ColumnEntry *pColumn;

#ifdef __SNMP_API_ENABLED__
    Boolean     boolMoreData = FALSE;
#endif

    pOutputBufferMarker = pOutputBuffer;
    idx                 = startIndex;

    if (OK != (status = RCC_RCB_DetermineEnd(pCliEnv, pTableDescr, &end)))
        return status;

    for (; idx <= end; idx++)
    {
        for (colEntryNum = 0, pColumn = pTableDescr->colEntries;
             colEntryNum < pTableDescr->numColEntries;
             colEntryNum++, pColumn++)
        {
            /* skip "invisible" column */

            if (pColumn->width < 0)
                continue;

            status = RCC_RCB_ReadValueFromRCB(pCliEnv, 
                                              pColumn->pRapidMark, 
                                              pColumn->pArgs,
                                              pOutputBuffer,
                                              (Length *) &outputLen);     

            /* Write out the separator between each column entry */
            if ( OK > status )
                break;

            RCC_EXT_PrintString(pCliEnv, pOutputBuffer, pColumn->width, ' ');
            RCC_EXT_Write(pCliEnv, " ", 1);

        }

        if (OK > status)
            break;

        /* Increment to next iteration, using appropriate method */
        if (! pTableDescr->bIsSNMP)
        {
            if (idx < end)
			    status = DB_HandleGetNextIndex(pCliEnv);

            Cache_FlushTempObjects(pCliEnv->phCacheHandle);

            if (OK != status)
                break;
        }
#ifdef __SNMP_API_ENABLED__
        else                /* SNMP repeat */
        {
            /* simulate EndRow MM  */
		    RCC_RCB_GetMetaRapidMark(pCliEnv, "endRow", pOtherTableArgs);

            if (OK != OCSNMP_MoreDataInTable(pCliEnv, pTableDescr->pTableName, &boolMoreData))
            {
                RCC_EXT_WriteStrLine(pCliEnv, "");
                MACRO_INDEX(pCliEnv)++;
                break;
            }

            if (! boolMoreData)
            {
                RCC_EXT_WriteStrLine(pCliEnv, "");
                MACRO_INDEX(pCliEnv)++;
                break;
            }

        }
#endif /* __SNMP_API_ENABLED__ */

        RCC_EXT_WriteStrLine(pCliEnv, "");
        MACRO_INDEX(pCliEnv)++;

    } /* end for (; Index <= End; Index++) */


#ifdef __SNMP_API_ENABLED__
    if (pTableDescr->bIsSNMP)
    {
        /* simulate EndTable MM  */
        RCC_RCB_GetMetaRapidMark(pCliEnv, "endTable", pOtherTableArgs);
    }
#endif

    *pOutputLen = (Length) (pOutputBuffer - pOutputBufferMarker);

    return status;
} 


#endif	/* ! __DISABLE_STRUCTURES__ */

/*-----------------------------------------------------------------------*/

/* this is needed if __ENABLE_MEMMGR_DEBUG__ is defined 
   -- have to pass a free() in arg list and debug version 
   doesn't match. This is just a convenience wrapper */

static void RCC_RCB_Free(void *pBuf)
{
	RC_FREE(pBuf);
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS
RCC_RCB_PrintTable(cli_env *pCliEnv, TableDescr *pTableDescr)
{
    sbyte       *pTable;
    sbyte       *pNewline;
    sbyte       *pTitle;
    sbyte4       columns = pTableDescr->numColEntries;
    sbyte4       width;
    sbyte4       tempWidth;
    sbyte4       loop;
    sbyte4       lines;
    sbyte4       titleLines;
    Length       outputLen;
    ColumnEntry *pEntry;
    RLSTATUS     status;
        
    if (0 >= columns)
        return RCC_ERROR_THROW(RCC_ERROR);

    if ( NULL == (pTable = (sbyte*) RC_MALLOC(kOUTPUT_BUFFER_SIZE)))
        return RCC_ERROR_THROW(ERROR_MEMMGR_NO_MEMORY);

    titleLines = 0;

    /* get widths of titles */
    pEntry = pTableDescr->colEntries;
    for (loop = 0; loop < columns; loop++, pEntry++)
    {
        /* initialize printing offset */
        pEntry->pOffset = pEntry->pTitle;

        if (NULL == pEntry->pTitle)
            continue;

        width  = 0;
        lines  = 1;
        pTitle = pEntry->pTitle;
        while (NULL != (pNewline = STRCHR(pTitle, '\n')))
        {
            tempWidth = pNewline - pTitle;
            if (tempWidth > width)
                width = tempWidth;

            pTitle = ++pNewline;
            lines++;
        }
        if (0 >= width)
            width = STRLEN(pEntry->pTitle);

        if (0 == pEntry->width)
                pEntry->width = width;

        if (titleLines < lines)
            titleLines = lines;
    }

    /* print titles */
    if (0 < titleLines)
    {
        for (lines = 0; lines < titleLines; lines++)
        {
            if (0 < lines)
                RCC_EXT_WriteStrLine(pCliEnv, "");

            pEntry = pTableDescr->colEntries;

            for (loop = 0; loop < columns; loop++, pEntry++)
            {
                if (0 > pEntry->width)
                    continue;

                width  = pEntry->width;
                pTitle = pEntry->pOffset;

                if (NULL == pTitle)
                    RCC_EXT_PrintString(pCliEnv, "", width + 1, ' ');
                else
                {
                    pNewline = STRCHR(pTitle, '\n');
                    if (NULL != pNewline)
                    {
                        tempWidth = pNewline - pTitle;
                        if (tempWidth < (width + 1))
                        {
                            RCC_EXT_Write(pCliEnv, pTitle, tempWidth);
                            RCC_EXT_PrintString(pCliEnv, "", 
                                                width - tempWidth + 1, ' ');
                        }
                        else
                            RCC_EXT_PrintString(pCliEnv, pTitle, width + 1, ' ');
                    }
                    else
                        RCC_EXT_PrintString(pCliEnv, pTitle, width + 1, ' ');

                    pEntry->pOffset = (NULL == pNewline) ? NULL : ++pNewline;
                }
            }
        }
        RCC_EXT_WriteStrLine(pCliEnv, "");

        pEntry = pTableDescr->colEntries;
        for (loop = 0; loop < columns; loop++, pEntry++)
        {
            if (0 > pEntry->width)
                continue;

            RCC_EXT_PrintString(pCliEnv, "", pEntry->width, '-');
            RCC_EXT_Write(pCliEnv, " ", 1);
        }
        RCC_EXT_WriteStrLine(pCliEnv, "");
    }

    status = RCC_RCB_ReadTableFromRCB(pCliEnv, pTableDescr, pTable, 
                                      &outputLen, kRCC_COL_SEPARATOR, 
                                      kRCC_ROW_SEPARATOR);

    FREEMEM(pTable);
    return status;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS
RCC_RCB_ReadTableFromRCB(cli_env *pCliEnv, TableDescr *pTableDescr, sbyte *pOutputBuffer,
                         Length *pOutputLen, sbyte *pColSeparator, sbyte *pRowSeparator)
{
#ifdef __DISABLE_STRUCTURES__
    RCC_EXT_WriteStrLine(pCliEnv, kRCC_MSG_NO_STRUCTS);
    return OK;
#else

    RLSTATUS     status             = OK;
    RLSTATUS     ListStatus         = OK;
    Counter      Index              = 0;
    sbyte       *pOtherTableArgs    = NULL;

#ifndef __MACRO_REPEAT_NEST_FULL__
    list        *listIndex = pCliEnv->p_lstIndexedValues;
#else	/* __MACRO_REPEAT_NEST_FULL__ */
    list        *listIndex = &(pCliEnv->p_MacroLstIndexedValues[p_envVar->Macro_NestDepth]);
#endif	/* __MACRO_REPEAT_NEST_FULL__ */

#ifdef __SNMP_API_ENABLED__
    sbyte       *pCreateTableArg    = NULL;
    Length       createTableArgLen  = 0;

    RCC_RCB_CalculateCreateTableArgLen(pTableDescr, &createTableArgLen);
    if (0 >= createTableArgLen)
        return OK;

    if (NULL == (pCreateTableArg = RC_MALLOC(createTableArgLen)))
        status = ERROR_MEMMGR_NO_MEMORY;
    else
    {
        pOtherTableArgs = RC_MALLOC(STRLEN(pTableDescr->pTableName) + 3);
        if (NULL == pOtherTableArgs)
            status = ERROR_MEMMGR_NO_MEMORY;
    }
#endif

    if (OK == status)
    {
        if (! pTableDescr->bIsSNMP)
        {   
            status = RCC_RCB_PrepNormalTableToStart( pCliEnv, pTableDescr,
                                                     &Index, &ListStatus );
        }
#ifdef __SNMP_API_ENABLED__
        else
        { 
            status = RCC_RCB_PrepSNMPTableToStart( pCliEnv, pTableDescr,
                                                   &Index, &ListStatus,
                                                   pCreateTableArg, pOtherTableArgs);
        }
#endif
    }

    if ((OK == status) && (OK == ListStatus))
    {
        status = RCC_RCB_IterateTable( pCliEnv, pTableDescr,
                                       Index, pOutputBuffer, pOutputLen,
                                       pColSeparator, pRowSeparator, 
                                       pOtherTableArgs);
    }

#ifdef __SNMP_API_ENABLED__        
    FREEMEM(pOtherTableArgs);
    FREEMEM(pCreateTableArg);
#endif

    /* clear the index list */
    if (NULL != listIndex)
        List_Destruct(&listIndex, (void(*)(void *))RCC_RCB_Free);

#ifndef __MACRO_REPEAT_NEST_FULL__
    pCliEnv->p_lstIndexedValues = List_Construct();
#else	/* __MACRO_REPEAT_NEST_FULL__ */
#error
    pCliEnv->p_MacroLstIndexedValues[p_envVar->Macro_NestDepth] = List_Construct();
#endif	/* __MACRO_REPEAT_NEST_FULL__ */

    MACRO_INDEX(pCliEnv) = 0;

    return status;

#endif /* ! __DISABLE_STRUCTURES__ */

} /* RCC_RCB_ReadTableFromRCB */

#endif /* __RCC_ENABLED__ */

