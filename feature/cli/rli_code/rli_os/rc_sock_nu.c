/*
 *  rc_sock_nu.c
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

$History: rc_sock_nu.c $
 * 
 * *****************  Version 21  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 20  *****************
 * User: David        Date: 6/09/00    Time: 2:01p
 * Cleaned up some warnings.
 * 
 * *****************  Version 19  *****************
 * User: James        Date: 5/25/00    Time: 11:38a
 * Cleaned-up the thread creation code.
 *
 * *****************  Version 18  *****************
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
#include "rc_database.h"
#include "rc_convert.h"

#ifdef  __NUCLEUS_OS__

#include "rcw_persist.h"
#include "nucleus.h"

/* Nucleus glue and sockets files */

#include <Target.h>
#include <sockext.h>

/* Files for Socket-Like Integ */
#include <externs.h>
#include <socketd.h>    /* socket interface structures */

#ifdef __OCP_ENABLED__
#include "rca_ocp.h"
#include "rca_ocpd.h"
#endif	/* __OCP_ENABLED__ */

#include "rc_access.h"


/* Defaults */
#define     kANY_PORT   0

/* Structure to set local machine characteristics */
struct addr_struct  mLocalAddr;

ProcessComChannel g_pccObjects[kHTTPD_QUEUE_SIZE];

/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_SOCKET_HANDLE SOCKET_TcpCreate()
{
    sbyte   *pMsg;
    OS_SPECIFIC_SOCKET_HANDLE   soc;

    soc = NU_Socket(NU_FAMILY_IP, NU_TYPE_STREAM, NU_NONE);
    if (0 > soc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CREATE, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
    }

    return soc;
}

/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_DGRAM_HANDLE SOCKET_UdpCreate()
{
    sbyte   *pMsg;
    OS_SPECIFIC_SOCKET_HANDLE   soc;

    soc = NU_Socket(NU_FAMILY_IP, NU_TYPE_DGRAM, NU_NONE);
    if (0 > soc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CREATE, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
    }

    return soc;
}



/*-----------------------------------------------------------------------*/

static RLSTATUS BindSocketToAddr(OS_SPECIFIC_SOCKET_HANDLE soc, ubyte4 port)
{
    sbyte               *pMsg;

    MEMSET(&mLocalAddr, 0x00, sizeof(mLocalAddr));

	/* ask the system to allocate a port and bind to INADDR_ANY */
    /* get system to allocate a port number by binding a host address */

    mLocalAddr.family = NU_FAMILY_IP;
    mLocalAddr.port   = (ubyte2)port;
	*(uint32 *)mLocalAddr.id.is_ip_addrs = IP_ADDR_ANY;

    /* In older versions of nucleus net, IP_ADDR_ANY is undefined */
	/* fill in ip address here */

	/*
	mLocalAddr.id.is_ip_addrs[0] = (uint8)(chnl_info[0].my_ip_address & 0xff);
    mLocalAddr.id.is_ip_addrs[1] = (uint8)((chnl_info[0].my_ip_address >> 8) & 0xff);
    mLocalAddr.id.is_ip_addrs[2] = (uint8)((chnl_info[0].my_ip_address >> 16) & 0xff);
    mLocalAddr.id.is_ip_addrs[3] = (uint8)((chnl_info[0].my_ip_address >> 24) & 0xff);
    */

    /* bind socket to local address */
    if (0 > NU_Bind(soc, (struct addr_struct*)&mLocalAddr, 0))
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_BIND, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
        NU_Close_Socket(soc);

        return ERROR_GENERAL_ACCESS_DENIED;
    }
    else
        return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Initialize(void)
{
	return OK;
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_OpenServer(OS_SPECIFIC_SOCKET_HANDLE *pSoc, ubyte4 port)
{
    RLSTATUS    rlStatus;
    sbyte       *pMsg;
    STATUS      nuStatus;

    /* Create a stream socket */
    *pSoc = SOCKET_TcpCreate();
    if (0 > *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    rlStatus = BindSocketToAddr(*pSoc, port);
    if (OK != rlStatus)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a incoming Client connect request queue */
    nuStatus = NU_Listen(*pSoc, kTCP_LISTEN_BACKLOG);
    if (NU_SUCCESS != nuStatus)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_LISTEN, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
        NU_Close_Socket(*pSoc);

        return ERROR_GENERAL_ACCESS_DENIED;
    }
    else
        return OK;
}



/*-----------------------------------------------------------------------*/

static void PersistentConnectionHandler(void *pccObject)
{
#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: Thread start.\n");
#endif

    /* process the connection */
	((ProcessComChannel *)pccObject)->pfuncConnHandler(pccObject);

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: closing socket(threadState=%d)!!!!!!!!!!!!!\n", ((ProcessComChannel *)pccObject)->ThreadState);
#endif

    /* close down the socket */
    NU_Close_Socket(((ProcessComChannel *)pccObject)->sock);

    /* mark the com channel as free */
    ((ProcessComChannel *)pccObject)->InUse = FALSE;

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: Thread end!!!!!!!!!!!!!!!!\n");
#endif

}



/*-----------------------------------------------------------------------*/

static VOID NucleusPersistentWrapper(UNSIGNED argc, VOID *argv)
{
	PersistentConnectionHandler(argv);
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Accept(OS_SPECIFIC_SOCKET_HANDLE *soc,
                              OS_SPECIFIC_SOCKET_HANDLE *accSoc, ubyte4 port)
{
    struct addr_struct  ClientAddr;
    sbyte*              pMsg;

    MEMSET(&ClientAddr, 0x00, sizeof(ClientAddr));
    *accSoc = NU_Accept(*soc, (struct addr_struct*)&ClientAddr, 0);

    if (0 > *accSoc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_ACCEPT, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        NU_Close_Socket(*accSoc);
        NU_Close_Socket(*soc);

        SOCKET_OpenServer(soc, port);
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
    return 0;
}



/*-----------------------------------------------------------------------*/

#if defined( __MULTI_THREADED_SERVER_ENABLED__) || defined(__OCP_MULTI_THREADED_SERVER_ENABLED__)

extern RLSTATUS SOCKET_CreateTask(ProcessComChannel* pPCC)
{
    STATUS              nuStatus;

    MEMSET(pPCC->stackAddress, 0x00,
           pPCC->stackLen);

    MEMSET(&(pPCC->task), 0x00, sizeof(NU_TASK));

    CONVERT_ToStr(&pPCC->index, &(pPCC->Name[STRLEN(pPCC->Name)]), kDTinteger);

    nuStatus    = NU_Create_Task(&(pPCC->task), pPCC->Name,
					        (void*)NucleusPersistentWrapper,
                            1, pPCC, pPCC->stackAddress,
                            pPCC->stackLen, kHTTPD_SERVER_PRIO, 0,
                            NU_PREEMPT, NU_START);

    if (NU_SUCCESS != nuStatus)
        return SYS_ERROR_SOCKET_CREATE_TASK;

    return OK;
}

#endif


/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Close(OS_SPECIFIC_SOCKET_HANDLE sock)
{
    STATUS      nuStatus;

    nuStatus = NU_Close_Socket(sock);
    if (NU_SUCCESS != nuStatus )
        return ERROR_GENERAL_ACCESS_DENIED;
    else
        return OK;
}



/*-----------------------------------------------------------------------*/

static RLSTATUS
SOCKET_ConnectToServer(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pName, ubyte4 port )
{
    RLSTATUS            status;
    struct addr_struct  RemAddr;
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

    RemAddr.family = NU_FAMILY_IP;
    RemAddr.port   = (ubyte2)port;

    RemAddr.id.is_ip_addrs[0] = (uint8)(ulHostAddress & 0xff);
    RemAddr.id.is_ip_addrs[1] = (uint8)((ulHostAddress >> 8) & 0xff);
    RemAddr.id.is_ip_addrs[2] = (uint8)((ulHostAddress >> 16) & 0xff);
    RemAddr.id.is_ip_addrs[3] = (uint8)((ulHostAddress >> 24) & 0xff);

     /* Create a Client connection */
    err = NU_Connect( sock, (struct addr_struct *)&RemAddr, sizeof(RemAddr));
    if ( 0 > err )
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
    if (0 > *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    status = BindSocketToAddr(*pSoc, kANY_PORT);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a Client connection */
    status = SOCKET_ConnectToServer( *pSoc, pName, port );

    return status;
}


#endif /* __SOCKET_NUCLEUS__ */
