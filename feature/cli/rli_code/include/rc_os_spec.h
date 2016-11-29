/*  
 *  rc_os_spec.h
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
 Fixed  6/20/2000:
    Fixed header include.
    Modified semaphore support VxWorks to properly support AgentX.
    Added support for UNIX System V.
    maped OS_SPECIFIC_SOCKET_ACCEPT to SOCKET_Accept for all os's
    SSL Socket Abstraction layer added
    Include history and enable auto archiving feature from VSS

*/ 



#ifndef __OS_SPEC_HEADER__
#define __OS_SPEC_HEADER__

/*
 *-------------------------------------------------------------------------------
 *
 * OS Specific typedefs and #defines.
 *  
 */

/* different degrees of errors */
#define kDebugError                 0
#define kWarningError               1
#define kAssertError                2
#define kSevereError                3
#define kUnrecoverableError         4


/*-----------------------------------------------------------------------*/

#ifdef  __WIN32_OS__

#include <winsock.h>

typedef SOCKET                      OS_SPECIFIC_SOCKET_HANDLE;
typedef SOCKET                      OS_SPECIFIC_DGRAM_HANDLE;                            
typedef unsigned int                OS_SPECIFIC_THREAD_HANDLE;
typedef void*                       OS_SPECIFIC_MUTEX;

#define NULLHANDLE                  0
#define NULLMUTEX                   ((void*)0)

/*SSL Begin*/
#ifdef __RLI_SSL_ENABLED__




#define OS_SPECIFIC_MUTEX_CREATE            Win32_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              Win32_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           Win32_MutexRelease
#define OS_SPECIFIC_MALLOC                  Win32_Malloc
#define OS_SPECIFIC_FREE                    Win32_Free
#define OS_SPECIFIC_YIELD                   Win32_Sleep

#define REAL_OS_SPECIFIC_SOCKET_DATA_AVAILABLE  Win32_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE       rc_ssl_SocketDataAvailable

#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect

#define REAL_OS_SPECIFIC_SOCKET_ACCEPT      SOCKET_Accept
#define OS_SPECIFIC_SOCKET_ACCEPT           rc_ssl_accept

#define REAL_OS_SPECIFIC_SOCKET_READ        Win32_SocketRead
#define OS_SPECIFIC_SOCKET_READ             rc_ssl_SocketRead

#define REAL_OS_SPECIFIC_SOCKET_WRITE       Win32_SocketWrite
#define OS_SPECIFIC_SOCKET_WRITE            rc_ssl_SocketWrite

#define REAL_OS_SPECIFIC_SOCKET_CLOSE       SOCKET_Close
#define OS_SPECIFIC_SOCKET_CLOSE            rc_ssl_close

#define OS_SPECIFIC_LOG_ERROR               Win32_LogError
#define OS_SPECIFIC_GET_SECS                Win32_GetSecs

#define REAL_OS_SPECIFIC_GET_ADDR           Win32_GetClientAddr
#define OS_SPECIFIC_GET_ADDR                rc_ssl_GetClientAddr

#define OS_SPECIFIC_SLEEP_TIME              Win32_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           Win32_CreateThread

#else
/*SSL End*/

#define OS_SPECIFIC_MUTEX_CREATE            Win32_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              Win32_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           Win32_MutexRelease
#define OS_SPECIFIC_MALLOC                  Win32_Malloc
#define OS_SPECIFIC_FREE                    Win32_Free
#define OS_SPECIFIC_YIELD                   Win32_Sleep
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   Win32_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect
#define OS_SPECIFIC_SOCKET_READ             Win32_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            Win32_SocketWrite
#define OS_SPECIFIC_SOCKET_CLOSE            SOCKET_Close
#define OS_SPECIFIC_SOCKET_ACCEPT           SOCKET_Accept
#define OS_SPECIFIC_LOG_ERROR               Win32_LogError
#define OS_SPECIFIC_GET_SECS                Win32_GetSecs
#define OS_SPECIFIC_GET_ADDR                Win32_GetClientAddr
#define OS_SPECIFIC_SLEEP_TIME              Win32_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           Win32_CreateThread



#endif  /*__RLI_SSL_ENABLED__*/
#endif /* __WIN32__OS__  */



/*-----------------------------------------------------------------------*/

#ifdef __PSOS_OS__

#include <pna.h>

typedef int                         OS_SPECIFIC_SOCKET_HANDLE;
typedef int                         OS_SPECIFIC_DGRAM_HANDLE;
typedef unsigned long               OS_SPECIFIC_THREAD_HANDLE;
typedef ubyte4                      OS_SPECIFIC_MUTEX;

#define OS_SPECIFIC_MUTEX_CREATE            spSOS_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              spSOS_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           spSOS_MutexRelease
#define OS_SPECIFIC_MALLOC                  pSOS_Malloc
#define OS_SPECIFIC_FREE                    pSOS_Free
#define OS_SPECIFIC_YIELD                   pSOS_Sleep
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   pSOS_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect
#define OS_SPECIFIC_SOCKET_READ             pSOS_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            pSOS_SocketWrite
#define OS_SPECIFIC_SOCKET_CLOSE            SOCKET_Close
#define OS_SPECIFIC_LOG_ERROR               pSOS_LogError
#define OS_SPECIFIC_GET_SECS                pSOS_GetSecs
#define OS_SPECIFIC_GET_ADDR                pSOS_GetClientAddr
#define OS_SPECIFIC_SLEEP_TIME              pSOS_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           pSOS_CreateThread
#define OS_SPECIFIC_SOCKET_ACCEPT           SOCKET_Accept

#endif /* __PSOS_OS__ */



/*-----------------------------------------------------------------------*/

#ifdef __NUCLEUS_OS__

#include <nucleus.h>

typedef sbyte2                      OS_SPECIFIC_SOCKET_HANDLE;
typedef int                         OS_SPECIFIC_DGRAM_HANDLE;
typedef NU_TASK                     OS_SPECIFIC_THREAD_HANDLE;
typedef NU_SEMAPHORE                OS_SPECIFIC_MUTEX;

/* Nucleus requires a memory pool specification.  */

extern  NU_MEMORY_POOL              System_Memory;
#define OS_SPECIFIC_MEMORY_POOL     &System_Memory
/*
extern  NU_MEMORY_POOL              HTTPLMemoryCB;
#define OS_SPECIFIC_MEMORY_POOL     &HTTPLMemoryCB
*/
#define OS_SPECIFIC_MUTEX_CREATE            Nucleus_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              Nucleus_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           Nucleus_MutexRelease
#define OS_SPECIFIC_MALLOC                  Nucleus_Malloc
#define OS_SPECIFIC_FREE                    Nucleus_Free
#define OS_SPECIFIC_YIELD                   Nucleus_Sleep
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   Nucleus_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect
#define OS_SPECIFIC_SOCKET_READ             Nucleus_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            Nucleus_SocketWrite
#define OS_SPECIFIC_SOCKET_CLOSE            SOCKET_Close
#define OS_SPECIFIC_LOG_ERROR               Nucleus_LogError
#define OS_SPECIFIC_GET_SECS                Nucleus_GetSecs
#define OS_SPECIFIC_GET_ADDR                Nucleus_GetClientAddr
#define OS_SPECIFIC_SLEEP_TIME              Nucleus_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           Nucleus_CreateThread
#define OS_SPECIFIC_SOCKET_ACCEPT           SOCKET_Accept

#endif /* __NUCLEUS_OS__ */



/*-----------------------------------------------------------------------*/

#ifdef __CUSTOMER_SPECIFIC_OS__
/* PUT YOUR OWN VALUES HERE... */

typedef int                         OS_SPECIFIC_SOCKET_HANDLE;
typedef int                         OS_SPECIFIC_DGRAM_HANDLE;
typedef int                         OS_SPECIFIC_THREAD_HANDLE;
typedef ubyte4                      OS_SPECIFIC_MUTEX;

#define OS_SPECIFIC_MUTEX_CREATE            Prop_OS_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              Prop_OS_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           Prop_OS_MutexRelease
#define OS_SPECIFIC_MALLOC                  Prop_OS_Malloc
#define OS_SPECIFIC_FREE                    Prop_OS_Free
#define OS_SPECIFIC_YIELD                   Prop_OS_Sleep
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   Prop_OS_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect
#define OS_SPECIFIC_SOCKET_READ             Prop_OS_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            Prop_OS_SocketWrite
#define OS_SPECIFIC_SOCKET_CLOSE            SOCKET_Close
#define OS_SPECIFIC_LOG_ERROR               Prop_OS_LogError
#define OS_SPECIFIC_GET_SECS                Prop_OS_GetSecs
#define OS_SPECIFIC_GET_ADDR                Prop_OS_GetClientAddr
#define OS_SPECIFIC_SLEEP_TIME              Prop_OS_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           Prop_OS_CreateThread
#define OS_SPECIFIC_SOCKET_ACCEPT           SOCKET_Accept

#endif /* __CUSTOMER_SPECIFIC_OS__ */



/*-----------------------------------------------------------------------*/

#ifdef  __VXWORKS_OS__

#ifdef __RC_DISABLE_POSIXS_MUTEX__
#include <semLib.h>
#else
#include <semaphore.h>
#endif

typedef int                         OS_SPECIFIC_SOCKET_HANDLE;
typedef int                         OS_SPECIFIC_DGRAM_HANDLE;
typedef int                         OS_SPECIFIC_THREAD_HANDLE;
#ifdef __RC_DISABLE_POSIXS_MUTEX__
typedef void*                       OS_SPECIFIC_MUTEX;
#else
typedef sem_t*                      OS_SPECIFIC_MUTEX;
#endif

#define OS_SPECIFIC_MUTEX_CREATE            VxWorks_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              VxWorks_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           VxWorks_MutexRelease
#define OS_SPECIFIC_MALLOC                  VxWorks_Malloc
#define OS_SPECIFIC_FREE                    VxWorks_Free
#define OS_SPECIFIC_YIELD                   VxWorks_Sleep
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   VxWorks_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect
#define OS_SPECIFIC_SOCKET_READ             VxWorks_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            VxWorks_SocketWrite
#define OS_SPECIFIC_SOCKET_CLOSE            SOCKET_Close
#define OS_SPECIFIC_LOG_ERROR               VxWorks_LogError
#define OS_SPECIFIC_GET_SECS                VxWorks_GetSecs
#define OS_SPECIFIC_GET_ADDR                VxWorks_GetClientAddr
#define OS_SPECIFIC_SLEEP_TIME              VxWorks_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           VxWorks_CreateThread
#define OS_SPECIFIC_SOCKET_ACCEPT           SOCKET_Accept

#endif /* __VXWORKS_OS__ */

/*-----------------------------------------------------------------------*/

#ifdef  __FreeRTOS_OS__

#include <FreeRTOS.h>
#include <semphr.h>

typedef int                         OS_SPECIFIC_SOCKET_HANDLE;
typedef int                         OS_SPECIFIC_DGRAM_HANDLE;
typedef int                         OS_SPECIFIC_THREAD_HANDLE;
typedef xSemaphoreHandle            OS_SPECIFIC_MUTEX;

#define OS_SPECIFIC_MUTEX_CREATE            FreeRTOS_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              FreeRTOS_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           FreeRTOS_MutexRelease
#define OS_SPECIFIC_MALLOC                  FreeRTOS_Malloc
#define OS_SPECIFIC_FREE                    FreeRTOS_Free
#define OS_SPECIFIC_YIELD                   FreeRTOS_Sleep
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   FreeRTOS_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect
#define OS_SPECIFIC_SOCKET_READ             FreeRTOS_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            FreeRTOS_SocketWrite
#define OS_SPECIFIC_SOCKET_CLOSE            SOCKET_Close
#define OS_SPECIFIC_LOG_ERROR               FreeRTOS_LogError
#define OS_SPECIFIC_GET_SECS                FreeRTOS_GetSecs
#define OS_SPECIFIC_GET_ADDR                FreeRTOS_GetClientAddr
#define OS_SPECIFIC_SLEEP_TIME              FreeRTOS_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           FreeRTOS_CreateThread
#define OS_SPECIFIC_SOCKET_ACCEPT           SOCKET_Accept

#endif /* __FreeRTOS_OS__ */

/*-----------------------------------------------------------------------*/

#ifdef  __POSIX_OS__

#include <semaphore.h> 

typedef int                         OS_SPECIFIC_SOCKET_HANDLE;
typedef int                         OS_SPECIFIC_DGRAM_HANDLE;
typedef int                         OS_SPECIFIC_THREAD_HANDLE;
typedef sem_t *                     OS_SPECIFIC_MUTEX;

#ifdef __RLI_SSL_ENABLED__

#define OS_SPECIFIC_MUTEX_CREATE            Posix_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              Posix_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           Posix_MutexRelease
#define OS_SPECIFIC_MALLOC                  Posix_Malloc
#define OS_SPECIFIC_FREE                    Posix_Free
#define OS_SPECIFIC_YIELD                   Posix_Sleep

#define REAL_OS_SPECIFIC_SOCKET_DATA_AVAILABLE   Posix_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE        rc_ssl_SocketDataAvailable

/* connects aren't SSL: */
#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect 

#define REAL_OS_SPECIFIC_SOCKET_ACCEPT      SOCKET_Accept
#define OS_SPECIFIC_SOCKET_ACCEPT           rc_ssl_accept

#define REAL_OS_SPECIFIC_SOCKET_READ        Posix_SocketRead
#define OS_SPECIFIC_SOCKET_READ             rc_ssl_SocketRead

#define REAL_OS_SPECIFIC_SOCKET_WRITE       Posix_SocketWrite
#define OS_SPECIFIC_SOCKET_WRITE            rc_ssl_SocketWrite

#define REAL_OS_SPECIFIC_SOCKET_CLOSE       SOCKET_Close
#define OS_SPECIFIC_SOCKET_CLOSE            rc_ssl_close

#define OS_SPECIFIC_LOG_ERROR               Posix_LogError
#define OS_SPECIFIC_GET_SECS                Posix_GetSecs

#define REAL_OS_SPECIFIC_GET_ADDR           Posix_GetClientAddr
#define OS_SPECIFIC_GET_ADDR                rc_ssl_GetClientAddr

#define OS_SPECIFIC_SLEEP_TIME              Posix_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           Posix_CreateThread

/* have to do this here for the fuction pointers in rc_ssl.c to
** come out right: */
RLSTATUS Posix_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen);
RLSTATUS Posix_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize);



#else /* __RLI_SSL_ENABLED__ */


#define OS_SPECIFIC_MUTEX_CREATE            Posix_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              Posix_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           Posix_MutexRelease
#define OS_SPECIFIC_MALLOC                  Posix_Malloc
#define OS_SPECIFIC_FREE                    Posix_Free
#define OS_SPECIFIC_YIELD                   Posix_Sleep
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   Posix_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_ACCEPT           SOCKET_Accept /*Needed for SSL*/
#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect
#define OS_SPECIFIC_SOCKET_READ             Posix_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            Posix_SocketWrite
#define OS_SPECIFIC_SOCKET_CLOSE            SOCKET_Close
#define OS_SPECIFIC_LOG_ERROR               Posix_LogError
#define OS_SPECIFIC_GET_SECS                Posix_GetSecs
#define OS_SPECIFIC_GET_ADDR                Posix_GetClientAddr
#define OS_SPECIFIC_SLEEP_TIME              Posix_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           Posix_CreateThread

#endif /*__RLI_SSL_ENABLED__*/
#endif /* __POSIX_OS__ */



/*-----------------------------------------------------------------------*/

#ifdef  __NETBSD_OS__

#include <sys/types.h>
#include <sys/ipc.h>

typedef int                         OS_SPECIFIC_SOCKET_HANDLE;
typedef int                         OS_SPECIFIC_DGRAM_HANDLE;
typedef int                         OS_SPECIFIC_THREAD_HANDLE;
typedef void*                       OS_SPECIFIC_MUTEX;

#define OS_SPECIFIC_MUTEX_CREATE            NETBSD_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              NETBSD_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           NETBSD_MutexRelease
#define OS_SPECIFIC_MALLOC                  NETBSD_Malloc
#define OS_SPECIFIC_FREE                    NETBSD_Free
#define OS_SPECIFIC_YIELD                   NETBSD_Sleep
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   NETBSD_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_ACCEPT           SOCKET_Accept
#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect
#define OS_SPECIFIC_SOCKET_READ             NETBSD_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            NETBSD_SocketWrite
#define OS_SPECIFIC_SOCKET_CLOSE            SOCKET_Close
#define OS_SPECIFIC_LOG_ERROR               NETBSD_LogError
#define OS_SPECIFIC_GET_SECS                NETBSD_GetSecs
#define OS_SPECIFIC_GET_ADDR                NETBSD_GetClientAddr
#define OS_SPECIFIC_SLEEP_TIME              NETBSD_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           NETBSD_CreateThread

#endif /* __NETBSD_OS__ */

/*-----------------------------------------------------------------------*/
#ifdef  __VRTX_OS__

#ifdef __SOCK_ATTACHE_TCPIP_STACK__
typedef void*                       OS_SPECIFIC_SOCKET_HANDLE;
#else
typedef int                         OS_SPECIFIC_SOCKET_HANDLE;
#endif

typedef int                         OS_SPECIFIC_DGRAM_HANDLE;                            
typedef int                         OS_SPECIFIC_THREAD_HANDLE;
typedef int                         OS_SPECIFIC_MUTEX;

#define OS_SPECIFIC_MUTEX_CREATE            VRTX_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              VRTX_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           VRTX_MutexRelease
#define OS_SPECIFIC_MALLOC                  VRTX_Malloc
#define OS_SPECIFIC_FREE                    VRTX_Free
#define OS_SPECIFIC_YIELD                   VRTX_Sleep

#ifdef __SOCK_ATTACHE_TCPIP_STACK__
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   ATTACHE_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_READ             ATTACHE_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            ATTACHE_SocketWrite
#else
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   VRTX_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_READ             VRTX_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            VRTX_SocketWrite
#endif

#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect
#define OS_SPECIFIC_SOCKET_CLOSE            SOCKET_Close
#define OS_SPECIFIC_LOG_ERROR               VRTX_LogError
#define OS_SPECIFIC_GET_SECS                VRTX_GetSecs
#define OS_SPECIFIC_GET_ADDR                VRTX_GetClientAddr
#define OS_SPECIFIC_SLEEP_TIME              VRTX_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           VRTX_CreateThread
#define OS_SPECIFIC_SOCKET_ACCEPT           SOCKET_Accept

#endif /* __VRTX_OS__  */

/*-----------------------------------------------------------------------*/
#ifdef  __CHORUS_OS__

#include <semaphore.h> 

typedef int                         OS_SPECIFIC_SOCKET_HANDLE;
typedef int                         OS_SPECIFIC_DGRAM_HANDLE;
typedef int                         OS_SPECIFIC_THREAD_HANDLE;
typedef sem_t *                     OS_SPECIFIC_MUTEX;

#define OS_SPECIFIC_MUTEX_CREATE            CHORUS_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              CHORUS_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           CHORUS_MutexRelease
#define OS_SPECIFIC_MALLOC                  CHORUS_Malloc
#define OS_SPECIFIC_FREE                    CHORUS_Free
#define OS_SPECIFIC_YIELD                   CHORUS_Sleep
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   CHORUS_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect
#define OS_SPECIFIC_SOCKET_READ             CHORUS_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            CHORUS_SocketWrite
#define OS_SPECIFIC_SOCKET_CLOSE            SOCKET_Close
#define OS_SPECIFIC_LOG_ERROR               CHORUS_LogError
#define OS_SPECIFIC_GET_SECS                CHORUS_GetSecs
#define OS_SPECIFIC_GET_ADDR                CHORUS_GetClientAddr
#define OS_SPECIFIC_SLEEP_TIME              CHORUS_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           CHORUS_CreateThread
#define OS_SPECIFIC_SOCKET_ACCEPT           SOCKET_Accept

#endif /* __CHORUS_OS__  */



/*-----------------------------------------------------------------------*/

#ifdef __SYSTEMV_OS__

typedef int                         OS_SPECIFIC_SOCKET_HANDLE;
typedef int                         OS_SPECIFIC_DGRAM_HANDLE;
typedef int                         OS_SPECIFIC_THREAD_HANDLE;
typedef int                         OS_SPECIFIC_MUTEX;

#define OS_SPECIFIC_MUTEX_CREATE            SystemV_MutexCreate
#define OS_SPECIFIC_MUTEX_WAIT              SystemV_MutexWait
#define OS_SPECIFIC_MUTEX_RELEASE           SystemV_MutexRelease
#define OS_SPECIFIC_MALLOC                  SystemV_Malloc
#define OS_SPECIFIC_FREE                    SystemV_Free
#define OS_SPECIFIC_YIELD                   SystemV_Sleep
#define OS_SPECIFIC_SOCKET_DATA_AVAILABLE   SystemV_SocketDataAvailable
#define OS_SPECIFIC_SOCKET_CONNECT          SOCKET_Connect
#define OS_SPECIFIC_SOCKET_READ             SystemV_SocketRead
#define OS_SPECIFIC_SOCKET_WRITE            SystemV_SocketWrite
#define OS_SPECIFIC_SOCKET_CLOSE            SOCKET_Close
#define OS_SPECIFIC_LOG_ERROR               SystemV_LogError
#define OS_SPECIFIC_GET_SECS                SystemV_GetSecs
#define OS_SPECIFIC_GET_ADDR                SystemV_GetClientAddr
#define OS_SPECIFIC_SLEEP_TIME              SystemV_SleepTime
#define OS_SPECIFIC_CREATE_THREAD           SystemV_CreateThread
#define OS_SPECIFIC_SOCKET_ACCEPT           SOCKET_Accept

#endif /* __SYSTEMV_OS__ */

/*-----------------------------------------------------------------------*/
/* rc_environ.h has the relevant environment structure, plus function prototypes,
 * used to monitor the usage of the CGI Handlers.
 */
#include "rc_environ.h"


/* Mutex Semaphores */
extern RLSTATUS   OS_SPECIFIC_MUTEX_CREATE  ( OS_SPECIFIC_MUTEX *pMutex );
extern RLSTATUS   OS_SPECIFIC_MUTEX_WAIT    ( OS_SPECIFIC_MUTEX mutex   );
extern RLSTATUS   OS_SPECIFIC_MUTEX_RELEASE ( OS_SPECIFIC_MUTEX mutex   );

/* Thread Management */
extern void         OS_SPECIFIC_YIELD   ( void );
extern void         OS_SPECIFIC_SLEEP_TIME (ubyte4 mSecs);
extern RLSTATUS     OS_SPECIFIC_CREATE_THREAD(void* pHandlerFcn, ubyte* TaskName, 
                                              void* pArg, ubyte4 priority,
                                              ubyte4 ext1, void* ext2  );

/* Memory Management */
extern void *   OS_SPECIFIC_MALLOC          ( Length memSize    );
extern void     OS_SPECIFIC_FREE            ( void *pBuffer     );

/* Socket Management */
extern RLSTATUS   OS_SPECIFIC_SOCKET_CONNECT(OS_SPECIFIC_SOCKET_HANDLE *pSock, sbyte *pName, ubyte4 port    );
extern RLSTATUS   OS_SPECIFIC_SOCKET_CLOSE  (OS_SPECIFIC_SOCKET_HANDLE sock );
extern RLSTATUS   OS_SPECIFIC_SOCKET_DATA_AVAILABLE  (OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout );
extern RLSTATUS   OS_SPECIFIC_SOCKET_READ   (OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize   );
extern RLSTATUS   OS_SPECIFIC_SOCKET_WRITE  (OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen    );

/* Environment Variables */
extern ubyte4   OS_SPECIFIC_GET_SECS(void);
extern ubyte4   OS_SPECIFIC_GET_ADDR(environment *p_envVar);

/* Error Log Interface */
extern void     OS_SPECIFIC_LOG_ERROR(ubyte4 ErrorLevel, sbyte *ErrorMessage);

#endif
