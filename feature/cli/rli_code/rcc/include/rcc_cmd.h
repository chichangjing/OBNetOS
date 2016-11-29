/*  
 *  rcc_cmd.h
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

#ifndef __RCC_CMD_H__
#define __RCC_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

extern RLSTATUS RCC_CMD_Clear(cli_env *pCliEnv);
extern RLSTATUS RCC_CMD_VxWorksShell(cli_env *pCliEnv);
extern RLSTATUS RCC_CMD_Exec (cli_env *pCliEnv, sbyte *pBuffer, sbyte4 bufferLen, Boolean echo);
extern RLSTATUS RCC_CMD_ExecFile(cli_env *pCliEnv, sbyte *pFileName, Boolean echo);
extern RLSTATUS RCC_CMD_ExecRM(cli_env *pCliEnv, sbyte *pRapidMark, Boolean echo);
extern RLSTATUS RCC_CMD_Write(cli_env *pCliEnv, sbyte *recipient, sbyte *message);
extern RLSTATUS RCC_CMD_Who(cli_env *pCliEnv);
extern RLSTATUS RCC_CMD_Snmp_Auto(cli_env *pCliEnv);
extern RLSTATUS RCC_CMD_Snmp_Commit(cli_env *pCliEnv);
extern RLSTATUS RCC_CMD_SyslogWrite(cli_env *pCliEnv, sbyte *destination, sbyte *pBuf);

#ifdef __cplusplus
}
#endif


#endif /* __RCC_CMD_H__ */
