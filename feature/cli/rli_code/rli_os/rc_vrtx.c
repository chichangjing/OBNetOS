/*  
 *  rc_vrtx.c
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

$History: rc_vrtx.c $
 * 
 * *****************  Version 9  *****************
 * User: Pstuart      Date: 1/05/01    Time: 1:36p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * added missing include, removed extraneous paren
 * 
 * *****************  Version 8  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments


 */

#include "rc_options.h"

#ifdef __VRTX_OS__

#include <malloc.h>

#include <errno.h>
#include <stdio.h>

#include <vrtxil.h>

#ifndef __SOCK_ATTACHE_TCPIP_STACK__
#include <tnxvisi.h>
#endif

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_base64.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_socks.h"

/*-----------------------------------------------------------------------*/

/* Wrappers to VRTX System Calls */
extern RLSTATUS VRTX_MutexCreate(int *pMutex)
{
    int err;
    RLSTATUS    status = OK;

    *pMutex = sc_screate(1, 0, &err);

    if ( RET_OK != err )
        status = SYS_ERROR_MUTEX_CREATE;
    
    return status; 
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS VRTX_MutexWait(int mutex)
{
    int err;
    RLSTATUS    status = OK;

    sc_spend(mutex, 0, &err);

    if ( RET_OK != err )
        status = SYS_ERROR_MUTEX_WAIT;

    return status;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS VRTX_MutexRelease(int mutex)
{
    int err;
    RLSTATUS    status = OK;

    sc_spost(mutex, &err);

    if ( RET_OK != err )
        status = SYS_ERROR_MUTEX_RELEASE;

    return status;
}



/*-----------------------------------------------------------------------*/

extern void *VRTX_Malloc(unsigned int memSize)
{
    return (void *)malloc(memSize);
}



/*-----------------------------------------------------------------------*/

extern void VRTX_Free(void *pBuffer)
{
    free((char *)pBuffer);
}



/*-----------------------------------------------------------------------*/

extern void VRTX_Sleep(void)
{
    sc_delay(2);

    return;
}



/*-----------------------------------------------------------------------*/

#ifndef  __SOCK_ATTACHE_TCPIP_STACK__
RLSTATUS VRTX_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
{
    int     BufLen;
    int     status;

    BufLen = tnx_recv(sock, pBuf, BufSize, 0, &status);

	/* !!!!!!!!! TODO: detect timeout, return SYS_ERROR_SOCKET_TIMEOUT so OCP can stay alive */
    if ( TNX_OK != status )
        return ERROR_GENERAL_ACCESS_DENIED;

    return OK;
}
#endif



/*-----------------------------------------------------------------------*/

#ifndef  __SOCK_ATTACHE_TCPIP_STACK__
RLSTATUS VRTX_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
    int     bytesWritten;
    int     status;

    bytesWritten = tnx_send(sock, pBuf, BufLen, 0, &status);

    if ( TNX_OK != status )
        return ERROR_GENERAL_ACCESS_DENIED;

    return OK;
}
#endif



/*-----------------------------------------------------------------------*/

void VRTX_LogError(ubyte4 ErrorLevel, sbyte *ErrorMessage)
{
    if (ErrorLevel > kDebugError)
		if (NULL != ErrorMessage)
	        printf("ErrLog:  Error Level=%d\t[%s]\n", ErrorLevel, ErrorMessage);
		else
	        printf("ErrLog:  Error Level=%d\t[Message-handler not enabled]\n", ErrorLevel);
}



/*-----------------------------------------------------------------------*/

extern ubyte4 VRTX_GetClientAddr(environment *p_envVar)
{
    p_envVar->IpAddr = SOCKET_GetClientsAddr(p_envVar->sock);

    return p_envVar->IpAddr;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS VRTX_CreateThread(void* pHandlerFcn, ubyte* TaskName,
                                   void* pArg, ubyte4 priority, 
                                   ubyte4 ext1, void* ext2)
{
    int status;

	if (0==priority)
	{
	    sc_tcreate( (void *) pHandlerFcn, (int) pArg, 
	                 kHTTPD_SERVER_PRIO, &status);
	}
	else
	{
	    sc_tcreate( (void *) pHandlerFcn, (int) pArg, 
	                priority, &status);
	}

    if (RET_OK != status)
        return SYS_ERROR_SOCKET_CREATE_TASK;

    return OK;
}

#endif /* __VRTX_OS__ */

