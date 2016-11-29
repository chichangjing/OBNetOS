/*
 *  rc_nucleus.c
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

$History: rc_nucleus.c $
 * 
 * *****************  Version 27  *****************
 * User: James        Date: 12/18/00   Time: 7:19p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * 1 is the smallest amount of time required to do a yield, zero causes a
 * deep sleep.  JAB
 * 
 * *****************  Version 26  *****************
 * User: Pstuart      Date: 11/13/00   Time: 5:05p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * if task failed didn't release mutex.
 * 
 * *****************  Version 25  *****************
 * User: James        Date: 9/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * For Nucleus, added spooling for writes, to prevent the TCP/IP from
 * choking on large chunks.  Fixed GetSecs().  Per customer suggestion,
 * fixed child task data so that it's stored on the heap, and not on the
 * stack.  
 * 
 * *****************  Version 24  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 23  *****************
 * User: Leech        Date: 6/14/00    Time: 3:44p
 * Fixed task control structure corruption.  Added code to track task
 * control elements (control struct, stack allocation) within
 * CreateThread() call.
 * 
 * *****************  Version 22  *****************
 * User: David        Date: 6/09/00    Time: 2:01p
 * Cleaned up some warnings.
 * 
 * *****************  Version 20  *****************
 * User: James        Date: 5/25/00    Time: 11:38a
 * Cleaned-up the thread creation code.
 *
 * *****************  Version 19  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/
#include "rc_options.h"
#include "rc.h"

#ifdef __NUCLEUS_OS__

/* Nucleus glue should include all of these files..*/
#include "nucleus.h"

/* Nucleus glue and sockets files */

#include <target.h>
#include <sockext.h>

/* Files for Socket-Like Integ */
#include <externs.h>
#include <socketd.h>    /* socket interface structures */

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"


/*NU_MEMORY_POOL              HTTPLMemoryCB; */

/*-----------------------------------------------------------------------*/

/* Wrappers to Nucleus System Calls */
extern RLSTATUS Nucleus_MutexCreate(OS_SPECIFIC_MUTEX *pMutex)
{
    STATUS    nuStatus;

    nuStatus = NU_Create_Semaphore(pMutex, "nuSem", 1, NU_FIFO);
    if (NU_SUCCESS != nuStatus)
        return SYS_ERROR_MUTEX_CREATE;

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS Nucleus_MutexWait(OS_SPECIFIC_MUTEX mutex)
{
    NU_Obtain_Semaphore(&mutex, NU_SUSPEND);
    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS Nucleus_MutexRelease(OS_SPECIFIC_MUTEX mutex)
{
    NU_Release_Semaphore(&mutex);
    return OK;
}



/*-----------------------------------------------------------------------*/

extern void *Nucleus_Malloc(Length memSize)
{
    void    *pReturnPtr;
    STATUS  nuStatus;

    nuStatus = NU_Allocate_Memory(OS_SPECIFIC_MEMORY_POOL, &pReturnPtr,
                                  memSize, NU_NO_SUSPEND);
    if (NU_SUCCESS != nuStatus)
        return NULL;

    return pReturnPtr;
}



/*-----------------------------------------------------------------------*/

extern void Nucleus_Free(void *pBuffer)
{
    NU_Deallocate_Memory(pBuffer);
}



/*-----------------------------------------------------------------------*/

extern void Nucleus_Sleep(void)
{
    NU_Sleep(1);                       /* yield the processor */
}



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_RTOS_NETWORK_STACK__

extern RLSTATUS
Nucleus_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
{
    sbyte4  bufLen;

#if 0
    sbyte4          numReaders;
    fd_set          fdSocks;
    struct timeval  tTimeout;

    /* The following code prevents the reader from blocking ad infinitum on the
     * socket. */


    FD_ZERO(&fdSocks);
    FD_SET(sock, &fdSocks);

    tTimeout.tv_sec  = kSocketTimeOut;
    tTimeout.tv_usec = 0;
    numReaders = select(1, &fdSocks, 0, 0, &tTimeout);
	if (0 == numReaders)
	{
		return SYS_ERROR_SOCKET_TIMEOUT;
	}
    if (1 > numReaders)
    {
        return SYS_ERROR_SOCKET_GENERAL;
    }
#endif

    /* Receive BufSize number of bytes ...??????? - Make it a blocking read ?*/
    NU_Fcntl (sock, NU_SETFLAG, NU_BLOCK);
    bufLen = NU_Recv(sock, pBuf, (sbyte2)BufSize, 0);
    NU_Fcntl (sock, NU_SETFLAG, NU_FALSE);

    if (0 > bufLen)
        return ERROR_GENERAL_ACCESS_DENIED;

    return bufLen;
}


/*-----------------------------------------------------------------------*/

#define kMaxBytesBytesToWrite 4096

extern RLSTATUS
Nucleus_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
	Counter bytesToSend, bytesSent;
    sbyte4  sentLen;

	bytesSent = 0;

	/* don't overload the TCP/IP stack, spool the data out to the socket. JAB */
	while (bytesSent < BufLen)
	{
		bytesToSend = (BufLen - bytesSent);

		if (kMaxBytesBytesToWrite < bytesToSend)
		{
			/* only yield the processor, if we have written a big chunk of data. JAB */
			bytesToSend = kMaxBytesBytesToWrite;

			sentLen = NU_Send(sock, &pBuf[bytesSent], (sbyte2)bytesToSend, 0);
			if (0 > sentLen)
				return ERROR_GENERAL_ACCESS_DENIED;

		    NU_Sleep(10);                       /* !-!-!-!-! yield the processor, may need to adjust */
		}
		else
		{
			/* no need to yield */
			sentLen = NU_Send(sock, &pBuf[bytesSent], (sbyte2)bytesToSend, 0);
			if (0 > sentLen)
				return ERROR_GENERAL_ACCESS_DENIED;
		}

		bytesSent += bytesToSend;
	}

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS
Nucleus_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout)
{
	sbyte2			max_sockets = NSOCKETS; /* 1 */
    FD_SET          fdSocks;
    STATUS          status;
    
    NU_FD_Init(&fdSocks);
    NU_FD_Set(sock, &fdSocks);

    if (timeout == (-1))
	{
        timeout = NU_SUSPEND;
    }
	else
	{
	    timeout = timeout * kHwTicksPerSecond; /* !!!! replace with appropriate constant for given system */
    }

	status = NU_Select(max_sockets, &fdSocks, NULL, NULL, timeout);

    if (status != NU_SUCCESS)
	{
        if (status == NU_NO_DATA)
            return SYS_ERROR_SOCKET_TIMEOUT;
        else
		    return SYS_ERROR_SOCKET_GENERAL;
    }

	return OK;
}

#endif /* __DISABLE_RTOS_NETWORK_STACK__ */



/*-----------------------------------------------------------------------*/

extern ubyte4 Nucleus_GetSecs(void)
{
    /* need method for retrieving the system time in seconds */
	ubyte4 hwTicks = NU_Retrieve_Clock();

    return hwTicks / kHwTicksPerSecond;
}


/*-----------------------------------------------------------------------*/

extern ubyte4 Nucleus_GetClientAddr(environment *p_envVar)
{
    return 0;
}



/*-----------------------------------------------------------------------*/

extern void
Nucleus_LogError(ubyte4 ErrorLevel, sbyte *ErrorMessage)
{
/*    if (ErrorLevel > kDebugError)
        printf("ErrLog:  Error Level=%d\t[%s]\n", ErrorLevel, ErrorMessage);
*/
    ;
}



/*-----------------------------------------------------------------------*/

static VOID NucleusWrapper(UNSIGNED argc, VOID *pArgv)
{
    void (*p_funcEntryPoint)(void *);
	void** ppArgv = (void **)pArgv;

	p_funcEntryPoint = ppArgv[0];

	p_funcEntryPoint((void *)(ppArgv[1]));

}



/*-----------------------------------------------------------------------*/

typedef struct rc_nucleus_task
{
	NU_TASK		nu_task;
	sbyte		stack[kOCP_THREAD_STACK_SIZE];
	sbyte4		stackLen;
	struct rc_nucleus_task *pPrev;
	struct rc_nucleus_task *pNext;
	int			argc;
	void* 		argv[2];

} rc_nucleus_task;


rc_nucleus_task *m_nucleus_taskList = NULL;
OS_SPECIFIC_MUTEX	m_nucleus_task_mutex;
sbyte4				m_nucleus_task_init = 0;

/*-----------------------------------------------------------------------*/

/* adds a new task to the tail of the global task list */

static rc_nucleus_task * Nucleus_AddTask(rc_nucleus_task *pTaskList)
{
	rc_nucleus_task *pNewTask = RC_MALLOC(sizeof(rc_nucleus_task));
	rc_nucleus_task *pTmpTask = pTaskList;

	if (NULL == pNewTask)
		return NULL;

    MEMSET(pNewTask, 0x00, sizeof(rc_nucleus_task));
	pNewTask->stackLen = kOCP_THREAD_STACK_SIZE;

	if (NULL == pTmpTask)
		m_nucleus_taskList = pNewTask;
	else
	{
		while(NULL != pTmpTask->pNext)
			pTmpTask = pTmpTask->pNext;
		pTmpTask->pNext = pNewTask;
		pNewTask->pPrev = pTmpTask;
	}
	return pNewTask;
}


/*-----------------------------------------------------------------------*/

static void Nucleus_RemoveTask(rc_nucleus_task *pTask, DATA_ELEMENT taskStatus)
{
	if (NULL == pTask->pPrev)
	{
		/* head of task list */
		if (NULL != pTask->pNext)
		{
			pTask->pNext->pPrev = NULL;
			m_nucleus_taskList = pTask->pNext;
		}
		else
			m_nucleus_taskList = NULL;
	}
	else
	{
		/* end of task list */
		if (NULL == pTask->pNext)
		{
			pTask->pPrev->pNext = NULL;
		}
		/* middle of task list */
		else
		{
			pTask->pPrev->pNext = pTask->pNext;
			pTask->pNext->pPrev = pTask->pPrev;
		}
	}
	if(NU_FINISHED == taskStatus)
		NU_Delete_Task(&pTask->nu_task);
	RC_FREE(pTask);
	
	return;
}

/*-----------------------------------------------------------------------*/

static void Nucleus_CleanupTaskList(rc_nucleus_task *pTaskList)
{
    rc_nucleus_task *pTask = pTaskList;
    rc_nucleus_task *pNextTask;
	STATUS status;
	DATA_ELEMENT taskStatus;

	/* useless storage fields for the NU_Task_Information call...*/
	sbyte name[8];
	UNSIGNED scheduledCount, stacksize, minimumStack, timeSlice;
	OPTION priority, preempt;
	void *stackBase;

	while (NULL != pTask)
	{
		status = NU_Task_Information(&pTask->nu_task, name, &taskStatus, &scheduledCount, &priority, 
									 &preempt, &timeSlice, &stackBase, &stacksize, &minimumStack);
		pNextTask = pTask->pNext;
		if ((NU_INVALID_TASK == status) || (NU_TERMINATED == taskStatus) ||
			(NU_FINISHED == taskStatus))
			Nucleus_RemoveTask(pTask, taskStatus);
		pTask = pNextTask;
	}
	return;
}

/*-----------------------------------------------------------------------*/


extern RLSTATUS Nucleus_CreateThread(void* pHandlerFcn, ubyte* TaskName,
                                     void* pArg, ubyte4 priority,
                                     ubyte4 ext1, void* ext2)
{
    STATUS      nuStatus;
	rc_nucleus_task *pTask = NULL;

	if (0 == m_nucleus_task_init)
	{
		if (OK > Nucleus_MutexCreate(&m_nucleus_task_mutex))
			return ERROR_GENERAL_CREATE_TASK;
		m_nucleus_task_init = 1;
	}

	Nucleus_MutexWait(m_nucleus_task_mutex);
	Nucleus_CleanupTaskList(m_nucleus_taskList);
	pTask =  Nucleus_AddTask(m_nucleus_taskList);

	if (NULL == pTask)
		return ERROR_GENERAL_CREATE_TASK;

    pTask->argc = 2;
	pTask->argv[0] = pHandlerFcn;
	pTask->argv[1] = pArg;

    if ((0==priority) || (255 < priority))
        priority = kOCP_SERVER_PRIO;

    nuStatus = NU_Create_Task(&pTask->nu_task, (char*)TaskName, NucleusWrapper,
	                          pTask->argc, pTask->argv, pTask->stack,
	                          pTask->stackLen, (ubyte) priority, 0,
	                          NU_PREEMPT, NU_START);


	Nucleus_MutexRelease(m_nucleus_task_mutex);

    if (NU_SUCCESS != nuStatus)
        return ERROR_GENERAL_CREATE_TASK;

    return OK;
}

#endif /* __NUCLEUS_OS__ */

