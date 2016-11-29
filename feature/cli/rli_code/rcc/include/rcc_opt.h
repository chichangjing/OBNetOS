/*  
 *  rcc_opt.h
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

#ifndef __RCC_OPT_H__
#define __RCC_OPT_H__

const static dbtInfo stateInfo[] =
{
    {kRCC_TS_INVALID , "Bad "},
    {kRCC_TS_DATA    , "Data"},
    {kRCC_TS_IAC     , "Cmd "},
    {kRCC_TS_CR      , "CrLf"},
    {kRCC_TS_SB      , "Sub "},
    {kRCC_TS_WILL    , "Will"},
    {kRCC_TS_WONT    , "Wont"},
    {kRCC_TS_DO      , "Do  "},
    {kRCC_TS_DONT    , "Dont"},
    {kRCC_TS_CURSOR  , "Curs"},
    {kRCC_TS_ESC     , "Esc "}
};

const static dbtInfo dbtiCommand[] =
{
    {kRCC_TC_IAC     , "interpret as command: "},
    {kRCC_TC_DONT    , "you are not to use option "},
    {kRCC_TC_DO      , "please, you use option "},
    {kRCC_TC_WONT    , "I won't use option "},
    {kRCC_TC_WILL    , "I will use option "},
    {kRCC_TC_SB      , "interpret as subnegotiation "},
    {kRCC_TC_GA      , "you may reverse the line "},
    {kRCC_TC_EL      , "erase the current line "},
    {kRCC_TC_EC      , "erase the current character "},
    {kRCC_TC_AYT     , "are you there "},
    {kRCC_TC_AO      , "abort output--but let prog finish "},
    {kRCC_TC_IP      , "interrupt process--permanently "},
    {kRCC_TC_BREAK   , "break "},
    {kRCC_TC_DM      , "data mark--for connect. cleaning "},
    {kRCC_TC_NOP     , "nop "},
    {kRCC_TC_SE      , "end sub negotiation "},
    {kRCC_TC_EOR     , "end of record (transparent mode) "},
    {kRCC_TC_ABORT   , "Abort process "},
    {kRCC_TC_SUSP    , "Suspend process "},
    {kRCC_TC_EOF     , "End of file "},
    {kRCC_TC_SYNCH   , "for telfunc calls "}
};

const static dbtInfo dbtiOption[] =
{
    {kRCC_TELOPT_BINARY          ,  "8-bit data path"},
    {kRCC_TELOPT_ECHO            ,  "echo"},
    {kRCC_TELOPT_RCP             ,  "prepare to reconnect"},
    {kRCC_TELOPT_SGA             ,  "suppress go ahead"},
    {kRCC_TELOPT_NAMS            ,  "approximate message size"},
    {kRCC_TELOPT_STATUS          ,  "give status"},
    {kRCC_TELOPT_TM              ,  "timing mark"},
    {kRCC_TELOPT_RCTE            ,  "remote controlled transmission and echo"},
    {kRCC_TELOPT_NAOL            ,  "negotiate output line width"},
    {kRCC_TELOPT_NAOP            ,  "negotiate output page size"},
    {kRCC_TELOPT_NAOCRD          ,  "negotiate CR disposition"},
    {kRCC_TELOPT_NAOHTS          ,  "negotiate horizontal tabstops"},
    {kRCC_TELOPT_NAOHTD          ,  "negotiate horizontal tab disposition"},
    {kRCC_TELOPT_NAOFFD          ,  "negotiate formfeed disposition"},
    {kRCC_TELOPT_NAOVTS          ,  "negotiate vertical tab stops"},
    {kRCC_TELOPT_NAOVTD          ,  "negotiate vertical tab disposition"},
    {kRCC_TELOPT_NAOLFD          ,  "negotiate output LF disposition"},
    {kRCC_TELOPT_XASCII          ,  "extended ascic character set"},
    {kRCC_TELOPT_LOGOUT          ,  "force logout"},
    {kRCC_TELOPT_BM              ,  "byte macro"},
    {kRCC_TELOPT_DET             ,  "data entry terminal"},
    {kRCC_TELOPT_SUPDUP          ,  "supdup protocol"},
    {kRCC_TELOPT_SUPDUPOUTPUT    ,  "supdup output"},
    {kRCC_TELOPT_SNDLOC          ,  "send location"},
    {kRCC_TELOPT_TTYPE           ,  "terminal type"},
    {kRCC_TELOPT_EOR             ,  "end or record"},
    {kRCC_TELOPT_TUID            ,  "TACACS user identification"},
    {kRCC_TELOPT_OUTMRK          ,  "output marking"},
    {kRCC_TELOPT_TTYLOC          ,  "terminal location number"},
    {kRCC_TELOPT_3270REGIME      ,  "3270 regime"},
    {kRCC_TELOPT_X3PAD           ,  "X.3 PAD"},
    {kRCC_TELOPT_NAWS            ,  "window size"},
    {kRCC_TELOPT_TSPEED          ,  "terminal speed"},
    {kRCC_TELOPT_LFLOW           ,  "remote flow control"},
    {kRCC_TELOPT_LINEMODE        ,  "Linemode option"},
    {kRCC_TELOPT_XDISPLOC        ,  "X Display Location"},
    {kRCC_TELOPT_OLD_ENVIRON     ,  "Environment variables (Old)"},
    {kRCC_TELOPT_AUTHENTICATION  ,  "Authenticate"},
    {kRCC_TELOPT_ENCRYPT         ,  "Encryption option"},
    {kRCC_TELOPT_NEW_ENVIRON     ,  "Environment variables (New)"},
    {kRCC_TELOPT_EXOPL           ,  "extended-options-list"}
};

#endif /* __RCC_OPT_H__ */
