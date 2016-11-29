/*  
 *  rcc_user.h
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

#ifndef __RCC_USER_HEADER__
#define __RCC_USER_HEADER__

typedef struct userInfo {
    sbyte  login[kRCC_MAX_LOGIN_LEN];
    sbyte  password[kRCC_MAX_PASSWORD_LEN];
    Access accessLevel;
} userInfo;

#define kRCC_MAX_USERS   10


extern Boolean  RCC_USER_ValidateUser(cli_env *pCliEnv, sbyte *pLogin, sbyte *pPassword, Access *pAccess);
extern RLSTATUS RCC_USER_AddUser(cli_env *pCliEnv, sbyte *pLogin, sbyte *pAccess);
extern RLSTATUS RCC_USER_DeleteUser(cli_env *pCliEnv, sbyte *pLogin);
extern RLSTATUS RCC_USER_ChangePassword(cli_env *pCliEnv, sbyte *pLogin);
extern RLSTATUS RCC_USER_ChangeAccess(cli_env *pCliEnv, sbyte *pLogin, sbyte *pAccess);
extern void     RCC_USER_ShowUsers(cli_env *pCliEnv);

#endif /* __RCC_USER_HEADER__ */
