
#include "rc_options.h"

#ifdef  __FreeRTOS_OS__

#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_errors.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_msghdlr.h"
#include "rc_socks.h"
#include "rc_convert.h"
#include "rc_database.h"

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


#ifdef __MASTER_SLAVE_SNMP_STACK__
#include "rc_vxdev.h"
#endif


#ifdef __OCP_ENABLED__
#include "rca_ocp.h"
#include "rca_ocpd.h"
#endif	/* __OCP_ENABLED__ */

#ifdef __COMMON_LISTENER__
#include "rc_tcpd.h"
#endif

#include "rc_access.h"

#ifdef __MASTER_SLAVE_SNMP_STACK__
extern int WebControlFd;
#endif

#define kANY_PORT       0

ProcessComChannel g_pccObjects[kHTTPD_QUEUE_SIZE];


/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_SOCKET_HANDLE SOCKET_TcpCreate()
{
    char*                       pMsg;
    OS_SPECIFIC_SOCKET_HANDLE   soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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
    char                *pMsg;

    /* ask the system to allocate a port and bind to INADDR_ANY */
    MEMSET(&LocalAddr, 0x00, sizeof(LocalAddr));

    /* get system to allocate a port number by binding a host address */
    LocalAddr.sin_family        = AF_INET;
    LocalAddr.sin_addr.s_addr   = htonl(INADDR_ANY);
    LocalAddr.sin_port          = htons((ubyte2)port);

    /* bind socket to local address */
    if (0 != bind(soc, (struct sockaddr *)&LocalAddr, sizeof(LocalAddr)))
    {
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
    if (-1 == *pSoc)
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
    RLSTATUS  status;
    char    *pMsg;
	int optval = 1;
	
    SOCKET_Initialize();

    /* Create a stream socket */
    *pSoc = SOCKET_TcpCreate();
    if (-1 == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

	#if 0
    setsockopt(*pSoc, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));
    setsockopt(*pSoc, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle));
    setsockopt(*pSoc, IPPROTO_TCP,TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
    setsockopt(*pSoc, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
	#else
	//setsockopt(*pSoc, IPPROTO_TCP, TCP_NODELAY, (void*)&optval, sizeof(optval));
	#endif
	
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
SOCKET_UdpSendTo(OS_SPECIFIC_SOCKET_HANDLE sock, UdpParams *pParams)
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
                        sizeof(struct sockaddr)) )
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
    socklen_t iAddrLen;
    int RecvResult;

    iAddrLen = sizeof(struct sockaddr_in);
    if ( NULL == (pAddr = (struct sockaddr_in *) RC_MALLOC(iAddrLen)) )
        return ERROR_MEMMGR_NO_MEMORY;

    RecvResult = recvfrom(sock, (sbyte *) pParams->pRecPacket, kMaxUdpPacketSize,
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
#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: Thread %s %d start.\n", ((ProcessComChannel *)pccObject)->Name, ((ProcessComChannel *)pccObject)->index);
#endif /* __RLI_DEBUG_SOCK__ */

    /* process the connection */
	((ProcessComChannel *)pccObject)->pfuncConnHandler(pccObject); 
	

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: closing socket(threadState=%d)!!!!!!!!!!!!!\n", (int)(((ProcessComChannel *)pccObject)->ThreadState));
#endif

    /* close down the socket */
    close(((ProcessComChannel *)pccObject)->sock);

    /* mark the com channel as free */
    ((ProcessComChannel *)pccObject)->InUse = FALSE;

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: Thread %s %d end!!!!!!!!!!!!!!!!\n", ((ProcessComChannel *)pccObject)->Name, ((ProcessComChannel *)pccObject)->index);
#endif	/* __RLI_DEBUG_SOCK__ */

	return NULL;
}

#endif /* __MULTI_THREADED_SERVER_ENABLED__ */


/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Accept(OS_SPECIFIC_SOCKET_HANDLE *soc,
                              OS_SPECIFIC_SOCKET_HANDLE *accSoc, ubyte4 port)
{
    struct sockaddr_in  ClientAddr;
    socklen_t           iAddrLen;
    char*               pMsg;
    int                 keepAlive = 1;
    int                 keepIdle = 10;
    int                 keepInterval = 2;
    int                 keepCount = 1;

    iAddrLen = sizeof(ClientAddr);
    memset(&ClientAddr, 0x00, iAddrLen);
    
    *accSoc = accept(*soc, (struct sockaddr *)&ClientAddr, &iAddrLen);

    if (0 >= *accSoc)
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
    if (0 > (WebControlFd = creat(wcDev, O_RDONLY)))     /* Let SR know our handle */
    {
#ifdef __RLI_DEBUG_SOCK__
        printf("SOCKET_HttpD:  Not able to create file handle!  Server shutting down.\n");
#endif
        return ERROR_GENERAL_ACCESS_DENIED;
    }

#ifdef __RLI_DEBUG_SOCK__
    printf("SOCKET_HttpD: WebControl Handle = %d.\n", WebControlFd);
#endif

    return OK;
}

#endif



/*-----------------------------------------------------------------------*/

extern ubyte4 SOCKET_GetClientsAddr(OS_SPECIFIC_SOCKET_HANDLE ClientSock)
{
    struct sockaddr_in  ClientAddr;
    socklen_t           iAddrLen;
    ubyte4              clientIpAddr = 0;

    iAddrLen = sizeof(ClientAddr);
    memset(&ClientAddr, 0x00, iAddrLen);

    if (OK == getpeername(ClientSock, (struct sockaddr *)&ClientAddr, &iAddrLen))
        clientIpAddr = ntohl(ClientAddr.sin_addr.s_addr);

    return clientIpAddr;
}


/*-----------------------------------------------------------------------*/

#if defined( __MULTI_THREADED_SERVER_ENABLED__) || defined(__OCP_MULTI_THREADED_SERVER_ENABLED__)


extern RLSTATUS SOCKET_CreateTask(ProcessComChannel* pPCC)
{
    signed portBASE_TYPE     status;

    CONVERT_ToStr(&pPCC->index, &(pPCC->Name[STRLEN(pPCC->Name)]), kDTinteger);

	status = xTaskCreate( (pdTASK_CODE)PersistentConnectionHandler, (signed char const *)pPCC->Name, kSTANDARD_RC_THREAD_STACK_SIZE, pPCC, kSTANDARD_RC_THREAD_PRIO, 0 );
	
    if (pdPASS != status)
        return SYS_ERROR_SOCKET_CREATE_TASK;

    return OK;
}

#endif

/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Close(OS_SPECIFIC_SOCKET_HANDLE sock)
{
    int err;

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
    struct sockaddr_in  RemAddr;
    ubyte4              ulHostAddress;
    int                 err;
    sbyte               *pMsg;

    MEMSET(&RemAddr, 0x00, sizeof(RemAddr));

    ulHostAddress = inet_addr(pName);
 
    if ( INADDR_NONE == ulHostAddress )
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CONNECT, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        return ERROR_GENERAL_ACCESS_DENIED;
    }

    RemAddr.sin_family      = AF_INET;
    RemAddr.sin_addr.s_addr = ulHostAddress;
    RemAddr.sin_port        = htons((ubyte2)port);

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


#endif /* __FreeRTOS_OS__ */
