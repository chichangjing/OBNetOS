/*  
 *  rcc_telnet.c
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

#ifdef __FreeRTOS_OS__
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif

#ifndef __USE_OTHER_TELNETD__

#include "rcc_opt.h"

#ifndef RL_STATIC
#define RL_STATIC static
#endif

#define DOUBLE_BYTE(low, high)  (low + (high << 8))
#define __INVERT_RFC__

#ifdef __RCC_DEBUG_TELNET__
static FILE *telnetDebug;
#endif /* __RCC_DEBUG_TELNET__ */

typedef struct OptInit
{
    optType   option;
    optAction action;
    Boolean   desired;
    Boolean   toggle;
} OptInit;


static OptInit requiredOptions[] =
{
    { kRCC_TELOPT_ECHO       ,  kRCC_TC_WONT, TRUE,   FALSE},
    { kRCC_TELOPT_SGA        ,  0,            TRUE,   FALSE},
    { kRCC_TELOPT_TTYPE      ,  kRCC_TC_DO,   TRUE,   FALSE},
    { kRCC_TELOPT_NAWS       ,  kRCC_TC_DO,   TRUE,   FALSE},
    { kRCC_TELOPT_LFLOW      ,  0,            FALSE,  TRUE},
    { kRCC_TELOPT_LINEMODE   ,  kRCC_TC_WILL, FALSE,  TRUE},
    { kRCC_TELOPT_STATUS     ,  0,            TRUE,   FALSE}
};

static dbtDescribe dbtdOption =
{
    (dbtInfo *) &dbtiOption,
    "Option",
    ARRAY_SIZE(dbtiOption)
};


static struct TelStateInfo
{
    TelState  state;
    sbyte    *pName;
} TelStateInfo[] =
{
    {TS_Invalid,    "Invalid"   },
    {TS_No,         "No"        },
    {TS_WantNo,     "Want No"   },
    {TS_WantYes,    "Want Yes"  },
    {TS_Yes,        "Yes"       }
};


/*-----------------------------------------------------------------------*/

/* for debugging sub option negotiation */
RL_STATIC sbyte * 
TELNET_OptionString(optType option)
{
    sbyte2       index;
    dbtDescribe *dbtd = &dbtdOption;
    dbtInfo     *dbti = dbtd->info;

    for (index = 0; index < dbtd->count; index++)
    {
        if (dbti->option == option)
        {
            return dbti->description;
        }
        dbti++;
    }
    return NULL;
}


/*-----------------------------------------------------------------------*/

RL_STATIC PairState * 
TELNET_NewOption(cli_env *pCliEnv, ubyte option, sbyte desired 
                     /* TelState hostState, TelState clientState */)
{
    PairState   *pOption     = MMISC_GetOptHandled(pCliEnv);
    TelState     hostState   = TS_Invalid;
    TelState     clientState = TS_Invalid;
    sbyte4       index;

    for (index = 0; index < kRCC_MAX_OPT_HANDLED; index++, pOption++)
    {
        if (pOption->option == option)
            return pOption;

        if (pOption->option == 0)
        {
            pOption->option             = option;
            pOption->desired            = desired;
            pOption->client.count       = 0;
            pOption->client.name        = 'C';
            pOption->client.optState    = clientState;
            pOption->client.queueState  = QUEUE_Empty;
            pOption->host.count         = 0;
            pOption->host.name          = 'H';
            pOption->host.optState      = hostState;
            pOption->host.queueState    = QUEUE_Empty;

            return pOption;
        }
    }
    return NULL;
}

/*-----------------------------------------------------------------------*/

RL_STATIC PairState * 
TELNET_GetOption(cli_env *pCliEnv, ubyte option)
{
    sbyte4       index;
    PairState   *pOption = MMISC_GetOptHandled(pCliEnv);

    /* find existing option */
    for (index = 0; index < kRCC_MAX_OPT_HANDLED; index++, pOption++)
    {
        if (pOption->option == option)
            return pOption;
    }

    /* create default new option */
    pOption = TELNET_NewOption(pCliEnv, option, FALSE);

    return pOption;
}

/*-----------------------------------------------------------------------*/

#ifdef __RCC_DEBUG_TELNET__
static Boolean log_start = FALSE;
static sbyte   ErrorMsg[64];
#endif /* __RCC_DEBUG_TELNET__ */


#ifdef __RCC_DEBUG_TELNET__

#define kFORMAT_TELNET_SHOW "%-6s  %-4s  %-8s  %-8s  %-16s  %s\r\n"

RL_STATIC void 
TELNET_Show(cli_env *pCliEnv, sbyte from, optType option, optAction action, sbyte *pMsg)
{
    PairState *pOption  = TELNET_GetOption(pCliEnv, option);
    sbyte     *pSide    = ('H' == from ? "Host  " : "Client");
    sbyte     *pInfo    = TELNET_OptionString(option);
    sbyte     *pAction  = " - ";
    sbyte     *pHost    = TelStateInfo[pOption->host.optState].pName;
    sbyte     *pClient  = TelStateInfo[pOption->client.optState].pName;
    sbyte     buffer[256];

    if (0 == action)
        return;

    ErrorMsg[0] = 0;

    switch (action)
    {
		case kRCC_TC_WILL:
            pAction = "WILL";
            break;
		case kRCC_TC_WONT:
            pAction = "WONT";
            break;
		case kRCC_TC_DO:
            pAction = "DO";
            break;
		case kRCC_TC_DONT:
            pAction = "DONT";
            break;
        default:
            pAction = "HUH?";
            break;
    }

    if (FALSE == log_start)
    {
        sprintf(buffer, kFORMAT_TELNET_SHOW,
                "Side", "Req.", "Host", "Client", "Option", "Message");
        LogWrite(telnetDebug, buffer);
        sprintf(buffer, kFORMAT_TELNET_SHOW,
                "----", "----", "----", "------", "------", "-------");
        LogWrite(telnetDebug, buffer);
        log_start = TRUE;
    }

    sprintf(buffer, kFORMAT_TELNET_SHOW,
            pSide, pAction, pHost, pClient, pInfo, ErrorMsg);
    LogWrite(telnetDebug, buffer);

    RCC_EXT_WriteStr(pCliEnv, buffer);
}
#else
#define TELNET_Show(pCliEnv, from, option, action, pMsg)
#endif /* __RCC_DEBUG_TELNET__ */

/*-----------------------------------------------------------------------*/

#ifdef __RCC_DEBUG_TELNET__
RL_STATIC void 
TELNET_Error(cli_env *pCliEnv, sbyte *pMsg)
{
    STRNCPY(ErrorMsg, pMsg, sizeof(ErrorMsg));
}
#else
#define TELNET_Error(pChannel, pMsg)
#endif /* __RCC_DEBUG_TELNET__ */

/*-----------------------------------------------------------------------*/

#ifdef __RCC_DEBUG_TELNET__
RL_STATIC void 
TELNET_Log(sbyte from, ubyte option, ubyte action)
{
    sbyte  buffer[256];
    sbyte *fromText   = 'H' == from ? "Host" : "Term";
    sbyte *optionText = TELNET_OptionString(option);
    sbyte *actionText;

    switch (action)
    {
		case kRCC_TC_WILL:
            actionText = "WILL";
            break;
		case kRCC_TC_WONT:
            actionText = "WONT";
            break;
		case kRCC_TC_DO:
            actionText = "DO";
            break;
		case kRCC_TC_DONT:
            actionText = "DONT";
            break;
        default:
            actionText = " ? ";
            break;
    }

    sprintf(buffer, "%-4s %-4s %s\r\n",
            fromText, actionText, optionText);
    LogWrite(telnetDebug, buffer);
}
#else
#define TELNET_Log(from, option, action)
#endif /* __RCC_DEBUG_TELNET__ */

/*-----------------------------------------------------------------------*/

RL_STATIC sbyte * TELNET_StateInfo(TelState state)
{
    //if ((state < 0) || (state > ARRAY_SIZE(TelStateInfo)))
	if (state > ARRAY_SIZE(TelStateInfo))
        state = TS_Invalid;

    return TelStateInfo[state].pName;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
TELNET_StartOption(cli_env *pCliEnv, cliChar data)
{
    sbyte     *buffer    = MCONN_OptBufferPtr(pCliEnv);
    sbyte4     index     = MCONN_GetOptBufferIndex(pCliEnv);
#ifdef __RCC_DEBUG_TELNET__
    optType    subOption = MCONN_GetSubOption(pCliEnv);
    sbyte     *info      = TELNET_OptionString(subOption);
#endif /* __RCC_DEBUG_TELNET__ */

    /* beginning capture? */
    if (0 > index)
    {
#ifdef __RCC_DEBUG_TELNET__
        info     = TELNET_OptionString(data);
        RCC_EXT_WriteStr(pCliEnv, "Start option data: ");
        RCC_EXT_WriteStrLine(pCliEnv, info);
#endif
        MCONN_SetSubOption(pCliEnv, (optType) data);
        MCONN_SetOptBufferIndex(pCliEnv, 0);
        return;
    }

    buffer[index] = (sbyte) data;

    /* prevent buffer overrun */
    if ( ++index >= kRCC_OPT_BUF_SIZE)
        index = kRCC_OPT_BUF_SIZE;

    MCONN_SetOptBufferIndex(pCliEnv, index);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
TELNET_OptWindowSize(cli_env *pCliEnv)
{
    ubyte      *buffer = (ubyte *) MCONN_OptBufferPtr(pCliEnv);
    EditType    width;
    EditType    height;

    /* error check for index != 4 ? */

    width  = DOUBLE_BYTE(buffer[1], buffer[0]);
    height = DOUBLE_BYTE(buffer[3], buffer[2]);

    RCC_EXT_SetWidth(pCliEnv, width);
    RCC_EXT_SetHeight(pCliEnv, height);
}

/*-----------------------------------------------------------------------*/

/* Display current telnet settings */
extern void 
RCC_TELNET_Status(cli_env *pCliEnv)
{
    PairState   *pOptions  = MMISC_GetOptHandled(pCliEnv);
    sbyte4       index;
    sbyte       *pHost;
    sbyte       *pClient;
    sbyte       *pInfo     = NULL;
    sbyte       *pWant;
    sbyte        buffer[128];

#define STATUS_FORMAT   "%-30s   %-4s  %-8s   %-8s"

    RCC_EXT_WriteStrLine(pCliEnv, "");
    sprintf(buffer, STATUS_FORMAT, "Option", "Want", "Host", "Client");
    RCC_EXT_WriteStrLine(pCliEnv, buffer);

    if (NULL == pOptions)
        return;

    for (index = 0; index < kRCC_MAX_OPT_HANDLED; index++, pOptions++)
    {
        if (0 == pOptions->option)
            continue;

        pInfo   = TELNET_OptionString(pOptions->option);
        pHost   = TELNET_StateInfo(pOptions->host.optState);
        pClient = TELNET_StateInfo(pOptions->client.optState);
        pWant   = pOptions->desired ? "Yes" : "No";

        sprintf(buffer, STATUS_FORMAT, pInfo, pWant, pHost, pClient);
        RCC_EXT_WriteStrLine(pCliEnv, buffer);
    }
}

/*-----------------------------------------------------------------------*/

RL_STATIC optAction 
TELNET_Enable(cli_env *pCliEnv, optType option, Boolean enable)
{
    optAction    reply     = 0;
    PairState   *pOption   = TELNET_GetOption(pCliEnv, option);
    OptionState *pStatus   = &pOption->client;

    /* keep track of whether we want this option */
    pOption->desired = enable;
/*

      If we decide to ask them to enable:
         NO            them=WANTYES, send DO.
         YES           Error: Already enabled.
         WANTNO  EMPTY If we are queueing requests, themq=OPPOSITE;
                       otherwise, Error: Cannot initiate new request
                       in the middle of negotiation.
              OPPOSITE Error: Already queued an enable request.
         WANTYES EMPTY Error: Already negotiating for enable.
              OPPOSITE themq=EMPTY.
*/
    if (enable)
    {
        switch (pStatus->optState)
        {
        case TS_Invalid:
        case TS_No:
            pStatus->optState = TS_WantYes;
#ifdef __INVERT_RFC__
            reply = kRCC_TC_WILL;
#else
            reply = kRCC_TC_DO; 
#endif
            break;
        case TS_Yes:
            TELNET_Error(pCliEnv, "Already enabled"); 
            break;
        case TS_WantNo:
            TELNET_Error(pCliEnv, "Waiting for NO"); 
            break;
        case TS_WantYes:
            if (QUEUE_Opposite == pStatus->queueState)
                pStatus->queueState = QUEUE_Empty;
            break;
        default:
            TELNET_Error(pCliEnv, "Unknown state"); 
            break;
        }
    }
    else
    {
/*
      If we decide to ask them to disable:
         NO            Error: Already disabled.
         YES           them=WANTNO, send DONT.
         WANTNO  EMPTY Error: Already negotiating for disable.
              OPPOSITE themq=EMPTY.
         WANTYES EMPTY If we are queueing requests, themq=OPPOSITE;
                       otherwise, Error: Cannot initiate new request
                       in the middle of negotiation.
              OPPOSITE Error: Already queued a disable request.
*/
        switch (pStatus->optState)
        {
        case TS_Invalid:
        case TS_No:
            TELNET_Error(pCliEnv, "Already disabled");
            break;
        case TS_Yes:
            pStatus->optState = TS_WantNo;
#ifdef __INVERT_RFC__
            reply = kRCC_TC_WONT;
#else
            reply = kRCC_TC_DONT; 
#endif
            break;
        case TS_WantNo:
            if (QUEUE_Opposite == pStatus->queueState)
                pStatus->queueState = QUEUE_Empty;
            else
                TELNET_Error(pCliEnv, "Queue is empty!"); 
            break;
        case TS_WantYes:
            TELNET_Error(pCliEnv, "Unexpected DONT");
            break;
        default:
            TELNET_Error(pCliEnv, "Unknown state"); 
            break;
        }
    }

    TELNET_Show(pCliEnv, 'H', option, reply, "");

    return reply;
}


/*-----------------------------------------------------------------------*/

/* tell client all the options we want */
RL_STATIC RLSTATUS 
TELNET_Required(cli_env *pCliEnv)
{
    PairState   *pOption;
    sbyte        desired;
    optAction    action;
    optType      option;
    sbyte4       index;
    OptInit     *pInit = requiredOptions;
#ifdef __RCC_DEBUG_TELNET__
    sbyte       *pInfo;
#endif

    for (index = 0; index < ARRAY_SIZE(requiredOptions); index++, pInit++)
    {
        option  = pInit->option;
        desired = pInit->desired;

        pOption = TELNET_NewOption(pCliEnv, option, desired);
        if (NULL == pOption)
            return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

#ifdef __RCC_DEBUG_TELNET__
        pInfo  = TELNET_OptionString(option);
#endif
        if (0 < pInit->action)
            RCC_TELNET_Handshake(pCliEnv, pInit->option, pInit->action);

        if (pInit->desired)
        {
            action = TELNET_Enable(pCliEnv, option, desired);
            if (0 != action)
                RCC_TELNET_Handshake(pCliEnv, option, action);
        }
    }
    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_TELNET_Init(cli_env *pCliEnv)
{
    sbyte4  optSize  = sizeof(PairState) * kRCC_MAX_OPT_HANDLED;

    MCONN_SetSubOption(pCliEnv,      0);
    MCONN_SetConnType(pCliEnv,       kRCC_CONN_TELNET);
    MCONN_SetWriteHandle(pCliEnv,    TELNET_SEND_FN);
    MCONN_SetReadHandle(pCliEnv,     TELNET_RECV_FN);
    MCONN_SetRecvState(pCliEnv,      kRCC_TS_DATA);
    MCONN_SetOptBufferIndex(pCliEnv, -1);

    MEMSET(MMISC_GetOptHandled(pCliEnv), 0, optSize);

    TELNET_Required(pCliEnv);

    return OK;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
TELNET_SaveTerminalType(cli_env *pCliEnv)
{
    sbyte  *pBuffer  = MCONN_OptBufferPtr(pCliEnv);
    sbyte  *pName    = MCONN_GetTermName(pCliEnv);
    sbyte4  index    = MCONN_GetOptBufferIndex(pCliEnv);

    pBuffer++;
    STRNCPY(pName, pBuffer, RC_MIN(index, kRCC_TERM_TYPE_SIZE));
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
TELNET_SaveOption(cli_env *pCliEnv)
{
    Boolean      saved     = TRUE;
    optType      subOption = MCONN_GetSubOption(pCliEnv);
#ifdef __RCC_DEBUG_TELNET__
    sbyte       *info      = TELNET_OptionString(subOption);
#endif

    switch (subOption)
    {
        case kRCC_TELOPT_NAWS:
            TELNET_OptWindowSize(pCliEnv);
            break;
        case kRCC_TELOPT_TTYPE:
            TELNET_SaveTerminalType(pCliEnv);
            break;
        default:
            saved = FALSE;
            break;
    }

#ifdef __RCC_DEBUG_TELNET__
    if (saved)
        RCC_EXT_WriteStr(pCliEnv, "Saved:  ");
    else
        RCC_EXT_WriteStr(pCliEnv, "Tossed: ");
    RCC_EXT_WriteStrLine(pCliEnv, info);
#endif

    MCONN_SetOptBufferIndex(pCliEnv, -1);
    MCONN_SetRecvState(pCliEnv, kRCC_TS_DATA);
    MCONN_SetSubOption(pCliEnv, 0);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_TELNET_Send(cli_env *pEnv, sbyte *pBuf, sbyte4 BufLen)
{
    CliChannel *pChannel           = MMISC_GetChannel(pEnv);
    OS_SPECIFIC_SOCKET_HANDLE sock = pChannel->sock;

    return OS_SPECIFIC_SOCKET_WRITE(sock, pBuf, BufLen);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
TELNET_Request(cli_env *pCliEnv, optType option)
{
    CliChannel *pChannel  = MMISC_GetChannel(pCliEnv);
    ubyte       command[] = {kRCC_TC_IAC, kRCC_TC_SB, 0, 
                             1, kRCC_TC_IAC, kRCC_TC_SE};

    switch (option)
    {
        case kRCC_TELOPT_STATUS:
        case kRCC_TELOPT_TTYPE:
            break;
        default:
            return;
    }

    command[2] = option;

    OS_SPECIFIC_SOCKET_WRITE(pChannel->sock, (sbyte *) command, sizeof(command));
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
TELNET_Negotiate(cli_env *pCliEnv, optType option, optAction action)
{
    PairState   *pOption  = NULL;
    optType      reply    = 0;

#ifdef __RCC_DEBUG_TELNET__
    TELNET_Log('C', option, action); 
#endif /* __RCC_DEBUG_TELNET__ */

    if (NULL == (pOption = TELNET_GetOption(pCliEnv, option)))
        return;

    reply = RCC_TELNET_StateChange(pCliEnv, 'C', option, action);

    if (kRCC_TS_INVALID != reply)
        RCC_TELNET_Handshake(pCliEnv, option, reply);
}

/*-----------------------------------------------------------------------*/

extern sbyte4 RCC_TELNET_GetTimeOut(cli_env *pCliEnv)
{
    return MCONN_GetTimeOut(pCliEnv);
}

/*-----------------------------------------------------------------------*/

extern void RCC_TELNET_SetTimeOut(cli_env *pCliEnv, sbyte4 timeout)
{
    MCONN_SetTimeOut(pCliEnv, timeout);
}

/*-----------------------------------------------------------------------*/

/* return TRUE if data is text (not handshaking) */

extern Boolean
RCC_TELNET_Process(cli_env *pCliEnv, cliChar data)
{
    ubyte       state   = MCONN_GetRecvState(pCliEnv);
	Boolean     isData  = FALSE;
    
    switch (state) 
    {			
        case kRCC_TS_DATA:
            if (kRCC_TC_IAC == data)
    			state = kRCC_TS_IAC;
            else
                isData = TRUE;
            break;
		case kRCC_TS_IAC:
    		switch (data) 
            {
				case kRCC_TC_IP:
				case kRCC_TC_BREAK:
				case kRCC_TC_AYT:
				case kRCC_TC_AO:
				case kRCC_TC_EC:
				case kRCC_TC_EL:
				case kRCC_TC_DM:
				case kRCC_TC_EOR:
				case kRCC_TC_EOF:
				case kRCC_TC_SUSP:
				case kRCC_TC_ABORT:
					break;
				case kRCC_TC_SB:
					state = kRCC_TS_SB;
					break;
				case kRCC_TC_WILL:
					state = kRCC_TS_WILL;
					break;
				case kRCC_TC_WONT:
					state = kRCC_TS_WONT;
					break;
				case kRCC_TC_DO:
					state = kRCC_TS_DO;
					break;
				case kRCC_TC_DONT:
					state = kRCC_TS_DONT;
					break;
				case kRCC_TC_IAC:
	        		state = kRCC_TS_DATA;
					break;
                case kRCC_TC_SE:
                    TELNET_SaveOption(pCliEnv);
	          		state = kRCC_TS_DATA;
					break;
				default:
		        	state = kRCC_TS_DATA;
					break;
			} /* switch (data) */
			break;

		case kRCC_TS_SB:
			if (kRCC_TC_IAC == data)
				state = kRCC_TS_IAC;
            else
                TELNET_StartOption(pCliEnv, data);
			break;
		case kRCC_TS_WILL:
		case kRCC_TS_WONT:
		case kRCC_TS_DO:
		case kRCC_TS_DONT:
            TELNET_Negotiate(pCliEnv, (optType) data, state);
    		state = kRCC_TS_DATA;
			break;
		default:
#ifndef __RCC_DEBUG_TELNET__
            state = kRCC_TS_DATA;
            break;
#else
            printf("Default case reached\n");
	    	status = ERROR_RCC_FAILURE;
#endif
	    } /* switch (*state) */
    
    MCONN_SetRecvState(pCliEnv, state);

	return isData;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS
RCC_TELNET_Recv(cli_env *pCliEnv, cliChar *charReceived)
{
	RLSTATUS                    status  = OK;
    ubyte                       charIn  = 0;
    sbyte4                      timeout = MCONN_GetTimeOut(pCliEnv);
    OS_SPECIFIC_SOCKET_HANDLE   sock    = MCONN_GetSock(pCliEnv);

    while (1)
    {
    	status = OS_SPECIFIC_SOCKET_DATA_AVAILABLE (sock, timeout);

        if (SYS_ERROR_SOCKET_TIMEOUT == status)
		    return RCC_ERROR_THROW(ERROR_RCC_TIMEOUT);
	
        if (SYS_ERROR_SOCKET_GENERAL == status)
		    return RCC_ERROR_THROW(ERROR_RCC_FAILURE);

        if (1 != OS_SPECIFIC_SOCKET_READ(sock, (sbyte *) &charIn, 1))
		    return RCC_ERROR_THROW(ERROR_RCC_READ);
        
        if (RCC_TELNET_Process(pCliEnv, charIn))
            break;
    } 

    *charReceived = charIn;
	return status;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_TELNET_Handshake(cli_env *pCliEnv, ubyte option, ubyte action)
{
	ubyte        command[3];

    if (0 == action)
        return OK; /* should be an error someday */

    TELNET_Log('H', option, action); 

    command[0] = kRCC_TC_IAC;
    command[1] = action;
	command[2] = option;

    return RCC_TELNET_Send(pCliEnv, (sbyte *) command, sizeof(command));
}

/*****************************************************************************
 *                       New telnet negotiation code                         *
 *****************************************************************************/

/*
      Per RFC 1143:

      There are two sides, we (us) and he (them).  We keep four
      variables:

         us: state of option on our side (NO/WANTNO/WANTYES/YES)
         usq: a queue bit (EMPTY/OPPOSITE) if us is WANTNO or WANTYES
         them: state of option on his side
         themq: a queue bit if them is WANTNO or WANTYES

      An option is enabled if and only if its state is YES.  Note that
      us/usq and them/themq could be combined into two six-choice states.

      "Error" below means that producing diagnostic information may be a
      good idea, though it isn't required.

      Upon receipt of WILL, we choose based upon them and themq:
         NO            If we agree that he should enable, them=YES, send
                       DO; otherwise, send DONT.
         YES           Ignore.
         WANTNO  EMPTY Error: DONT answered by WILL. them=NO.
              OPPOSITE Error: DONT answered by WILL. them=YES*,
                       themq=EMPTY.
         WANTYES EMPTY them=YES.
              OPPOSITE them=WANTNO, themq=EMPTY, send DONT.

      * This behavior is debatable; DONT will never be answered by WILL
        over a reliable connection between TELNETs compliant with this
        RFC, so this was chosen (1) not to generate further messages,
        because if we know we're dealing with a noncompliant TELNET we
        shouldn't trust it to be sensible; (2) to empty the queue
        sensibly.

      Upon receipt of WONT, we choose based upon them and themq:
         NO            Ignore.
         YES           them=NO, send DONT.
         WANTNO  EMPTY them=NO.
              OPPOSITE them=WANTYES, themq=NONE, send DO.
         WANTYES EMPTY them=NO.*
              OPPOSITE them=NO, themq=NONE.**

      * Here is the only spot a length-two queue could be useful; after
        a WILL negotiation was refused, a queue of WONT WILL would mean
        to request the option again. This seems of too little utility
        and too much potential waste; there is little chance that the
        other side will change its mind immediately.

      ** Here we don't have to generate another request because we've
         been "refused into" the correct state anyway.

      If we decide to ask them to enable:
         NO            them=WANTYES, send DO.
         YES           Error: Already enabled.
         WANTNO  EMPTY If we are queueing requests, themq=OPPOSITE;
                       otherwise, Error: Cannot initiate new request
                       in the middle of negotiation.
              OPPOSITE Error: Already queued an enable request.
         WANTYES EMPTY Error: Already negotiating for enable.
              OPPOSITE themq=EMPTY.

      If we decide to ask them to disable:
         NO            Error: Already disabled.
         YES           them=WANTNO, send DONT.
         WANTNO  EMPTY Error: Already negotiating for disable.
              OPPOSITE themq=EMPTY.
         WANTYES EMPTY If we are queueing requests, themq=OPPOSITE;
                       otherwise, Error: Cannot initiate new request
                       in the middle of negotiation.
              OPPOSITE Error: Already queued a disable request.

      We handle the option on our side by the same procedures, with DO-
      WILL, DONT-WONT, them-us, themq-usq swapped.

*/

/*-----------------------------------------------------------------------*/

/*
      There are two sides, we (us) and he (them).  We keep four
      variables:

         us:    state of option on our side (NO/WANTNO/WANTYES/YES)
         usq:   a queue bit (EMPTY/OPPOSITE) if us is WANTNO or WANTYES
         them:  state of option on his side
         themq: a queue bit if them is WANTNO or WANTYES

      An option is enabled if and only if its state is YES.  Note that
      us/usq and them/themq could be combined into two six-choice states.

      "Error" below means that producing diagnostic information may be a
      good idea, though it isn't required.
*/

/*-----------------------------------------------------------------------*/

extern optAction 
RCC_TELNET_StateChange(cli_env *pCliEnv, sbyte from, optType option, optAction action)
{
    PairState   *pOption   = TELNET_GetOption(pCliEnv, option);
    OptionState *pStatus   = 'H' == from ? &pOption->host : &pOption->client;
    optAction    reply     = 0;
    optAction    affirm    = 0;
    optAction    negate    = 0;

    /* if we are thrashing then just give up (32 is arbitrary)*/
    if (pStatus->count++ > 32)
        return kRCC_TS_INVALID;

    switch (action)
    {
    case kRCC_TS_DONT:
    case kRCC_TS_DO:
        affirm = kRCC_TS_WILL;
        negate = kRCC_TS_WONT;
        break;
    case kRCC_TS_WILL:
    case kRCC_TS_WONT:
        affirm = kRCC_TS_DO;
        negate = kRCC_TS_DONT;
        break;
    }

    switch (action)
    {
/*
      Upon receipt of WILL, we choose based upon them and themq:
         NO            If we agree that he should enable, them=YES, send
                       DO; otherwise, send DONT.
         YES           Ignore.
         WANTNO  EMPTY Error: DONT answered by WILL. them=NO.
              OPPOSITE Error: DONT answered by WILL. them=YES*,
                       themq=EMPTY.
         WANTYES EMPTY them=YES.
              OPPOSITE them=WANTNO, themq=EMPTY, send DONT.
*/
    case kRCC_TS_WILL:
        /* send suboption request */
        if ('C' == from)
            TELNET_Request(pCliEnv, option);
    case kRCC_TS_DO:
        switch (pStatus->optState)
        {
        case TS_Invalid:
        case TS_No:
            if (pOption->desired)
            {
                reply = affirm;
                pStatus->optState = TS_Yes;
            }
            else
                reply = negate;
            break;
        case TS_Yes:
            break;
        case TS_WantNo:
            if (QUEUE_Empty == pStatus->queueState)
                pStatus->optState   = TS_No;
            else
            {
                pStatus->optState   = TS_Yes;
                pStatus->queueState = QUEUE_Empty;
            } 
            break;
        case TS_WantYes:
            if (QUEUE_Empty == pStatus->queueState)
                pStatus->optState = TS_Yes;
            else
            {
                pStatus->queueState = QUEUE_Empty;
                pStatus->optState = TS_WantNo;
                reply = negate;
            }
            break;
        default:
            TELNET_Error(pCliEnv, "Unknown state");
            break;
        }
        break;
/*
      Upon receipt of WONT, we choose based upon them and themq:
         NO            Ignore.
         YES           them=NO, send DONT.
         WANTNO  EMPTY them=NO.
              OPPOSITE them=WANTYES, themq=NONE, send DO.
         WANTYES EMPTY them=NO.*
              OPPOSITE them=NO, themq=NONE.**
*/
        case kRCC_TS_WONT:
        case kRCC_TS_DONT:
        switch (pStatus->optState)
        {
        case TS_Invalid:
        case TS_No:
            break;
        case TS_Yes:
            pStatus->optState = TS_No;
            reply = negate;
            break;
        case TS_WantNo:
            if (QUEUE_Empty == pStatus->queueState)
                pStatus->optState = TS_No;
            else
            {
                pStatus->queueState = QUEUE_Empty;
                pStatus->optState = TS_WantYes;
                reply = affirm;
            }
            break;
        case TS_WantYes:
            pStatus->optState = TS_No;
            if (QUEUE_Opposite == pStatus->queueState)
                pStatus->queueState = QUEUE_Empty;
            break;
        default:
            TELNET_Error(pCliEnv, "Unknown state");
            break;
        }
        break;
    }

    TELNET_Show(pCliEnv, from, option, action, "");

    return reply;    
}

/*-----------------------------------------------------------------------*/

extern void 
RCC_TELNET_StartSession(void * pSession)
{
    CliChannel *pTelnetSession = (CliChannel *) pSession;
	extern xTaskHandle xTelnetHandle;
	
    RCC_TELNETD_CreateSession(pTelnetSession, RCC_TELNET_Init,
                              TELNET_RECV_FN, TELNET_SEND_FN, 
                                  kRCC_CONN_TELNET, "telnet");

    SOCKET_Close(pTelnetSession->sock);
#ifdef __FreeRTOS_OS__
	vTaskDelete(xTelnetHandle);
#endif
}

#endif /* ! __USE_OTHER_TELNETD__ */
#endif /* __RCC_ENABLED__ */

