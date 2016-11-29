/*  
 *  rc_sysv.c
 *
 *  This is a part of the RapidControl SDK source code library. 
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

$History: rc_sysv.c $
 * 
 * *****************  Version 4  *****************
 * User: Schew        Date: 6/22/00    Time: 12:56p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Change #include *.h to #include rc_*.h
 * 
 * *****************  Version 3  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments


 */


#include "rc_options.h"

#ifdef __SYSTEMV_OS__

/* Add the relevant #includes that are necessary for the proprietary
 * system calls here */
#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* System V */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"

static int mSystemV_SemId;


extern RLSTATUS SystemV_MutexCreate(OS_SPECIFIC_MUTEX *pMutex)
{
    static Boolean  semCreated = FALSE;
    static int      semIndex   = 0;

    int             nsems  = 21;
    int             flags  = IPC_CREAT | 0666;
    key_t           ipc_key;

    if (FALSE == semCreated)
    {
        ipc_key = ftok(".", 'S');

        mSystemV_SemId = semget(ipc_key, nsems, flags);

        if (-1 == mSystemV_SemId)
        {
            printf("MutexCreate: semget() failed.\n");
            return -1;
        }

        semCreated = TRUE;

        /*!!!!!!! should initialize the semaphore set here. JAB */
    }

    *pMutex = semIndex;

    semIndex++;

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS SystemV_MutexWait(OS_SPECIFIC_MUTEX mutex)
{
    struct sembuf buf;

    buf.sem_num = mutex;
    buf.sem_op  = 1;
    buf.sem_flg = IPC_NOWAIT;

    if (0 > semop(mSystemV_SemId, &buf, 1))
    {
        printf("MutexWait: failed.\n");

        return SYS_ERROR_MUTEX_WAIT;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS SystemV_MutexRelease(OS_SPECIFIC_MUTEX mutex)
{
    struct sembuf buf;

    buf.sem_num = mutex;
    buf.sem_op  = -1;
    buf.sem_flg = IPC_NOWAIT;

    if (0 > semop(mSystemV_SemId, &buf, 1))
    {
        printf("MutexRelease: failed.\n");

        return SYS_ERROR_MUTEX_WAIT;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

extern void *SystemV_Malloc(Length memSize)
{
    return malloc(memSize);
}



/*-----------------------------------------------------------------------*/

extern void SystemV_Free(void *pBuffer)
{
    free(pBuffer);
}



/*-----------------------------------------------------------------------*/

extern void SystemV_Sleep(void)
{
     usleep(1);
}

/*-----------------------------------------------------------------------*/

extern void SystemV_SleepTime(ubyte4 mSecs)
{
    unsigned long ticks = (mSecs * kHwTicksPerSecond ) / 1000;
    usleep(ticks);  
    
} /* SystemV_SleepTime */


/*-----------------------------------------------------------------------*/

extern RLSTATUS 
SystemV_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
{
    RLSTATUS        BufLen;
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
SystemV_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
    RLSTATUS  err;

    err = send(sock, pBuf, BufLen, 0);

    if (-1 == err)
        return SYS_ERROR_SOCKET_GENERAL;

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
SystemV_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout)
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

extern ubyte4 SystemV_GetSecs(void)
{
    /* This function is used to get the number of seconds since the
     * epoch on January 1st, 1970.
     *
     * The return value of this function is the number of seconds.
     */
    time_t ltime;

    /* Get UNIX-style time */
    time( &ltime );

    return (ubyte4)ltime;
}


/*-----------------------------------------------------------------------*/

extern ubyte4 SystemV_GetClientAddr(environment *p_envVar)
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

extern void 
SystemV_LogError(ubyte4 ErrorLevel, sbyte *ErrorMessage)
{
    if (ErrorLevel > kDebugError)
        printf("ErrLog:  Error Level=%d\t[%s]\n", (int)ErrorLevel, ErrorMessage);
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS SystemV_CreateThread(void* pHandlerFcn, ubyte* TaskName,
                                     void* pArg, ubyte4 priority,
                                     ubyte4 ext1, void* ext2)
{
    pid_t pid;
    void (*p_funcExec)(void *) = pHandlerFcn;

    pid = fork();

    if (-1 == pid)
        return ERROR_GENERAL_CREATE_TASK;

    if (0 == pid)
    {
        p_funcExec(pArg);
        exit(0);
    }

    return OK;
}   

#endif /* __SYSTEMV_OS__ */

