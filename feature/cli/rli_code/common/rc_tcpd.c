/*******************************************************************
 *  rc_tcpd.c
 *
 *  This is a part of the OpenControl SDK source code library. 
 *
 *  Copyright (C) 1998 Rapid Logic, Inc.
 *  All rights reserved.
 *
 *  DESCRIPTION:
 *
 *  AUTHOR:
 *
 *  HISTORY:    
 *      1999-01-20      Created
 *******************************************************************/

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

/**- include files **/
#include "rc_options.h"
#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_msghdlr.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#ifdef __COMMON_LISTENER__
#include "rc_socks.h"
#include "rc_tcpd.h"
#include "rcw_request.h"
#include "rc_access.h"

#ifdef __OCP_ENABLED__
#include "rca_ocp.h"
#include "rca_ocpconn.h"
#endif

#ifdef __RLI_DEBUG_SOCK__
#include "stdio.h"
#endif
#ifdef __ENABLE_MEMMGR_DEBUG__
extern void MEMMGR_DisplayDebug();
extern void MEMMGR_InitDebug();
#endif
/**- local definitions **/

/*- default settings */

/**- external functions **/

/**- external data **/

/**- internal functions **/

/**- public data **/

/**- private data **/
static TcpdListener g_Listeners[kTCP_MAXLISTENERS];
static  sbyte2 gNumListeners;
static Length   gQBytesNeeded;
/* TBD: need mutex? */
/* static OS_SPECIFIC_MUTEX    mMutex; */

/**- private functions **/

/*
 **- tcpd_ID_Listener
 *
 *  DESCRIPTION:
 *		Identify which listener should get this connection
 *  PARAMETERS:
 *		ProcessComChannel *pPCC
 *  RETURNS:
 *		RLSTATUS
 *	NOTES:
 *		Writes pfuncConnHandler and Name into PCC object
 */
static RLSTATUS tcpd_ID_Listener(ProcessComChannel *pPCC)
{
    sbyte*      pRequest_BufFeeder;
    RLSTATUS    BytesRead;
    RLSTATUS status = OK;
    Length      BytesToRead = gQBytesNeeded;
    sbyte2 idx;    
    
    /* get the qualifier bytes */
    pRequest_BufFeeder = pPCC->QualBuff;
	while (0 < BytesToRead)
	{
        BytesRead = OS_SPECIFIC_SOCKET_READ(pPCC->sock, pRequest_BufFeeder, BytesToRead);

        if (0 == BytesRead)
        {
            return SYS_ERROR_UNEXPECTED_END;
        }
#ifdef __OCP_ENABLED__
        if (SYS_ERROR_SOCKET_TIMEOUT == BytesRead)
        {
			/* socket timed out, but that's OK for OCP */
			/* we'll check the OCP connection age, kill it if decrepit, else just listen some more */
			if (TRUE != OCPCONN_IsAlive(pPCC->ConnID))
			{
				return ERROR_OCP_CONN_TIMEOUT;				
			}
#ifdef OCP_DEBUG
			printf("tcpd_ID_Listener:skt timed out, continuing\n", pPCC->index);
#endif
			BytesRead = 0;
        }
#endif /* __OCP_ENABLED__ */
        if (BytesRead < 0)
        {
            /* a tcp error occurred - release the buffers and exit */
            return BytesRead;
        }

        
        pRequest_BufFeeder += BytesRead;
        pPCC->QBytesRead += BytesRead;
        BytesToRead -= BytesRead;
		
	}

    /* walk thru each listener, invoke each qualifier until one passes */   
    for (idx=0;idx < gNumListeners ;idx++ )
    {
    	if (TRUE == g_Listeners[idx].pfuncQualifier(pPCC))
    	{
    		pPCC->ListenerIndex = idx;
    		pPCC->pfuncConnHandler = g_Listeners[idx].pfuncConnHandler;
			STRCPY( pPCC->Name, g_Listeners[idx].Name );
			return OK;
    	}
    }
    return ERROR_GENERAL_NOT_FOUND;
} /* tcpd_ID_Listener */

static RLSTATUS tcpd_InitListeners(void)
{
    sbyte2 idx;    

    for (idx=0;idx < kTCP_MAXLISTENERS ;idx++ )
    {
        g_Listeners[idx].index = idx;
        MEMSET(g_Listeners[idx].Name, 0x00, kTCPd_TASKNAME_LENGTH);
        g_Listeners[idx].QBytesNeeded = 0;
        g_Listeners[idx].pfuncQualifier = NULL;
        g_Listeners[idx].pfuncConnHandler = NULL;
    }
    return OK;
}
    
/*-----------------------------------------------------------------------*/
/**- tcpd_InitPccPool
 *
 *  DESCRIPTION:
 *      Initialize the pool of ProcessComChannel structures
 *  PARAMETERS:
 *      ProcessComChannel*  pccObjects: Ptr to array of ProcessComChannel structs
 *      Counter             QueuSize:   number of entries in array
 *  RETURNS:
 *      
 *  Notes:
 *      derived (and replaces) from WC's Persist_InitChildren
 */
static RLSTATUS TCPD_InitPccPool(ProcessComChannel *pccObjects,
                                     Counter QueueSize)
{
    Counter idx;

    for (idx = 0; idx < QueueSize; idx++)
    {
        pccObjects[idx].index       = idx;
        pccObjects[idx].env         = NULL;
        pccObjects[idx].InUse       = FALSE;
        pccObjects[idx].ThreadState = kThreadDead;
        pccObjects[idx].pfuncConnHandler = NULL;
        MEMSET(pccObjects[idx].Name, 0x00, kTCPd_TASKNAME_LENGTH);
        pccObjects[idx].QBytesRead = 0;
        pccObjects[idx].ListenerIndex = -1;
        MEMSET(pccObjects[idx].QualBuff, 0x00, kTCP_QUALBUFSIZE);

#ifdef __NUCLEUS_OS__
        if (NULL == (pccObjects[idx].stackAddress = RC_MALLOC(kHTTP_THREAD_STACK_SIZE)))
        {
            int idx1;

            for (idx1 = 0; idx1 < idx; idx1++)
                RC_FREE(pccObjects[idx1].stackAddress);

            return SYS_ERROR_NO_MEMORY;
        }

        pccObjects[idx].stackLen = kHTTP_THREAD_STACK_SIZE;
#endif

    }

    return OK;
} /* TCPD_InitPccPool */


/**- public functions **/


/*-----------------------------------------------------------------------*/
/**- TCPD_TcpdMain
 *
 *  DESCRIPTION:
 *      
 *  PARAMETERS:
 *      
 *  RETURNS:
 *      
 *  Notes:
 *      
 */
extern void TCPD_TcpdMain()
{
    RLSTATUS    status = OK;
    ubyte4              clientAddr   = 0;   /* actual ip address */
    Counter             numConn      = 0;
    int                 findConn     = 0;
    OS_SPECIFIC_SOCKET_HANDLE soc;
    OS_SPECIFIC_SOCKET_HANDLE accSoc;
	Boolean bFound;
#ifdef __RLI_DEBUG_SOCK__
	Boolean bReportedPend;
#endif

    if (OK != TCPD_InitPccPool(&g_pccObjects[0], kHTTPD_QUEUE_SIZE))
        return;

    if (OK != SOCKET_OpenServer(&soc, kFIXED_PORT))
        return;

#ifdef __MASTER_SLAVE_SNMP_STACK__

    if (OK > PERSIST_TickleMutexCreate())
    {
        sbyte *pMsg;

        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_MUTEX_CREATE, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        return;
    }

    if (OK != SOCKET_CreateMasterInterface())
        return;

#endif /* __MASTER_SLAVE_SNMP_STACK__ */

    while (1)
    {
        if (OK != SOCKET_Accept(&soc, &accSoc, kFIXED_PORT))
            continue;                       /* an error occurred, just continue */

        bFound = FALSE;
#ifdef __RLI_DEBUG_SOCK__
		bReportedPend = FALSE;
#endif

#ifdef __MULTI_THREADED_SERVER_ENABLED__
		while (FALSE==bFound)
		{
#endif

#ifdef __OCP_ENABLED__
			OCPCONN_RemoveDeadConnections();
#endif

	         /* find a free pccObject */
	        numConn  = 0;
	        findConn = (findConn + 1) % kHTTPD_QUEUE_SIZE;

	        while (numConn < kHTTPD_QUEUE_SIZE)
	        {
	            if (TRUE == g_pccObjects[findConn].InUse)
	            {
	                numConn++;
	                findConn = (findConn + 1) % kHTTPD_QUEUE_SIZE;
	            }
                else 
                {
                    /* found one */
                    bFound = TRUE;
                    break;
                }
	        }

#ifdef __MULTI_THREADED_SERVER_ENABLED__
			if (FALSE==bFound)
			{
#ifdef __RLI_DEBUG_SOCK__
				if (FALSE==bReportedPend)
				{
					printf("Common Listener Waiting for PCC\n");
					bReportedPend = TRUE;
				}
#endif
                /* yield and try again later */
                OS_SPECIFIC_YIELD();
			}
		} /* end while !bFound */

#ifdef __RLI_DEBUG_SOCK__
		if (TRUE==bReportedPend)
		{
			printf("Common Listener Pend complete, got PCC # %d\n", findConn);
		}
#endif
#endif /* __MULTI_THREADED_SERVER_ENABLED__ */

        if (FALSE == bFound)
        {
            /* all of the connections are in use, hang up. */
            SOCKET_Close(accSoc);
            continue;
        }

        clientAddr = SOCKET_GetClientsAddr(accSoc);

#ifdef __ENABLE_LAN_IP_FILTER__
		if (TRUE != ACCESS_ValidIpAddress(clientAddr))
		{
#ifdef __RLI_DEBUG_SOCK__
            printf("TCPD_TcpdMain:  Applying filter --- sorry.\n");
#endif

            g_pccObjects[findConn].InUse = FALSE;

            SOCKET_Close(accSoc);
            continue;
		}
#endif /* __ENABLE_LAN_IP_FILTER__ */

        /* initialize information for child thread  */
        g_pccObjects[findConn].ThreadState = kThreadCreate;
        g_pccObjects[findConn].sock        = accSoc;
        g_pccObjects[findConn].env         = NULL;
        g_pccObjects[findConn].InUse       = TRUE;
        g_pccObjects[findConn].IpAddr      = clientAddr;

        /* Determine which listener gets this by peeking at first few bytes*/
        status = tcpd_ID_Listener(&g_pccObjects[findConn]);
#ifdef __RLI_DEBUG_SOCK__
        if (OK == status)
        {
            printf("TCPD_TcpdMain:  tcpd_ID_Listener() failed.\n");
        }
#endif

#ifdef __MULTI_THREADED_SERVER_ENABLED__
        if (OK == status)
        {
            status = SOCKET_CreateTask(&g_pccObjects[findConn]);
#ifdef __RLI_DEBUG_SOCK__
	        if (OK > status)
	        {
	            printf("TCPD_TcpdMain:  SOCKET_CreateTask() failed.\n");
	        	
	        }
#endif
        }
        if (OK > status)
        {
            g_pccObjects[findConn].InUse  = FALSE;
#ifdef __OCP_ENABLED__
			g_pccObjects[findConn].ConnID = 0;
#endif

            SOCKET_Close(accSoc);
            continue;
        }
#else /* ! __MULTI_THREADED_SERVER_ENABLED__ */
        if (OK == status)
        {
#ifdef __ENABLE_MEMMGR_DEBUG__
		    printf("======================== START Common TCP ==================\n");
#endif
            /* process the connection */
            g_Listeners[g_pccObjects[findConn].ListenerIndex].pfuncConnHandler(&g_pccObjects[findConn]);

            /* close down the socket */
            SOCKET_Close(g_pccObjects[findConn].sock);

            /* mark the com channel as free */
            g_pccObjects[findConn].InUse = FALSE;
#ifdef __OCP_ENABLED__
			g_pccObjects[findConn].ConnID = 0;
#endif

#ifdef __ENABLE_MEMMGR_DEBUG__
	        MEMMGR_DisplayDebug();
	        MEMMGR_InitDebug();
		    printf("======================== END Common TCP=============================\n");
#endif
        }

#endif /* ! __MULTI_THREADED_SERVER_ENABLED__ */

    } /* while(1) */
} /* TCPD_TcpdMain */

/*
 **- TCPD_Register
 *
 *  DESCRIPTION:
 *      Register a server (listener) with the common port listener tcpd
 *  PARAMETERS:
 *      Name:               optional string identifying listener
 *      QBytesNeeded:       number of bytes needed from buffer for qualifier
 *      pfuncQualifier:     ptr to qualifier function
 *      pfuncConnHandler:   ptr to connection handler function
 *  RETURNS:
 *      OK if successful, <0 if not
 *  NOTES:
 *
 */
extern RLSTATUS TCPD_Register( sbyte* Name, Length QBytesNeeded, 
                                Boolean (*pfuncQualifier)(void *pPccObj),
                                void (*pfuncConnHandler)(void *pccObject) )
{
    RLSTATUS status;
    Counter idx;

    if (OK != (status = TCPD_Init()))
    {
        return status;
    }
    if (kTCP_MAXLISTENERS <= gNumListeners)
    {
        return ERROR_GENERAL;
    }

    if (kTCP_QUALBUFSIZE < QBytesNeeded)
    {
        return ERROR_GENERAL_ILLEGAL_VALUE;
    }
    idx = gNumListeners++;
    g_Listeners[idx].QBytesNeeded = QBytesNeeded;
    if (gQBytesNeeded < QBytesNeeded)
    {
        gQBytesNeeded = QBytesNeeded;
    }


    g_Listeners[idx].pfuncQualifier = pfuncQualifier;
    g_Listeners[idx].pfuncConnHandler = pfuncConnHandler;
    if (NULL != Name)
    {
        STRNCPY(g_Listeners[idx].Name, Name, kTCPd_TASKNAME_LENGTH);
        g_Listeners[idx].Name[kTCPd_TASKNAME_LENGTH]='\0';
    }
	return OK;
} /* TCPD_Register */

extern RLSTATUS TCPD_Init(void)
{
    static int  bInitYet = FALSE;
    RLSTATUS    status;
    if (TRUE==bInitYet)
    {
        return OK;
    }
    gNumListeners = 0;
    gQBytesNeeded = 0;
    if (OK < (status = tcpd_InitListeners()))
    {
        return status;
    }

    bInitYet = TRUE;
    return OK;
}    
/* end rc_tcpd.c */

#endif  /* __COMMON_LISTENER__ */

