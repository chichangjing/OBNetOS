/*  
 *  vd_basic.h
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


#ifndef __VD_BASIC_HEADER__
#define __VD_BASIC_HEADER__

extern int         gGlobalDecimalInteger;
extern long        gGlobalHexadecimalInteger;

extern IpAddress   gDeviceIpAddress;
extern IpAddress   gDeviceNetMask;


extern void VD_BASIC_ReadHexadecimalVariable(environment *pEnv, 
                                             void *pHtmlOutputBuf, 
                                             void *pObject, char *pArgs);
extern void VD_BASIC_ReadIpAddress          (environment *pEnv, 
                                             void *pHtmlOutputBuf, 
                                             void *pObject, char *pArgs);
extern void VD_BASIC_ReadDefaultGateway     (environment *pEnv, 
                                             void *pHtmlOutputBuf, 
                                             void *pObject, char *pArgs);
extern void VD_BASIC_ReadPrivateVariable    (environment *pEnv, 
                                             void *pHtmlOutputBuf, 
                                             void *pObject, char *pArgs);
extern void VD_BASIC_Init                   ( void *pArg );
extern void VD_CLIPromptExampleRead(environment *pEnv, void *pOutputBuf, 
                                            void *pObject, sbyte *pArgs);

extern RLSTATUS VD_CLIPromptExampleWrite(environment *pEnv, void *pDest, 
                                            void *pInputBuf, sbyte *pArgs, ...);


#endif /* __VD_BASIC_HEADER__ */
