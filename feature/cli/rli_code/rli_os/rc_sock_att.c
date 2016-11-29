/*
 *  rc_sock_att.c (socket interface for Attache Plus6 v4.0)
 *
 *  will only work with server-side tcp/ip (i.e. RCW only, no SMTP, nor UDP). JAB
 *
 *  This is a part of the RapidControl SDK source code library.
 *
 *  Copyright (C) 1999 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */
/*

$History: rc_sock_att.c $
 * 
 * *****************  Version 6  *****************
 * User: Pstuart      Date: 1/05/01    Time: 1:26p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * added missing param
 * 
 * *****************  Version 5  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 4  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/
#include "rc_options.h"

#ifdef  __SOCK_ATTACHE_TCPIP_STACK__

#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_errors.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_msghdlr.h"
#include "rc_socks.h"
#include "rc_convert.h"
#include "rc_database.h"

#include <errno.h>
#if __OLDER_ATTACHE_STACK__
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

#ifdef __OCP_ENABLED__
#include "rca_ocp.h"
#include "rca_ocpd.h"
#endif  /* __OCP_ENABLED__ */

#ifdef __COMMON_LISTENER__
#include "rc_tcpd.h"
#endif

#include "rc_access.h"
#include "rc_sockif.h"

#if 0
#define __RLI_DEBUG_SOCK__
#endif

#ifdef __RLI_DEBUG_SOCK__
#include <stdio.h>
#endif

#define kANY_PORT       0

#ifndef kUdpSendPort
#define kUdpSendPort    2010
#endif

ProcessComChannel g_pccObjects[kHTTPD_QUEUE_SIZE];

/*!-!-!-!-! when we make this code for general purpose usage we should
  consider using a type 'int' for our socket handles, rather 
  than a 'void *'. JAB */



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Initialize(void)
{
    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_OpenServer(OS_SPECIFIC_SOCKET_HANDLE *pSoc, ubyte4 port)
{
    char*        pMsg;

    /* Create a incoming Client connect request queue */
    if (NULL == (*pSoc = SOCKIF_listen(port)))
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_LISTEN, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        return ERROR_GENERAL_ACCESS_DENIED;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Accept(OS_SPECIFIC_SOCKET_HANDLE *soc,
                              OS_SPECIFIC_SOCKET_HANDLE *accSoc, ubyte4 port)
{
    /* this will probably just return a cloned TCB handle */
    char*               pMsg;

    *accSoc = SOCKIF_accept(*soc);

    if (NULL == *accSoc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_ACCEPT, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        return SYS_ERROR_SOCKET_ACCEPT;
    }

    return OK;
}


/*-----------------------------------------------------------------------*/

extern ubyte4 SOCKET_GetClientsAddr(OS_SPECIFIC_SOCKET_HANDLE sock)
{
	if (NULL == sock)
		return 0;

#if __OLDER_ATTACHE_STACK__
    return tcp_get_remote_address(RLI_GET_TCB(sock));
#else
    return tcp_get_remote_ipaddr(RLI_GET_TCB(sock));
#endif
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Close(OS_SPECIFIC_SOCKET_HANDLE sock)
{
	if (NULL == sock)
		return OK;

    tcp_close(RLI_GET_TCB(sock));

    SOCKIF_close(sock);

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS ATTACHE_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufSize)
{
    struct		sockIfDescr *p_sidObject = sock;
    char*		pReceiveBuffer = p_sidObject->pReceiveBuffer;
    Counter		totalBytesRead, bytesToRead;
    sbyte4		Head, Tail;
    long		Mesg;
    bits16_t	octetsAvail;

    /* circular buffer: the consumer... */

    /* to keep the code atomic, this function can only modify the head index. */
    /* similiarly, SOCKIF_receive_upcall() may only modify the tail. JAB */

    if (TRUE != p_sidObject->boolSockActive)
    {
#ifdef __RLI_DEBUG_SOCK__
        printf("ATTACHE_SocketRead: Trying to read from a closed socket.\n");		
#endif
        return SYS_ERROR_SOCKET_GENERAL;
    }

    if (0 == BufSize)
		return 0;

	totalBytesRead = 0;

    do
    {
        Head = p_sidObject->ReceiveHeadIndex;
        Tail = p_sidObject->ReceiveTailIndex;

        if (Tail == Head)
        {
            /* no data in buffer, wait. */
            p_sidObject->signalMePlease = TRUE;

#ifdef __RLI_DEBUG_SOCK__
            printf("ATTACHE_SocketRead: No data available --- waiting for notification.\n");
#endif
			Mesg = -1;

            /* no data available, must wait for SOCKIF_transmit_upcall() to signal. JAB */
            SOCKIF_RecvQueue(RLI_GET_QUEUE(sock), kReadTimeoutInTicks, &Mesg);

            /*!-!-!-!-! need to check the message to see, if it's a close. JAB */
            /* check message to see if an error occurred. JAB */

#ifdef __RLI_DEBUG_SOCK__
            printf("ATTACHE_SocketRead: Notification received (%d).\n", Mesg);		
#endif

            if (kSOCKET_READ_DATA != Mesg)
            {
#ifdef __RLI_DEBUG_SOCK__
                printf("ATTACHE_SocketRead: unexpected notification.\n");		
#endif
                return SYS_ERROR_SOCKET_GENERAL;
            }

            continue;
        }
        else if (Tail > Head)
        {
            /* the simple case... */
            bytesToRead = ((Tail - Head) > (BufSize - totalBytesRead)) ? (BufSize - totalBytesRead) : (Tail - Head);

            MEMCPY(&pBuf[totalBytesRead], &pReceiveBuffer[Head], bytesToRead);
            Head += bytesToRead;

            totalBytesRead += bytesToRead;
        }
        else
        {
            /* move the upper chunk... */
            bytesToRead = ((p_sidObject->receiveBufSize - Head) > (BufSize - totalBytesRead)) 
                              ? (BufSize - totalBytesRead) : (p_sidObject->receiveBufSize - Head);

            MEMCPY(&pBuf[totalBytesRead], &pReceiveBuffer[Head], bytesToRead);
            totalBytesRead += bytesToRead;

            /* adjust circular buffer head index. we may(/not) wrap around. JAB */
            Head = (Head + bytesToRead) & (p_sidObject->receiveBufSize - 1);
        }

        /* store back the new Head */
        p_sidObject->ReceiveHeadIndex = Head;

		/* open the receive window wider. JAB */
		Tail = p_sidObject->ReceiveTailIndex;

		if (Head > Tail)
			octetsAvail = (Head - (Tail + 1));
		else
			octetsAvail = ((p_sidObject->receiveBufSize - 1) - (Tail - Head));

	    tcp_set_receive_window(p_sidObject->pTcb, octetsAvail);

    } while (totalBytesRead < BufSize);

    return totalBytesRead;

} /* ATTACHE_SocketRead */



/*-----------------------------------------------------------------------*/

extern RLSTATUS ATTACHE_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, Counter BufLen)
{
    sbyte4  bytesToWrite = 0;
	sbyte4	totalBytesWritten = 0;
    sbyte4  remainingBuf = ((sockIfDescr *)sock)->windowRemainderSize;
    packet* pPacket  = NULL;
    long    Mesg;

    if (TRUE != ((sockIfDescr *)sock)->boolSockActive)
    {
#ifdef __RLI_DEBUG_SOCK__
        printf("ATTACHE_SocketWrite: Trying to write to a closed socket.\n");		
#endif

        return SYS_ERROR_SOCKET_GENERAL;
    }

    if (0 == BufLen)
        return OK;

    do
    {
        /* calculate buffer size needed. JAB */
        bytesToWrite = (remainingBuf < (BufLen - totalBytesWritten)) ? remainingBuf : (BufLen - totalBytesWritten);

#ifdef __RLI_DEBUG_SOCK__
		if (BufLen > 100)
			printf("ATTACHE_SocketWrite: BufLen = %d, totalBytesWritten = %d, bytesToWrite = %d.\n", BufLen, totalBytesWritten, bytesToWrite);		
#endif

        /* allocate an outbound packet */
        if (NULL == (pPacket = tcp_alloc(bytesToWrite)))
        {
#ifdef __RLI_DEBUG_SOCK__
            printf("ATTACHE_SocketWrite: Out of memory.\n");		
#endif

            return ERROR_GENERAL_ACCESS_DENIED;
        }

        /* copy the data */
        MEMCPY(pPacket->pkt_data, &pBuf[totalBytesWritten], bytesToWrite);

        /* write the data */
        remainingBuf = tcp_write(RLI_GET_TCB(sock), pPacket);
        ((sockIfDescr *)sock)->windowRemainderSize = remainingBuf;

        /* update counters */
        totalBytesWritten += bytesToWrite;

        /* check to see if we have filled up the transmit window. JAB */
        if (0 == remainingBuf)
        {
#ifdef __RLI_DEBUG_SOCK__
            printf("ATTACHE_SocketWrite: Buffer full --- waiting for notification.\n");		
#endif
			Mesg = -1;

            /* buffer full, wait. */
            ((sockIfDescr *)sock)->signalMePlease = TRUE;

            /* no more space available, must wait for SOCKIF_transmit_upcall() to signal. JAB */
            SOCKIF_RecvQueue(RLI_GET_QUEUE(sock), kWriteTimeoutInTicks, &Mesg);

            /*!-!-!-!-! need to check the message to see, if it's a close. JAB */

#ifdef __RLI_DEBUG_SOCK__
            printf("ATTACHE_SocketWrite: Notification received (%d).\n", Mesg);		
#endif

            if (kSOCKET_SEND_DATA != Mesg)
            {
#ifdef __RLI_DEBUG_SOCK__
                printf("ATTACHE_SocketWrite: Unexpected notification received.\n");		
#endif
                return SYS_ERROR_SOCKET_GENERAL;
            }

            remainingBuf = ((sockIfDescr *)sock)->windowRemainderSize;
        }

    } while (totalBytesWritten < BufLen);

#ifdef __RLI_DEBUG_SOCK__
	if (BufLen > 100)
	{
		printf("ATTACHE_SocketWrite: BufLen = %d, totalBytesWritten = %d, bytesToWrite = %d.\n", BufLen, totalBytesWritten, bytesToWrite);		
	}
#endif

    return OK;

} /* ATTACHE_SocketWrite */



/*-----------------------------------------------------------------------*/

extern Boolean ATTACHE_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte4 timeout)
{
    struct  sockIfDescr *p_sidObject = sock;

	if (NULL == sock)
		return SYS_ERROR_SOCKET_GENERAL;

    return (p_sidObject->ReceiveHeadIndex != p_sidObject->ReceiveTailIndex) ? TRUE : FALSE;
}

#endif /* __SOCK_ATTACHE_TCPIP_STACK__ */
