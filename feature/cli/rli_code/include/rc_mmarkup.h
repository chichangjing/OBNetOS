/*  
 *  rc_mmarkup.h
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

  $History: rc_mmarkup.h $
 * 
 * *****************  Version 8  *****************
 * User: Dreyna       Date: 7/30/00    Time: 10:41p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Fixed error for RM end code definition, and now fully supports
 * re-definable RapidMark delimiters
 * 
 * *****************  Version 7  *****************
 * User: Dreyna       Date: 7/25/00    Time: 11:20p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Support for variable RapidMark delimiters
 * 
 * *****************  Version 6  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Include history and enable auto archiving feature from VSS

  */
#ifndef __MMARKUP_HEADER__
#define __MMARKUP_HEADER__

/*
 *-------------------------------------------------------------------------------
 *
 * Relevant #defines.
 *
 */

/* MagicMarkup delimiters */
#ifdef __RL_FIXED_MM_DELIM__
#ifdef  __BRACKET_DELIMS__
#define kMagicMarkupStart                       (('[' * 0x100) + '[')
#define kMagicMarkupEnd                         ((']' * 0x100) + ']')
#else  /* default delimitators */
#define kMagicMarkupStart                       (('$' * 0x100) + '%')
#define kMagicMarkupEnd                         (('#' * 0x100) + '$')
#endif
#else
#define kMagicMarkupStart                       ((kMagicMarkupStartChar0 * 0x100) + kMagicMarkupStartChar1)
#define kMagicMarkupStartLen					2
#define kMagicMarkupEnd                         ((kMagicMarkupEndChar0   * 0x100) + kMagicMarkupEndChar1  )
#define kMagicMarkupEndLen						2
#endif /* __RL_FIXED_MM_DELIM__ */

#define kSizeofMagicMarkupStart                 sizeof(sbyte2)
#define kSizeofMagicMarkupEnd                   sizeof(sbyte2)

#ifdef  __SNMP_API_ENABLED__
#define kMaxArgsLen                             600
#else
#define kMaxArgsLen                             40
#endif

#define kMaxMagicMarkupIdLen                    (40 + kMaxArgsLen)
#define kMaxMagicMarkupSubstitutionValueLen     kMagicMarkupBufferSize

#ifdef __MACRO_REPEAT_NEST__
#define kMaxMagicMarkupEndLen                   (40 + 40)
#endif

#endif
