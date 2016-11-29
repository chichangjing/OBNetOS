/*  
 *  rc_prop_os.c
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

/* This file is designed to serve as a guide to porting our OpenControl
 * Products to your system.
 *
 * To port OpenControl to your system, you must first:
 *
 * 1) Modify the code in rc_prop_os.c (this file).  The code in this file is a 
 * series of wrapper functions to the basic system services required by 
 * OpenControl.  The code in each of the wrapper functions has been taken
 * from posix.c, just to serve as a guide.  However, in addition, each
 * wrapper function has a comment in it describing the specific system service 
 * that must be fulfilled by that call.  Possible return values are specified 
 * as well.
 *
 * 2) Modify the code in the file rc_skt_prop.c.  The code in that file is a 
 * connection server to handle packets as they are presented by the socket.
 * The code in that file has been taken from rc_sktposix.c, to serve as a guide.
 * Specific system calls or data structures that need to be modified are 
 * highlighted by comments.  In addition, each function has a comment in
 * it describing the specific service that must be fulfilled by that call, along
 * with possible return values.
 * 
 * 3) Modify the code in rc_os_spec.h that is bracketed by the compiler flag
 * #ifdef __CUSTOMER_SPECIFIC_OS__ 
 *
 * ...C Source Code...
 *
 * #endif 
 *
 * In that code block, you will need to specify the typedefs for
 * OS_SPECIFIC_SOCKET_HANDLE, OS_SPECIFIC_THREAD_HANDLE, and OS_SPECIFIC_MUTEX. 
 *
 * 4) In the Integration Tool, Go to 
 * Build | rc_options.h... | Device Real-Time Operating System | Host OS:
 * and select "Proprietary / Other".  This will cause the compiler flag
 * __CUSTOMER_SPECIFIC_OS__ to be #defined in rc_options.h, thereby 
 * causing the code in this file to be compiled and linked in with your 
 * system image.  
 *
 * Note that the Integration Tool allows you to customize the compiler flag
 * __CUSTOMER_SPECIFIC_OS__.  If you do modify the flag, make sure that you 
 * change the flag in rc_prop_os.c, rc_skt_prop.c, and rc_os_spec.h.  Otherwise,
 * the relevant code won't get compiled in.
 *
 * At that point, this OpenControl Product should be fully ported to your system.
 */

/*

$History: rc_prop_os.c $
 * 
 * *****************  Version 12  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 11  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/
#include "rc_options.h"

#ifdef __CUSTOMER_SPECIFIC_OS__

/* Add the relevant #includes that are necessary for the proprietary
 * system calls here */
#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"


/* Wrappers to Proprietary RTOS System Calls */
/*-----------------------------------------------------------------------*/

extern RLSTATUS Prop_OS_MutexCreate(OS_SPECIFIC_MUTEX *pMutex)
{
    /* This function must provide a handle for a mutual exclusion (mutex)
     * semaphore.  The handle is that is created by the system call
     * must be used to initialize the argument pMutex.  
     *
     * The return value of this wrapper must be OK if the system call
     * completed successfully, and SYS_ERROR_MUTEX_CREATE if there
     * was a problem.
     *
     * If you are using this OpenControl Product in single threaded mode
     * only, then you don't need to provide code for this function.
     * Instead, have it simply return OK.
     */
    RLSTATUS        status  = OK;
    int             pshared = 0;        /* future */

    *pMutex = (OS_SPECIFIC_MUTEX) RC_MALLOC(sizeof(sem_t));
    if (NULL == *pMutex)
      return SYS_ERROR_MUTEX_CREATE;

    if (0 != sem_init(*pMutex, pshared, 1))
        status = SYS_ERROR_MUTEX_CREATE;
    
    return status; 
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS Prop_OS_MutexWait(OS_SPECIFIC_MUTEX mutex)
{
    /* This function is used by OpenControl to wait on the mutex semaphore
     * provided as the argument.
     *
     * This function must return OK if the system call was successfully
     * executed, and it should return SYS_ERROR_MUTEX_WAIT if there was 
     * a problem.
     *
     * If you are using this OpenControl Product in single threaded mode
     * only, then you don't need to provide code for this function.
     * Instead, have it simply return OK.
     */
    RLSTATUS status  = OK;

    if (0 != sem_wait(mutex))
        status = SYS_ERROR_MUTEX_WAIT;

    return status;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS Prop_OS_MutexRelease(OS_SPECIFIC_MUTEX mutex)
{
    /* This function is used by OpenControl to signal or release the mutex 
     * semaphore provided as the argument.
     *
     * This function must return OK if the system call was successfully
     * executed.  It should return SYS_ERROR_MUTEX_RELEASE if there was 
     * a problem.
     *
     * If you are using this OpenControl Product in single threaded mode
     * only, then you don't need to provide code for this function.
     * Instead, have it simply return OK.
     */
    RLSTATUS status  = OK;

    if (0 != sem_post(mutex))
        status = SYS_ERROR_MUTEX_WAIT;

    return status;
}



/*-----------------------------------------------------------------------*/

extern void *Prop_OS_Malloc(Length memSize)
{
    /* This function provides a pointer to a memory buffer
     * with a size equal to or greater than the memSize argument.
     *
     * This function must return the pointer if the memory allocation
     * was successful, and NULL if it was not.
     */
    return malloc(memSize);
}



/*-----------------------------------------------------------------------*/

extern void Prop_OS_Free(void *pBuffer)
{
    /* This function is used to free up the memory buffer specified by
     * the argument pointer pBuffer.
     * 
     * The function has no return value.
     */
    free(pBuffer);
}



/*-----------------------------------------------------------------------*/

extern void Prop_OS_Sleep(void)
{
    /* OpenControl uses this function to voluntarily yield the processor.
     * It is the equivalent of a sleep(0) call in Unix.
     *
     * This function has no return value.
     */
    if (0 != sched_yield())                       /* yield the processor */
    {
        /* weird error, try again. */
        sched_yield();
    }
}

/*-----------------------------------------------------------------------*/

extern void Prop_OS_SleepTime(ubyte4 mSecs)
{
	/* OpenControl uses this to cause a thread to sleep for a time
	 * (currently only the Event Queue thread uses this)
     * Items to replace/modify:
	 *		tm_wkafter: replace with the appropriate function for your RTOS
	 *		kWwTicksPerSecond: set in rc_options.h to appropriate value
	 */

	unsigned long ticks = (mSecs * kHwTicksPerSecond ) / 1000;
    tm_wkafter(ticks);  
	
} /* Prop_OS_SleepTime */


/*-----------------------------------------------------------------------*/

extern RLSTATUS 
Prop_OS_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
{
    /* This function is used to read data from a TCP socket.  The actual 
     * descriptor to specify the socket is the argument sock.  The buffer 
     * into which the data should be copied is pBuf.  The number of bytes to 
     * be read is BufSize.  
     *
     * The function should ONLY READ IN THE NUMBER OF BYTES SPECIFIED BY 
     * BufSize.  If your socket handler reads in the entire buffer regardless
     * of the size specification, memory may be overrun and this OpenControl 
     * product will not function properly.
     *
     * The function should return the number of bytes read from the socket.
     * If there was an error in accessing the socket, then 
     * ERROR_GENERAL_ACCESS_DENIED should be returned.  If there was an error
     * in reading the data, then the function should return 
     * SYS_ERROR_SOCKET_GENERAL,
     */ 
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
Prop_OS_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
    /* This function is used to write data to a TCP socket.  The descriptor
     * used to specify the socket is the argument sock.  pBuf is a pointer
     * to the buffer to be written.  BufLen is the length of the buffer, in
     * bytes.
     *
     * The function should return OK if the write was successfully completed. If
     * there was a problem, then SYS_ERROR_SOCKET_GENERAL should be returned.
     */ 
    RLSTATUS  err;

    err = send(sock, pBuf, BufLen, 0);

    if (-1 == err)
        return SYS_ERROR_SOCKET_GENERAL;

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
Prop_OS_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout)
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

extern ubyte4 Prop_OS_GetSecs(void)
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

extern ubyte4 Prop_OS_GetClientAddr(environment *p_envVar)
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
Prop_OS_LogError(ubyte4 ErrorLevel, sbyte *ErrorMessage)
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

extern RLSTATUS Prop_OS_CreateThread(void* pHandlerFcn, ubyte* TaskName,
                                     void* pArg, ubyte4 priority,
                                     ubyte4 ext1, void* ext2)
{
    /* This function spawns threads.
     *    
     * Items to replace/modify:
     * Thread ID type (pthread_t in this example)
     * pthread_create()
     * The check of the return value of the thread create function.
	 * Use TaskName as appropriate
     *
     * This function should return OK if successful, ERROR_GENERAL_CREATE_TASK 
     * otherwise
     */

    pthread_t threadID;
    
    int Result = pthread_create(&threadID, NULL, pHandlerFcn, pArg);
    
    if (0 != Result)
    {
	    return ERROR_GENERAL_CREATE_TASK;
    }
    return OK;
	
}	

#endif /* __CUSTOMER_SPECIFIC_OS__ */

