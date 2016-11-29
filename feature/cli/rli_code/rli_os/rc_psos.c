/*  
 *  rc_pSOS.c
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

$History: rc_psos.c $
 * 
 * *****************  Version 18  *****************
 * User: Leech        Date: 6/22/00    Time: 5:49p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * 
 * *****************  Version 17  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments


 */

#include "rc_options.h"

#ifdef __PSOS_OS__

#include <psos.h>
#include <pna.h>
#include <stdlib.h>
#include <time.h>

#include "rc_rlstddef.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_rlstdlib.h"
#include "rc_errors.h"
#include "rc_msghdlr.h"

#ifdef __RCC_ENABLED__
#include "rcc.h"
#endif

#define kSPV_STACK_SIZE     8000
#define kUSR_STACK_SIZE     8000
#define kTASK_FLAGS         T_LOCAL
#define kTASK_PRIO          100
#define kTASK_MODE          (T_LOCAL | T_NOFPU )

#define kMinimalSocketTimeout   10

/*-----------------------------------------------------------------------*/
/* Wrappers to pSOS System Calls */
/*-----------------------------------------------------------------------*/

extern RLSTATUS spSOS_MutexCreate ( OS_SPECIFIC_MUTEX *pMutex )
{
    unsigned long   retvalue;
    unsigned long   smid;
    static char     semName[4] = { 'R', 'L', 0, 0 };

    /* increment the 'name' */
   *((ubyte4*)(&semName[0])) += 1;
    retvalue = sm_create( semName, 1, SM_LOCAL | SM_FIFO, &smid ); 
                                                    /* Queueing tasks by FIFO so
                                                     * that no jobs are starved 
                                                     * ( vs. priority-based 
                                                     * queueing, where high 
                                                     * priority jobs might not
                                                     * yield to lower ones...
                                                     */
    if ( retvalue != 0 )
        return SYS_ERROR_MUTEX_CREATE;
    
    *pMutex = (OS_SPECIFIC_MUTEX)smid;      
    return OK; 

} /* spSOS_MutexCreate */



/*-----------------------------------------------------------------------*/

extern RLSTATUS spSOS_MutexWait ( OS_SPECIFIC_MUTEX mutex )
{
    sm_p( mutex, SM_WAIT, 0 );
    return OK;

} /* spSOS_MutexWait */



/*-----------------------------------------------------------------------*/

extern RLSTATUS spSOS_MutexRelease ( OS_SPECIFIC_MUTEX mutex )
{
    sm_v( mutex );
    return OK;

} /* spSOS_MutexRelease */



/*-----------------------------------------------------------------------*/

extern void *pSOS_Malloc ( Length memSize )
{
    return malloc( memSize );   

} /* spSOS_Malloc */



/*-----------------------------------------------------------------------*/

extern void pSOS_Free ( void *pBuffer )
{
    free( pBuffer );

} /* pSOS_Free */



/*-----------------------------------------------------------------------*/

extern void pSOS_Sleep ( void )
{
    tm_wkafter(0);  
                    
} /* pSOS_Sleep */



/*-----------------------------------------------------------------------*/

extern void pSOS_SleepTime(ubyte4 mSecs)
{
	unsigned long ticks = (mSecs * kHwTicksPerSecond ) / 1000;
    tm_wkafter(ticks);  
	
} /* pSOS_SleepTime */



/*-----------------------------------------------------------------------*/

#ifdef __DEBUG_PSOS_SOCKET__

static void pSOS_DEBUG_PACKET(sbyte *pBuf, Counter BufLen)
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
extern RLSTATUS pSOS_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout)
{
    sbyte4          numReaders = 1;

    /* !!!!!!! The following code swatch is necessary for HTTP 1.1.  Tentatively,
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

RLSTATUS pSOS_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
{
    RLSTATUS        BufLen;


    /* !!!!!!! The following code swatch is necessary for HTTP 1.1.  Tentatively,
     * we're using the BSD select() system call to determine if any more data is
     * available. */
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

    if ( BufLen < 0 )   
        return ERROR_GENERAL_ACCESS_DENIED;

#ifdef __DEBUG_PSOS_SOCKET__
    if (BufSize > 1)
        printf("\n\n-----------------> Client Request:\n");

    pSOS_DEBUG_PACKET(pBuf, BufLen);
#endif

    return BufLen;
}



/*-----------------------------------------------------------------------*/

RLSTATUS pSOS_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
    RLSTATUS  err;

#ifdef __DEBUG_PSOS_SOCKET__
    printf("\n\n-----------------> Server Sending:\n");

    pSOS_DEBUG_PACKET(pBuf, BufLen);
#endif

    err = send(sock, pBuf, BufLen, 0);

    if ( err < 0 )
        return ERROR_GENERAL_ACCESS_DENIED;

    return OK;
}



/*-----------------------------------------------------------------------*/

void pSOS_LogError(ubyte4 ErrorLevel, sbyte *ErrorMessage)
{
    if (ErrorLevel > kDebugError)
        printf("ErrLog:  Error Level=%d\t[%s]\n", ErrorLevel, ErrorMessage);
}



/*-----------------------------------------------------------------------*/

extern ubyte4 pSOS_GetSecs(void)
{
    return (ubyte4)(time(NULL));    /* pSOS manual - 3-173 */
}



/*-----------------------------------------------------------------------*/

extern ubyte4 pSOS_GetClientAddr(environment *p_envVar)
{
    struct sockaddr_in  ClientAddr;
    int                 iAddrLen;

    p_envVar->IpAddr = 0;

    iAddrLen = sizeof(ClientAddr);

    if (OK == getpeername(p_envVar->sock, &ClientAddr, &iAddrLen))
        p_envVar->IpAddr = ClientAddr.sin_addr.s_addr;

    return p_envVar->IpAddr;
}

/*-----------------------------------------------------------------------*/

static void pSOS_ThreadWrapper(void *pHandlerFn, void *pArg)
{
    void     (*p_func) (void *) = pHandlerFn;   
	p_func(pArg);
	t_delete(0);
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS pSOS_CreateThread(void* pHandlerFcn, ubyte* TaskName,
                                   void* pArg, ubyte4 priority, 
                                   ubyte4 ext1, void* ext2)
{
    unsigned long               tid;
    char*                       pMsg;
	ubyte4	ActualPriority;
	unsigned long				args[4];

#ifdef __RCC_ENABLED__
	CliChannel *pChannel = pArg;
	OS_SPECIFIC_SOCKET_HANDLE shrSoc = kINVALID_SOCKET;
	OS_SPECIFIC_SOCKET_HANDLE tmpSoc = kINVALID_SOCKET;
#endif

	if ((0==priority) || (255 < priority) )
	{
		if (0 != t_create(TaskName, kTASK_PRIO, kSPV_STACK_SIZE, 
		    kUSR_STACK_SIZE, kTASK_FLAGS, &tid))
		{
		    return ERROR_GENERAL_CREATE_TASK;
		}
	}
	else
	{
		if (0 != t_create(TaskName, priority, kSPV_STACK_SIZE, 
		    kUSR_STACK_SIZE, kTASK_FLAGS, &tid))
		{
		    return ERROR_GENERAL_CREATE_TASK;
		}
	}

#ifdef __RCC_ENABLED__
	/* share the socket with the child task */
	if (NULL != pChannel)
    {	
		shrSoc = shr_socket(pChannel->sock, (int)tid);
		if (kINVALID_SOCKET == shrSoc)
		{
			close(pChannel->sock);
			/*!!!!!!!!!!!!!!!!!!! need to kill the child, if share fails! JAB */
			t_delete(tid);
			return SYS_ERROR_SOCKET_SHARE;
		}

		tmpSoc = pChannel->sock;
		pChannel->sock = shrSoc;
	    close(tmpSoc);
	}
#endif

    /* activate the child task */
	args[0] = (unsigned long) pHandlerFcn;
	args[1] = (unsigned long) pArg;
    if (0 != t_start(tid, kTASK_MODE, pSOS_ThreadWrapper, args ) )
    {
        return ERROR_GENERAL_CREATE_TASK;
    }

    return OK;
	
}	

#endif /* __PSOS_OS__ */
