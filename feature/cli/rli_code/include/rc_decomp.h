/*  
 *  rc_decomp.h
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







#ifndef __DECOMP_HEADER__
#define __DECOMP_HEADER__

#ifdef __DECOMPRESSION_ENABLED__
/* Prototypes */

#ifdef __cplusplus
extern "C" {
#endif

extern RLSTATUS     DECOMP_Init             (   void                                );
extern void         DECOMP_IsCompressed     (   sbyte *pData, 
                                                Boolean *pIsCompressed              );
extern sbyte        DECOMP_GetNextByte      (   Inflate_Control_Block *pICB  );
extern RLSTATUS     DECOMP_DecompressData   (   sbyte *pData, sbyte4 dataLen, 
                                                sbyte **ppDecompressedData, 
                                                sbyte4 *pDecompressedLen            );

#ifdef __cplusplus
}
#endif

#endif /* __DECOMPRESSION_ENABLED__ */
#endif /* !__DECOMP_HEADER__ */
