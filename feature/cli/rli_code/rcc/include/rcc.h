/*  
 *  rcc.h
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

#ifdef __RCC_ENABLED__
#ifndef __RCC_H__
#define __RCC_H__

#include <stdio.h>
#include <string.h>

#ifdef __SNMP_API_ENABLED__
#ifdef __ENVOY_STACK__
#include "rcm_envoy.h"
#include "rcm_ev_cnv.h"
#include "rcm_snmp.h"
#endif
#endif /* __SNMP_API_ENABLED__ */

#include "rcc_custom.h"
#include "rcc_structs.h"
#include "rcc_cmd.h"
#include "rcc_error.h"
#include "rcc_hist.h"
#include "rcc_telnetd.h"
#include "rcc_db.h"
#include "rcc_ext.h"
#include "rcc_rcb.h"
#include "rcc_task.h"
#include "rcc_telnet.h"
#include "rcc_util.h"
#include "rcc_help.h"
#include "rcc_alias.h"

#ifdef __RCC_DEBUG__
#include "rcc_log.h"
#define RCC_ERROR_THROW(status)     RCC_UTIL_ErrorThrow(status)
#else
#define RCC_ERROR_THROW(status)     status
#endif

#ifdef RCC_STATUS_LINE_FN
void    RCC_STATUS_LINE_FN(cli_env *pCliEnv);
#else
#define RCC_STATUS_LINE_FN(pCliEnv)
#endif

#define STDIN 0

#ifndef CONSOLE_SESSION_FN
#define CONSOLE_SESSION_FN TELNETD_ConsoleSession
#endif

#ifndef TELNET_SEND_FN
#define TELNET_SEND_FN RCC_TELNET_Send
#endif

#ifndef TELNET_RECV_FN
#define TELNET_RECV_FN RCC_TELNET_Recv
#endif

#ifdef __RCC_PARAMETER_ORDER_NONE__
#define HANDLER_PARAMS(x)      NULL
#define HANDLER_PARAM_COUNT(x) 0
#else
#define HANDLER_PARAMS(x)      x->pParams
#define HANDLER_PARAM_COUNT(x) x->paramCount
#endif

#ifdef __RCC_CASE_INSENSITIVE__
#define COMPARE  STRICMP
#define NCOMPARE STRNICMP
#else
#define COMPARE  STRCMP
#define NCOMPARE STRNCMP
#endif /* __RCC_CASE_INSENSITIVE__ */

#ifdef __RCC_DEBUG__
#define RCC_ASSERT(x)   \
if (FALSE==(x)) printf("Assertion failed. File:%s Line:%d\n", __FILE__, __LINE__)
#else
#define RCC_ASSERT(x)
#endif

#ifndef RL_STATIC
#define RL_STATIC static
#endif

#endif /* __RCC_H__       */
#endif /* __RCC_ENABLED__ */
