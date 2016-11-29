/*
 *  rc_sock_w32.c
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

$History: rc_sock_w32.c $
 * 
 * *****************  Version 30  *****************
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

#ifdef  __WIN32_OS__

#include <process.h>
#include <stdio.h>
#include <windows.h>
#include <winsock.h>
#include <wsnetbs.h>
#include <wsipx.h>
#include <winbase.h>
#include <wsnwlink.h>

#ifdef __OCP_ENABLED__
#include "rca_ocp.h"
#include "rca_ocpd.h"
#ifdef __DEBUG_WIN32_SOCKET__
extern void Win32_DEBUG_PACKET(sbyte *pBuf, Counter BufLen);
#endif
#endif	/* __OCP_ENABLED__ */

#ifdef __ENABLE_MEMMGR_DEBUG__
#include "rc_memmgr.h"
#endif  /* __ENABLE_MEMMGR_DEBUG__ */

#ifdef __COMMON_LISTENER__
#include "rc_tcpd.h"
#endif

#include "rc_access.h"

/*SSL Begin*/
#ifdef __RLI_SSL_ENABLED__
#include <rc_ssl_defs.h>
#endif
/*SSL End*/

/* Defaults */
#define     kANY_PORT                       0

#if ((defined __COMMON_LISTENER__) || (defined __WEBCONTROL_BLADE_ENABLED__))
ProcessComChannel g_pccObjects[kHTTPD_QUEUE_SIZE];
#endif


/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Initialize( void )
{
    static int  InitYet = FALSE;
    int         err;
    WORD        wVersionRequired;
    WSADATA     wsaData;

    if (TRUE == InitYet)
        return OK;

    wVersionRequired = MAKEWORD(1,1);
    err = WSAStartup(wVersionRequired, &wsaData);

    if (0 != err)
    {
        return ERROR_GENERAL_ACCESS_DENIED;
    }

    InitYet = TRUE;

    return OK;
}



/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_SOCKET_HANDLE SOCKET_TcpCreate()
{
    OS_SPECIFIC_SOCKET_HANDLE   soc = socket(AF_INET, SOCK_STREAM, 0);
    char*                       pMsg;

    if (INVALID_SOCKET == soc)
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

    if (INVALID_SOCKET == soc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CREATE, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
    }

    return soc;
}



/*-----------------------------------------------------------------------*/

static ubyte4 GetHostAddress(LPSTR pszHostName)
{
    struct hostent FAR *pHostAddr;

    pHostAddr = gethostbyname ( pszHostName );

    if (pHostAddr == NULL)
        return 0;

    return *((u_long *)pHostAddr->h_addr);
}



/*-----------------------------------------------------------------------*/

static ubyte4 GetLocalHostAddress()
{
    sbyte   szHostName[MAX_PATH];
    ubyte4  ulHostAddr;

    if (SOCKET_ERROR != gethostname(szHostName, sizeof(szHostName)))
        ulHostAddr = GetHostAddress ( szHostName );
    else
        return(0);

    return (ulHostAddr);
}



/*-----------------------------------------------------------------------*/

static RLSTATUS BindSocketToAddr(OS_SPECIFIC_SOCKET_HANDLE soc, ubyte4 port)
{
    struct sockaddr_in  LocalAddr;
    RLSTATUS              status;
    u_long              ulHostAddress;
    char                *pMsg;

    /* ask the system to allocate a port and bind to INADDR_ANY */
    MEMSET(&LocalAddr, 0x00, sizeof(LocalAddr));

    if (0 == (ulHostAddress = GetLocalHostAddress()))
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_BIND, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
        /*closesocket(soc);*/
		OS_SPECIFIC_SOCKET_CLOSE(soc);/*SSL*/
        return ERROR_GENERAL_ACCESS_DENIED;
    }

    /* get system to allocate a port number by binding a host address */
    LocalAddr.sin_family        = AF_INET;
    LocalAddr.sin_addr.s_addr   = htonl(INADDR_ANY);
    LocalAddr.sin_port          = htons((u_short)port);

    if (0 != bind(soc, (struct sockaddr *)&LocalAddr, sizeof(LocalAddr)))
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_BIND, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
        /*closesocket(soc);*/
		OS_SPECIFIC_SOCKET_CLOSE(soc);/*SSL*/

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
    if (INVALID_SOCKET == *pSoc)
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
    if (INVALID_SOCKET == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;
#ifdef __RLI_SSL_ENABLED__
	if (1) {
		/* init the SSL state: */
		status = rc_do_ssl_socket((OS_SPECIFIC_SOCKET_HANDLE *)pSoc);
		if (status < OK) { close(*pSoc); return status; }
	
		status = BindSocketToAddr(rc_ssl_table2sock(*pSoc), port);
		if (OK != status)
			return ERROR_GENERAL_ACCESS_DENIED;

		/* Create a incoming Client connect request queue */
		if (0 != listen(rc_ssl_table2sock(*pSoc), kTCP_LISTEN_BACKLOG))
		{
			MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_LISTEN, &pMsg);
			OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
			close(*pSoc);
	
			return ERROR_GENERAL_ACCESS_DENIED;
		}
	} else {
#endif
    status = BindSocketToAddr(*pSoc, port);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a incoming Client connect request queue */
    if (0 != listen(*pSoc, kTCP_LISTEN_BACKLOG))
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_LISTEN, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
        /*closesocket(*pSoc);*/
		OS_SPECIFIC_SOCKET_CLOSE(*pSoc); /*SSL*/
        return ERROR_GENERAL_ACCESS_DENIED;
    }

#ifdef __RLI_SSL_ENABLED__
	}
#endif /* __RLI_SSL_ENABLED__ */
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
    pAddrTo->sin_addr.S_un.S_addr = pParams->clientAddr;
    pAddrTo->sin_port = HTON2(pParams->clientPort);
    
#ifdef __DEBUG_WIN32_SOCKET__
    if (pParams->sendPacketLength > 1)                                        
        printf("\n\n-----------------> Server Response (UDP):\n"); 
                                                            
    Win32_DEBUG_PACKET(pParams->pSendPacket, pParams->sendPacketLength);                       
#endif

    if (SOCKET_ERROR == sendto(sock, pParams->pSendPacket, 
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
    sbyte4 iAddrLen;
	sbyte4 RecvResult;

    iAddrLen = sizeof(struct sockaddr_in);
    if ( NULL == (pAddr = (struct sockaddr_in *) RC_MALLOC(iAddrLen)) )
        return ERROR_MEMMGR_NO_MEMORY;

    RecvResult = recvfrom(sock, pParams->pRecPacket, kMaxUdpPacketSize, 
                                    0, (struct sockaddr *)pAddr, &iAddrLen );
    if (SOCKET_ERROR == RecvResult)
	{
        RC_FREE(pAddr);
		return ERROR_GENERAL;
	}
#ifdef __DEBUG_WIN32_SOCKET__
    if (RecvResult >= 1)                                        
        printf("\n\n-----------------> Client Request (UDP):\n"); 
                                                            
    Win32_DEBUG_PACKET(pParams->pRecPacket, RecvResult);                       
#endif

    pParams->clientAddr = pAddr->sin_addr.S_un.S_addr;
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

extern void PersistentConnectionHandler(void *pccObject)
{
#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: Thread %s %d start.\n", 
    	((ProcessComChannel *)pccObject)->Name, ((ProcessComChannel *)pccObject)->index);
#endif /* __RLI_DEBUG_SOCK__ */


    /* process the connection */
	((ProcessComChannel *)pccObject)->pfuncConnHandler(pccObject); 

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: closing socket(Thread %d, threadState=%d)!!!!!!!!!!\n", 
    	((ProcessComChannel *)pccObject)->index, ((ProcessComChannel *)pccObject)->ThreadState);
#endif	/* __RLI_DEBUG_SOCK__ */

    /* close down the socket 
    closesocket(((ProcessComChannel *)pccObject)->sock);*/
	OS_SPECIFIC_SOCKET_CLOSE(((ProcessComChannel *)pccObject)->sock);/*SSL*/
    /* mark the com channel as free */
    ((ProcessComChannel *)pccObject)->InUse = FALSE;

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: %s Thread %d end!!!!!!!!!!!!!\n", 
    	((ProcessComChannel *)pccObject)->Name, ((ProcessComChannel *)pccObject)->index);
#endif	/* __RLI_DEBUG_SOCK__ */

    _endthread();
}

#endif



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Accept(OS_SPECIFIC_SOCKET_HANDLE *soc, 
                              OS_SPECIFIC_SOCKET_HANDLE *accSoc,
                              ubyte4 port)
{
    char    ClientAddr[MAX_PATH];
    int     iAddrLen;
    char*   pMsg;

    iAddrLen = sizeof(ClientAddr);
    memset(&ClientAddr, 0x00, iAddrLen);

    *accSoc = accept(*soc, (struct sockaddr FAR *)&ClientAddr, &iAddrLen);

    if (INVALID_SOCKET == *accSoc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_ACCEPT, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

		/*
        closesocket(*accSoc);
        closesocket(*soc);
		*/

#ifdef __RLI_SSL_ENABLED__
		REAL_OS_SPECIFIC_SOCKET_CLOSE(*accSoc);/*SSL*/
        REAL_OS_SPECIFIC_SOCKET_CLOSE(*soc);
#else	
		OS_SPECIFIC_SOCKET_CLOSE(*accSoc);
        OS_SPECIFIC_SOCKET_CLOSE(*soc);	
#endif /*SSL elp*/



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
    int                 iAddrLen;
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
    unsigned int Result = _beginthread(PersistentConnectionHandler, 0, pPCC);

    if (-1 == Result)
        return SYS_ERROR_SOCKET_CREATE_TASK;

    return OK;
}

#endif



/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_Close( OS_SPECIFIC_SOCKET_HANDLE sock )
{
    RLSTATUS err;

    err = closesocket(sock);
    if (SOCKET_ERROR == err )
        return ERROR_GENERAL_ACCESS_DENIED;

    else
        return OK;
}



/*-----------------------------------------------------------------------*/

static RLSTATUS
SOCKET_ConnectToServer(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pName, ubyte4 port )
{
    struct sockaddr_in  RemAddr;
    u_long              ulHostAddress;
    int                 err;
    sbyte               *pMsg;

    MEMSET(&RemAddr, 0x00, sizeof(RemAddr));

    ulHostAddress = GetHostAddress(pName);
    if ( 0 == ulHostAddress )
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
    if ( SOCKET_ERROR == err )
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
    if (INVALID_SOCKET == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    status = BindSocketToAddr(*pSoc, kANY_PORT);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a Client connection */
    status = SOCKET_ConnectToServer( *pSoc, pName, port );

    return status;
}

#endif /* __WIN32_OS__ */
