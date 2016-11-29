/*  
 *  rc_win32.c
 *
 *  This is a part of the OpenControl SDK source code library. 
 *
 *  Copyright (C) 1998 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */
/*

$History: rc_win32.c $
 * 
 * *****************  Version 17  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 16  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


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

#include "rc_options.h"

#ifdef __WIN32_OS__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <winsock.h>
#include <windows.h>
#include <windef.h>
#include <wsnwlink.h>
#include <sys/stat.h>
#include <winbase.h>
#include <time.h>
#include <process.h>

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"


/* Wrappers to Win32 System Calls */
/*-----------------------------------------------------------------------*/

extern RLSTATUS
Win32_MutexCreate ( OS_SPECIFIC_MUTEX *pMutex )
{
    RLSTATUS  sStatus;

    sStatus  = OK;
    *pMutex = CreateMutex( NULL, FALSE, NULL );
    if ( *pMutex == NULLMUTEX )
        sStatus = SYS_ERROR_MUTEX_CREATE;
    
    return sStatus; 

} 



/*-----------------------------------------------------------------------*/

extern RLSTATUS
Win32_MutexWait ( OS_SPECIFIC_MUTEX mutex )
{
    WaitForSingleObject( mutex, INFINITE );
    return OK;

} 



/*-----------------------------------------------------------------------*/

extern RLSTATUS
Win32_MutexRelease ( OS_SPECIFIC_MUTEX mutex )
{
    ReleaseMutex( mutex );
    return OK;

} 



/*-----------------------------------------------------------------------*/

extern void *Win32_Malloc( Length memSize )
{
    return malloc( memSize );
} 



/*-----------------------------------------------------------------------*/

extern void
Win32_Free ( void *pBuffer )
{
    free( pBuffer );

} 



/*-----------------------------------------------------------------------*/

extern void Win32_Sleep(void)
{
    Sleep(0);                       /* yield the processor */
}


extern void Win32_SleepTime(ubyte4 mSecs)
{
    Sleep((DWORD)mSecs);
}

/*-----------------------------------------------------------------------*/

#ifdef __DEBUG_WIN32_SOCKET__

extern void Win32_DEBUG_PACKET(sbyte *pBuf, Counter BufLen)
{
    int Base = 0, Offset = 0, Temp;

    if (1 >= BufLen)
        return;

    for (; ((Base + Offset) < (int)BufLen); )
    {
        printf ("%4x:  ", Base);

        for (;((Offset < 16) && ((Base + Offset) < (int)BufLen)); Offset++)
            printf("%02x ", (int)(((unsigned int)pBuf[Base + Offset]) & 0xff));

        for (Temp = Offset; (Temp < 16); Temp++)
            printf("   ");

        Offset = 0;
        printf("\t");

        for (; ((Offset < 16) && ((Base + Offset) < (int)BufLen)); Offset++)
            if ((pBuf[Base + Offset] < 0x20) || (pBuf[Base + Offset] > 0x7f))
                printf(".");
            else
                printf("%c", pBuf[Base + Offset]);

        Base += Offset;
        Offset = 0;

        printf("\n");
    }
}
#endif



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
Win32_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
{
    RLSTATUS        BufLen;
    sbyte4          numReaders;
    fd_set          fdSocks;
    struct timeval  tTimeout;

    /* The following code prevents the reader from blocking ad infinitum on the 
     * socket. */
    FD_ZERO(&fdSocks);
    FD_SET(sock, &fdSocks);

    tTimeout.tv_sec = kSocketTimeOut;
    tTimeout.tv_usec = 0;
    numReaders = select(1, &fdSocks, 0, 0, &tTimeout);
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

    if (SOCKET_ERROR == BufLen)     /* convert a windows error to our system */
        return ERROR_GENERAL_ACCESS_DENIED;

#ifdef __DEBUG_WIN32_SOCKET__
    if (BufSize > 1)
        printf("\n\n-----------------> Client Request:\n");

    Win32_DEBUG_PACKET(pBuf, BufLen);
#endif

    return BufLen;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
Win32_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
    RLSTATUS  err;

#ifdef __DEBUG_WIN32_SOCKET__
    printf("\n\n-----------------> Server Sending:\n");

    Win32_DEBUG_PACKET(pBuf, BufLen);
#endif

    err = send(sock, pBuf, BufLen, 0);

    if (SOCKET_ERROR == err)
        return ERROR_GENERAL_ACCESS_DENIED;

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
Win32_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout)
{
    sbyte4          numReaders = 1;
 
    fd_set          fdSocks;
    struct timeval  tTimeout;

    FD_ZERO(&fdSocks);
    FD_SET(sock, &fdSocks);

    tTimeout.tv_sec  = timeout;
    tTimeout.tv_usec = 0;
    numReaders = select(1, &fdSocks, 0, 0, &tTimeout);

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

extern ubyte4 Win32_GetSecs(void)
{
    time_t ltime;

    /* Get UNIX-style time */
    time( &ltime );

    return (ubyte4)ltime;
}



/*-----------------------------------------------------------------------*/

extern ubyte4 Win32_GetClientAddr(environment *p_envVar)
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

extern void Win32_LogError(ubyte4 ErrorLevel, sbyte *ErrorMessage)
{
    if (ErrorLevel > kDebugError)
        printf("ErrLog:  Error Level=%d\t[%s]\n", ErrorLevel, ErrorMessage);
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS Win32_CreateThread(void* pHandlerFcn, ubyte* TaskName,
                                   void* pArg, ubyte4 priority, 
                                   ubyte4 ext1, void* ext2)
{
    unsigned int Result = _beginthread(pHandlerFcn, 0, pArg);

    if (-1 == Result)
        return ERROR_GENERAL_CREATE_TASK;

    return OK;
}




#endif /* __WIN32_OS__ */
