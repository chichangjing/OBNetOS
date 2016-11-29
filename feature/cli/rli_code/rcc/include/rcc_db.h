/*  
 *  rcc_db.h
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


#ifndef __RCC_DB_HEADER__
#define __RCC_DB_HEADER__

#define kSTRING_DELIM   "\""
#define kSPACECHAR      " "

#define kTREE_FLAG_SYNTAX   0x0001

#define kDoubleQuote '\"'
#define kSingleQuote '\''
#define kNoQuote     '\0'
#define kSpaceChar   ' '

#ifdef __RCC_DEBUG__
extern ubyte4  gDebugFlags;

Boolean debugFeature      (ubyte4 feature);
void    debugFeatureSet   (ubyte4 feature);
void    debugFeatureClear (ubyte4 feature);
void    debugExecShow     (cmdNode * pNode);
#else
#define debugFeature(feature)       FALSE
#define debugFeatureSet(feature)
#define debugFeatureClear(feature)
#define debugExecShow(pNode)
#endif /* __RCC_DEBUG__ */

#ifdef __cplusplus
extern "C" {
#endif
extern RLSTATUS RCC_DB_AccessValidate(cli_env *pCliEnv, sbyte *pString, 
                                      DTTypeInfo *pInfo, Boolean errMsg);
extern void     RCC_DB_AppendErrorText(cli_env *pCliEnv, sbyte *pText, sbyte4 length);
extern sbyte   *RCC_DB_BadParam(cli_env *pCliEnv);
extern RLSTATUS RCC_DB_BuildParamList(cli_env *pCliEnv, Boolean exec);
extern RLSTATUS RCC_DB_CommandComplete(cli_env *pCliEnv);
extern handlerDefn *RCC_DB_CurrentHandler(cli_env *pCliEnv);
extern RLSTATUS RCC_DB_EnvironmentCreate(cli_env **ppCliEnv, CliChannel *pCliTaskObj,
                                         sbyte *pBuffer, sbyte4 size);
extern void     RCC_DB_EnvironmentDestroy(cli_env *pCliEnv);
extern RLSTATUS RCC_DB_EnvironmentReset(cli_env *pCliEnv);
extern void     RCC_DB_ErrorLine(cli_env *pCliEnv);
extern RLSTATUS RCC_DB_Execute(sbyte *pCmd, sbyte *pOutput, 
                               sbyte4 outputBuffSize, sbyte4 *outputSize);
extern RLSTATUS RCC_DB_ExecuteCommand(cli_env *pCliEnv, sbyte *pCmd);
extern RLSTATUS RCC_DB_Exit(cli_env *pCliEnv, Boolean exitAll);
extern RLSTATUS RCC_DB_ExpandToken(cli_env *pCliEnv, Boolean sameLine);
extern void     RCC_DB_FreeTasks(void);
extern sbyte *  RCC_DB_FullEnum(paramDefn *pParam, sbyte *pData);
extern sbyte4   RCC_DB_GetArraySize(paramDescr *pParamDesc);
extern sbyte   *RCC_DB_GetArrayElement(paramList  *pParamList, 
                                       paramDescr *pParamDesc, sbyte4 offset);
extern cmdNode *RCC_DB_GetCommandNode(cli_env *pCliEnv);
extern sbyte4   RCC_DB_GetParamCount(cli_env *pCliEnv, cmdNode *pNode);
extern cmdNode *RCC_DB_GetParentNode(cli_env *pCliEnv, cmdNode *pNode);
extern cmdNode *RCC_DB_GetRootNode(void);
extern RLSTATUS RCC_DB_InitTasks(void);
extern void     RCC_DB_InvalidParam(cli_env *pCliEnv, BitMask paramMask);
extern Boolean  RCC_DB_IsAssigned(cli_env *pCliEnv, void *item);
extern Boolean  RCC_DB_IsNoCommand(paramList  *pParamList);
extern paramDefn *RCC_DB_IncompleteParam(cli_env *pCliEnv);
extern sbyte *  RCC_DB_LastToken(cli_env *pCliEnv);
extern paramDefn *RCC_DB_OrderedParam(cli_env *pCliEnv, sbyte4 index);
extern RLSTATUS RCC_DB_Parameterize(cli_env *pCliEnv);
extern Boolean  RCC_DB_ParamAccess(cli_env *pCliEnv, paramDefn *pParam);
extern RLSTATUS RCC_DB_ParamValue(cli_env *pCliEnv, BitMask bitmask, 
                                  sbyte *pArgs, sbyte *pRapidMark, void *pOutput);
extern Boolean	RCC_DB_ParamPossible(cli_env *pCliEnv, cmdNode *pNode, paramDefn *pParam);
extern Boolean  RCC_DB_ParamRequired(cli_env *pCliEnv, paramDefn *pParam);
extern RLSTATUS RCC_DB_ParseCmdString(cli_env *pCliEnv);
extern void     RCC_DB_Possibilities(cli_env *pCliEnv);
extern void     RCC_DB_PrintType(cli_env *pCliEnv, paramDefn *pParam);
extern void     RCC_DB_PrintRM(cli_env *pCliEnv, sbyte *pRapidMark);
extern RLSTATUS RCC_DB_Process_CLI(cli_env *pCliEnv);
extern RLSTATUS RCC_DB_RetrieveParam(paramList *pParamList, sbyte *pKeyword, 
                                     paramID id, paramDescr **ppParamDescr);
extern void     RCC_DB_SetCommand(cli_env *pCliEnv, sbyte *pString, sbyte4 commandLen);
extern void     RCC_DB_SetErrorPos(cli_env *pCliEnv, sbyte4 position);
extern void     RCC_DB_ShowCommand(cli_env *pCliEnv, cmdNode *pNode);
extern void     RCC_DB_ShowNode(cli_env *pCliEnv);
extern void     RCC_DB_ShowInput(cli_env *pCliEnv);
extern void     RCC_DB_ShowParameters(cli_env *pCliEnv);
extern void     RCC_DB_ShowParentNodes(cli_env *pCliEnv);
extern void     RCC_DB_ShowTree(cli_env *pCliEnv, cmdNode *pNode, 
                                sbyte4 depth, ubyte4 mask, ubyte4 flags);
extern void     RCC_DB_SystemSettings(cli_env * pCliEnv);
extern RLSTATUS RCC_DB_SystemOrder(cli_env *pCliEnv, ubyte4 order);

#ifdef __cplusplus
}
#endif

#define RCC_DB_CreateAlias          RCC_ALIAS_CreateAlias      
#define RCC_DB_DeleteAlias          RCC_ALIAS_DeleteAlias
#define RCC_DB_DisplayAliases       RCC_ALIAS_DisplayAliases
#define RCC_DB_DestroyEnvironment   RCC_DB_EnvironmentDestroy
#define RCC_DB_InitEnvironment      RCC_DB_EnvironmentCreate

#endif /* __RCC_DB_HEADER__ */
