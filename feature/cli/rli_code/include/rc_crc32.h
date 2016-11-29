/*  
 *  rc_crc32.h
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
 * *****************  Version 5  *****************
 * Updated by Epeterson    on 4/25/00
 * Include history and enable auto archiving feature from VSS
*/




/* This header was created by Kedron Wolcott. */
#ifndef __CRC32_HEADER__   /* prevent multiple inclusions */
#define __CRC32_HEADER__

#if defined(__DECOMPRESSION_ENABLED__) || defined(__JAVACONTROL_ENABLED__) || defined(__OCP_ENABLED__)

#define kCrcInitialValue    0

#ifdef __cplusplus
extern "C" {
#endif

extern  RLSTATUS    CRC32_Init  (void                                   );
extern  ulg         crc32       (ulg crc, ZCONST uch *buf, extent len   );   

#ifdef __cplusplus
}
#endif


#endif /* __DECOMPRESSION_ENABLED__ */


#endif /* !__CRC32_HEADER__ */
