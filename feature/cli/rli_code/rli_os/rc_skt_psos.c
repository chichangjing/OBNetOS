/*  
 *  rc_skt_psos.c
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

$History: rc_skt_psos.c $
 * 
 * *****************  Version 25  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments


 */


#include "rc_options.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_errors.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_msghdlr.h"
#include "rc_socks.h"

#ifdef  __PSOS_OS__

#include "rc_database.h"
#include "rc_convert.h"

#ifdef __OCP_ENABLED__
#include "rca_ocp.h"
#include "rca_ocpd.h"
#endif	/* __OCP_ENABLED__ */

#include "rc_access.h"

#ifdef NULL
#undef NULL
#endif

#include <pna.h>
#include <psos.h>

#ifndef NULL
#define NULL    ((void*)0)
#endif

typedef     unsigned long                   STATUS_pSOS;

#define     kSOCKET_ADDRESS_FAMILY_IP       AF_INET
#define     kSOCKET_TYPE_STREAM             SOCK_DGRAM
#define     kSOCKET_PROTOCOL                IPPROTO_TCP

#define     kSPV_STACK_SIZE                 8000
#define     kUSR_STACK_SIZE                 8000
#define     kHTTP_TASK_FLAGS                T_LOCAL
#define     kHTTP_TASK_PRIO                 100
#define     kHTTP_TASK_MODE                 (T_LOCAL | T_NOFPU )
#define     kINVALID_SOCKET                 -1
#define     kANY_PORT                       0


ProcessComChannel g_pccObjects[kHTTPD_QUEUE_SIZE];

/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_SOCKET_HANDLE SOCKET_TcpCreate()
{
    char*                       pMsg;
    OS_SPECIFIC_SOCKET_HANDLE   soc = socket(AF_INET, SOCK_STREAM, 0);

    if (kINVALID_SOCKET == soc)      /* pSOS manual --- System Calls 4-72 */
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CREATE, &pMsg);
#ifndef __IGNORE_MESSAGE_HANDLER__
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
#else
        OS_SPECIFIC_LOG_ERROR(kSevereError, "SOCKET_TcpCreate");
#endif

    }

    return soc;
}


/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_DGRAM_HANDLE SOCKET_UdpCreate()
{
    OS_SPECIFIC_DGRAM_HANDLE   soc = socket(AF_INET, SOCK_DGRAM, 0);
    char*                      pMsg;

    if (kINVALID_SOCKET == soc)      /* pSOS manual --- System Calls 4-72 */
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CREATE, &pMsg);
#ifndef __IGNORE_MESSAGE_HANDLER__
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
#else
        OS_SPECIFIC_LOG_ERROR(kSevereError, "SOCKET_UdpCreate");
#endif

    }

    return soc;
}


/*-----------------------------------------------------------------------*/

static RLSTATUS BindSocketToAddr(OS_SPECIFIC_SOCKET_HANDLE soc, ubyte4 port)
{
    struct sockaddr_in  LocalAddr;
    RLSTATUS            status;
    char                *pMsg;

    MEMSET(&LocalAddr, 0x00, sizeof(LocalAddr));

    LocalAddr.sin_family        = AF_INET;
    LocalAddr.sin_addr.s_addr   = htonl(INADDR_ANY);
    LocalAddr.sin_port          = htons((ubyte2)port);

    /* pSOS Manual --- System Calls 4-6 */
    if (0 != bind(soc, &LocalAddr, sizeof(LocalAddr)))
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_BIND, &pMsg);
#ifndef __IGNORE_MESSAGE_HANDLER__
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
#else
        OS_SPECIFIC_LOG_ERROR(kSevereError, "BindSocketToAddr");
#endif
        close(soc);
        status = ERROR_GENERAL_ACCESS_DENIED;
    }
    else
    {
        OS_SPECIFIC_LOG_ERROR(kDebugError, "BindSocketToAddr:  Successful Binding");
        status = OK;
    }

    return (status);
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Initialize(void)
{
	return OK;
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_UdpBind(OS_SPECIFIC_SOCKET_HANDLE *pSoc, ubyte4 port)
{
    RLSTATUS status;

    /* Create a stream socket */
    *pSoc = SOCKET_UdpCreate();
    if (kINVALID_SOCKET == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    status = BindSocketToAddr(*pSoc, port);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    return OK;
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_UdpBindFixed(OS_SPECIFIC_DGRAM_HANDLE *pSoc, ubyte4 port)
{
    return SOCKET_UdpBind(pSoc, port);
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_UdpBindAny(OS_SPECIFIC_DGRAM_HANDLE *pSoc)
{
    return SOCKET_UdpBind(pSoc, kANY_PORT);
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_OpenServer(OS_SPECIFIC_SOCKET_HANDLE *pSoc, ubyte4 port)
{
    RLSTATUS status;
    char    *pMsg;

    OS_SPECIFIC_LOG_ERROR(kDebugError, "HttpD:  Opening for business!");

    /* Create a stream socket */
    *pSoc = SOCKET_TcpCreate();

    if (kINVALID_SOCKET == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Bind the socket */
    status = BindSocketToAddr(*pSoc, port);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a incoming Client connect request queue */
    if (0 != listen(*pSoc, kTCP_LISTEN_BACKLOG))   /* pSOS Manual --- System Calls 4-35 */
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_LISTEN, &pMsg);
#ifndef __IGNORE_MESSAGE_HANDLER__
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
#else
        OS_SPECIFIC_LOG_ERROR(kSevereError, "SOCKET_OpenServer");
#endif
        close(*pSoc);
        
        return ERROR_GENERAL_ACCESS_DENIED;
    }

    return OK;
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_UdpSendTo(OS_SPECIFIC_SOCKET_HANDLE sock, UdpParams *pParams)
{
    struct sockaddr_in *pAddrTo;

    if ( NULL == (pAddrTo = (struct sockaddr_in *) RC_MALLOC(sizeof(struct sockaddr_in))) )
        return ERROR_MEMMGR_NO_MEMORY;

    pAddrTo->sin_family = AF_INET;
    pAddrTo->sin_addr.s_addr = pParams->clientAddr;
    pAddrTo->sin_port = HTON2(pParams->clientPort);

    if (kINVALID_SOCKET == sendto(sock, (sbyte *) pParams->pSendPacket, /* pSOS manual - system calls 4-59 */
                        pParams->sendPacketLength, 0,
                        pAddrTo,
                        sizeof(struct sockaddr_in)) )
    {
        RC_FREE(pAddrTo);
        return SYS_ERROR_SOCKET_GENERAL;
    }

    RC_FREE(pAddrTo);
    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_UdpReceive(OS_SPECIFIC_SOCKET_HANDLE sock, UdpParams *pParams)
{
    struct sockaddr_in *pAddr;
    int iAddrLen;
    int RecvResult;

    iAddrLen = sizeof(struct sockaddr_in);
    if ( NULL == (pAddr = (struct sockaddr_in *) RC_MALLOC(iAddrLen)) )
        return ERROR_MEMMGR_NO_MEMORY;

    RecvResult = recvfrom(sock, (sbyte *) pParams->pRecPacket, kMaxUdpPacketSize, /* pSOS manual - system calls 4-45 */
            0, pAddr, &iAddrLen );

    if (kINVALID_SOCKET == RecvResult)
	{
        RC_FREE(pAddr);
		return ERROR_GENERAL;
	}

    pParams->clientAddr = pAddr->sin_addr.s_addr;
    pParams->clientPort = NTOH2(pAddr->sin_port);
    pParams->recvPacketLength = (ubyte4)RecvResult;

    RC_FREE(pAddr);

#ifdef __ENABLE_LAN_IP_FILTER__
	if (TRUE != ACCESS_ValidIpAddress(pParams->clientAddr))
		return ERROR_GENERAL;
#endif

	return OK;
}


/*-----------------------------------------------------------------------*/

#if defined( __MULTI_THREADED_SERVER_ENABLED__) || defined(__OCP_MULTI_THREADED_SERVER_ENABLED__)

static void PersistentConnectionHandler(ProcessComChannel *pccObject)
{
    sbyte   *pMsg;
    unsigned long tid;

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: Thread start.\n");
#endif

    /* process the connection */
	((ProcessComChannel *)pccObject)->pfuncConnHandler(pccObject); 

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: closing socket(threadState=%d)!!!!!!!!!!!!!\n", ((ProcessComChannel *)pccObject)->ThreadState);
#endif

    /* close down the socket */
    close(((ProcessComChannel *)pccObject)->sock);

    /* mark the com channel as free */
    ((ProcessComChannel *)pccObject)->InUse = FALSE;

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: Thread end!!!!!!!!!!!!!!!!\n");
#endif

    close(0);
    free((void *)-1);

    /* commit suicide!  pSOS Manual --- System Calls 1-136 */
    t_ident(NULL, 0, &tid);

    t_delete(tid);

    MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_DELETE, &pMsg);
    OS_SPECIFIC_LOG_ERROR(kUnrecoverableError, pMsg);
}

#endif /* __MULTI_THREADED_SERVER_ENABLED__ */

/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Accept(OS_SPECIFIC_SOCKET_HANDLE *pSoc, 
                              OS_SPECIFIC_SOCKET_HANDLE *pAccSoc, ubyte4 port)
{
    struct sockaddr_in  ClientAddr;
    int                 iAddrLen;
    sbyte*              pMsg;

    /* Wait for clients to call connect */
    iAddrLen = sizeof(struct sockaddr_in);
    MEMSET(&ClientAddr, 0x00, iAddrLen);
    
    *pAccSoc = accept(*pSoc, &ClientAddr, &iAddrLen);

    if (kINVALID_SOCKET == *pAccSoc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_ACCEPT, &pMsg);
#ifndef __IGNORE_MESSAGE_HANDLER__
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
#else
        OS_SPECIFIC_LOG_ERROR(kSevereError, "SOCKET_Accept");
#endif

        close(*pAccSoc);      /* let's close to be safe */
        close(*pSoc);

        SOCKET_OpenServer(pSoc, port);
        return SYS_ERROR_SOCKET_ACCEPT;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

#ifdef __MASTER_SLAVE_SNMP_STACK__

extern RLSTATUS SOCKET_CreateMasterInterface()
{
    return ERROR_GENERAL_NOT_FOUND;         /* N/A for pSOS */
}

#endif


/*-----------------------------------------------------------------------*/

extern ubyte4 SOCKET_GetClientsAddr(OS_SPECIFIC_SOCKET_HANDLE ClientSock)
{
    struct sockaddr_in  ClientAddr;
    int                 iAddrLen;
    ubyte4              clientIpAddr = 0;

    iAddrLen = sizeof(ClientAddr);
    memset(&ClientAddr, 0x00, iAddrLen);

    if (0 == getpeername(ClientSock, &ClientAddr, &iAddrLen))
        clientIpAddr = ntohl(ClientAddr.sin_addr.s_addr);

    return clientIpAddr;
}



/*-----------------------------------------------------------------------*/

#if defined( __MULTI_THREADED_SERVER_ENABLED__) || defined(__OCP_MULTI_THREADED_SERVER_ENABLED__)

extern RLSTATUS SOCKET_CreateTask(ProcessComChannel* pPCC)
{
    OS_SPECIFIC_SOCKET_HANDLE   shrSoc = kINVALID_SOCKET;
    OS_SPECIFIC_SOCKET_HANDLE   tmpSoc = kINVALID_SOCKET;
    unsigned long               tid;
    char*                       pMsg;
    STATUS_pSOS                 status;
    unsigned long               args[4];

#ifdef __RLI_DEBUG_SOCK__
    printf("SOCKET_CreateTask: here.\n");
#endif

    CONVERT_ToStr(&pPCC->index, &(pPCC->Name[STRLEN(pPCC->Name)]), kDTinteger);

    /* pSOS manual --- System Calls 1-130 */
    status = t_create(pPCC->Name, kHTTP_TASK_PRIO, kSPV_STACK_SIZE, 
        kUSR_STACK_SIZE, kHTTP_TASK_FLAGS, &tid);

    if (0 != status)
    {
        printf("SOCKET_CreateTask: t_create returned = %d.\n", status);
    }

#ifdef __RLI_DEBUG_SOCK__
    printf("SOCKET_CreateTask: t_create() succeeded.\n");
#endif

    /* share the socket with the child task (pSOS manual --- System Calls 4-69) */
    shrSoc = shr_socket(pPCC->sock, (int)tid);

    if (kINVALID_SOCKET == shrSoc)
    {
        close(pPCC->sock);

        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_SHARE, &pMsg);
#ifndef __IGNORE_MESSAGE_HANDLER__
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
#else
        OS_SPECIFIC_LOG_ERROR(kSevereError, "SOCKET_CreateTask");
#endif

        /*!!!!!!!!!!!!!!!!!!! need to kill the child, if share fails! JAB */
        return SYS_ERROR_SOCKET_SHARE;
    }

    tmpSoc = pPCC->sock;
    pPCC->sock = shrSoc;

#ifdef __RLI_DEBUG_SOCK__
    printf("SOCKET_CreateTask: shr_socket() succeeded.\n");
#endif

    /* activate the child task */

    /* note:  do not make a copy of &g_pccObjects[findConn] on to the stack */
    /* and pass that value onto the child task, by doing so would create  */
    /* a race condition.  be safe and pass the address in directly.  JAB  */

    args[0] = (unsigned long)pPCC;

    if (0 != (status = t_start(tid, kHTTP_TASK_MODE, 
                               PersistentConnectionHandler, 
                               args)) )
    {
        printf("SOCKET_CreateTask: t_start returned %d.\n", status);

        /*!!!!!!!!!!!!!!!!!! this might be dangerous to enable.  JAB
        close(shrSoc);
        */

        return SYS_ERROR_SOCKET_CREATE_TASK;
    }

#ifdef __RLI_DEBUG_SOCK__
    printf("SOCKET_CreateTask: t_start() succeeded.\n");
#endif

    close(tmpSoc);

    return OK;
}

#endif  /* __MULTI_THREADED_SERVER_ENABLED__ */


/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Close(OS_SPECIFIC_SOCKET_HANDLE sock)
{
    int err;

    err = close(sock);

    if (kINVALID_SOCKET == err )
        return ERROR_GENERAL_ACCESS_DENIED;
    else
        return OK;
}



/*-----------------------------------------------------------------------*/



static RLSTATUS
SOCKET_ConnectToServer(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pName, ubyte4 port )
{
    RLSTATUS            status;
    struct sockaddr_in  RemAddr;
    ubyte4              ulHostAddress;
    int                 err;
    sbyte               *pMsg;

    MEMSET(&RemAddr, 0x00, sizeof(RemAddr));

    status = CONVERT_StrTo(pName, &ulHostAddress, kDTipaddress);
    if ( OK > status )
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CONNECT, &pMsg);
#ifndef __IGNORE_MESSAGE_HANDLER__
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
#else
        OS_SPECIFIC_LOG_ERROR(kSevereError, "SOCKET_ConnectToServer");
#endif

        return ERROR_GENERAL_ACCESS_DENIED;
    }

    RemAddr.sin_family      = AF_INET;
    RemAddr.sin_addr.s_addr = ulHostAddress;
    RemAddr.sin_port        = htons((ubyte2)port);

    /* Create a Client connection */
    err = connect( sock, &RemAddr, sizeof(RemAddr));
    if ( 0 != err )
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CONNECT, &pMsg);
#ifndef __IGNORE_MESSAGE_HANDLER__
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
#else
        OS_SPECIFIC_LOG_ERROR(kSevereError, "SOCKET_ConnectToServer");
#endif

        return ERROR_GENERAL_ACCESS_DENIED;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_Connect(OS_SPECIFIC_SOCKET_HANDLE *pSoc, sbyte *pName, ubyte4 port )
{
    RLSTATUS status;

    /* Create a stream socket */
    *pSoc = SOCKET_TcpCreate();
    if (kINVALID_SOCKET == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    status = BindSocketToAddr(*pSoc, kANY_PORT);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a Client connection */
    status = SOCKET_ConnectToServer( *pSoc, pName, port );

    return status;
}


#endif /* __PSOS_OS__ */
