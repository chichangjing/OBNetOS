/*
 *  rc_enum.h
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

/*
$History: rc_enum.h $
 * 
 * *****************  Version 10  *****************
 * User: Pstuart      Date: 12/11/00   Time: 11:30a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added c++ wrappers for prototypes
 * 
 * *****************  Version 9  *****************
 * User: James        Date: 9/07/00    Time: 11:14a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added support for reverse enum lookups. JAB
 * 
 * *****************  Version 8  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:25p
 * Updated in $/Rapid Logic/Code Line/rli_code/mibway/include
 * Include history and enable auto archiving feature from VSS


*/


#ifndef __ENUM_HEADER__
#define __ENUM_HEADER__

#ifdef __IN_ENUM_C__

/* !-!-!-!-!-!-! following will only be included if in the file rc_enum.c */

#define ENUM_LIST_LENGTH(x)     (sizeof(x) / sizeof(enumStruct))
#define kEnumLookupTableSize    (sizeof(enumTableList) / sizeof(enumTableLookupStruct))
 
typedef struct enumStruct
{
    sbyte4    enumValue;
    sbyte*    p_enumLabel;
 
} enumStruct;
 
typedef struct enumTableLookupStruct
{
    sbyte*      p_TableName;
    enumStruct* p_enumStruct;
    sbyte4      enumListLength;
 
} enumTableLookupStruct;

#endif /* __IN_ENUM_C__ */

#ifdef __cplusplus
extern "C" {
#endif
 
extern void ENUM_DisplayLabel(environment *p_envVar, void *pHtmlOutputBuf,
                              void *pDataObjectInC, sbyte *pHtmlArgs);

extern RLSTATUS ENUM_ConvertLabelToIndex(sbyte *pEnumTableName, sbyte *pEnumLabel, 
                                         sbyte4 *pRetEnumValue);

extern void ENUM_DropDownControl(environment *p_envVar, void *pHtmlOutputBuf,
                                 void *pDataObjectInC, sbyte *pHtmlArgs);

extern void ENUM_PartialDropDownControl(environment *p_envVar, void *pHtmlOutputBuf,
                                        void *pDataObjectInC, sbyte *pHtmlArgs);


#ifdef __cplusplus
}
#endif

#endif /* __ENUM_HEADER__ */
