/*
 *  rc_filemgr.h
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




#ifndef __FILEMGR_HEADER__
#define __FILEMGR_HEADER__

/* prototypes */

#ifdef __cplusplus
extern "C" {
#endif

extern void     FILEMGR_Construct();
extern void*    FILEMGR_RetrieveFile(int *pFileLen, OS_SPECIFIC_RESOURCE_ID fileName);
extern void     FILEMGR_ReleaseFile(void *pDummy);

extern void*    FILEMGR_RetrieveResource(int *pFileLen, OS_SPECIFIC_RESOURCE_ID fileName);
extern void     FILEMGR_ReleaseResource(void *pDummy);

#ifdef __cplusplus
}
#endif

#endif
