/*  
 *  rc_posix.c
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

$History: rc_posix.c $
 * 
 * *****************  Version 23  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 22  *****************
 * User: Pstuart      Date: 5/26/00    Time: 3:32p
 * Changed Posix_MutexCreate to use OS_SPECIFIC_MALLOC
 * 
 * *****************  Version 21  *****************
 * User: Pstuart      Date: 5/11/00    Time: 10:55a
 * fixed gcc warnings
 * 
 * *****************  Version 20  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/
#include "rc_options.h"

#ifdef __POSIX_OS__

#ifndef _REENTRANT
#define _REENTRANT
#endif


#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"


#define kMinimalSocketTimeout   10

#ifdef  __LINUX__
  typedef unsigned short u_short;
  typedef unsigned long  u_long;
#endif


/* in order to build correctly you will need to link in the following libraries
 * pthread
 * xnet
 * posix4
 */

/* for thread creation */
typedef void * StartFunc(void *);

/*-----------------------------------------------------------------------*/

/* Wrappers to Posix System Calls */
extern RLSTATUS Posix_MutexCreate(OS_SPECIFIC_MUTEX *pMutex)
{
    RLSTATUS        status  = OK;
    int             pshared = 0;        /* future */

    *pMutex = (OS_SPECIFIC_MUTEX) OS_SPECIFIC_MALLOC(sizeof(sem_t));
    if (NULL == *pMutex)
      return SYS_ERROR_MUTEX_CREATE;

    if (0 != sem_init(*pMutex, pshared, 1))
        status = SYS_ERROR_MUTEX_CREATE;
    
    return status; 
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS Posix_MutexWait(OS_SPECIFIC_MUTEX mutex)
{
    RLSTATUS status  = OK;

    if (0 != sem_wait(mutex))
        status = SYS_ERROR_MUTEX_WAIT;

    return status;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS Posix_MutexRelease(OS_SPECIFIC_MUTEX mutex)
{
    RLSTATUS status  = OK;

    if (0 != sem_post(mutex))
        status = SYS_ERROR_MUTEX_WAIT;

    return status;
}



/*-----------------------------------------------------------------------*/

extern void *Posix_Malloc(Length memSize)
{
    return malloc(memSize);
}



/*-----------------------------------------------------------------------*/

extern void Posix_Free(void *pBuffer)
{
    free(pBuffer);
}



/*-----------------------------------------------------------------------*/

extern void Posix_Sleep(void)
{
    if (0 != sched_yield())                       /* yield the processor */
    {
        /* weird error, try again. */
        sched_yield();
    }
}

extern void Posix_SleepTime(ubyte4 mSecs)
{
    int seconds = mSecs/1000;
    sleep(seconds);
}    

/*-----------------------------------------------------------------------*/

RLSTATUS Posix_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
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
    numReaders = select(sock+1, &fdSocks, 0, 0, &tTimeout);
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

    BufLen = recv(sock, pBuf, BufSize, 0);

    if (-1 == BufLen)
        return ERROR_GENERAL_ACCESS_DENIED;

    return BufLen;
}



/*-----------------------------------------------------------------------*/

RLSTATUS Posix_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
    RLSTATUS  err;

    err = send(sock, pBuf, BufLen, 0);

    if (-1 == err)
        return ERROR_GENERAL_ACCESS_DENIED;

    return OK;
}



/*-----------------------------------------------------------------------*/

extern ubyte4 Posix_GetSecs(void)
{
    time_t ltime;

    /* Get UNIX-style time */
    time( &ltime );

    return (ubyte4)ltime;
}



/*-----------------------------------------------------------------------*/

extern ubyte4 Posix_GetClientAddr(environment *p_envVar)
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

void Posix_LogError(ubyte4 ErrorLevel, sbyte *ErrorMessage)
{
    if (ErrorLevel > kDebugError)
        printf("ErrLog:  Error Level=%d\t[%s]\n", ErrorLevel, ErrorMessage);
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
Posix_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout)
{
    sbyte4          numReaders = 1;
    fd_set          fdSocks;
    struct timeval  tTimeout;

    FD_ZERO(&fdSocks);
    FD_SET(sock, &fdSocks);

    tTimeout.tv_sec  = timeout;
    tTimeout.tv_usec = 0;
    numReaders = select(sock+1, &fdSocks, 0, 0, &tTimeout);

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
	else
		return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS Posix_CreateThread(void* pHandlerFcn, ubyte* TaskName,
                                   void* pArg, ubyte4 priority,
                                   ubyte4 ext1, void* ext2)
{
    pthread_t threadID;
    
    int Result = pthread_create(&threadID, NULL, (StartFunc *) pHandlerFcn, pArg);
    
    if (0 != Result)
    {
        printf("fPosix_CreateThread failed thr_Create - errno %d\n", Result);
        printf("EAGAIN: %d EINVAL:%d ENOMEM:%d\n", EAGAIN, EINVAL, ENOMEM);

        return ERROR_GENERAL_CREATE_TASK;
    }
    return OK;
}


#endif /* __POSIX_OS__ */

