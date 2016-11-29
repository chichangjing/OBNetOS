/*
 *  rc_ocb_init.h
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

  $History: rc_ocb_init.h $
 * 
 * *****************  Version 8  *****************
 * User: Pstuart      Date: 12/11/00   Time: 11:30a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added c++ wrappers for prototypes
 * 
 * *****************  Version 7  *****************
 * User: Pstuart      Date: 7/28/00    Time: 5:26p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 6  *****************
 * User: Pstuart      Date: 7/20/00    Time: 10:20a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added proto for Ignite_Database
 * 
 * *****************  Version 5  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Include history and enable auto archiving feature from VSS

  */
#ifndef __OCB_INIT_HEADER__
#define __OCB_INIT_HEADER__

/*-----------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

extern RLSTATUS     OCB_Init(Startup *pStartup);
extern void         Ignite_Database(void);
extern void         Ignite_Groups(void);

#ifdef __cplusplus
}
#endif

#endif /* __OCB_INIT_HEADER__ */
