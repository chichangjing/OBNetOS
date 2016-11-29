/*  
 *  rcc_alias.h
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

#ifndef __RCC_ALIAS_HEADER__
#define __RCC_ALIAS_HEADER__

#define kSTRING_DELIM   "\""
#define kSPACECHAR      " "

#define kDoubleQuote '\"'
#define kSingleQuote '\''
#define kNoQuote     '\0'
#define kSpaceChar   ' '

#ifdef __cplusplus
extern "C" {
#endif

extern RLSTATUS RCC_ALIAS_AliasDump(cli_env *pCliEnv, sbyte *pDumpBuffer, sbyte4 bufferSize);
extern RLSTATUS RCC_ALIAS_AliasRestore(cli_env *pCliEnv, sbyte *pDumpBuffer, sbyte4 bufferSize);
extern RLSTATUS RCC_ALIAS_CreateAlias(cli_env *pCliEnv, sbyte *aliasName, sbyte *aliasText);
extern RLSTATUS RCC_ALIAS_DeleteAlias(cli_env *pCliEnv, sbyte *aliasName);
extern void     RCC_ALIAS_DisplayAliases(cli_env *pCliEnv);


#ifdef __cplusplus
}
#endif

#endif /* __RCC_ALIAS_HEADER__ */
