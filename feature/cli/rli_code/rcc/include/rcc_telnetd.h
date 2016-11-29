/*  
 *  rcc_telnetd.h
 *
 *  This is a part of the RapidControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
 *  All rights reserved.
 *
 *  RapidControl, RapidControl for Web, RapidControl Backplane,
 *  RapidControl for Applets, MIBway, RapidControl Protocol, and
 *  RapidMark are trademarks of Rapid Logic, Inc.  All rights reserved.
 *
 */

#ifndef __RCC_TELNETD_H__
#define __RCC_TELNETD_H__

#include <stdio.h>
/*-----------------------------------------------------------------------*/

/* Defaults */

#define kRCC_CONN_TELNET    1
#define kRCC_CONN_CONSOLE   2
#define kRCC_CONN_SERIAL    3
#define kRCC_CONN_EXTERNAL  4

#ifndef kRCC_TIMEOUT
#define kRCC_TIMEOUT					1800
#endif
#define kTELNET_TIMEOUT_IN_MINUTES		(kRCC_TIMEOUT/60)

/* number of CliChannel objects in array (# TCP threads) */
#define kTELNET_QUEUE_SIZE 10

/*-----------------------------------------------------------------------*/

/* DOS console key codes */
/* to convert to signed char */
#define kRCC_DOSKEY_ESC         0xE0
#define kRCC_DOSKEY_UP          0x48
#define kRCC_DOSKEY_DN          0x50
#define kRCC_DOSKEY_RT          0x4D
#define kRCC_DOSKEY_LT          0x4B
#define kRCC_DOSKEY_PAGE_UP     0x49
#define kRCC_DOSKEY_PAGE_DN     0x51
#define kRCC_DOSKEY_DEL         0x53
/*-----------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

extern void     RCC_TELNETD();
extern void     RCC_TELNETD_AddSession(struct cli_env *pCliEnv);
extern RLSTATUS RCC_TELNETD_Broadcast(struct cli_env *pCliEnv, sbyte *pMessage);
extern RLSTATUS RCC_TELNETD_BroadcastMessage(sbyte *pMessage, Access authLevel);
extern Boolean  RCC_TELNETD_CreateSession(struct CliChannel *pChannel, 
                                          SessionInit *pSessionInit,
                                          ReadHandle *pReadHandle, 
                                          WriteHandle *pWriteHandle,
                                          ubyte4 connectionType, sbyte *pName);
extern void     RCC_TELNETD_DelSession(struct cli_env *pCliEnv);
extern struct cli_env *    RCC_TELNETD_GetSession(sbyte4 index);
extern struct CliChannel * RCC_TELNETD_NewChannel(OS_SPECIFIC_SOCKET_HANDLE accSoc);
extern void     RCC_TELNETD_Kill();
extern RLSTATUS RCC_TELNETD_SessionCreate(struct CliChannel *pChannel, 
                                          SessionInit *pSessionInit,
                                          ReadHandle *pReadHandle, 
                                          WriteHandle *pWriteHandle,
                                          ubyte4 connectionType, 
                                          sbyte *pName, 
                                          struct cli_env ** ppCliEnv);

extern Boolean  RCC_TELNETD_SessionDestroy(struct cli_env *pCliEnv);

#ifdef __cplusplus
}
#endif

#ifdef __WIN32_OS__
#define GETCHAR() _getch()
#include <conio.h>
#else
#define GETCHAR() getchar()
#endif

#endif /*  __RCC_TELNETD_H__ */
