/*  
 *  rc_sockif.h
 *
 *  This is a part of the RapidControl SDK source code library. 
 *
 *  Copyright (C) 1999 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */




/*

$History: rc_sockif.h $
 * 
 * *****************  Version 4  *****************
 * User: Pstuart      Date: 12/11/00   Time: 11:30a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added c++ wrappers for prototypes
 * 
 * *****************  Version 3  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Include history and enable auto archiving feature from VSS

$/Rapid Logic/Code Line/rli_code/include/rc_sockif.h

2    Schew       3/22/00   7:32p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Pstuart     3/22/00  10:53a   Labeled 'PreCodeRename'                                
     David       3/07/00   8:06p   Labeled 'RC 3.01 Final Release'                        
     Builder     3/02/00   5:18p   Labeled 'RC 301 Build20000302'                         
     Builder     2/29/00   6:48p   Labeled 'Build301 20000229'                            
     Builder     2/28/00   6:35p   Labeled 'RC301 Build20000228'                          
     Builder     2/24/00   4:01p   Labeled 'RC301 Build20000224'                          
     David      12/20/99  10:55a   Labeled 'RC 3.0 Final Release'                         
     Builder    12/15/99   6:36p   Labeled 'RC30 Build19991215'                           
     Builder    12/14/99   7:08p   Labeled 'RC30 Build19991214'                           
     Builder    12/10/99   5:55p   Labeled 'RC30 Build19991210'                           
     Builder    11/17/99   4:32p   Labeled 'RC30 Build19991117 Release'                   
     Builder    11/16/99   5:37p   Labeled 'RC30 Build19991116'                           
     Builder    11/15/99   5:59p   Labeled 'RC30 Build19991115'                           
     Builder    11/12/99   5:27p   Labeled 'RC30 Build19991112'                           
     Builder    11/11/99   7:00p   Labeled 'RC30 Build19991111'                           
     Builder    11/10/99   7:12p   Labeled 'RC30 Build19991110'                           
     Builder    11/08/99   6:09p   Labeled 'RC30 Build19991108'                           
     Builder    11/05/99   6:33p   Labeled 'RC30 Build19991105'                           
     Builder    11/04/99   5:04p   Labeled 'RC30 Build19991104'                           
     Builder    11/03/99   6:23p   Labeled 'RC3.0 Build 19991103'                         
     Builder    11/01/99   5:51p   Labeled 'RC 3.0 Build 19991101'                        
     Builder    10/29/99   5:30p   Labeled 'RC30 Build 19991029'                          
1    Jab        10/12/99   5:29p   Created sockif.h                                       




  */



#ifndef __SOCKIF_HEADER__
#define __SOCKIF_HEADER__

/*-----------------------------------------------------------------------*/


/*!-!-!-!-!-!-! kMaxReceiveBufSize MUST be 2^X in size. JAB */
#define kMaxReceiveBufSize      512
#define kMaxTransmitBufSize     1024

#define kReadTimeoutInTicks		1000
#define kWriteTimeoutInTicks	1000

#define RLI_GET_TCB(x)      ((sockIfDescr *)(x))->pTcb
#define RLI_GET_QUEUE(x)    ((sockIfDescr *)(x))->sync_queue



/*-----------------------------------------------------------------------*/

/* abstraction layer for future ports... JAB */
#define OS_SPECIFIC_CREATE_QUEUE        SOCKIF_CreateQueue
#define OS_SPECIFIC_RELEASE_QUEUE       SOCKIF_ReleaseQueue
#define OS_SPECIFIC_SEND_QUEUE          SOCKIF_SendQueue
#define OS_SPECIFIC_RECV_QUEUE          SOCKIF_RecvQueue

typedef int    OS_SPECIFIC_QUEUE;
typedef long   OS_SPECIFIC_TIMER;



/*-----------------------------------------------------------------------*/

/* possible messages for the queue */
enum sockifStateChangeMessage { kSOCKET_QUEUED = 1, kSOCKET_READ_DATA, kSOCKET_SEND_DATA, kSOCKET_ERROR };



/*-----------------------------------------------------------------------*/

typedef struct sockIfDescr
{
    Boolean                 boolSockActive;     /* has the socket been activated? */

    struct tcb*             pTcb;               /* pointer back to parent */
    OS_SPECIFIC_QUEUE       sync_queue;         /* synchronize the upcalls with the main thread */

    sbyte4                  ReceiveHeadIndex;   /* head index into the rx circular buffer */
    sbyte4                  ReceiveTailIndex;   /* tail index into the rx circular buffer */

    sbyte*                  pReceiveBuffer;     /* pointer to a circular buffer */
    sbyte4                  receiveBufSize;     /* the size of the circular buffer */

    sbyte4                  windowRemainderSize;
    Boolean                 signalMePlease;

} sockIfDescr;                                  /* basename: sid */



/*-----------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* IPC stuff */
extern RLSTATUS SOCKIF_Init(void);
extern RLSTATUS SOCKIF_CreateQueue  (OS_SPECIFIC_QUEUE *pQid);
extern RLSTATUS SOCKIF_ReleaseQueue (OS_SPECIFIC_QUEUE *pQid);
extern RLSTATUS SOCKIF_SendQueue    (OS_SPECIFIC_QUEUE qid, long Mesg);
extern RLSTATUS SOCKIF_RecvQueue    (OS_SPECIFIC_QUEUE qid, OS_SPECIFIC_TIMER timeout, long *pRetMesg);

/* standard socket interface methods */
extern OS_SPECIFIC_SOCKET_HANDLE SOCKIF_listen(ubyte2 localport);
extern OS_SPECIFIC_SOCKET_HANDLE SOCKIF_accept(OS_SPECIFIC_SOCKET_HANDLE soc);
extern void                      SOCKIF_close (OS_SPECIFIC_SOCKET_HANDLE soc);

#ifdef __cplusplus
}
#endif

#endif /* __SOCKIF_HEADER__ */
