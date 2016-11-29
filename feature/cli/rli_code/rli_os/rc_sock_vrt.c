/*  
 *  rc_sock_vrt.c
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

$History: rc_sock_vrt.c $
 * 
 * *****************  Version 17  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 16  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/

#include "rc_options.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_errors.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_msghdlr.h"
#include "rc_socks.h"

#ifdef  __SOCKET_VRTX__

#ifndef __SOCK_ATTACHE_TCPIP_STACK__

#include <errno.h>
#include <malloc.h>

#include <vrtxvisi.h>
#include <vrtxil.h>
#include <tnxvisi.h>

#include "rcw_persist.h"

#ifdef __OCP_ENABLED__
#include "rca_ocp.h"
#include "rca_ocpd.h"
#endif	/* __OCP_ENABLED__ */

#include "rc_access.h"



/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_SOCKET_HANDLE SOCKET_TcpCreate()
{
    char        *pMsg;
    OS_SPECIFIC_SOCKET_HANDLE   soc;

    *pSoc = tnx_socket(AF_INET, SOCK_STREAM, 0, &status);

    if (TNX_OK != status)
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
    RLSTATUS            status = OK;
    char                *pMsg;
    int                 err;

    /* ask the system to allocate a port and bind to INADDR_ANY */
    MEMSET(&LocalAddr, 0x00, sizeof(LocalAddr));

    /* get system to allocate a port number by binding a host address */
    LocalAddr.sin_family        = AF_INET;
    LocalAddr.sin_addr          = INADDR_ANY;
    LocalAddr.sin_port          = port;

    /* bind socket to local address */
    tnx_bind(soc, (struct sockaddr_in *) &LocalAddr, 16, &err);

    if (TNX_OK != err)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_BIND, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
        tnx_close(soc, &err);

        status = ERROR_GENERAL_ACCESS_DENIED;
    }

    return status;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_OpenServer(OS_SPECIFIC_SOCKET_HANDLE *pSoc, ubyte4 port)
{
    RLSTATUS  status;
    char    *pMsg;
    int     err;

    /* Create a stream socket */
    *pSoc = SOCKET_TcpCreate();
    if (0 > *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    status = BindSocketToAddr(*pSoc, port);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a incoming Client connect request queue */
    tnx_listen(*pSoc, kTCP_LISTEN_BACKLOG, &err);
    if (TNX_OK != err)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_LISTEN, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
        tnx_close(*pSoc, &err);
        
        return ERROR_GENERAL_ACCESS_DENIED;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

static void PersistentConnectionHandler(void *pccObject)
{
    int err;

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: Thread start.\n");
#endif

    /* process the connection */
	((ProcessComChannel *)pccObject)->pfuncConnHandler(pccObject); 

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: closing socket(threadState=%d)!!!!!!!!!!!!!\n", ((ProcessComChannel *)pccObject)->ThreadState);
#endif

    /* close down the socket */
    tnx_close(((ProcessComChannel *)pccObject)->sock, &err);

    /* mark the com channel as free */
    ((ProcessComChannel *)pccObject)->InUse = FALSE;

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: Thread end!!!!!!!!!!!!!!!!\n");
#endif

    /* check if we need to kill thread here */
    sc_tdelete(0, 0, &err);
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Accept(OS_SPECIFIC_SOCKET_HANDLE *soc, 
                              OS_SPECIFIC_SOCKET_HANDLE *accSoc, ubyte4 port)
{
    struct sockaddr_in  ClientAddr;
    int                 iAddrLen, err;
    char*               pMsg;

    iAddrLen = sizeof(ClientAddr);
    MEMSET(&ClientAddr, 0x00, (unsigned int)iAddrLen);

    *accSoc = tnx_accept(*soc, (struct sockaddr_in *)&ClientAddr, &iAddrLen, &err);

    if (TNX_OK != err)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_ACCEPT, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        tnx_close(*accSoc, &err);        
        tnx_close(*soc, &err);

        SOCKET_OpenServer(soc, port);
        return SYS_ERROR_SOCKET_ACCEPT;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

#ifdef __MASTER_SLAVE_SNMP_STACK__

extern RLSTATUS SOCKET_CreateMasterInterface()
{
    return ERROR_GENERAL_NOT_FOUND;         /* N/A for VRTX */
}

#endif



/*-----------------------------------------------------------------------*/

extern ubyte4 SOCKET_GetClientsAddr(OS_SPECIFIC_SOCKET_HANDLE ClientSock)
{
    return 0;
}



/*-----------------------------------------------------------------------*/

extern sbyte4 SOCKET_CreateTask(ProcessComChannel* pPCC)
{
    int status;

    sc_tcreate((void *)PersistentConnectionHandler, (int)pPCC), 
                kHTTPD_SERVER_PRIO, &status);

    if (RET_OK != status)
        return SYS_ERROR_SOCKET_CREATE_TASK;

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Close(OS_SPECIFIC_SOCKET_HANDLE sock)
{
    int err;

    tnx_close(sock, &err);

    return OK;
}



/*-----------------------------------------------------------------------*/

#ifdef __SMTP_ENABLED__

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
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        return ERROR_GENERAL_ACCESS_DENIED;
    }

    RemAddr.sin_family      = AF_INET;
    RemAddr.sin_addr.s_addr = ulHostAddress;
    RemAddr.sin_port        = htons((ubyte2)port);

    /* Create a Client connection */
    err = tnx_connect( sock, (struct sockaddr *)&RemAddr, sizeof(RemAddr));
    if (TNX_OK != err)
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
    if (ERROR == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    status = BindSocketToAddr(*pSoc, kANY_PORT);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a Client connection */
    status = SOCKET_ConnectToServer( *pSoc, pName, port );

    return status;
}
#endif /* __SMTP_ENABLED__ */

#endif /* __SOCK_ATTACHE_TCPIP_STACK__ */

#endif /* __SOCKET_VRTX__ */
