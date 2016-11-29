/*  
 *  rc_chorus.c
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

$History: rc_chorus.c $
 * 
 * *****************  Version 11  *****************
 * User: Leech        Date: 6/21/00    Time: 11:49a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 10  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/


#include "rc_options.h"

#ifdef __CHORUS_OS__

/* Add the relevant #includes that are necessary for the proprietary
 * system calls here */

#include <stdio.h>
#include <sync/chMutex.h>  /* Chorus semaphores */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <pthread.h>

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"


/*-----------------------------------------------------------------------*/

extern RLSTATUS CHORUS_MutexCreate(OS_SPECIFIC_MUTEX *pMutex)
{
    RLSTATUS        status  = OK;

    *pMutex = (OS_SPECIFIC_MUTEX) RC_MALLOC(sizeof(KnMutex));
    if (NULL == *pMutex)
      return SYS_ERROR_MUTEX_CREATE;

    if (0 != mutexInit(*pMutex))
        status = SYS_ERROR_MUTEX_CREATE;
    
    return status; 
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS CHORUS_MutexWait(OS_SPECIFIC_MUTEX mutex)
{
    RLSTATUS status  = OK;

    if (0 != mutexGet(mutex))
        status = SYS_ERROR_MUTEX_WAIT;

    return status;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS CHORUS_MutexRelease(OS_SPECIFIC_MUTEX mutex)
{
    RLSTATUS status  = OK;

    if (0 != mutexRel(mutex))
        status = SYS_ERROR_MUTEX_WAIT;

    return status;
}



/*-----------------------------------------------------------------------*/

extern void *CHORUS_Malloc(Length memSize)
{
    return malloc(memSize);
}



/*-----------------------------------------------------------------------*/

extern void CHORUS_Free(void *pBuffer)
{
    free(pBuffer);
}



/*-----------------------------------------------------------------------*/

extern void CHORUS_Sleep(void)
{
    sched_yield();
}

extern void CHORUS_SleepTime(ubyte4 mSecs)
{
    int seconds = mSecs/1000;
    sleep(seconds);
}  

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
CHORUS_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
{
    RLSTATUS        BufLen;

    /* The following code swatch is necessary for HTTP 1.1.  Tentatively,
     * we're using the BSD select() system call to determine if any more data is
     * available.  However, the code itself might need to be modified to account
     * for your Unix-specific semantics. */
    sbyte4          numReaders;
    fd_set          fdSocks;
    struct timeval  tTimeout;

    /* The following code prevents the reader from blocking ad infinitum on the 
     * socket. */
    FD_ZERO(&fdSocks);
    FD_SET(sock, &fdSocks);

    tTimeout.tv_sec  = kSocketTimeOut;
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

    if (-1 == BufLen)
        return SYS_ERROR_SOCKET_GENERAL;

    return BufLen;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
CHORUS_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
    RLSTATUS  err;

    err = send(sock, pBuf, BufLen, 0);

    if (-1 == err)
        return SYS_ERROR_SOCKET_GENERAL;

    return OK;
}



/*-----------------------------------------------------------------------*/

extern  RLSTATUS
CHORUS_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout)
{
    /* This function is used to determine if there is any data left in a
     * TCP socket.  If there is more data, then this OpenControl product will
     * retrieve it as appropriate. 
     *
     * The descriptor to specify the socket is the argument sock.
     *
     * If there is more data available, then this function should return
     * TRUE.  Otherwise, it should return FALSE.
     */
    sbyte4          numReaders = 1;

    /* The following code swatch is necessary for HTTP 1.1.  Tentatively,
     * we're using the BSD select() system call to determine if any more data is
     * available.  This is necessary to keep the HTTP 1.1 server from blocking
     * on empty-socket reads. */
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

extern ubyte4 CHORUS_GetSecs(void)
{
    /* This function is used to get the number of seconds since the
     * epoch on January 1st, 1970.
     *
     * The return value of this function is the number of seconds.
     */
    struct timeval ltime;

    /* Get UNIX-style time */
    gettimeofday( &ltime, NULL );

    return (ubyte4)ltime.tv_sec;
}


/*-----------------------------------------------------------------------*/

extern ubyte4 CHORUS_GetClientAddr(environment *p_envVar)
{
    /* This function is used to retrieve the IP Address of the client
     * connected to the device.  
     *
     * The relevant information to get the client address, such as the
     * socket descriptor, is provided in the environment structure
     * argument p_envVar.  (The structure definition is provided in the
     * file rc_environ.h.)
     *
     * The return value is the IP Address, in network byte order.
     */
    struct sockaddr_in  ClientAddr;
    int                 iAddrLen;

    p_envVar->IpAddr = 0;

    iAddrLen = sizeof(ClientAddr);

    if (OK == getpeername(p_envVar->sock, (struct sockaddr *)&ClientAddr, &iAddrLen))
        p_envVar->IpAddr = ClientAddr.sin_addr.s_addr;

    return p_envVar->IpAddr;
}


/*-----------------------------------------------------------------------*/

extern void 
CHORUS_LogError(ubyte4 ErrorLevel, sbyte *ErrorMessage)
{
    /* This function is used, in conjunction with the Rapid Logic Message
     * Handler, to log any error message strings.
     *
     * The various error levels, as presented by the argument ErrorLevel
     * are #defined in the file rc_os_spec.h.
     *
     * There is no return value for this function.
     */
    if (ErrorLevel > kDebugError)
        printf("ErrLog:  Error Level=%d\t[%s]\n", (int)ErrorLevel, ErrorMessage);
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS CHORUS_CreateThread(void* pHandlerFcn, ubyte* TaskName,
                                    void* pArg, ubyte4 priority,
                                    ubyte4 ext1, void* ext2 )
{
    pthread_t threadID;
    
    int Result = pthread_create(&threadID, NULL, pHandlerFcn, pArg);
    
    if (0 != Result)
    {
        printf("CHORUS_CreateThread failed thr_Create - errno %d\n", Result);
        printf("EAGAIN: %d EINVAL:%d ENOMEM:%d\n", EAGAIN, EINVAL, ENOMEM);

        return ERROR_GENERAL_CREATE_TASK;
    }
    return OK;
}

#endif /* __CHORUS_OS__ */

