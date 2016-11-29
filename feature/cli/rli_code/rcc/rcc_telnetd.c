/*  
 *  rcc_telnetd.c
 *
 *  This is a part of the RapidControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
 *  All rights reserved.
 *
 *  RapidControl, RapidControl for Web, RapidControl Backplane,
 *  RapidControl for Applets, MIBway, RapidControl Protocol, and
 *  RapidMark are trademarks of Rapid Logic, Inc.  All rights reserved.
 *
 */

#include "rc.h"

#ifdef __RCC_ENABLED__

#include "rcc.h"

#ifdef __POSIX_OS__
#define __USE_MISC
# include <termios.h>
# ifdef __LINUX__
#  include <unistd.h>
# endif
#endif /* __POSIX_OS__ */

#ifdef __VXWORKS_OS__
# include <ioLib.h>
# ifdef __FILTER_IO__
#   include "eolDriver.h"
# endif
#endif /* __VXWORKS_OS__ */

#ifdef __FreeRTOS_OS__
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* LwIP include */
#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"

#include "console.h"
#include "mconfig.h"
#include "cli_util.h"
#include "halcmd_msg.h"
#endif

#ifdef AUX_TELNETD_FN
void AUX_TELNETD_FN(void);
#endif

#ifdef RCC_LOG_HEADER
# include RCC_LOG_HEADER
#endif

cli_env    **gppCliSessions;
CliChannel gTelnetObject[kRCC_MAX_CLI_TASK];
CliChannel mConsoleSession;
Boolean    gMainLoop    = TRUE;
Boolean    gSleepLoop   = TRUE;

#ifdef __RCC_DEBUG__
FILE *telnetDebug;
#endif

/* write a message to the standard console */

extern void RCC_TELNETD_ConsoleWrite(sbyte * pMsg)
{
    sbyte4   index;
    cli_env *pCliEnv;

    if (NULL == pMsg)
        return;

    for (index = 0; index < kRCC_MAX_CLI_TASK+1; index++)
    {
        if (NULL == (pCliEnv = (gppCliSessions[index])))
            continue;

        if (&mConsoleSession == MMISC_GetChannel(pCliEnv))
            break;

        pCliEnv = NULL;
    }

    if (NULL != pCliEnv)
        RCC_EXT_WriteStrLine(pCliEnv, pMsg);    
}

/*-----------------------------------------------------------------------*/

extern void RCC_TELNETD_AddSession(cli_env *pCliEnv)
{
    sbyte4 index;

    if (NULL == gppCliSessions)
        return;

    for (index = 0; index < kRCC_MAX_CLI_TASK+1; index++)
    {
        if (NULL != gppCliSessions[index])
            continue;

        gppCliSessions[index] = pCliEnv;
        break;
    }
}

/*-----------------------------------------------------------------------*/

extern void RCC_TELNETD_DelSession(cli_env *pCliEnv)
{
    sbyte4      index;
    CliChannel *pChannel;
        
    if (NULL == pCliEnv)
        return;

    if (NULL == gppCliSessions)
        return;

    pChannel = MMISC_GetChannel(pCliEnv);

    if (NULL != pChannel)
    {
        pChannel->ThreadState = kThreadDead;
        pChannel->InUse       = FALSE;  
    }

    for (index = 0; index < kRCC_MAX_CLI_TASK+1; index++)
    {
        if (pCliEnv != gppCliSessions[index])
            continue;

        gppCliSessions[index] = NULL;
        break;
    }
}

/*-----------------------------------------------------------------------*/

extern cli_env * RCC_TELNETD_GetSession(sbyte4 index)
{
    if (NULL == gppCliSessions)
        return NULL;

    if (index >= kRCC_MAX_CLI_TASK+1)
        return NULL;

    return gppCliSessions[index];
}

/*-----------------------------------------------------------------------*/

RL_STATIC CliChannel *TELNETD_GetCliChannels()
{
    return &gTelnetObject[0];
}

/*-----------------------------------------------------------------------*/

#ifdef __RCC_CONSOLE_ENABLED__
RL_STATIC CliChannel *TELNETD_GetConsoleChannel()
{
    return &mConsoleSession;
}
#endif

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS TELNETD_InitChildren()
{
    Counter     idx;
    CliChannel *pRccTaskObj = TELNETD_GetCliChannels(); 

    for (idx=0; idx < kRCC_MAX_CLI_TASK; idx++)
    {
        pRccTaskObj[idx].ThreadState = kThreadDead;
        pRccTaskObj[idx].InUse       = FALSE;
        pRccTaskObj[idx].index       = idx + 1;

#ifdef __NUCLEUS_OS__
        if (NULL == (pRccTaskObj[idx].stackAddress = RC_MALLOC(kRCC_THREAD_STACK_SIZE)))
        {
            Counter idx1;

            for (idx1 = 0; idx1 < idx; idx1++)
                FREEMEM(pRccTaskObj[idx1].stackAddress);

            return RCC_ERROR_THROW(SYS_ERROR_NO_MEMORY);
        }

        pRccTaskObj[idx].stackLen = kRCC_THREAD_STACK_SIZE;
#endif
    }
    return OK;
}

/*-----------------------------------------------------------------------*/

/* supercedes RCC_TELNETD_BroadcastMessage */
extern RLSTATUS 
RCC_TELNETD_Broadcast(cli_env *pCliEnv, sbyte *pMessage)
{
    EditType    cursorPos;
    EditType    length;
    sbyte4      index;
    cli_env    *pCliDest;
    sbyte      *pFrom = NULL;

    if (NULL != pCliEnv)
        pFrom = MMISC_GetLogin(pCliEnv);

    for (index = 0; index < kRCC_MAX_CLI_TASK+1; index++)
    {
        if (NULL == (pCliDest = RCC_TELNETD_GetSession(index)))
            continue;

        if (NULL == MMISC_GetChannel(pCliDest))
            continue;

        if (kThreadWorking != MMISC_GetThreadState(pCliDest))
            continue;

        cursorPos = MEDIT_GetCursor(pCliDest);
        length    = MEDIT_GetLength(pCliDest);

        if ((0 < length) && (pCliDest != pCliEnv))
            RCC_EXT_PutStrLine(pCliDest, "");

        /* doesn't update line count -- avoids triggering "more" */
#ifdef kRCC_MSG_BROADCAST
        if (NULL != pFrom)
        {
            if (pCliDest != pCliEnv)
                RCC_EXT_PutStrLine(pCliDest, "");
            RCC_EXT_PutStr(pCliDest, kRCC_MSG_BROADCAST);
            RCC_EXT_PutStrLine(pCliDest, pFrom);
            RCC_EXT_PutStrLine(pCliDest, "");
        }
#endif
        RCC_EXT_PutStrLine(pCliDest, pMessage);
    }

    return OK;
}


/*-----------------------------------------------------------------------*/

/* legacy interface -- use RCC_TELNETD_Broadcast */
extern RLSTATUS RCC_TELNETD_BroadcastMessage(sbyte *pMessage, Access authLevel)
{
    RCC_TELNETD_Broadcast(NULL, pMessage);

    return STATUS_RCC_NO_PROMPT;
}

/*-----------------------------------------------------------------------*/

#ifdef __RCC_CONSOLE_ENABLED__

RL_STATIC RLSTATUS 
TELNETD_ConsoleRead(cli_env *pEnv, cliChar *charRead)
{
    ubyte   charIn;
    ubyte   halcmdflag = 0;
    StHalCmdMsg HalCmdMsg; 
    
    HalCmdMsg.len = 0;

#ifndef __FreeRTOS_OS__

#ifdef __ALLOW_CONSOLE_TIMEOUT__
    ubyte4  timeDiff;
    ubyte4  timeOut;
    ubyte4  time;
    ubyte4  last;
#endif /* __ALLOW_CONSOLE_TIMEOUT__ */

    charIn = GETCHAR();

#ifdef __ALLOW_CONSOLE_TIMEOUT__
    timeOut  = MCONN_GetTimeOut(pEnv);
    time     = OS_SPECIFIC_GET_SECS();
    last     = MCONN_GetInputTime(pEnv);
    timeDiff = OS_SPECIFIC_GET_SECS() - MCONN_GetInputTime(pEnv);
    if ((timeOut / 2) < timeDiff)
        return RCC_ERROR_THROW(ERROR_RCC_TIMEOUT);

    MCONN_SetInputTime(pEnv, OS_SPECIFIC_GET_SECS());
#endif /* __ALLOW_CONSOLE_TIMEOUT__ */

    if (kRCC_DOSKEY_ESC == charIn)
    {
        charIn = GETCHAR();

        switch(charIn)
        {
        case kRCC_DOSKEY_UP:
            charIn = kKEY_MOVE_UP;
            break;
        case kRCC_DOSKEY_DN:  
            charIn = kKEY_MOVE_DOWN;
            break;
        case kRCC_DOSKEY_LT:  
            charIn = kKEY_MOVE_LEFT;
            break;
        case kRCC_DOSKEY_RT:
            charIn = kKEY_MOVE_RIGHT;
            break;
        case kRCC_DOSKEY_DEL:
            charIn = kKEY_DELETE_CHAR;
            break;
        default:
            DEBUG_MSG_1("Keypressed: %x\n", charIn);
            break;
        }
    }

#else
	extern xQueueHandle xRxedChars;

READ_CONSOLE:
	while(1) {
        if(NULL == xRxedChars)
        { 
            printf("error: the variables xRxedChars is not initialized delete tconsole task\r\n");
            vTaskDelete( NULL );
        }
		if(xQueueReceive(xRxedChars, &charIn, 500) == pdTRUE) 
        {
#if BOARD_GV3S_HONUE_QM
            if(HalCmdMsg.len >= HAL_CMD_MSG_LEN)
            {
                printf("error: bigger than %d Byte \r\n",HAL_CMD_MSG_LEN);
                halcmdflag = 0;
                HalCmdMsg.len = 0;
                continue;
            }
                
            if(charIn == 0xf0)
            {
                halcmdflag = 1;
                HalCmdMsg.len = 0;
                HalCmdMsg.buff[HalCmdMsg.len++] = charIn;
                continue;
            }
                 
            if(halcmdflag == 1)
            {
                HalCmdMsg.buff[HalCmdMsg.len++] = charIn; 
                if(charIn == 0xff)
                {
				    HalQueueWrite(&HalCmdMsg, 10);
                    HalCmdMsg.len = 0;
                    halcmdflag = 0;
                    continue;
                }
                continue;
            }
#endif /* BOARD_GV3S_HONUE_QM */
            break;
        }else{
            halcmdflag = 0;
            HalCmdMsg.len = 0;
            continue;
        }
	}
	#if 0
    if (kRCC_DOSKEY_ESC == charIn)
    {
		if(xQueueReceive(xRxedChars, &charIn, 10) == pdTRUE)
		{
	        switch(charIn)
	        {
	        case kRCC_DOSKEY_UP:
	            charIn = kKEY_MOVE_UP;
	            break;
	        case kRCC_DOSKEY_DN:  
	            charIn = kKEY_MOVE_DOWN;
	            break;
	        case kRCC_DOSKEY_LT:  
	            charIn = kKEY_MOVE_LEFT;
	            break;
	        case kRCC_DOSKEY_RT:
	            charIn = kKEY_MOVE_RIGHT;
	            break;
	        case kRCC_DOSKEY_DEL:
	            charIn = kKEY_DELETE_CHAR;
	            break;
	        default:
	            DEBUG_MSG_1("Keypressed: %x\n", charIn);
				goto READ_CONSOLE;
	        }
		}
		else
		{
			goto READ_CONSOLE;
		}
    }
	#endif
#endif

/* unix uses lf as end of line */
#ifndef __WIN32_OS__
    if (kLF == charIn)
        charIn = kCR;
#endif

    *charRead = charIn;

    return OK;
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS 
TELNETD_ConsoleWrite(cli_env *pEnv, sbyte *pBuf, sbyte4 BufSize)
{
#ifdef __FreeRTOS_OS__
	sbyte4 i;
	sbyte *pBuffer=pBuf;
		
	for(i=0; i<BufSize; i++, pBuffer++)
		ConsolePutChar(*pBuffer);
#else
    fwrite(pBuf, 1, BufSize, stdout);
    fflush(stdout);
#endif	
    return OK;
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS TELNETD_ConsoleInit(cli_env *pCliEnv)
{
#ifdef __POSIX_OS__
    struct termios TermIO;

    tcgetattr ( STDIN,  &TermIO );

    DEBUG_MSG_1("I/O flag initially: %x\n", TermIO.c_lflag);

    TermIO.c_lflag &= ~ICANON;
    TermIO.c_lflag &= ~ECHO;
    TermIO.c_oflag &= ~TABDLY;
    TermIO.c_oflag |=  TAB0;
    tcsetattr ( STDIN, TCSANOW, &TermIO );

    DEBUG_MSG_1("I/O flag is now: %x\n", TermIO.c_lflag);

#endif /* __POSIX_OS__ */

#ifdef __VXWORKS_OS__
    ioctl(STDIN, FIOSETOPTIONS, OPT_RAW);
#endif

#ifdef __WIN32_OS__
    CONSOLE_SCREEN_BUFFER_INFO screenBuffer;
    HANDLE                     console;

    console = GetStdHandle(STD_OUTPUT_HANDLE);
        
    if (INVALID_HANDLE_VALUE == console)
        return OK;

    if (0 == GetConsoleScreenBufferInfo(console, &screenBuffer))
        return OK;

    if (NULL == pCliEnv)
        return RCC_ERROR_THROW(ERROR_GENERAL);

    RCC_EXT_SetWidth(pCliEnv,  (EditType) screenBuffer.dwSize.X);
    RCC_EXT_SetHeight(pCliEnv, (EditType) screenBuffer.dwSize.Y);
#endif

    return OK;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void TELNETD_ConsoleSession()
{
    while (gMainLoop)
    {
        RCC_TELNETD_CreateSession(&mConsoleSession, TELNETD_ConsoleInit,
                                   TELNETD_ConsoleRead, TELNETD_ConsoleWrite, 
                                   kRCC_CONN_CONSOLE, kRCC_CONSOLE_NAME);
    }

}


#endif /* __RCC_CONSOLE_ENABLED__ */

/*-----------------------------------------------------------------------*/

extern CliChannel * 
RCC_TELNETD_NewChannel(OS_SPECIFIC_SOCKET_HANDLE accSoc)
{
    Counter     numConn       = 0;
    sbyte4      findConn      = 0;
    CliChannel *telnetSession = NULL;
    CliChannel *telnetObjects;

#ifdef __MULTI_THREADED_SERVER_ENABLED__
    /* look for a free telnet thread descriptor */
    while (NULL == telnetSession)
    {
#endif
        numConn  = 0;
        findConn = (findConn + 1) % kRCC_MAX_CLI_TASK;
        telnetObjects = TELNETD_GetCliChannels();

        while (numConn < kRCC_MAX_CLI_TASK)
        {
            if (FALSE == telnetObjects[findConn].InUse)
            {
                telnetSession = &(telnetObjects[findConn]);
                break;
            }
            numConn++;
            findConn = (findConn + 1) % kRCC_MAX_CLI_TASK;
        }
            
#ifdef __MULTI_THREADED_SERVER_ENABLED__
        /* give up -- nothing available */
        if (NULL == telnetSession)
            return NULL;

    } /* end while !telnetSession */

#endif /* __MULTI_THREADED_SERVER_ENABLED__ */

    /* initialize telnet descriptor for child thread  */
    telnetSession->ThreadState     = kThreadCreate;
    telnetSession->sock            = accSoc;
    telnetSession->InUse           = TRUE;

    return telnetSession;
}

/*-----------------------------------------------------------------------*/

#ifdef __RCC_CONSOLE_ENABLED__
RL_STATIC void TELNETD_ConsoleClose()
{
    CliChannel *theConsole = TELNETD_GetConsoleChannel();

    theConsole->ThreadState = kThreadDead;
    theConsole->InUse       = FALSE;  
}
#endif

/*-----------------------------------------------------------------------*/

/*
 * This is the main routine that starts both the shell and the telnet deamon.  
 */
xTaskHandle xTelnetHandle;

extern void RCC_TELNETD()
{
#ifndef AUX_TELNETD_FN
    CliChannel                *pTelnetSession;
    OS_SPECIFIC_SOCKET_HANDLE  soc;
    OS_SPECIFIC_SOCKET_HANDLE  accSoc;
#endif

    gppCliSessions = RC_CALLOC(sizeof(gppCliSessions) * (kRCC_MAX_CLI_TASK+1), 1);
    if (NULL == gppCliSessions)
        return;

#ifdef __FILTER_IO__
	eolInit();
#endif

    RCC_DB_InitTasks();

#ifdef __RCC_CONSOLE_ENABLED__
    /* start a console thread */
    if (OK != OS_SPECIFIC_CREATE_THREAD(
            (void *)  CONSOLE_SESSION_FN, 
            (ubyte *) "tConsole", 
            NULL, 
            0, 
            0, 
            NULL))
    {
        TELNETD_ConsoleClose();
    }
#endif /* __RCC_CONSOLE_ENABLED__ */


#ifdef AUX_TELNETD_FN
    AUX_TELNETD_FN();
#else

    /* initialization of thread structure for telnet sessions*/
    if ( OK != TELNETD_InitChildren() )
    {
        RC_FREE(gppCliSessions);
        return;
    }

    /* open a socket to listen to the telnet requests */
    if ( OK != SOCKET_OpenServer(&soc, kRCC_FIXED_PORT) )
    {
        RC_FREE(gppCliSessions);
        return;
    }

    while (gMainLoop) 
    {
        /* accept a telnet connection */
        if (OK != SOCKET_Accept(&soc, &accSoc, kRCC_FIXED_PORT) )
            continue;
		
		{
			CliChannel *telnetSession;
			cli_env *pCliEnv = gppCliSessions[1];
			int optval = 1, KeepIdle = 1;
			
			lwip_setsockopt(accSoc, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
			//lwip_setsockopt(accSoc, IPPROTO_TCP, TCP_KEEPIDLE, &KeepIdle, sizeof(KeepIdle));
			//lwip_setsockopt(accSoc, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
			telnetSession = TELNETD_GetCliChannels();
			if(telnetSession->InUse == TRUE) {
				if(pCliEnv != NULL) {
			    	SOCKET_Close(telnetSession->sock);				
				}
			}
        }
		
        if (NULL == (pTelnetSession = RCC_TELNETD_NewChannel(accSoc)))
        {
            OS_SPECIFIC_SOCKET_WRITE(accSoc, kRCC_MSG_FAIL, 
                                     STRLEN(kRCC_MSG_FAIL));
            SOCKET_Close(accSoc);
            continue;
        }

		debug_module_clear();
		
#ifdef __MULTI_THREADED_SERVER_ENABLED__
        /* start a thread for the telnet session */
        if (OK > OS_SPECIFIC_CREATE_THREAD(
                (void  *)  RCC_TELNET_StartSession, 
                (ubyte *) "tTelnetd", 
                (void  *)  pTelnetSession, 
                0, 
                0, 
                &xTelnetHandle))
        {
            /* cleanup if thread can not be started */
            SOCKET_Close(accSoc);
            continue;
        }
#else /* ! __MULTI_THREADED_SERVER_ENABLED__ */
        /* start a telnet session if single threaded */
        RCC_TELNET_StartSession(pTelnetSession);

#endif /* __MULTI_THREADED_SERVER_ENABLED__ */

    }  /* end while(gMainLoop) */

    SOCKET_Close(soc);

#endif /* __USE_OTHER_TELNETD__ */

    RCC_DB_FreeTasks();

    RC_FREE(gppCliSessions);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS
RCC_TELNETD_SessionCreate(CliChannel *pChannel, SessionInit *pSessionInit,
                         ReadHandle *pReadHandle, WriteHandle *pWriteHandle,
                         ubyte4 connectionType, sbyte *pName, cli_env ** ppCliEnv)
{
    RLSTATUS  status;
    cli_env * pCliEnv = NULL;
    Boolean   kill    = FALSE;
    
    if (NULL == ppCliEnv)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    if (NULL != pChannel)
    {
        pChannel->ThreadState = kThreadCreate;
        pChannel->InUse       = TRUE;
    }

    *ppCliEnv = NULL;
    status    = RCC_DB_EnvironmentCreate(ppCliEnv, pChannel, NULL, kOUTPUT_BUFFER_SIZE);
    pCliEnv   = *ppCliEnv;

    if (OK != status)
    {
        if (NULL != pCliEnv)
            pWriteHandle(pCliEnv, kRCC_MSG_FAIL, STRLEN(kRCC_MSG_FAIL));

        return status;
    }

    if (OK != (status = RCC_DB_EnvironmentReset(pCliEnv)))
    {
        RCC_TASK_Cleanup(pCliEnv);
        return status;
    }


    if (OK != (status = RCC_UTIL_Init(pCliEnv)))
    {
        RCC_TASK_Cleanup(pCliEnv);
        return status;
    }

    if (NULL != pSessionInit)
    {
        if (OK != (status = pSessionInit(pCliEnv)))
        {
            RCC_TASK_Cleanup(pCliEnv);
            return status;
        }
    }

    MCONN_SetInputTime(pCliEnv,   OS_SPECIFIC_GET_SECS());
    MCONN_SetConnType(pCliEnv,    connectionType);
    MCONN_SetWriteHandle(pCliEnv, pWriteHandle);
    MCONN_SetReadHandle(pCliEnv,  pReadHandle);
    MCONN_SetTermName(pCliEnv,    pName);
    MCONN_SetTimeOut(pCliEnv,     kRCC_TIMEOUT);
    MCONN_SetKeyState(pCliEnv,    KEY_STATE_DATA);  

    if (0 >= MSCRN_GetWidth(pCliEnv))
        RCC_EXT_SetWidth(pCliEnv,  kRCC_DEFAULT_WIDTH);

    if (0 >= MSCRN_GetHeight(pCliEnv))
        RCC_EXT_SetHeight(pCliEnv, kRCC_DEFAULT_HEIGHT);

    RCC_TELNETD_AddSession(pCliEnv);

    if (NULL != pChannel)
        pChannel->ThreadState = kThreadWorking;

#ifdef __FILTER_IO__
    if (NULL != pChannel)
        MMISC_SetFD(pCliEnv, vxRedirect(MCONN_GetSock(pCliEnv)));
#endif
    
    return status;
}


extern Boolean
RCC_TELNETD_SessionDestroy(cli_env  *pCliEnv)
{
    CliChannel *pChannel    = MMISC_GetChannel(pCliEnv);
    Boolean     kill        = FALSE;

#ifdef __FILTER_IO__
    if (NULL != pChannel)
        close(MMISC_GetFD(pCliEnv));
#endif

    kill = RCC_IsEnabled(pCliEnv, kRCC_FLAG_KILL);

    RCC_TASK_Cleanup(pCliEnv);

    return kill;
}


/* return TRUE to kill server */
extern Boolean 
RCC_TELNETD_CreateSession(CliChannel *pChannel, SessionInit *pSessionInit,
                         ReadHandle *pReadHandle, WriteHandle *pWriteHandle,
                         ubyte4 connectionType, sbyte *pName)
{
    RLSTATUS  status;
    cli_env  *pCliEnv = NULL;
    Boolean   kill    = FALSE;

    status = RCC_TELNETD_SessionCreate(pChannel, pSessionInit, pReadHandle, 
                                       pWriteHandle, connectionType, pName, 
                                       &pCliEnv);
    if (OK != status) 
        return FALSE;

    RCC_TASK_Readline(pCliEnv);

    kill = RCC_IsEnabled(pCliEnv, kRCC_FLAG_KILL);
	debug_module_clear();
    RCC_TELNETD_SessionDestroy(pCliEnv);

    return kill;
}

/*-----------------------------------------------------------------------*/

/* clean up and exit */
extern void RCC_TELNETD_Kill()
{
    cli_env *pCliEnv;
    sbyte4   index;

    for (index = 1; index < kRCC_MAX_CLI_TASK+1; index++)
    {
        pCliEnv = gppCliSessions[index];

        if ((NULL == pCliEnv) || (NULL == CLIENV(pCliEnv)))
            continue;

        OS_SPECIFIC_SOCKET_CLOSE(MCONN_GetSock(pCliEnv));

        RCC_EnableFeature(pCliEnv, kRCC_FLAG_KILL);
    }

    gMainLoop  = FALSE;
    gSleepLoop = FALSE;
}

#endif /* __RCC_ENABLED__ */
