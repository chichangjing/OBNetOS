/*  
 *  rc_vxworks.c
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

$History: rc_vxworks.c $
 * 
 * *****************  Version 23  *****************
 * User: James        Date: 10/09/00   Time: 3:38p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Added compiler flag disabling POSIX mutexes:
 * 
 * #define __RC_DISABLE_POSIXS_MUTEX__
 * 
 * *****************  Version 1  *****************
 * User: Kedron       Date: 9/06/00    Time: 5:30p
 * Created in $/Rapid Logic/Mediation/rli_code/rli_os
 * 
 * *****************  Version 21  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments


 */


#include "rc_options.h"

#ifdef __VXWORKS_OS__

#include <vxWorks.h>

#ifdef __RC_DISABLE_POSIXS_MUTEX__
#include <semLib.h>
#else
#include <semaphore.h>
#endif

#include <stdlib.h>
#include <taskLib.h>
#include <sockLib.h>
#include <inetLib.h>
#include <stdio.h>
#include <time.h>
#include <tickLib.h>
#include <selectLib.h>
#include <string.h>

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_socks.h"


#define kMinimalSocketTimeout   10
/*-----------------------------------------------------------------------*/

extern RLSTATUS VxWorks_MutexCreate(OS_SPECIFIC_MUTEX *pMutex)
{

#ifdef __RC_DISABLE_POSIXS_MUTEX__
    RLSTATUS  status = OK;

    *pMutex = semBCreate(SEM_Q_FIFO, SEM_FULL);

    if ( NULL == *pMutex )
        status = SYS_ERROR_MUTEX_CREATE;
    
    return status; 

#else

    RLSTATUS        status  = OK;
    int             pshared = 0;        /* future */

    *pMutex = (OS_SPECIFIC_MUTEX) OS_SPECIFIC_MALLOC(sizeof(sem_t));
    if (NULL == *pMutex)
      return SYS_ERROR_MUTEX_CREATE;

    if (0 != sem_init(*pMutex, pshared, 1))
        status = SYS_ERROR_MUTEX_CREATE;
    
    return status; 

#endif

}



/*-----------------------------------------------------------------------*/

extern RLSTATUS VxWorks_MutexWait(OS_SPECIFIC_MUTEX mutex)
{

#ifdef __RC_DISABLE_POSIXS_MUTEX__

    semTake(mutex, WAIT_FOREVER);
    return OK;

#else

    RLSTATUS status  = OK;

    if (0 != sem_wait(mutex))
        status = SYS_ERROR_MUTEX_WAIT;

    return status;

#endif

}



/*-----------------------------------------------------------------------*/

extern RLSTATUS VxWorks_MutexRelease(OS_SPECIFIC_MUTEX mutex)
{

#ifdef __RC_DISABLE_POSIXS_MUTEX__

    semGive(mutex);
    return OK;

#else

    RLSTATUS status  = OK;

    if (0 != sem_post(mutex))
        status = SYS_ERROR_MUTEX_WAIT;

    return status;

#endif

}




/*-----------------------------------------------------------------------*/

extern void *VxWorks_Malloc(Length memSize)
{
    return malloc(memSize);
}



/*-----------------------------------------------------------------------*/

extern void VxWorks_Free(void *pBuffer)
{
    free(pBuffer);
}



/*-----------------------------------------------------------------------*/

extern void VxWorks_Sleep(void)
{
    taskDelay(NO_WAIT);                       /* yield the processor */
}

extern void VxWorks_SleepTime(ubyte4 mSecs)
{
	int ticks = (mSecs * kHwTicksPerSecond ) / 1000;
    taskDelay(ticks);
}



/*-----------------------------------------------------------------------*/

RLSTATUS VxWorks_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
{
    RLSTATUS        BufLen;
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

    /* There's data in the socket, so nab all you can... */
    BufLen = recv(sock, pBuf, BufSize, 0);

    if (ERROR == BufLen)
        return ERROR_GENERAL_ACCESS_DENIED;

    return BufLen;
}



/*-----------------------------------------------------------------------*/

RLSTATUS VxWorks_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
    RLSTATUS  err;

    err = send(sock, pBuf, BufLen, 0);

    if (ERROR == err)
        return ERROR_GENERAL_ACCESS_DENIED;

    return OK;
}



/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VxWorks_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout)
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

extern ubyte4 VxWorks_GetSecs(void)
{
#ifdef HwGetNumTicks
    return (ubyte4)(HwGetNumTicks / kHwTicksPerSecond);
#else
    return (ubyte4)(tickGet() / kHwTicksPerSecond);
#endif
}



/*-----------------------------------------------------------------------*/

extern ubyte4 VxWorks_GetClientAddr(environment *p_envVar)
{
    struct sockaddr_in  ClientAddr;
    int                 iAddrLen;

    p_envVar->IpAddr = 0;

    iAddrLen = sizeof(ClientAddr);

    if (OK == getpeername(p_envVar->sock, (struct sockaddr *)&ClientAddr, &iAddrLen))
        p_envVar->IpAddr = ClientAddr.sin_addr.s_addr;

    return p_envVar->IpAddr;
}



/*-----------------------------------------------------------------------*/

void VxWorks_LogError(ubyte4 ErrorLevel, sbyte *ErrorMessage)
{
    if (ErrorLevel > kDebugError)
        printf("ErrLog:  Error Level=%d\t[%s]\n", (int)ErrorLevel, ErrorMessage);
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS VxWorks_CreateThread(void* pHandlerFcn, ubyte* TaskName,
                                     void* pArg, ubyte4 priority,
                                     ubyte4 ext1, void* ext2)
{
    int     status;

	if ((0==priority)||(255<priority))
	{
	    status = taskSpawn(TaskName, kSTANDARD_RC_THREAD_PRIO, 0, kSTANDARD_RC_THREAD_STACK_SIZE,
	                       (FUNCPTR)pHandlerFcn, (int)pArg,
	                       0,0,0,0,0,0,0,0,0 );
	}
	else
	{
	    status = taskSpawn(TaskName, priority, 0, kSTANDARD_RC_THREAD_STACK_SIZE,
	                       (FUNCPTR)pHandlerFcn, (int)pArg,
	                       0,0,0,0,0,0,0,0,0 );
	}

    if (ERROR == status)
        return ERROR_GENERAL_CREATE_TASK;

    return OK;
}


#endif /* __VXWORKS_OS__ */

