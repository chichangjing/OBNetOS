/*  
 *  rcc_rcb.h
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

#ifndef __RCC_RCB_HEADER__
#define __RCC_RCB_HEADER__


/*-----------------------------------------------------------------------*/
/* Table Structure 
 *
 * These structures are used to transfer information about a table of data
 * to the appropriate RCB abstraction function
 */

#define kMaxNumColEntries   15

typedef struct ColumnEntry
{
    sbyte   *pRapidMark;
    sbyte   *pArgs;
    sbyte   *pTitle;
    sbyte4   width;
    sbyte   *pOffset;
} ColumnEntry;

typedef struct TableDescr
{
    Boolean         bIsSNMP;

#ifdef __SNMP_API_ENABLED__
    sbyte           *pTableName;
    sbyte           *pLengthInstance;
    sbyte           *pFirstInstance;
    sbyte           *pSkipAhead;
    sbyte           *pFilterRule;
    sbyte           *pFilterType;
    sbyte           *pCreateTableArgList;
#endif

    sbyte           *pDynamicStart;
    sbyte           *pDynamicEnd;
    Counter         rowStart;
    Counter         rowEnd;
    Counter         numColEntries;
    ColumnEntry     colEntries[kMaxNumColEntries];

} TableDescr;



/*-----------------------------------------------------------------------*/
/* Prototypes */

#ifdef __cplusplus
extern "C" {
#endif

extern RLSTATUS
RCC_RCB_WriteValueToRCB(struct cli_env *pCliEnv, sbyte *pRapidMark, sbyte *pArgs, sbyte *pVal );

extern RLSTATUS
RCC_RCB_ReadValueFromRCB(struct cli_env *pCliEnv, sbyte *pRapidMark, 
                         sbyte *pArgs, sbyte *pOutputBuffer,
                         Length *pOutputLen );

extern RLSTATUS
RCC_RCB_OverloadWriteToRCB(struct cli_env *pCliEnv, sbyte *pSetThis, sbyte *pInputBuffer);

extern RLSTATUS
RCC_RCB_OverloadReadFromRCB(struct cli_env *pCliEnv, sbyte *pGetThis,
                                  sbyte *pOutputBuffer, Length *pOutputLen);

extern RLSTATUS
RCC_RCB_ReadTableFromRCB(struct cli_env *pCliEnv, TableDescr *pTableDescr, sbyte *pOutputBuffer, 
                         Length *pOutputLen, sbyte *pColSeparator, sbyte *pRowSeparator);

extern RLSTATUS
RCC_RCB_PrintTable(struct cli_env *pCliEnv, TableDescr *pTableDescr);

#ifdef __cplusplus
}
#endif

#endif /* __RCC_RCB_HEADER__ */
