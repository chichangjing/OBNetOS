/*
 *  rc_access.h
 *
 *  This is a part of the OpenControl SDK source code library.
 *
 *  Copyright (C) 1999 Rapid Logic, Inc.
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

#ifndef __RC_ACCESS_HEADER__
#define __RC_ACCESS_HEADER__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __ENABLE_LAN_IP_FILTER__
extern Boolean ACCESS_ValidIpAddress(ubyte4 ClientIpAddr);
#endif

/* Access validation */
extern Boolean  RC_ACCESS_Allowed(Access nResourceAccess, Access nUserAccess);


#ifdef __cplusplus
}
#endif

#endif /* __RC_ACCESS_HEADER__ */
