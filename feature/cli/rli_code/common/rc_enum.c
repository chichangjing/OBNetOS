/*
 *  rc_enum.c
 *
 *  This is a part of the OpenControl SDK source code library.
 *
 *  Copyright (C) 1998 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */

/*----------------------------------------------------------------------
 *
 * NAME CHANGE NOTICE:
 *
 * On May 11th, 1999, Rapid Logic changed its corporate naming scheme.
 * The changes are as follows:
 *
 *      OLD NAME                        NEW NAME
 *
 *      OpenControl                     RapidControl
 *      WebControl                      RapidControl for Web
 *      JavaControl                     RapidControl for Applets
 *      MIBway                          MIBway for RapidControl
 *
 *      OpenControl Backplane (OCB)     RapidControl Backplane (RCB)
 *      OpenControl Protocol (OCP)      RapidControl Protocol (RCP)
 *      MagicMarkup                     RapidMark
 *
 * The source code portion of our product family -- of which this file 
 * is a member -- will fully reflect this new naming scheme in an upcoming
 * release.
 *
 *
 * RapidControl, RapidControl for Web, RapidControl Backplane,
 * RapidControl for Applets, MIBway, RapidControl Protocol, and
 * RapidMark are trademarks of Rapid Logic, Inc.  All rights reserved.
 *
 */



#include "rc_options.h"

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_database.h"

#ifdef __SNMP_API_ENABLED__
#include "rcm_mibway.h"
#endif /* __SNMP_API_ENABLED__ */

#ifdef __WEBCONTROL_BLADE_ENABLED__
#include <stdio.h>
#endif /* __WEBCONTROL_BLADE_ENABLED__ */

#define __IN_ENUM_C__

#include "rc_enum.h"
#include "rc_enum_tab.h"

#define kMaxArgLen              80



/*-----------------------------------------------------------------------*/

#define kArgOpen    '('
#define kArgDelim   ','
#define kArgClose   ')'

/* assumes "(0,1,2)" argument structure */
extern RLSTATUS OCBTOOLS_RetrieveArgument(char *pArgString, int Arg,
                                          char *pArgRetBuf, int ArgRetBufLen)
{
    /* relocated function, previously resided in rcm_snmp.c. JAB */
    sbyte4  stringIndex  = 0;
    sbyte4  stringLength, argWriteIndex;
    Counter ArgNum = Arg;

    if ((NULL == pArgRetBuf) || (0 > Arg))
        return ERROR_GENERAL_NO_DATA;

    *pArgRetBuf = '\0';

    if (NULL == pArgString)
        return ERROR_GENERAL_NO_DATA;

    if (0 >= (--ArgRetBufLen))
        return ERROR_GENERAL_ACCESS_DENIED;

    stringLength = STRLEN(pArgString);

    /* determine length:  search for closing ')' */
    while ((0 < stringLength) && (kArgClose != pArgString[stringLength]))
        stringLength--;

    if (0 == stringLength)
        return ERROR_GENERAL_NO_DATA;

    /* determine start:  skip past leading '(' */
    while (stringIndex < stringLength)
    {
        stringIndex++;

        if (kArgOpen == pArgString[stringIndex - 1])
            break;
    }

    /* sync to start of arg */
    while (0 < ArgNum)
    {
        while (stringIndex < stringLength)
        {
            stringIndex++;

            if (kArgDelim == pArgString[stringIndex - 1])
                break;
        }

        if (stringIndex == stringLength)
            return OK;

        ArgNum--;
    }

    argWriteIndex = 0;

    while ((stringIndex < stringLength) && (kArgDelim != pArgString[stringIndex]))
    {
        if (argWriteIndex < ArgRetBufLen)
            pArgRetBuf[argWriteIndex++] = pArgString[stringIndex++];
        else break;
    }

    pArgRetBuf[argWriteIndex] = '\0';

    return OK;

} /* OCBTOOLS_RetrieveArgument */



/*-----------------------------------------------------------------------*/
 
static enumStruct *FindEnumList(sbyte *p_TableName, sbyte4 *p_enumListLength)
{
    sbyte4 index;

    if ((NULL == p_TableName) || (NULL == p_enumListLength) )
        return NULL;

    *p_enumListLength = 0;

    /*!!!!!!!!!!!! should use a binary search here !!!!!!!!!!!!!!*/
    for (index = 0; kEnumLookupTableSize > index; index++)
        if (0 == STRCMP(p_TableName, enumTableList[index].p_TableName) )
        {
            *p_enumListLength =  enumTableList[index].enumListLength;

            return enumTableList[index].p_enumStruct;
        }

    /* table label not found! */    
    return NULL;
}
 
 
 
/*-----------------------------------------------------------------------*/
 
extern void ENUM_DisplayLabel(environment *p_envVar, void *pHtmlOutputBuf,
                              void *pDataObjectInC, sbyte *pHtmlArgs)
{
    /* usage: <html>$%enum(tablename,magicMarkup)#$</html> */
    /* or enum(tablename,magicMarkup) */
    /* strategy: gets the MM value and then converts to an enumeration */

    sbyte*      pArg = RC_MALLOC(kMaxArgLen + 1);
    sbyte*      pMM  = RC_MALLOC(kMaxArgLen + 1);
    sbyte*      pStartOfArg;
    enumStruct* p_enumStruct;
    sbyte4      index, enumListLength, enumValue;

    if ((NULL == pArg) || (NULL == pMM))
        goto enum_error;

    if (OK != OCBTOOLS_RetrieveArgument(pHtmlArgs, 1, pMM, kMaxArgLen) )
        goto enum_error;

    pStartOfArg = pMM;

    while (TRUE == ISALPHANUMERIC(*pStartOfArg))
        pStartOfArg++;

    /* divide string into magic markup and arg components: */
    STRCPY(pArg, pStartOfArg);
    *pStartOfArg = '\0';

    if (OK != DB_ReadData(p_envVar, pMM, pHtmlOutputBuf, pArg))
        goto enum_error;

    if (FALSE == ISDIGIT(*((sbyte *)pHtmlOutputBuf)))
        goto enum_error;

    RC_FREE(pMM);  pMM  = NULL;

    if (OK != OCBTOOLS_RetrieveArgument(pHtmlArgs, 0, pArg, kMaxArgLen) )
        goto enum_error;

    enumValue = ATOI(pHtmlOutputBuf);

    if ((NULL == (p_enumStruct = FindEnumList(pArg, &enumListLength))) ||
        (0 >= enumListLength) )
    {
        goto enum_error;
    }

    RC_FREE(pArg); pArg = NULL;

    for (index = 0; enumListLength > index; index++)
        if (enumValue == p_enumStruct[index].enumValue)
            STRCPY(pHtmlOutputBuf, p_enumStruct[index].p_enumLabel);

enum_error:

    if (NULL != pArg)
        RC_FREE(pArg);

    if (NULL != pMM)
        RC_FREE(pMM);

} /* ENUM_DisplayLabel */
 
 
 
/*-----------------------------------------------------------------------*/
 
extern RLSTATUS ENUM_ConvertLabelToIndex(sbyte *pEnumTableName, sbyte *pEnumLabel, 
                                         sbyte4 *pRetEnumValue)
{
    /* reverse of ENUM_DisplayLabel: takes an ascii label, locates in an EnumTable and returns the (integer) EnumValue */
    enumStruct* p_enumStruct;
    sbyte4      index, enumListLength;
    RLSTATUS    status;

    status = ERROR_GENERAL_NULL_POINTER;

    if ((NULL == pEnumTableName) || (NULL == pEnumLabel) || (NULL == pRetEnumValue))
        goto enum_error;

    status = ERROR_GENERAL_NOT_FOUND;

    if ((NULL == (p_enumStruct = FindEnumList(pEnumTableName, &enumListLength))) ||
        (0 >= enumListLength) )
    {
        goto enum_error;
    }

    for (index = 0; enumListLength > index; index++)
        if (0 == STRCMP(p_enumStruct[index].p_enumLabel, pEnumLabel))
        {
            *pRetEnumValue = p_enumStruct[index].enumValue;
            return OK;
        }

enum_error:

    return status;
}
 
 
 
/*-----------------------------------------------------------------------*/

#ifdef __WEBCONTROL_BLADE_ENABLED__
#ifdef __SNMP_API_ENABLED__

extern void ENUM_DropDownControl(environment *p_envVar, void *pHtmlOutputBuf,
                                 void *pDataObjectInC, sbyte *pHtmlArgs)
{
    /* usage: <html>$%enumDropDown(htmlNameField,      value to place in the name field
                                   enumTableName,      enum table to use (see: rc_enum_tab.h)
                                   magicMarkup,        the MM to use for selects (optional)
                                  )#$</html> */

    sbyte*      pArg = RC_MALLOC(kMaxArgLen + 1);
    sbyte*      pMM  = RC_MALLOC(kMaxArgLen + 1);
    sbyte*      pStartOfArg;
    sbyte*      pHtmlOutputBufferMarker;
    enumStruct* p_enumStruct;
    sbyte4      index, enumListLength, IndexValue, selectedEnumValue;

    if ((NULL == pArg) || (NULL == pMM))
        goto enum_error;

    pHtmlOutputBufferMarker = (sbyte*)pHtmlOutputBuf;

    if ((OK   == OCBTOOLS_RetrieveArgument(pHtmlArgs, 2, pMM, kMaxArgLen)) &&
        ('\0' != *pMM) )
    {
        pStartOfArg = pMM;

        while (TRUE == ISALPHANUMERIC(*pStartOfArg))
            pStartOfArg++;

        /* divide string into magic markup and arg components: */
        STRCPY(pArg, pStartOfArg);
        *pStartOfArg = '\0';

        if (OK != DB_ReadData(p_envVar, pMM, pHtmlOutputBufferMarker, pArg))
            goto enum_error;

        if (FALSE == ISDIGIT(*(pHtmlOutputBufferMarker)))
            goto enum_error;

        IndexValue = ATOI(pHtmlOutputBufferMarker);
    }
    else IndexValue = -1;

    if (OK != OCBTOOLS_RetrieveArgument(pHtmlArgs, 1, pArg, kMaxArgLen) )
        goto enum_error;

    if ((NULL == (p_enumStruct = FindEnumList(pArg, &enumListLength))) ||
        (0 >= enumListLength) )
    {
        goto enum_error;
    }

    if ((OK   == OCBTOOLS_RetrieveArgument(pHtmlArgs, 0, pMM, kMaxArgLen)) &&
        ('\0' != *pMM) )
    {
        Boolean boolTemp;

        pStartOfArg = pMM;

        while (TRUE == ISALPHANUMERIC(*pStartOfArg))
            pStartOfArg++;

        /* divide string into magic markup and arg components: */
        STRCPY(pArg, pStartOfArg);
        *pStartOfArg = '\0';

        *(pHtmlOutputBufferMarker) = '\0';
        OCSNMP_BuildFullMibName(p_envVar, pHtmlOutputBufferMarker, pMM, pArg, &boolTemp);

        STRCPY(pArg, pHtmlOutputBufferMarker);
    }
    else goto enum_error;

    sprintf(pHtmlOutputBufferMarker, "<select name=\"%s\">\n", pArg);

    RC_FREE(pMM);  pMM  = NULL;
    RC_FREE(pArg); pArg = NULL;

    pHtmlOutputBufferMarker += STRLEN(pHtmlOutputBufferMarker);

    for (index = 0; enumListLength > index; index++)
    {
        selectedEnumValue = p_enumStruct[index].enumValue;

        if (selectedEnumValue == IndexValue)
            sprintf(pHtmlOutputBufferMarker, "<option selected value=\"%d\">%s</option>\n",
                        (int)selectedEnumValue, p_enumStruct[index].p_enumLabel);
        else
            sprintf(pHtmlOutputBufferMarker, "<option value=\"%d\">%s</option>\n", 
                        (int)selectedEnumValue, p_enumStruct[index].p_enumLabel);

        pHtmlOutputBufferMarker += STRLEN(pHtmlOutputBufferMarker);
    }

    sprintf(pHtmlOutputBufferMarker, "</select>\n");

enum_error:

    if (NULL != pArg)
        RC_FREE(pArg);

    if (NULL != pMM)
        RC_FREE(pMM);
}
 
 
 
/*-----------------------------------------------------------------------*/

extern void ENUM_PartialDropDownControl(environment *p_envVar, void *pHtmlOutputBuf,
                                        void *pDataObjectInC, sbyte *pHtmlArgs)
{
    /* usage:  <html>
               <select name="xyz">
                    $%enumNamelessDropDown(enumTableName,      enum table to use (see: rc_enum_tab.h)
                                           magicMarkup,        the MM to use for selects (optional)
                                          )#$
               </select>
               </html> */

    sbyte*      pArg = RC_MALLOC(kMaxArgLen + 1);
    sbyte*      pMM  = RC_MALLOC(kMaxArgLen + 1);
    sbyte*      pStartOfArg;
    sbyte*      pHtmlOutputBufferMarker;
    enumStruct* p_enumStruct;
    sbyte4      index, enumListLength, IndexValue, selectedEnumValue;

    if ((NULL == pArg) || (NULL == pMM))
        goto enum_error;

    pHtmlOutputBufferMarker = (sbyte*)pHtmlOutputBuf;

    if ((OK   == OCBTOOLS_RetrieveArgument(pHtmlArgs, 1, pMM, kMaxArgLen)) &&
        ('\0' != *pMM) )
    {
        pStartOfArg = pMM;

        while (TRUE == ISALPHANUMERIC(*pStartOfArg))
            pStartOfArg++;

        /* divide string into magic markup and arg components: */
        STRCPY(pArg, pStartOfArg);
        *pStartOfArg = '\0';

        if (OK != DB_ReadData(p_envVar, pMM, pHtmlOutputBufferMarker, pArg))
            goto enum_error;

        if (FALSE == ISDIGIT(*(pHtmlOutputBufferMarker)))
            goto enum_error;

        IndexValue = ATOI(pHtmlOutputBufferMarker);
    }
    else IndexValue = -1;

    if (OK != OCBTOOLS_RetrieveArgument(pHtmlArgs, 0, pArg, kMaxArgLen) )
        goto enum_error;

    if ((NULL == (p_enumStruct = FindEnumList(pArg, &enumListLength))) ||
        (0 >= enumListLength) )
    {
        goto enum_error;
    }

    RC_FREE(pMM);  pMM  = NULL;
    RC_FREE(pArg); pArg = NULL;

    for (index = 0; enumListLength > index; index++)
    {
        selectedEnumValue = p_enumStruct[index].enumValue;

        if (selectedEnumValue == IndexValue)
            sprintf(pHtmlOutputBufferMarker, "<option selected value=\"%d\">%s</option>\n",
                        (int)selectedEnumValue, p_enumStruct[index].p_enumLabel);
        else
            sprintf(pHtmlOutputBufferMarker, "<option value=\"%d\">%s</option>\n", 
                        (int)selectedEnumValue, p_enumStruct[index].p_enumLabel);

        pHtmlOutputBufferMarker += STRLEN(pHtmlOutputBufferMarker);
    }

enum_error:

    if (NULL != pArg)
        RC_FREE(pArg);

    if (NULL != pMM)
        RC_FREE(pMM);
}

#endif /* __SNMP_API_ENABLED__ */

#else /* __WEBCONTROL_BLADE_ENABLED__ */

extern void ENUM_DropDownControl(environment *p_envVar, void *pHtmlOutputBuf,
                                 void *pDataObjectInC, sbyte *pHtmlArgs)
{
    /* only applicable to RCW */
    return;
}

extern void ENUM_PartialDropDownControl(environment *p_envVar, void *pHtmlOutputBuf,
                                        void *pDataObjectInC, sbyte *pHtmlArgs)
{
    /* only applicable to RCW */
    return;
}


#endif /* __WEBCONTROL_BLADE_ENABLED__ */
