/*  
 *  rcc_hist.h
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

#ifndef _RCC_HIST_H_
#define _RCC_HIST_H_

#define HIST_BUFFER_FULL(x)  (x->bufferIndex == (x->iMaxHistCmds - 1))
#define LEAST_RECENT_HIST(x) (RC_MAX(x->iNumCmds - x->iMaxHistCmds + 1, 0))
#define HistBuffPtr(x, y)    ((sbyte *) &(x->pHistBuff[y].histCmd))

#ifdef __cplusplus
extern "C" {
#endif

extern RLSTATUS RCC_HIST_DispHistBuff (cli_env *pCliEnv);
extern RLSTATUS RCC_HIST_DispHistLine(cli_env *pCliEnv, Boolean enable); 
extern void     RCC_HIST_AddHistLine (cli_env *pCliEnv);
extern void     RCC_HIST_Scroll(cli_env *pCliEnv, sbyte4 index);
extern RLSTATUS RCC_HIST_InitHistInfo(cli_env *pCliEnv);
extern RLSTATUS RCC_HIST_ResizeHistBuff(cli_env *pCliEnv, sbyte4 NumCmds);
extern RLSTATUS RCC_HIST_ExecHistory(cli_env *pCliEnv);

#ifdef __cplusplus
}
#endif


#endif /* _RCC_HIST_H_ */
