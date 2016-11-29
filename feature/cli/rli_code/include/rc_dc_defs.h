/*  
 *  dc_defs.h
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


#ifndef __DC_DEFS_HEADER__
#define __DC_DEFS_HEADER__

#if defined(__DECOMPRESSION_ENABLED__) || defined(__JAVACONTROL_ENABLED__) || defined(__OCP_ENABLED__)

/* typedefs used throughout the decompression code */
typedef unsigned char   uch;    
typedef unsigned short  ush;    
typedef unsigned long   ulg;    

typedef ubyte4          extent;
typedef void            zvoid;

#define ZCONST const

#endif /* __DECOMPRESSION_ENABLED__ */

#endif /* __DC_DEFS_HEADER__ */
