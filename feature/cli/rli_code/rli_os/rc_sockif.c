/*  
 *  rc_sockif.c
 *
 *  This is a part of the RapidControl SDK source code library. 
 *
 *  Copyright (C) 1999 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */

/*

$History: rc_sockif.c $
 * 
 * *****************  Version 6  *****************
 * User: Pstuart      Date: 1/05/01    Time: 1:33p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * fixed funky #if statements
 * 
 * *****************  Version 5  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments


 */

#include "rc_options.h"

#ifdef __SOCK_ATTACHE_TCPIP_STACK__

#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_errors.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_msghdlr.h"
#include "rc_socks.h"

#include <errno.h>
#ifdef __OLDER_ATTACHE_STACK__
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include <vrtxil.h>

#include <attache/h/config.h>
#include <epilogue/types.h>
#include <attache/h/mib.h>
#include <attache/h/timer.h>
#include <attache/h/packet.h>
#include <attache/h/net.h>
#include <attache/h/route.h>
#include <attache/h/ip.h>
#include <attache/h/slowtime.h>
#include <attache/h/glue.h>
#include <attache/h/tcp.h>

#include "rcw_persist.h"

#ifdef __OCP_ENABLED__
#include "rca_ocp.h"
#include "rca_ocpd.h"
#endif	/* __OCP_ENABLED__ */

#include "rc_access.h"
#include "rc_sockif.h"

#ifdef __OLDER_ATTACHE_STACK__
#include <platform/task.h>
#include <include/vrtxil.h>
#endif

#if 0
#define __DEBUG_SOCK_EMULATION_INTERFACE__
#define __DEBUG_SOCK_UPCALL_INTERFACE__
#endif

#undef __SOCKIF_TEST_CIRCULAR_BUFFER__

#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
#include <stdio.h>
#endif

/*-----------------------------------------------------------------------*/

/* !-!-!-!-!-!-! single threaded webserver ONLY. JAB */
#define kMaxThreads             1
#define kMaxQueueSize           5

#ifdef __OLDER_ATTACHE_STACK__
#define kRCW_QueueId            ACCEPT_WEB_ID
#define kRCW_ParentId           LISTEN_WEB_ID
#else
#define kRCW_QueueId            InsertWebQueueIdHere
#define kRCW_ParentId           InsertWebQueueIdHere
#endif

/*!-!-!-!-!-! getting things setup to make life easier in the future. JAB */
#define kMaxListenPorts             1
#define kMaxSimultaneousConnections kMaxQueueSize

/* unfortunately, this version of Attache requires that the window size */
/* MUST be non-zero in the listen state. JAB */

/* prototype */
static sockIfDescr *construct_sidObject(Boolean createChild);
static void destruct_sidObject(sockIfDescr **pp_sidObject);



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKIF_Init(void)
{
    RLSTATUS status;
    int      qid1 = kRCW_QueueId;
    int      qid2 = kRCW_ParentId;
	int		 loop;

#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
    printf("SOCKIF_Init: attempting...\n");		
#endif

    if (OK != (status = SOCKIF_CreateQueue(&qid1)))
    {
#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
        printf("SOCKIF_Init: Worker queue initialization failed.\n");		
#endif
        goto error;
    }

    if (OK != (status = SOCKIF_CreateQueue(&qid2)))
    {
#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
        printf("SOCKIF_Init: Parent queue initialization failed.\n");		
#endif
        goto error;
    }

#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
    printf("SOCKIF_Init: success.\n");		
#endif

error:

    return status;

} /* SOCKIF_Init */


/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKIF_CreateQueue(OS_SPECIFIC_QUEUE *pQid)
{
    RLSTATUS status = OK;
#ifdef __OLDER_ATTACHE_STACK__
    long     err;
#else
    int      err;
#endif


	/* sc_qcreate is the OS specific call - this needs to be changed to reflect 
	   your system call. in this case, sc_qcreate is really just a semaphore list */
    sc_qcreate(*pQid, kMaxQueueSize, &err);

    if (RET_OK != err)
    {
        status = SYS_ERROR_GENERAL;

#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
        if (ER_MEM == err)
		{
            printf("SOCKIF_CreateQueue: Out of memory.\n");		
		}
        else if (ER_QID == err)
		{
            printf("SOCKIF_CreateQueue: Queue Id already assigned.\n");		
		}
        else
		{
            printf("SOCKIF_CreateQueue: Unknown error (%d).\n", err);		
		}
#endif
    }
    
    return status;

} /* SOCKIF_CreateQueue */



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKIF_ReleaseQueue(OS_SPECIFIC_QUEUE *pQid)
{
    /* Not needed: VRTX doesn't have support for releasing queues.  JAB */

    /* If we wanted to generize, we would probably have some code that emptied the queue. JAB */

    return OK;
}



/*-----------------------------------------------------------------------*/

/* the API will change based on the version of Nucleus */
extern RLSTATUS SOCKIF_SendQueue(OS_SPECIFIC_QUEUE qid, long Mesg)
{
    RLSTATUS status = OK;
    int      err;

	
	/* sc_qpost is the OS specific call - this needs to be changed to reflect 
	   your system call. in this case, sc_qpost is part of the packetizing routine */
	
	sc_qpost(qid, (long)Mesg, (long *)&err);

    if (RET_OK != err)
    {
        status = SYS_ERROR_GENERAL;

#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
        if (ER_QID == err)
		{
            printf("SOCKIF_SendQueue: Bad Queue Id.\n");		
		}
        else if (ER_QFL == err)
		{
            printf("SOCKIF_SendQueue: Queue overflow.\n");		
		}
        else
		{
            printf("SOCKIF_SendQueue: Unknown error (%d).\n", err);		
		}
#endif
    }

    return status;
}



/*-----------------------------------------------------------------------*/

/* the API will change based on the version of Nucleus */

extern RLSTATUS SOCKIF_RecvQueue(OS_SPECIFIC_QUEUE qid, OS_SPECIFIC_TIMER timeout, long *pRetMesg)
{
    RLSTATUS status = OK;
    long     err;

#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
    printf("SOCKIF_RecvQueue: Pending on queue.\n");		
#endif

	/* sc_qpend is the OS specific call - this needs to be changed to reflect 
	   your system call. in this case, sc_qpend is part of the packetizing routine */
	
    *pRetMesg = (sc_qpend(qid, timeout, (long *)&err));

    if (RET_OK != err)
    {
        if (ER_TMO != err)
            status = SYS_ERROR_GENERAL;
        else
            status = SYS_ERROR_UNEXPECTED_END;

#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
        if (ER_QID == err)
		{
            printf("SOCKIF_RecvQueue: Bad Queue Id.\n");		
		}
        else if (ER_TMO != err)
		{
            printf("SOCKIF_RecvQueue: Unknown error (%d).\n", err);		
		}
#endif
    }

#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
    printf("SOCKIF_RecvQueue: Exit on queue.\n");		
#endif

    return status;

} /* SOCKIF_RecvQueue */



/*-----------------------------------------------------------------------*/

/* EPILOGUE Attache Plus specific upcall methods */

static void SOCKIF_open_upcall(struct tcb *rcw_tcb)
{
    struct sockIfDescr* rcw_conn     = tcp_get_cookie(rcw_tcb);
    struct sockIfDescr* new_rcw_conn = construct_sidObject(TRUE);

	tcp_set_cookie(rcw_tcb, NULL);

#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
    printf("SOCKIF_open_upcall: rcw_conn = %x.\n", rcw_conn);		
#endif

	if (NULL == new_rcw_conn)
	{
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
        printf("SOCKIF_open_upcall: construct_sidObject() failed.\n");		
#endif
		goto error;
	}

    if (NULL == rcw_conn)
    {
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
        printf("SOCKIF_open_upcall: rcw_conn = NULL, listen() failed to configure port correctly!\n");		
#endif

		goto error;
    }

#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
    printf("SOCKIF_open_upcall: sending notification (%x),\n", rcw_tcb);		
#endif

	tcp_set_cookie(rcw_tcb, new_rcw_conn);
    new_rcw_conn->pTcb = rcw_tcb;

    /* signal consumer, SOCKIF_Accept() */
/*    if (OK != SOCKIF_SendQueue(rcw_conn->sync_queue, (long)rcw_tcb))*/
    if (OK != SOCKIF_SendQueue(LISTEN_WEB_ID, (long)rcw_tcb))
    {
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
        printf("SOCKIF_open_upcall: SOCKIF_SendQueue() failed!\n");		
#endif

		goto error;
    }

	return;

error:

	tcp_set_cookie(rcw_tcb, NULL);

	if (NULL != new_rcw_conn)
		destruct_sidObject(&new_rcw_conn);

    tcp_abort(rcw_tcb);

	return;

} /* SOCKIF_open_upcall */



/*-----------------------------------------------------------------------*/

static bits16_t SOCKIF_receive_upcall(struct tcb *rcw_tcb, packet *p)
{
    struct sockIfDescr* rcw_conn       = tcp_get_cookie(rcw_tcb);
    char*               pReceiveBuffer = NULL;
    ubyte*              pPktPayload    = p->pkt_data;
    sbyte4              numOctets      = p->pkt_datalen;
    sbyte4              octetsToMove   = 0;
    bits16_t            octetsAvail    = 0;
    sbyte4              Head, Tail;

    /* circular buffer: the producer... */

    /* to keep the code atomic, this function may only modify the tail index. */
    /* similiarly, ATTACHE_SocketRead() may only modify the head. JAB */

	if (NULL == rcw_conn)
	{
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
	    printf("SOCKIF_receive_upcall: rcw_conn = NULL.\n");
#endif
		goto error;
	}

    pReceiveBuffer = rcw_conn->pReceiveBuffer;

#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
    printf("SOCKIF_receive_upcall: rcw_conn = %x, numOctets = %d.\n", rcw_conn, numOctets);
#endif

	if (NULL == pReceiveBuffer)
	{
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
        printf("SOCKIF_receive_upcall: pReceiveBuffer == NULL! (rcw_tcb = %x)\n", rcw_tcb);
#endif
		goto error;
	}

    Head = rcw_conn->ReceiveHeadIndex;
    Tail = rcw_conn->ReceiveTailIndex;

    /* check to see if remote-side is overflowing us. JAB */
    if (numOctets > rcw_conn->receiveBufSize)
    {
        /*!-!-!-!-!-! sending more bytes than we can possibly handle. JAB */

#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
        printf("SOCKIF_receive_upcall: packet too big (%d).\n", numOctets);
#endif

        goto error;
    }

    /* copy out the data from the packet and place into our buffer. JAB */
    if ((Tail + 1) < Head)
    {
        /* the simple case... */

        if ((Head - (Tail + 1)) < numOctets) 
        {
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
            printf("SOCKIF_receive_upcall: too much data being pushed (asking: %d, handle: %d).\n", numOctets, Head-Tail+1);
#endif
            goto error;
        }

        MEMCPY(&pReceiveBuffer[Tail], pPktPayload, numOctets);
        Tail += numOctets;
    }
    else if ((Tail + 1) > Head)
    {
        if (((rcw_conn->receiveBufSize - 1) - (Tail - Head)) < numOctets)
        {
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
            printf("SOCKIF_receive_upcall: too much data being pushed (asking: %d, hdl = %d).\n", numOctets, ((rcw_conn->receiveBufSize - 1) - (Tail - Head)));
#endif
            goto error;
        }

        /* upper circular buffer octet count... */
        octetsToMove = ((rcw_conn->receiveBufSize - Head) < numOctets) ? (rcw_conn->receiveBufSize - Head) : numOctets;

        /* move the upper chunk */
        MEMCPY(&pReceiveBuffer[Tail], pPktPayload, octetsToMove);
        Tail = ((Tail + octetsToMove) & (rcw_conn->receiveBufSize - 1));

        if (numOctets > octetsToMove)
        {
            /* move a lower chunk, if necessary */
            MEMCPY(&pReceiveBuffer[Tail], &pPktPayload[octetsToMove], (numOctets - octetsToMove));
            Tail += (numOctets - octetsToMove);
        }
    }
    else
    {
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
        printf("SOCKIF_receive_upcall: too much data being pushed (asking: %d, buffer full).\n", numOctets);		
#endif
        goto error;
    }

	/* update tail */
    rcw_conn->ReceiveTailIndex = Tail;

    /* free the packet */
    tcp_free(p); p = NULL;

    /* check to see if the upper layer wants to be notified, when data is available. JAB */
    if (TRUE == rcw_conn->signalMePlease)
    {
        rcw_conn->signalMePlease = FALSE;

        /* notify */
        if (OK != SOCKIF_SendQueue(rcw_conn->sync_queue, kSOCKET_READ_DATA))
        {
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
            printf("SOCKIF_receive_upcall: queue failed.\n");		
#endif
        }
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
		else { 
			printf("SOCKIF_receive_upcall: queue succeeded, ticks %lu.\n", sc_gtime() );		
		}
#endif
    }

    /* calculate the new receive window size. note: update Head in-case consumer ate some... */
    Head = rcw_conn->ReceiveHeadIndex;

    if (Head > Tail)
        octetsAvail = (Head - (Tail + 1));
    else
        octetsAvail = ((rcw_conn->receiveBufSize - 1) - (Tail - Head));

#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
	printf("SOCKIF_receive_upcall: rx window set to %lu.\n", octetsAvail );		
#endif
    /* based on the num octets remaining in our buffer, set the RX window size to that amount. JAB */
    return octetsAvail;

error:

    if (NULL != p)
    {
        /* free the packet */
        tcp_free(p); p = NULL;
    }

	if (NULL != rcw_conn)
	{
		/* check to see if the upper layer wants to be notified, when data is available. JAB */
		if (TRUE == rcw_conn->signalMePlease)
		{
			rcw_conn->signalMePlease = FALSE;

			/* notify */
			if (OK != SOCKIF_SendQueue(rcw_conn->sync_queue, kSOCKET_ERROR))
			{
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
				printf("SOCKIF_receive_upcall: queue failed.\n");		
#endif
			}
		}
	}

	/* tcp_abort is part of the downcall for attache  */

    tcp_abort(rcw_tcb);

    return 0;

} /* SOCKIF_receive_upcall */



/*-----------------------------------------------------------------------*/

static void SOCKIF_transmit_upcall(struct tcb *rcw_tcb)
{
    struct sockIfDescr *rcw_conn = tcp_get_cookie(rcw_tcb);

#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
    printf("SOCKIF_transmit_upcall: rcw_conn = %x.\n", rcw_conn);		
#endif

	if (NULL != rcw_conn)
	{
		/* check to see if the upper layer wants to be notified, when data is available. JAB */
		if (TRUE == rcw_conn->signalMePlease)
		{
			rcw_conn->signalMePlease = FALSE;

			/* we can send data again. JAB */
			rcw_conn->windowRemainderSize = kMaxTransmitBufSize;

			/* notify */
			if (OK != SOCKIF_SendQueue(rcw_conn->sync_queue, kSOCKET_SEND_DATA))
			{
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
				printf("SOCKIF_transmit_upcall: queue failed.\n");		
#endif
			}
		}
	}
}



/*-----------------------------------------------------------------------*/

static void SOCKIF_remote_close_upcall(struct tcb *rcw_tcb)
{
    struct sockIfDescr *rcw_conn = tcp_get_cookie(rcw_tcb);

#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
    printf("SOCKIF_remote_close_upcall: rcw_conn = %x.\n", rcw_conn);		
#endif
    /* check to see if anyone is waiting for something to complete, if so, notify and clean-up. JAB */

    tcp_close(rcw_tcb);

	if (NULL == rcw_conn)
		return;

    if (TRUE == rcw_conn->signalMePlease)
	{
		rcw_conn->signalMePlease = FALSE;

        /* notify */
        if (OK != SOCKIF_SendQueue(rcw_conn->sync_queue, kSOCKET_ERROR))
        {
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
            printf("SOCKIF_remote_close_upcall: queue failed.\n");		
#endif
        }
	}
}



/*-----------------------------------------------------------------------*/

static void SOCKIF_closed_upcall(struct tcb *rcw_tcb, int why)
{
    struct sockIfDescr *rcw_conn = tcp_get_cookie(rcw_tcb);

#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
    printf("SOCKIF_closed_upcall: rcw_conn = %x.\n", rcw_conn);		
#endif

	if (NULL == rcw_conn)
		return;

	rcw_conn->boolSockActive = FALSE;

    /* check to see if anyone is waiting for something to complete, if so, notify and clean-up. JAB */
    if (TRUE == rcw_conn->signalMePlease)
	{
		rcw_conn->signalMePlease = FALSE;

        /* notify */
        if (OK != SOCKIF_SendQueue(rcw_conn->sync_queue, kSOCKET_ERROR))
        {
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
            printf("SOCKIF_remote_close_upcall: queue failed.\n");		
#endif
        }
	}
}



/*-----------------------------------------------------------------------*/

/* standard socket interface code */

static void destruct_sidObject(sockIfDescr **pp_sidObject)
{
    if (NULL != *pp_sidObject)
    {
        if (NULL != (*pp_sidObject)->pReceiveBuffer)
		{
            RC_FREE((*pp_sidObject)->pReceiveBuffer);
		}

        RC_FREE (*pp_sidObject);
        *pp_sidObject = NULL;
    }
}



/*-----------------------------------------------------------------------*/

static sockIfDescr *construct_sidObject(Boolean createChild)
{
    sockIfDescr* p_acceptSock = NULL;
    sbyte*       pBuf         = NULL;

#ifdef __SOCKIF_TEST_CIRCULAR_BUFFER__
    static int   start_index  = (kMaxReceiveBufSize - 0xc9);
#endif

    if (NULL == (p_acceptSock = RC_MALLOC(sizeof(sockIfDescr))))
        goto error;

    if (TRUE == createChild)
        if (NULL == (pBuf = RC_MALLOC(kMaxReceiveBufSize)))
            goto error;

    MEMSET(p_acceptSock, 0x00, sizeof(sockIfDescr));

    p_acceptSock->boolSockActive      = FALSE;

    p_acceptSock->pTcb                = NULL;

    /* need to deal with blocks later on. JAB */
    if (TRUE == createChild)
        p_acceptSock->sync_queue      = kRCW_QueueId;
    else
        p_acceptSock->sync_queue      = kRCW_ParentId;

    p_acceptSock->ReceiveHeadIndex    = 0;
    p_acceptSock->ReceiveTailIndex    = 0;

#ifdef __SOCKIF_TEST_CIRCULAR_BUFFER__
	if ((kMaxReceiveBufSize <= start_index) || (0 > start_index))
		start_index = 0;

    p_acceptSock->ReceiveHeadIndex    = start_index;
    p_acceptSock->ReceiveTailIndex    = start_index;

    start_index++;
#endif

    p_acceptSock->pReceiveBuffer      = pBuf;
    p_acceptSock->receiveBufSize      = kMaxReceiveBufSize;

    p_acceptSock->windowRemainderSize = kMaxTransmitBufSize;
    p_acceptSock->signalMePlease      = FALSE;

    return p_acceptSock;

error:

    if (NULL != p_acceptSock)
        RC_FREE(p_acceptSock);

    if (NULL != pBuf)
        RC_FREE(pBuf);

    return NULL;

} /* construct_sidObject */



/*-----------------------------------------------------------------------*/

extern OS_SPECIFIC_SOCKET_HANDLE SOCKIF_listen(ubyte2 localport)
{
    sockIfDescr* p_listenSock = NULL;
    struct tcb*  pTcb         = NULL;
    
    if (NULL == (p_listenSock = construct_sidObject(FALSE)))
        goto error;

    if (NULL == (pTcb = tcp_create()))
        goto error;

    /* setup the upcall handlers */
    tcp_set_open_upcall(pTcb, SOCKIF_open_upcall);
    tcp_set_receive_upcall(pTcb, SOCKIF_receive_upcall);
    tcp_set_transmit_upcall(pTcb, SOCKIF_transmit_upcall);
    tcp_set_remote_close_upcall(pTcb, SOCKIF_remote_close_upcall);
    tcp_set_closed_upcall(pTcb, SOCKIF_closed_upcall);

    /* set the window size and cookie data */
    tcp_set_cookie(pTcb, p_listenSock);
    tcp_set_receive_window(pTcb, (kMaxReceiveBufSize - 1));

    /* bind to the local port only */
    tcp_bind(pTcb, 0, 0, 0, localport);

    /* add it to the internal queue */
    if (TCP_START_OK != tcp_start(pTcb, TCP_PASSIVE))
    {
        tcp_close(pTcb);
        goto error;
    }

    p_listenSock->pTcb = pTcb;
    p_listenSock->boolSockActive = TRUE;

    return p_listenSock;

error:

    if (NULL != p_listenSock)
        destruct_sidObject(&p_listenSock);

    return NULL;

} /* SOCKIF_listen */



/*-----------------------------------------------------------------------*/

extern OS_SPECIFIC_SOCKET_HANDLE SOCKIF_accept(OS_SPECIFIC_SOCKET_HANDLE soc)
{
    sockIfDescr* p_acceptSock = NULL;
    RLSTATUS     status       = OK;
    long         Mesg;
    struct tcb*  pTcb         = NULL;

    /* get notification from SOCKIF_open_upcall() to proceed. JAB */
/*    if (OK != SOCKIF_RecvQueue(RLI_GET_QUEUE(soc), 0, &Mesg))*/
    if (OK != SOCKIF_RecvQueue(LISTEN_WEB_ID, 0, &Mesg))
    {
#ifdef __RLI_DEBUG_SOCK__
        printf("SOCKIF_accept: SOCKIF_RecvQueue() failed.\n");		
#endif
        status = SYS_ERROR_SOCKET_ACCEPT;

        goto cleanup;
    }
#ifdef __DEBUG_SOCK_UPCALL_INTERFACE__
	else { 
		printf("SOCKIF_accept: pend succeeded, ticks %lu.\n", sc_gtime() );		
	}
#endif

    if (NULL == (pTcb = (struct tcb *)Mesg))
    {
#ifdef __RLI_DEBUG_SOCK__
        printf("SOCKIF_accept: NULL notification received.\n");		
#endif
        status = SYS_ERROR_SOCKET_ACCEPT;

        goto cleanup;
    }

	if (pTcb == ((sockIfDescr *)soc)->pTcb)
	{
#ifdef __RLI_DEBUG_SOCK__
        printf("SOCKIF_accept: Bad TCB received.\n");
#endif

        status = SYS_ERROR_SOCKET_ACCEPT;

        goto cleanup;
	}

#ifdef __RLI_DEBUG_SOCK__
    printf("SOCKIF_accept: Notification received (%x).\n", Mesg);		
#endif

    /* enable flow on the socket */
    if (NULL == (p_acceptSock = tcp_get_cookie(pTcb)))
	{
#ifdef __RLI_DEBUG_SOCK__
        printf("SOCKIF_accept: Bad cookie received.\n");
#endif

        status = SYS_ERROR_SOCKET_ACCEPT;

        goto cleanup;
	}

    p_acceptSock->boolSockActive = TRUE;

cleanup:

    if (OK != status)
    {
#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
        printf("SOCKIF_accept: accept failed!\n");		
#endif

        if (NULL != p_acceptSock)
		{
		    tcp_set_cookie(p_acceptSock->pTcb, NULL);
            destruct_sidObject(&p_acceptSock);
		}

        p_acceptSock = NULL;
    }

    return p_acceptSock;

} /* SOCKIF_accept */



/*-----------------------------------------------------------------------*/

extern void SOCKIF_close(OS_SPECIFIC_SOCKET_HANDLE soc)
{
    sockIfDescr* p_sidObject = soc;
	long         err;

	tcp_set_cookie(p_sidObject->pTcb, NULL);

	/* reset the queue. JAB */
	do
	{
		sc_qaccept(RLI_GET_QUEUE(soc), &err);

		if (ER_QID == err)
		{
#ifdef __DEBUG_SOCK_EMULATION_INTERFACE__
	        printf("SOCKIF_close: bad Qid.\n");		
#endif
			break;
		}

	} while (ER_NMP != err);

    destruct_sidObject(&p_sidObject);
}

#endif /* __SOCK_ATTACHE_TCPIP_STACK__ */
