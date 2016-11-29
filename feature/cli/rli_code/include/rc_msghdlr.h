/*  
 *  rc_msghdlr.h
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

  $History: rc_msghdlr.h $
 * 
 * *****************  Version 7  *****************
 * User: Pstuart      Date: 12/11/00   Time: 11:30a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added c++ wrappers for prototypes
 * 
 * *****************  Version 6  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Include history and enable auto archiving feature from VSS

  */

#ifndef __MSGHDLR_HEADER__
#define __MSGHDLR_HEADER__

/*-------------------------------------------------------------------------------*/

/* Possible Message Types */
#define kInternalMessage        0
#define kCustomerMessage        1

/* Number of discrete dictionaries */
#define kNumMessageDictionaries    10

#define MSGHDLR_COULD_NOT_BUILD_TABLE   "HTTP Mgmt: Message Handler could not build the message table"
#define kMessageDataStartDelimiter  ":"
#define kMessageNumberDelimiter     ">"
#define kMessageDelimiter           "%"

#define kBadMsgDictionaryID         -1 


/*-------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

extern RLSTATUS MsgHdlr_RetrieveMessage             (   RLSTATUS msg, 
                                                        sbyte2 msgDictionaryID, 
                                                        sbyte **ppMsg               );

extern RLSTATUS MsgHdlr_InitDictionary              (   sbyte   *pMessageData, 
                                                        sbyte4  strlenSizeOfMsgData,
                                                        sbyte2  *pMsgDictionaryID   );

extern RLSTATUS MsgHdlr_SystemInit                  (   void                        );

/* For backwards compatibility */
extern RLSTATUS MsgHdlr_RetrieveOpenControlMessage  (   RLSTATUS msg, sbyte **pMsg  );
extern RLSTATUS MsgHdlr_RetrieveCustomMessage       (   RLSTATUS msg, sbyte **pMsg  );
extern RLSTATUS MsgHdlr_Init                        (   sbyte   *pMessageData, 
                                                        sbyte4  strlenSizeOfMsgData,
                                                        sbyte   msgType             );

#ifdef __cplusplus
}
#endif

#endif /*  __MSGHDLR_HEADER__ */
