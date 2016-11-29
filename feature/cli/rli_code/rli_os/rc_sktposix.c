/*
 *  rc_sktposix.c
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

$History: rc_sktposix.c $
 * 
 * *****************  Version 28  *****************
 * User: Pstuart      Date: 3/23/01    Time: 10:58a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * fixed Linux dependency, satisfied function return requirement
 * 
 * *****************  Version 27  *****************
 * User: James        Date: 9/01/00    Time: 6:15p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Changed the order of header file inclusion. JAB
 * 
 * *****************  Version 26  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 25  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/
#include "rc_options.h"

#ifdef  __POSIX_OS__

#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_errors.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_msghdlr.h"
#include "rc_socks.h"
#include "rc_environ.h"
#include "rc_database.h"
#include "rc_convert.h"

#ifdef __OCP_ENABLED__
#include "rca_ocp.h"
#include "rca_ocpd.h"
#endif	/* __OCP_ENABLED__ */

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#ifndef  __LINUX__
#include <thread.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "rc_access.h"

#ifdef  __LINUX__
  typedef unsigned short u_short;
  typedef unsigned long  u_long;
#endif


/* in order to build correctly you will need to link in the following libraries
 * pthread
 * xnet
 * posix4
 */

/* Defaults */
#define     kANY_PORT                       0


ProcessComChannel g_pccObjects[kHTTPD_QUEUE_SIZE];


/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Initialize( void )
{
    return OK;
}



/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_SOCKET_HANDLE SOCKET_TcpCreate()
{
    OS_SPECIFIC_SOCKET_HANDLE   soc = socket(AF_INET, SOCK_STREAM, 0);
    char*                       pMsg;

    if (-1 == soc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CREATE, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
    }

    return soc;
}



/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_DGRAM_HANDLE SOCKET_UdpCreate()
{
    OS_SPECIFIC_DGRAM_HANDLE   soc = socket(AF_INET, SOCK_DGRAM, 0);
    char*                      pMsg;

    if (-1 == soc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CREATE, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
    }

    return soc;
}


/*-----------------------------------------------------------------------*/

static RLSTATUS BindSocketToAddr(OS_SPECIFIC_SOCKET_HANDLE soc, ubyte4 port)
{
    struct sockaddr_in  LocalAddr;
    RLSTATUS            status;
    char                *pMsg, *pErrMsg;

    /* ask the system to allocate a port and bind to INADDR_ANY */
    /* get system to allocate a port number by binding a host address */
    LocalAddr.sin_family        = AF_INET;
    LocalAddr.sin_addr.s_addr   = htonl(INADDR_ANY);
    LocalAddr.sin_port          = htons((u_short)port);

    if (0 != bind(soc, (struct sockaddr *)&LocalAddr, sizeof(LocalAddr)))
    {
        pErrMsg = strerror(errno);
        printf ("Bind failed w/ error: %d - %s\n", errno, pErrMsg);
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_BIND, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
        close(soc);

        status = ERROR_GENERAL_ACCESS_DENIED;
    }
    else
        status = OK;

    return status;
}



/*-----------------------------------------------------------------------*/

static RLSTATUS 
SOCKET_UdpBind(OS_SPECIFIC_DGRAM_HANDLE *pSoc, ubyte4 port)
{
    RLSTATUS status;
 
    /* Create a stream socket */
    *pSoc = SOCKET_UdpCreate();
    if (-1 == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* N.B. Since OS_SPECIFIC_DGRAM_HANDLE and OS_SPECIFIC_SOCKET_HANDLE
     * are typedef'ed to the same value, we'll go ahead and reuse the 
     * function BindSocketToAddr.  However, in the future, if Windows changes
     * their general DGRAM/SOCKET API, we'll need to reinvestigate this function 
     */
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

    SOCKET_Initialize();

    /* Create a stream socket */
    *pSoc = SOCKET_TcpCreate();

    if (-1 == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    status = BindSocketToAddr(*pSoc, port);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a incoming Client connect request queue */
    if (0 != listen(*pSoc, kTCP_LISTEN_BACKLOG))
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_LISTEN, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
        close(*pSoc);

        return ERROR_GENERAL_ACCESS_DENIED;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_UdpSendTo(OS_SPECIFIC_DGRAM_HANDLE sock, UdpParams *pParams)
{
    struct sockaddr_in *pAddrTo;

    if ( NULL == (pAddrTo = (struct sockaddr_in *) RC_MALLOC(sizeof(struct sockaddr_in))) )
        return ERROR_MEMMGR_NO_MEMORY;

    pAddrTo->sin_family = AF_INET;
    pAddrTo->sin_addr.s_addr = pParams->clientAddr;
    pAddrTo->sin_port = HTON2(pParams->clientPort);
    
    if (-1 == sendto(sock, pParams->pSendPacket, 
                                pParams->sendPacketLength, 0, 
                                (struct sockaddr *)pAddrTo, 
                                  sizeof(struct sockaddr)))
    {
        RC_FREE(pAddrTo);
        return SYS_ERROR_SOCKET_GENERAL;
    }

    RC_FREE(pAddrTo);
    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
SOCKET_UdpReceive(OS_SPECIFIC_DGRAM_HANDLE sock, UdpParams *pParams)
{
    struct sockaddr_in *pAddr;
    size_t iAddrLen;
    int    RecvResult;

    iAddrLen = sizeof(struct sockaddr_in);
    if ( NULL == (pAddr = (struct sockaddr_in *) RC_MALLOC(iAddrLen)) )
        return ERROR_MEMMGR_NO_MEMORY;

    RecvResult = recvfrom(sock, pParams->pRecPacket, kMaxUdpPacketSize, 
                                    0, (struct sockaddr *)pAddr, &iAddrLen );
    if (-1 == RecvResult )
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

static void * PersistentConnectionHandler(void *pccObject)
{
    RLSTATUS status = 0;
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

    pthread_exit(&status);

    return NULL;
}

#endif



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Accept(OS_SPECIFIC_SOCKET_HANDLE *soc, 
                              OS_SPECIFIC_SOCKET_HANDLE *accSoc, ubyte4 port)
{
    struct  sockaddr_in ClientAddr;
    ubyte4  iAddrLen;
    char*   pMsg;

    iAddrLen = sizeof(ClientAddr);
    memset(&ClientAddr, 0x00, iAddrLen);

    *accSoc = accept(*soc, (struct sockaddr *)&ClientAddr, &iAddrLen);

    if (-1 == *accSoc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_ACCEPT, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        close(*accSoc);
        close(*soc);

        SOCKET_OpenServer(soc, port);
        return SYS_ERROR_SOCKET_ACCEPT;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

#ifdef __MASTER_SLAVE_SNMP_STACK__

extern RLSTATUS SOCKET_CreateMasterInterface()
{
    return ERROR_GENERAL_NOT_FOUND;         /* not available for Windows NT/95 */
}

#endif



/*-----------------------------------------------------------------------*/

extern ubyte4 SOCKET_GetClientsAddr(OS_SPECIFIC_SOCKET_HANDLE ClientSock)
{
    struct sockaddr_in  ClientAddr;
    ubyte4              iAddrLen;
    ubyte4              clientIpAddr;

    iAddrLen     = sizeof(ClientAddr);
    clientIpAddr = 0;

    if (OK == getpeername(ClientSock, (struct sockaddr *)&ClientAddr, &iAddrLen))
        clientIpAddr = ClientAddr.sin_addr.s_addr;

    return clientIpAddr;
}



/*-----------------------------------------------------------------------*/

#if defined( __MULTI_THREADED_SERVER_ENABLED__) || defined(__OCP_MULTI_THREADED_SERVER_ENABLED__)

extern RLSTATUS SOCKET_CreateTask(ProcessComChannel* pPCC)
{
    pthread_t threadID;
    
    int Result = pthread_create(&threadID, NULL, PersistentConnectionHandler, pPCC);
    
    if (0 != Result)
    {
        printf("failed thr_Create - errno %d\n", Result);
        printf("EAGAIN: %d EINVAL:%d ENOMEM:%d\n", EAGAIN, EINVAL, ENOMEM);

        return SYS_ERROR_SOCKET_CREATE_TASK;
    }
    return OK;
}

#endif



/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_Close( OS_SPECIFIC_SOCKET_HANDLE sock )
{
    RLSTATUS err;

    err = close(sock);
    if (-1 == err )
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
    u_long              ulHostAddress;
    int                 err;
    sbyte               *pMsg;

    MEMSET(&RemAddr, 0x00, sizeof(RemAddr));

    status = CONVERT_StrTo(pName, &ulHostAddress, kDTipaddress);

    if ( OK > status )
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CONNECT, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        return ERROR_GENERAL_ACCESS_DENIED;
    }

    RemAddr.sin_family      = AF_INET;
    RemAddr.sin_addr.s_addr = ulHostAddress;
    RemAddr.sin_port        = htons((u_short)port);

    /* Create a Client connection */
    err = connect( sock, (struct sockaddr *)&RemAddr, sizeof(RemAddr));
    if ( -1 == err )
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CONNECT, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        return ERROR_GENERAL_ACCESS_DENIED;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_Connect(OS_SPECIFIC_SOCKET_HANDLE *pSoc, sbyte *pName, ubyte4 port )
{
    RLSTATUS status;

    SOCKET_Initialize();

    /* Create a stream socket */
    *pSoc = SOCKET_TcpCreate();
    if (-1 == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    status = BindSocketToAddr(*pSoc, kANY_PORT);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a Client connection */
    status = SOCKET_ConnectToServer( *pSoc, pName, port );

    return status;
}

#endif /* __POSIX_OS__ */
