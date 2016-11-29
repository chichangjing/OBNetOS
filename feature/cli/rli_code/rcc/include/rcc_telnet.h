/*  
 *  rcc_telnet.h
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

#ifndef __RCC_TELNET_H__
#define __RCC_TELNET_H__

#ifndef __USE_OTHER_TELNETD__

typedef struct dbtInfo
{
    SubOption  option;
    sbyte     *description;
} dbtInfo;

typedef struct dbtDescribe
{
    dbtInfo *info;
    sbyte   *type;
    ubyte2   count;
} dbtDescribe;



/*
 * Definitions for the TELNET protocol.
 */
#define kRCC_TC_IAC     255        /* interpret as command: */
#define kRCC_TC_DONT    254        /* you are not to use option */
#define kRCC_TC_DO      253        /* please, you use option */
#define kRCC_TC_WONT    252        /* I won't use option */
#define kRCC_TC_WILL    251        /* I will use option */
#define kRCC_TC_SB      250        /* interpret as subnegotiation */
#define kRCC_TC_GA      249        /* you may reverse the line */
#define kRCC_TC_EL      248        /* erase the current line */
#define kRCC_TC_EC      247        /* erase the current character */
#define kRCC_TC_AYT     246        /* are you there */
#define kRCC_TC_AO      245        /* abort output--but let prog finish */
#define kRCC_TC_IP      244        /* interrupt process--permanently */
#define kRCC_TC_BREAK   243        /* break */
#define kRCC_TC_DM      242        /* data mark--for connect. cleaning */
#define kRCC_TC_NOP     241        /* nop */
#define kRCC_TC_SE      240        /* end sub negotiation */
#define kRCC_TC_EOR     239        /* end of record (transparent mode) */
#define kRCC_TC_ABORT   238        /* Abort process */
#define kRCC_TC_SUSP    237        /* Suspend process */
#define kRCC_TC_EOF     236        /* End of file */
#define kRCC_TC_SYNCH   242        /* for telfunc calls */

/*
 * States for the telnet session
 */

enum keRCC_TS {
    kRCC_TS_INVALID = 0,            /* safety check */
    kRCC_TS_CR,                     /* CR-LF ->'s CR */
    kRCC_TS_DATA,                   /* base state */
    kRCC_TS_CURSOR,                 /* cursor movement */
    kRCC_TS_ESC     = kESC,         /* escape sequence */
    kRCC_TS_IAC     = kRCC_TC_IAC,  /* look for double IAC's */
    kRCC_TS_SB      = kRCC_TC_SB,   /* sub option */
    kRCC_TS_WILL    = kRCC_TC_WILL, /* will option negotiation */
    kRCC_TS_WONT    = kRCC_TC_WONT, /* wont " */
    kRCC_TS_DO      = kRCC_TC_DO,   /* do " */
    kRCC_TS_DONT    = kRCC_TC_DONT  /* dont " */
};

/* telnet options */
#define kRCC_TELOPT_INVALID         -1  /* no option selected */
#define kRCC_TELOPT_BINARY           0  /* 8-bit data path */
#define kRCC_TELOPT_ECHO             1  /* echo */
#define kRCC_TELOPT_RCP              2  /* prepare to reconnect */
#define kRCC_TELOPT_SGA              3  /* suppress go ahead */
#define kRCC_TELOPT_NAMS             4  /* approximate message size */
#define kRCC_TELOPT_STATUS           5  /* give status */
#define kRCC_TELOPT_TM               6  /* timing mark */
#define kRCC_TELOPT_RCTE             7  /* remote controlled transmission and echo */
#define kRCC_TELOPT_NAOL             8  /* negotiate about output line width */
#define kRCC_TELOPT_NAOP             9  /* negotiate about output page size */
#define kRCC_TELOPT_NAOCRD          10  /* negotiate about CR disposition */
#define kRCC_TELOPT_NAOHTS          11  /* negotiate about horizontal tabstops */
#define kRCC_TELOPT_NAOHTD          12  /* negotiate about horizontal tab disposition */
#define kRCC_TELOPT_NAOFFD          13  /* negotiate about formfeed disposition */
#define kRCC_TELOPT_NAOVTS          14  /* negotiate about vertical tab stops */
#define kRCC_TELOPT_NAOVTD          15  /* negotiate about vertical tab disposition */
#define kRCC_TELOPT_NAOLFD          16  /* negotiate about output LF disposition */
#define kRCC_TELOPT_XASCII          17  /* extended ascic character set */
#define kRCC_TELOPT_LOGOUT          18  /* force logout */
#define kRCC_TELOPT_BM              19  /* byte macro */
#define kRCC_TELOPT_DET             20  /* data entry terminal */
#define kRCC_TELOPT_SUPDUP          21  /* supdup protocol */
#define kRCC_TELOPT_SUPDUPOUTPUT    22  /* supdup output */
#define kRCC_TELOPT_SNDLOC          23  /* send location */
#define kRCC_TELOPT_TTYPE           24  /* terminal type */
#define kRCC_TELOPT_EOR             25  /* end or record */
#define kRCC_TELOPT_TUID            26  /* TACACS user identification */
#define kRCC_TELOPT_OUTMRK          27  /* output marking */
#define kRCC_TELOPT_TTYLOC          28  /* terminal location number */
#define kRCC_TELOPT_3270REGIME      29  /* 3270 regime */
#define kRCC_TELOPT_X3PAD           30  /* X.3 PAD */
#define kRCC_TELOPT_NAWS            31  /* window size */
#define kRCC_TELOPT_TSPEED          32  /* terminal speed */
#define kRCC_TELOPT_LFLOW           33  /* remote flow control */
#define kRCC_TELOPT_LINEMODE        34  /* Linemode option */
#define kRCC_TELOPT_XDISPLOC        35  /* X Display Location */
#define kRCC_TELOPT_OLD_ENVIRON     36  /* Old - Environment variables */
#define kRCC_TELOPT_AUTHENTICATION  37  /* Authenticate */
#define kRCC_TELOPT_ENCRYPT         38  /* Encryption option */
#define kRCC_TELOPT_NEW_ENVIRON     39  /* New - Environment variables */
#define kRCC_TELOPT_EXOPL          255  /* extended-options-list */

/*-----------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* function prototypes */
extern RLSTATUS  RCC_TELNET_Handshake(cli_env *pCliEnv, ubyte option, ubyte value);
extern RLSTATUS  RCC_TELNET_Init(cli_env *pCliEnv);
extern Boolean   RCC_TELNET_Process(cli_env *pCliEnv, cliChar data);
extern RLSTATUS  RCC_TELNET_Recv(cli_env *pEnv, cliChar *charIn);
extern RLSTATUS  RCC_TELNET_Send(cli_env *pEnv, sbyte *pBuf, sbyte4  bufLen);
extern optAction RCC_TELNET_StateChange(cli_env *pCliEnv, sbyte from, optType option, optAction action);
extern void      RCC_TELNET_StartSession(void * socketPtr);
extern void      RCC_TELNET_Status(cli_env *pCliEnv);
extern sbyte4    RCC_TELNET_GetTimeOut(cli_env *pCliEnv);
extern void      RCC_TELNET_SetTimeOut(cli_env *pCliEnv, sbyte4 timeout);

#ifdef __cplusplus
}
#endif

#endif /* __USE_OTHER_TELNETD__ */
#endif /* __RCC_TELNET_H__      */
