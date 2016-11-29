
#include "rc_options.h"

#ifdef __FreeRTOS_OS__

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* LwIP include */
#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"


#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_socks.h"


#define kMinimalSocketTimeout   10
/*-----------------------------------------------------------------------*/

extern RLSTATUS FreeRTOS_MutexCreate(OS_SPECIFIC_MUTEX *pMutex)
{
    RLSTATUS  status = OK;

    *pMutex = xSemaphoreCreateMutex();

    if ( NULL == *pMutex )
        status = SYS_ERROR_MUTEX_CREATE;
    
    return status; 
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS FreeRTOS_MutexWait(OS_SPECIFIC_MUTEX mutex)
{
	portTickType StartTime, EndTime, Elapsed;

	StartTime = xTaskGetTickCount();
	while( xSemaphoreTake( mutex, portMAX_DELAY ) != pdTRUE ){}
	EndTime = xTaskGetTickCount();
	Elapsed = (EndTime - StartTime) * portTICK_RATE_MS;

	return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS FreeRTOS_MutexRelease(OS_SPECIFIC_MUTEX mutex)
{
	xSemaphoreGive(mutex);
	return OK;
}

/*-----------------------------------------------------------------------*/

extern void *FreeRTOS_Malloc(Length memSize)
{
	//printf("FreeRTOS_Malloc : %d\r\n",memSize);
    return pvPortMalloc(memSize);
}

/*-----------------------------------------------------------------------*/

extern void FreeRTOS_Free(void *pBuffer)
{
    vPortFree(pBuffer);
}

/*-----------------------------------------------------------------------*/

extern void FreeRTOS_Sleep(void)
{
    vTaskDelay(0);                       /* yield the processor */
}

extern void FreeRTOS_SleepTime(ubyte4 mSecs)
{
	int ticks = (mSecs * kHwTicksPerSecond ) / 1000;
    vTaskDelay(ticks);
}

/*-----------------------------------------------------------------------*/

RLSTATUS FreeRTOS_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
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

RLSTATUS FreeRTOS_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
    RLSTATUS  err;

    err = send(sock, pBuf, BufLen, 0);

    if (-1 == err)
        return ERROR_GENERAL_ACCESS_DENIED;

    return OK;
}



/*-----------------------------------------------------------------------*/
extern RLSTATUS 
FreeRTOS_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout)
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

extern ubyte4 FreeRTOS_GetSecs(void)
{
    return (ubyte4)(xTaskGetTickCount() / kHwTicksPerSecond);
}



/*-----------------------------------------------------------------------*/

extern ubyte4 FreeRTOS_GetClientAddr(environment *p_envVar)
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

void FreeRTOS_LogError(ubyte4 ErrorLevel, sbyte *ErrorMessage)
{
    if (ErrorLevel > kDebugError)
        printf("ErrLog:  Error Level=%d\t[%s]\n", (int)ErrorLevel, ErrorMessage);
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS FreeRTOS_CreateThread(void *pHandlerFcn, ubyte* TaskName,
                                     void* pArg, ubyte4 priority,
                                     ubyte4 ext1, void* ext2)
{
#if 1
    int     status;
	
	if ((0==priority)||(255<priority))
	{
		status = xTaskCreate( (pdTASK_CODE)pHandlerFcn, (signed char const *)TaskName, kSTANDARD_RC_THREAD_STACK_SIZE, pArg, kSTANDARD_RC_THREAD_PRIO, ext2 );
	}
	else
	{
		status = xTaskCreate( (pdTASK_CODE)pHandlerFcn, (signed char const *)TaskName, kSTANDARD_RC_THREAD_STACK_SIZE, pArg, priority, ext2 );
	}

    if (ERROR == status)
        return ERROR_GENERAL_CREATE_TASK;

    return OK;
#else
    int     status;
	sys_thread_t CreatedThread;
	
	if ((0==priority)||(255<priority))
	{
		//status = xTaskCreate( (pdTASK_CODE)pHandlerFcn, (signed char const *)TaskName, kSTANDARD_RC_THREAD_STACK_SIZE, pArg, kSTANDARD_RC_THREAD_PRIO, ext2 );
		CreatedThread = sys_thread_new((char *)TaskName, (pdTASK_CODE)pHandlerFcn, pArg, kSTANDARD_RC_THREAD_STACK_SIZE, kSTANDARD_RC_THREAD_PRIO);
	}
	else
	{
		//status = xTaskCreate( (pdTASK_CODE)pHandlerFcn, (signed char const *)TaskName, kSTANDARD_RC_THREAD_STACK_SIZE, pArg, priority, ext2 );
		CreatedThread = sys_thread_new((char *)TaskName, (pdTASK_CODE)pHandlerFcn, pArg, kSTANDARD_RC_THREAD_STACK_SIZE, priority);
	}
	
#if 1
    if (NULL == CreatedThread)
        return ERROR_GENERAL_CREATE_TASK;	
	*(sys_thread_t *)ext2 = CreatedThread;
#else
    if (ERROR == status)
        return ERROR_GENERAL_CREATE_TASK;
#endif
	
    return OK;
#endif
}


#endif /* __FreeRTOS_OS__ */



