/*
 *  rcc_util.h
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

#ifndef __RCC_UTIL_H__
#define __RCC_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

extern RLSTATUS RCC_UTIL_ErrorThrow(RLSTATUS status);
extern RLSTATUS RCC_UTIL_Init(cli_env *pCliEnv);
extern void     RCC_UTIL_RapidPrompt(cli_env *pCliEnv, Boolean print);
extern RLSTATUS RCC_UTIL_SecuredAccess(cli_env *pCliEnv, sbyte *login, sbyte *password);
extern RLSTATUS RCC_UTIL_SetAccess(cli_env *pCliEnv, sbyte *pAccessStr);
extern RLSTATUS RCC_UTIL_SetAccessLevel(cli_env *pCliEnv, sbyte4 level);
extern RLSTATUS RCC_UTIL_SetAccessGroups(cli_env *pCliEnv, sbyte *pGroups);
extern void     RCC_UTIL_SetPrompt(cli_env *pCliEnv, sbyte *promptText);
extern void     RCC_UTIL_SetPromptTail(cli_env *pCliEnv, sbyte *pText);
extern void     RCC_UTIL_UpdatePrompt(cli_env *pCliEnv);

extern RLSTATUS RCC_ReplaceRapidMarkData(cli_env *pCliEnv, sbyte *pSrc, sbyte *pDest, Length bufLen);

#ifdef __cplusplus
}
#endif


#endif /* __RCC_UTIL_H__ */
