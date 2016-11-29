/*  
 *  rc_socks.h
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





#ifndef __SOCKS_HEADER__
#define __SOCKS_HEADER__

#include "rc_tcpd.h"

#ifdef __RLIWSA_ENABLED__
#include "wsa.h"
#endif /* __RLIWSA_ENABLED__ */

/*-----------------------------------------------------------------------*/

/* constants */
enum    RL_ThreadStates     {kThreadCreate,  kThreadIdle,     kThreadSnmpSync, 
                             kThreadWorking, kThreadFinished, kThreadDead };

/* Defaults */
#define kFIXED_PORT                     kHTTP_FIXED_PORT

#ifdef  kRCC_THREAD_STACK_SIZE
#define kSTANDARD_RC_THREAD_STACK_SIZE  kRCC_THREAD_STACK_SIZE
#else
#define kSTANDARD_RC_THREAD_STACK_SIZE  1024
#endif

#ifdef  kRCC_SERVER_PRIO
#define kSTANDARD_RC_THREAD_PRIO        kRCC_SERVER_PRIO
#else
#define kSTANDARD_RC_THREAD_PRIO        100
#endif

#define kHTTP_THREAD_STACK_SIZE         kSTANDARD_RC_THREAD_STACK_SIZE
#define kHTTPD_SERVER_PRIO              kSTANDARD_RC_THREAD_PRIO

#define kOCP_THREAD_STACK_SIZE         kSTANDARD_RC_THREAD_STACK_SIZE
#define kOCP_SERVER_PRIO              kSTANDARD_RC_THREAD_PRIO

#ifndef kMaxUdpPacketSize
#define kMaxUdpPacketSize 512
#endif

#define kTCP_QUALBUFSIZE    16

/* kHTTPD_QUEUE_SIZE determines number of PCC objects in array (# TCP threads) */
#ifndef kHTTPD_QUEUE_SIZE
#ifdef kOCP_TCPQUEUE_SIZE
#define kHTTPD_QUEUE_SIZE kOCP_TCPQUEUE_SIZE
#else
#define kHTTPD_QUEUE_SIZE 10
#endif
#endif

/* kTCP_LISTEN_BACKLOG determines value of arg backlog in socket listen. */
/* This argument sets the maximum length of the queue of pending connections. */
#define kTCP_LISTEN_BACKLOG 20

/*-----------------------------------------------------------------------*/

typedef struct UdpParams
{
    sbyte *pRecPacket;
	sbyte *pSendPacket;
    ubyte4 clientAddr;
    ubyte4 sendPacketLength;
    ubyte4 recvPacketLength;
    ubyte2 clientPort;

} UdpParams;


/*-----------------------------------------------------------------------*/

typedef struct ProcessComChannel
{
    OS_SPECIFIC_SOCKET_HANDLE   sock;
    Boolean                     InUse;      /* if TRUE, there is a request on this com channel ... */
    sbyte4                      ThreadState;
    ubyte4                      IpAddr;
    unsigned int                index;

    void*                       env;
    void*                       pdu;
    void (*pfuncConnHandler)(void *pccObject);
    sbyte                       Name[kTCPd_TASKNAME_LENGTH];
#ifdef __COMMON_LISTENER__
    sbyte                       QualBuff[kTCP_QUALBUFSIZE]; 
    Length                      QBytesRead;
    sbyte2                      ListenerIndex;
#endif


#ifdef __NUCLEUS_OS__

    NU_TASK                     task;
    void*                       stackAddress;
    ubyte4                      stackLen;

#endif

#ifdef __SYSTEMV_OS__
    int                         pid;
#endif

#ifdef __OCP_ENABLED__
    ubyte4                      ConnID;
#endif

#ifdef __RLIWSA_ENABLED__
    WsEnvironment               theWsEnv;
#endif /* __RLIWSA_ENABLED__ */
} ProcessComChannel;



/*-----------------------------------------------------------------------*/

/* prototypes */

#ifdef __cplusplus
extern "C" {
#endif

#if ((defined __COMMON_LISTENER__) || (defined __WEBCONTROL_BLADE_ENABLED__))
extern ProcessComChannel g_pccObjects[kHTTPD_QUEUE_SIZE];
#endif

extern RLSTATUS SOCKET_UdpBindFixed  
                                (OS_SPECIFIC_SOCKET_HANDLE *sock, ubyte4 port);
extern RLSTATUS SOCKET_UdpBindAny  
                                (OS_SPECIFIC_SOCKET_HANDLE *sock);
extern RLSTATUS SOCKET_Initialize( void );
extern RLSTATUS SOCKET_UdpReceive(OS_SPECIFIC_SOCKET_HANDLE soc, UdpParams *pParams);
extern RLSTATUS SOCKET_UdpSendTo(OS_SPECIFIC_SOCKET_HANDLE sock, UdpParams *pParams);
extern RLSTATUS SOCKET_Close    (OS_SPECIFIC_SOCKET_HANDLE sock);

extern RLSTATUS SOCKET_Accept(OS_SPECIFIC_SOCKET_HANDLE *soc, 
                              OS_SPECIFIC_SOCKET_HANDLE *accSoc,
                              ubyte4 port);
#ifdef __WIN32_OS__
/*elp begin win32 protos*/

extern ubyte4 Win32_GetClientAddr(environment *p_envVar);
extern RLSTATUS 
Win32_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize);
extern RLSTATUS 
Win32_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen);
extern RLSTATUS 
Win32_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout);

/*elp end*/
#endif
extern ubyte4 SOCKET_GetClientsAddr(OS_SPECIFIC_SOCKET_HANDLE ClientSock);

#if defined( __MULTI_THREADED_SERVER_ENABLED__) || defined(__OCP_MULTI_THREADED_SERVER_ENABLED__)
extern RLSTATUS SOCKET_CreateTask(ProcessComChannel* pPCC);
#endif

#ifdef __MASTER_SLAVE_SNMP_STACK__
extern RLSTATUS SOCKET_CreateMasterInterface();
#endif

extern RLSTATUS SOCKET_OpenServer(OS_SPECIFIC_SOCKET_HANDLE *pSoc,ubyte4 port);

#ifdef __cplusplus
}
#endif

#endif
