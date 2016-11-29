/*  
 *  rcc_task.h
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

#ifndef __RCC_TASK_H__
#define __RCC_TASK_H__

#define TASK_NONE_K     0x0000
#define TASK_RETRY_K    0x0001
#define TASK_NOMSG_K    0x0002

typedef struct ErrorTable {
    ubyte4    flags;
    RLSTATUS  status;
    sbyte    *pErrorText;
} ErrorTable;


#ifdef __cplusplus
extern "C" {
#endif

extern void     RCC_TASK_Cleanup(cli_env *pCliEnv);
extern ubyte4   RCC_TASK_EvalError(RLSTATUS status, sbyte **pErrorMsg);
extern Boolean  RCC_TASK_LineEval(cli_env *pCliEnv, Boolean haveInput);
extern void     RCC_TASK_PrintPrompt(cli_env *pCliEnv);
extern void     RCC_TASK_Readline(cli_env *pCliEnv);
extern Boolean  RCC_TASK_ValidateLogin(cli_env *pCliEnv, sbyte *login, sbyte *password, Access *AccLvl);

#ifdef __cplusplus
}
#endif

#endif /* __RCC_TASK_H__ */
