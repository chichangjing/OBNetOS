/*  
 *  rc_netbsd.c
 *
 *  This is a part of the OpenControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
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

$History: rc_netbsd.c $
 * 
 * *****************  Version 1  *****************
 * User: James        Date: 8/31/00    Time: 10:40a
 * Created in $/Rapid Logic/Code Line/rli_code/rli_os
 * Initial check in. JAB

*/
#include "rc_options.h"

#ifdef __NETBSD_OS__

#ifndef _REENTRANT
#define _REENTRANT
#endif


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/sched.h>
#include <sys/unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <pthread.h>
#include <pth.h>
#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_socks.h"


#define kMinimalSocketTimeout   10



/* in order to build correctly you will need to link in the following libraries
 * kthread
 * xnet
 * posix4
 */

/* for thread creation */
typedef void * StartFunc(void *);

/*-----------------------------------------------------------------------*/

/* Wrappers to NETBSD System Calls */
extern RLSTATUS NETBSD_MutexCreate(OS_SPECIFIC_MUTEX *pMutex)
{
    RLSTATUS              status  = OK;
    pthread_mutexattr_t   mutexAttr;

    if (0 != (pthread_mutexattr_init(&mutexAttr)))
    {
        return SYS_ERROR_MUTEX_CREATE;
    }

    *pMutex = (OS_SPECIFIC_MUTEX) OS_SPECIFIC_MALLOC(sizeof(pthread_mutex_t));
    if (NULL == *pMutex)
      return SYS_ERROR_MUTEX_CREATE;

    if (0 != pthread_mutex_init(*pMutex, &mutexAttr))
        status = SYS_ERROR_MUTEX_CREATE;
    
    return status; 
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS NETBSD_MutexWait(OS_SPECIFIC_MUTEX mutex)
{
    RLSTATUS status  = OK;

    if (0 != pthread_mutex_lock(mutex))
        status = SYS_ERROR_MUTEX_WAIT;

    return status;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS NETBSD_MutexRelease(OS_SPECIFIC_MUTEX mutex)
{
    RLSTATUS status  = OK;

    if (0 != pthread_mutex_unlock(mutex))
        status = SYS_ERROR_MUTEX_WAIT;

    return status;
}



/*-----------------------------------------------------------------------*/

extern void *NETBSD_Malloc(Length memSize)
{
    return malloc(memSize);
}



/*-----------------------------------------------------------------------*/

extern void NETBSD_Free(void *pBuffer)
{
    free(pBuffer);
}



/*-----------------------------------------------------------------------*/

extern void NETBSD_Sleep(void)
{
    pth_yield(pth_self());               /* yield the processor */
}



/*-----------------------------------------------------------------------*/

extern void NETBSD_SleepTime(ubyte4 mSecs)
{
    int seconds = mSecs/1000;
    pth_sleep(seconds);
}    



/*-----------------------------------------------------------------------*/

RLSTATUS NETBSD_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
{
    RLSTATUS BufLen;

    sbyte4          numReaders;
    fd_set          fdSocks;
    struct timeval  tTimeout;

    /* If we're only pulling in 1 byte at a time, we know that we're just doing
     * the buildup of the HTTP message header.  And since that message header
     * should fit inside of one TCP packet, we know that the entire packet should
     * be waiting in the buffer.  If it isn't--as in the case of a Netscape 3.03
     * packet (which is malformed due to a bug in Netscape)--then use the minimal
     * timeout mechanism.  Otherwise, use the user-specified timeout value.
     */
    FD_ZERO(&fdSocks);
    FD_SET(sock, &fdSocks);

    tTimeout.tv_sec  = (1 == BufSize) ? kMinimalSocketTimeout: kSocketTimeOut; 
    tTimeout.tv_usec = 0;
    numReaders = pth_select(sock+1, &fdSocks, 0, 0, &tTimeout);

    /* if timeout, report it */
    if (0 == numReaders)
    {
        return SYS_ERROR_SOCKET_TIMEOUT;
    }

    /* report any error */
    if (0 > numReaders)
    {
        return SYS_ERROR_SOCKET_GENERAL;
    }

    BufLen = pth_read(sock, pBuf, BufSize);

    if (-1 == BufLen)
        return ERROR_GENERAL_ACCESS_DENIED;

    return BufLen;
}



/*-----------------------------------------------------------------------*/

RLSTATUS NETBSD_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
    RLSTATUS  err;

    err = pth_write(sock, pBuf, BufLen);

    if (-1 == err)
        return ERROR_GENERAL_ACCESS_DENIED;

    return OK;
}



/*-----------------------------------------------------------------------*/

extern ubyte4 NETBSD_GetSecs(void)
{
    struct timeval curTime;

    /* Get UNIX-style time */
    gettimeofday( &curTime, NULL );

    return (ubyte4)(curTime.tv_sec);
}



/*-----------------------------------------------------------------------*/

extern ubyte4 NETBSD_GetClientAddr(environment *p_envVar)
{
    struct sockaddr_in  ClientAddr;
    socklen_t           iAddrLen;

    p_envVar->IpAddr = 0;

    iAddrLen = sizeof(ClientAddr);

    if (OK == getpeername(p_envVar->sock, (struct sockaddr *)&ClientAddr, &iAddrLen))
        p_envVar->IpAddr = ClientAddr.sin_addr.s_addr;

    return p_envVar->IpAddr;
}



/*-----------------------------------------------------------------------*/

void NETBSD_LogError(ubyte4 ErrorLevel, sbyte *ErrorMessage)
{
    if (ErrorLevel > kDebugError)
        printf("ErrLog:  Error Level=%d\t[%s]\n", ErrorLevel, ErrorMessage);
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
NETBSD_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout)
{
    sbyte4          numReaders = 1;
    fd_set          fdSocks;
    struct timeval  tTimeout;

    FD_ZERO(&fdSocks);
    FD_SET(sock, &fdSocks);

    tTimeout.tv_sec  = timeout;
    tTimeout.tv_usec = 0;
    numReaders = pth_select(sock+1, &fdSocks, 0, 0, &tTimeout);

    /* if the number of available readers is greater than 0, then 
     * data is available.  Otherwise, return various error messages */

    /* if timeout, report it */
    if (0 == numReaders)
    {
        return SYS_ERROR_SOCKET_TIMEOUT;
    }

    /* report any error */
    if (0 > numReaders)
    {
        return SYS_ERROR_SOCKET_GENERAL;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS NETBSD_CreateThread(void* pHandlerFcn, ubyte* TaskName,
                                   void* pArg, ubyte4 priority,
                                   ubyte4 ext1, void* ext2)
{
    static Boolean init = FALSE;
    static pth_attr_t attr;

    if (init == FALSE)
    {
        attr = pth_attr_new();
        pth_attr_set(attr, PTH_ATTR_STACK_SIZE, kSTANDARD_RC_THREAD_STACK_SIZE);
        pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);
        init = TRUE;
    }

    pth_attr_set(attr, PTH_ATTR_NAME, TaskName);

    pth_spawn(attr, pHandlerFcn, pArg);

    return OK;
}

#endif /* __NETBSD_OS__ */

